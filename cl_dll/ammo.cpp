/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "demo_api.h"

#include <string.h>
#include <stdio.h>

#include "dod_shared.h"
#include "ammohistory.h"

WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
WEAPON *gpLastSel;		// Last weapon menu selection

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;
int g_iWeaponFlags = 0;
float g_flWeaponHeat = 0.0f;

void WeaponsResource::LoadAllWeaponSprites( void )
{
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( rgWeapons[i].iId )
			LoadWeaponSprites( &rgWeapons[i] );
	}

	const char *name;
	int index;

	name = "weapon_scopedfg42";
	index = gHUD.GetSpriteIndex( name );
	scoped_fg42.hActive = gHUD.GetSprite( index );
	scoped_fg42.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_fcarb";
	index = gHUD.GetSpriteIndex( name );
	folding_carbine.hActive = gHUD.GetSprite( index );
	folding_carbine.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_paraknife";
	index = gHUD.GetSpriteIndex( name );
	gravity_knife.hActive = gHUD.GetSprite( index );
	gravity_knife.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_scopedenfield";
	index = gHUD.GetSpriteIndex( name );
	scoped_enfield.hActive = gHUD.GetSprite( index );
	scoped_enfield.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_britknife";
	index = gHUD.GetSpriteIndex( name );
	brit_knife.hActive = gHUD.GetSprite( index );
	brit_knife.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_britgrenade";
	index = gHUD.GetSpriteIndex( name );
	brit_grenade.hActive = gHUD.GetSprite( index );
	brit_grenade.rcActive = gHUD.GetSpriteRect( index );

	name = "weapon_gerbinoculars";
	index = gHUD.GetSpriteIndex( name );
	ger_binoculars.hActive = gHUD.GetSprite( index );
	ger_binoculars.rcActive = gHUD.GetSpriteRect( index );
}

int WeaponsResource::CountAmmo( int iId )
{
	if( iId < 0 )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource::HasAmmo( WEAPON *p )
{
	return TRUE;
}

void WeaponsResource::LoadWeaponSprites( WEAPON *pWeapon )
{
	if( !pWeapon )
		return;

	memset( &pWeapon->rcActive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcInactive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo2, 0, sizeof(wrect_t) );
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	int HUD_weapon_s = gHUD.GetSpriteIndex( pWeapon->szName );
	pWeapon->hActive = gHUD.GetSprite( HUD_weapon_s );
	pWeapon->rcActive = gHUD.GetSpriteRect( HUD_weapon_s );
}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource::GetFirstPos( int iSlot )
{
	WEAPON *pret = NULL;

	for( int i = 0; i < MAX_WEAPON_POSITIONS; i++ )
	{
		if ( rgSlots[iSlot][i] )
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}

WEAPON* WeaponsResource::GetNextActivePos( int iSlot, int iSlotPos )
{
	if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
		return NULL;

	WEAPON *p = gWR.rgSlots[iSlot][iSlotPos + 1];

	if ( !p )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}

int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

DECLARE_MESSAGE( m_Ammo, CurWeapon )	// Current weapon and clip
DECLARE_MESSAGE( m_Ammo, WeaponList )	// new weapon type
DECLARE_MESSAGE( m_Ammo, AmmoX )		// update known ammo type's count
DECLARE_MESSAGE( m_Ammo, AmmoShort )
DECLARE_MESSAGE( m_Ammo, AmmoPickup )	// flashes an ammo pickup record
DECLARE_MESSAGE( m_Ammo, WeapPickup )    // flashes a weapon pickup record
DECLARE_MESSAGE( m_Ammo, HideWeapon )	// hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE( m_Ammo, ItemPickup )
DECLARE_MESSAGE( m_Ammo, ReloadDone )

DECLARE_COMMAND( m_Ammo, Slot1 )
DECLARE_COMMAND( m_Ammo, Slot2 )
DECLARE_COMMAND( m_Ammo, Slot3 )
DECLARE_COMMAND( m_Ammo, Slot4 )
DECLARE_COMMAND( m_Ammo, Slot5 )
DECLARE_COMMAND( m_Ammo, Slot6 )
DECLARE_COMMAND( m_Ammo, Slot7 )
DECLARE_COMMAND( m_Ammo, Slot8 )
DECLARE_COMMAND( m_Ammo, Slot9 )
DECLARE_COMMAND( m_Ammo, Slot10 )
DECLARE_COMMAND( m_Ammo, Close )
DECLARE_COMMAND( m_Ammo, NextWeapon )
DECLARE_COMMAND( m_Ammo, PrevWeapon )

int CHudAmmo::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( CurWeapon );
	HOOK_MESSAGE( WeaponList );
	HOOK_MESSAGE( AmmoPickup );
	HOOK_MESSAGE( WeapPickup );
	HOOK_MESSAGE( ItemPickup );
	HOOK_MESSAGE( HideWeapon );
	HOOK_MESSAGE( AmmoX );
	HOOK_MESSAGE( AmmoShort );
	HOOK_MESSAGE( ReloadDone );

	HOOK_COMMAND( "slot1", Slot1 );
	HOOK_COMMAND( "slot2", Slot2 );
	HOOK_COMMAND( "slot3", Slot3 );
	HOOK_COMMAND( "slot4", Slot4 );
	HOOK_COMMAND( "slot5", Slot5 );
	HOOK_COMMAND( "slot6", Slot6 );
	HOOK_COMMAND( "slot7", Slot7 );
	HOOK_COMMAND( "slot8", Slot8 );
	HOOK_COMMAND( "slot9", Slot9 );
	HOOK_COMMAND( "slot10", Slot10 );
	HOOK_COMMAND( "slot0", Slot10 );
	HOOK_COMMAND( "cancelselect", Close );
	HOOK_COMMAND( "invnext", NextWeapon );
	HOOK_COMMAND( "invprev", PrevWeapon );

	Reset();

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();

	return 1;
}

void CHudAmmo::Reset( void )
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
}

int CHudAmmo::VidInit( void )
{
	ClipInfoArray[0].weapon_id = WEAPON_COLT;
	ClipInfoArray[0].full_index = gHUD.GetSpriteIndex( "clip_colt_full" );
	ClipInfoArray[0].empty_index = gHUD.GetSpriteIndex( "clip_colt_empty" );
	ClipInfoArray[0].extra_index = gHUD.GetSpriteIndex( "clip_colt_extra" );
	ClipInfoArray[0].lastDrop = 0.098039217f;
	ClipInfoArray[0].subseqDrop = 0.10784314f;

	ClipInfoArray[1].weapon_id = WEAPON_LUGER;
	ClipInfoArray[1].full_index = gHUD.GetSpriteIndex( "clip_luger_full" );
	ClipInfoArray[1].empty_index = gHUD.GetSpriteIndex( "clip_luger_empty" );
	ClipInfoArray[1].extra_index = gHUD.GetSpriteIndex( "clip_luger_extra" );
	ClipInfoArray[1].lastDrop = 0.18907563f;
	ClipInfoArray[1].subseqDrop = 0.079800002f;

	ClipInfoArray[2].weapon_id = WEAPON_GARAND;
	ClipInfoArray[2].full_index = gHUD.GetSpriteIndex( "clip_garand_full" );
	ClipInfoArray[2].empty_index = gHUD.GetSpriteIndex( "clip_garand_empty" );
	ClipInfoArray[2].extra_index = gHUD.GetSpriteIndex( "clip_garand_extra" );
	ClipInfoArray[2].lastDrop = 0.125f;
	ClipInfoArray[2].subseqDrop = 0.10416666f;

	ClipInfoArray[3].weapon_id = WEAPON_SCOPEDKAR;
	ClipInfoArray[3].full_index = gHUD.GetSpriteIndex( "clip_kar_full" );
	ClipInfoArray[3].empty_index = gHUD.GetSpriteIndex( "clip_kar_empty" );
	ClipInfoArray[3].extra_index = gHUD.GetSpriteIndex( "clip_kar_extra" );
	ClipInfoArray[3].lastDrop = 0.069767445f;
	ClipInfoArray[3].subseqDrop = 0.18604651f;

	ClipInfoArray[4].weapon_id = WEAPON_THOMPSON;
	ClipInfoArray[4].full_index = gHUD.GetSpriteIndex( "clip_tommy_full" );
	ClipInfoArray[4].empty_index = gHUD.GetSpriteIndex( "clip_tommy_empty" );
	ClipInfoArray[4].extra_index = gHUD.GetSpriteIndex( "clip_tommy_extra" );
	ClipInfoArray[4].lastDrop = 0.037999999f;
	ClipInfoArray[4].subseqDrop = 0.030769231f;

	ClipInfoArray[5].weapon_id = WEAPON_MP44;
	ClipInfoArray[5].full_index = gHUD.GetSpriteIndex( "clip_mp44_full" );
	ClipInfoArray[5].empty_index = gHUD.GetSpriteIndex( "clip_mp44_empty" );
	ClipInfoArray[5].extra_index = gHUD.GetSpriteIndex( "clip_mp44_extra" );
	ClipInfoArray[5].lastDrop = 0.067100003f;
	ClipInfoArray[5].subseqDrop = 0.030075189f;

	ClipInfoArray[6].weapon_id = WEAPON_SPRING;
	ClipInfoArray[6].full_index = gHUD.GetSpriteIndex( "clip_spring_full" );
	ClipInfoArray[6].empty_index = gHUD.GetSpriteIndex( "clip_spring_empty" );
	ClipInfoArray[6].extra_index = gHUD.GetSpriteIndex( "clip_spring_extra" );
	ClipInfoArray[6].lastDrop = 0.069767445f;
	ClipInfoArray[6].subseqDrop = 0.18604651f;

	ClipInfoArray[7].weapon_id = WEAPON_KAR;
	ClipInfoArray[7].full_index = gHUD.GetSpriteIndex( "clip_kar_full" );
	ClipInfoArray[7].empty_index = gHUD.GetSpriteIndex( "clip_kar_empty" );
	ClipInfoArray[7].extra_index = gHUD.GetSpriteIndex( "clip_kar_extra" );
	ClipInfoArray[7].lastDrop = 0.069767445f;
	ClipInfoArray[7].subseqDrop = 0.18604651f;

	ClipInfoArray[8].weapon_id = WEAPON_BAR;
	ClipInfoArray[8].full_index = gHUD.GetSpriteIndex( "clip_bar_full" );
	ClipInfoArray[8].empty_index = gHUD.GetSpriteIndex( "clip_bar_empty" );
	ClipInfoArray[8].extra_index = gHUD.GetSpriteIndex( "clip_bar_extra" );
	ClipInfoArray[8].lastDrop = 0.24347825f;
	ClipInfoArray[8].subseqDrop = 0.034782607f;

	ClipInfoArray[9].weapon_id = WEAPON_MP40;
	ClipInfoArray[9].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[9].empty_index = gHUD.GetSpriteIndex( "clip_mp40_empty" );
	ClipInfoArray[9].extra_index = gHUD.GetSpriteIndex( "clip_mp40_extra" );
	ClipInfoArray[9].lastDrop = 0.052000001f;
	ClipInfoArray[9].subseqDrop = 0.030075189f;

	ClipInfoArray[10].weapon_id = WEAPON_STICKGRENADE;
	ClipInfoArray[10].full_index = gHUD.GetSpriteIndex( "clip_stick_full" );
	ClipInfoArray[10].extra_index = gHUD.GetSpriteIndex( "clip_stick_extra" );

	ClipInfoArray[11].weapon_id = WEAPON_HANDGRENADE;
	ClipInfoArray[11].full_index = gHUD.GetSpriteIndex( "clip_grenade_full" );
	ClipInfoArray[11].extra_index = gHUD.GetSpriteIndex( "clip_grenade_extra" );

	ClipInfoArray[13].weapon_id = WEAPON_MG42;
	ClipInfoArray[13].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[13].empty_index = gHUD.GetSpriteIndex( "clip_mg42_empty" );
	ClipInfoArray[13].extra_index = gHUD.GetSpriteIndex( "clip_mg42_extra" );
	ClipInfoArray[13].lastDrop = 0.023f;
	ClipInfoArray[13].subseqDrop = 0.030999999f;

	ClipInfoArray[14].weapon_id = WEAPON_CAL30;
	ClipInfoArray[14].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[14].empty_index = gHUD.GetSpriteIndex( "clip_30cal_empty" );
	ClipInfoArray[14].extra_index = gHUD.GetSpriteIndex( "clip_30cal_extra" );
	ClipInfoArray[14].lastDrop = 0.023f;
	ClipInfoArray[14].subseqDrop = 0.030999999f;

	ClipInfoArray[15].weapon_id = WEAPON_M1CARBINE;
	ClipInfoArray[15].full_index = gHUD.GetSpriteIndex( "clip_m1carbine_full" );
	ClipInfoArray[15].empty_index = gHUD.GetSpriteIndex( "clip_m1carbine_empty" );
	ClipInfoArray[15].extra_index = gHUD.GetSpriteIndex( "clip_m1carbine_extra" );
	ClipInfoArray[15].lastDrop = 0.15151516f;
	ClipInfoArray[15].subseqDrop = 0.045454547f;

	ClipInfoArray[16].weapon_id = WEAPON_MG34;
	ClipInfoArray[16].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[16].empty_index = gHUD.GetSpriteIndex( "clip_mg34_empty" );
	ClipInfoArray[16].extra_index = gHUD.GetSpriteIndex( "clip_mg34_extra" );
	ClipInfoArray[16].lastDrop = 0.023f;
	ClipInfoArray[16].subseqDrop = 0.030999999f;

	ClipInfoArray[17].weapon_id = WEAPON_GREASEGUN;
	ClipInfoArray[17].full_index = gHUD.GetSpriteIndex( "clip_grease_full" );
	ClipInfoArray[17].empty_index = gHUD.GetSpriteIndex( "clip_grease_empty" );
	ClipInfoArray[17].extra_index = gHUD.GetSpriteIndex( "clip_grease_extra" );
	ClipInfoArray[17].lastDrop = 0.045000002f;
	ClipInfoArray[17].subseqDrop = 0.030075189f;

	ClipInfoArray[18].weapon_id = WEAPON_FG42;
	ClipInfoArray[18].full_index = gHUD.GetSpriteIndex( "clip_fg42_full" );
	ClipInfoArray[18].empty_index = gHUD.GetSpriteIndex( "clip_fg42_empty" );
	ClipInfoArray[18].extra_index = gHUD.GetSpriteIndex( "clip_fg42_extra" );
	ClipInfoArray[18].lastDrop = 0.24347825f;
	ClipInfoArray[18].subseqDrop = 0.034782607f;

	ClipInfoArray[19].weapon_id = WEAPON_K43;
	ClipInfoArray[19].full_index = gHUD.GetSpriteIndex( "clip_k43_full" );
	ClipInfoArray[19].empty_index = gHUD.GetSpriteIndex( "clip_k43_empty" );
	ClipInfoArray[19].extra_index = gHUD.GetSpriteIndex( "clip_k43_extra" );
	ClipInfoArray[19].lastDrop = 0.1388889f;
	ClipInfoArray[19].subseqDrop = 0.074074075f;

	ClipInfoArray[20].weapon_id = WEAPON_ENFIELD;
	ClipInfoArray[20].full_index = gHUD.GetSpriteIndex( "clip_enfield_full" );
	ClipInfoArray[20].empty_index = gHUD.GetSpriteIndex( "clip_enfield_empty" );
	ClipInfoArray[20].extra_index = gHUD.GetSpriteIndex( "clip_enfield_extra" );
	ClipInfoArray[20].lastDrop = 0.069767445f;
	ClipInfoArray[20].subseqDrop = 0.18604651f;

	ClipInfoArray[21].weapon_id = WEAPON_STEN;
	ClipInfoArray[21].full_index = gHUD.GetSpriteIndex( "clip_sten_full" );
	ClipInfoArray[21].empty_index = gHUD.GetSpriteIndex( "clip_sten_empty" );
	ClipInfoArray[21].extra_index = gHUD.GetSpriteIndex( "clip_sten_extra" );
	ClipInfoArray[21].lastDrop = 0.03846154f;
	ClipInfoArray[21].subseqDrop = 0.030769231f;

	ClipInfoArray[22].weapon_id = WEAPON_BREN;
	ClipInfoArray[22].full_index = gHUD.GetSpriteIndex( "clip_bren_full" );
	ClipInfoArray[22].empty_index = gHUD.GetSpriteIndex( "clip_bren_empty" );
	ClipInfoArray[22].extra_index = gHUD.GetSpriteIndex( "clip_bren_extra" );
	ClipInfoArray[22].lastDrop = 0.09375f;
	ClipInfoArray[22].subseqDrop = 0.020833334f;

	ClipInfoArray[23].weapon_id = WEAPON_WEBLEY;
	ClipInfoArray[23].full_index = gHUD.GetSpriteIndex( "clip_webley_full" );
	ClipInfoArray[23].empty_index = gHUD.GetSpriteIndex( "clip_webley_full" );
	ClipInfoArray[23].extra_index = gHUD.GetSpriteIndex( "clip_webley_extra" );
	ClipInfoArray[23].lastDrop = 0.0f;
	ClipInfoArray[23].subseqDrop = 0.0f;

	ClipInfoArray[24].weapon_id = WEAPON_BAZOOKA;
	ClipInfoArray[24].full_index = gHUD.GetSpriteIndex( "clip_bazooka_full" );
	ClipInfoArray[24].empty_index = gHUD.GetSpriteIndex( "clip_bazooka_empty" );
	ClipInfoArray[24].extra_index = gHUD.GetSpriteIndex( "clip_bazooka_extra" );

	ClipInfoArray[25].weapon_id = WEAPON_PSCHRECK;
	ClipInfoArray[25].full_index = gHUD.GetSpriteIndex( "clip_pschreck_full" );
	ClipInfoArray[25].empty_index = gHUD.GetSpriteIndex( "clip_pschreck_empty" );
	ClipInfoArray[25].extra_index = gHUD.GetSpriteIndex( "clip_pschreck_extra" );

	ClipInfoArray[26].weapon_id = WEAPON_PIAT;
	ClipInfoArray[26].full_index = gHUD.GetSpriteIndex( "clip_piat_full" );
	ClipInfoArray[26].empty_index = gHUD.GetSpriteIndex( "clip_piat_empty" );
	ClipInfoArray[26].extra_index = gHUD.GetSpriteIndex( "clip_piat_extra" );

	ClipInfoArray[27].weapon_id = WEAPON_MORTAR;
	ClipInfoArray[27].full_index = gHUD.GetSpriteIndex( "clip_mortar" );
	ClipInfoArray[27].empty_index = gHUD.GetSpriteIndex( "clip_mortar" );
	ClipInfoArray[27].extra_index = gHUD.GetSpriteIndex( "clip_mortar" );

	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		ClipInfoArray[i].FullSprite = gHUD.GetSprite( ClipInfoArray[i].full_index );
		ClipInfoArray[i].FullArea = &gHUD.GetSpriteRect( ClipInfoArray[i].full_index );

		ClipInfoArray[i].EmptySprite = gHUD.GetSprite( ClipInfoArray[i].empty_index );
		ClipInfoArray[i].EmptyArea = &gHUD.GetSpriteRect( ClipInfoArray[i].empty_index );

		ClipInfoArray[i].ExtraSprite = gHUD.GetSprite( ClipInfoArray[i].extra_index );
		ClipInfoArray[i].ExtraArea = &gHUD.GetSpriteRect( ClipInfoArray[i].extra_index );
	}

	BritGrenClipInfo.weapon_id = WEAPON_HANDGRENADE;
	BritGrenClipInfo.extra_index = gHUD.GetSpriteIndex( "clip_britgrenade_extra" );
	BritGrenClipInfo.ExtraSprite = gHUD.GetSprite( BritGrenClipInfo.extra_index );
	BritGrenClipInfo.ExtraArea = &gHUD.GetSpriteRect( BritGrenClipInfo.extra_index );

	int HUD_mgbarrel_20 = gHUD.GetSpriteIndex( "hud_barrel" );
	MGBarrelHUD = gHUD.GetSprite( HUD_mgbarrel_20 );
	MGBarrelHUDArea = &gHUD.GetSpriteRect( HUD_mgbarrel_20 );

	int HUD_mgbarrel2_20 = gHUD.GetSpriteIndex( "hud_barrelo" );
	MGBarrel2HUD = gHUD.GetSprite( HUD_mgbarrel2_20 );
	MGBarrel2HUDArea = &gHUD.GetSpriteRect( HUD_mgbarrel2_20 );

	giBucketWidth = 12;
	giBucketHeight = 12;

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if( ScreenWidth <= 639 )
	{
		giABWidth = 10;
		giABHeight = 2;
	}
	else
	{
		giABWidth = 20;
		giABHeight = 4;
	}

	return 1;
}

//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think( void )
{
	if( gHUD.m_fPlayerDead )
		return;

	bool bWeapons1Changed = gHUD.m_iWeaponBits != gWR.iOldWeaponBits;
	bool bWeapons2Changed = g_iWeaponBits2 != gWR.iOldWeaponBits2;

	if( bWeapons1Changed || bWeapons2Changed )
	{
		int i;

		gWR.DropAllWeapons();

		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for( i = MAX_WEAPONS / 2 - 1; i > 0; i-- )
		{
			WEAPON *p = gWR.GetWeapon( i );

			if( gHUD.m_iWeaponBits & ( 1 << p->iId ) )
				gWR.PickupWeapon( p );
		}

		gWR.iOldWeaponBits2 = g_iWeaponBits2;

		for( i = 0; i < MAX_WEAPONS / 2 - 1; i++ )
		{
			WEAPON *p = gWR.GetWeapon( i + MAX_WEAPONS / 2 );

			if( g_iWeaponBits2 & ( 1 << i ) )
				gWR.PickupWeapon( p );
		}
	}

	if( m_pWeapon )
		m_pWeapon->iLastWeaponState = g_iWeaponFlags;

	if( !gpActiveSel )
		return;

	// has the player selected one?
	if( gHUD.m_iKeyBits & IN_ATTACK )
	{
		if( gpActiveSel != (WEAPON *) 1 )
		{
			ServerCmd( gpActiveSel->szName );
			g_weaponselect = gpActiveSel->iId;
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;
	}
}

//
// Helper function to return a Ammo pointer from id
//
HSPRITE* WeaponsResource::GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect )
{
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( rgWeapons[i].iAmmoType == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if( rgWeapons[i].iAmmo2Type == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}

// Menu Selection Code
void WeaponsResource::SelectSlot( int iSlot, int fAdvance, int iDirection )
{
	if( gHUD.m_Menu.m_fMenuDisplayed && ( fAdvance  == FALSE ) && ( iDirection == 1 ) )
	{
		// menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );  // slots are one off the key numbers
		return;
	}

	if( iSlot > MAX_WEAPON_SLOTS )
		return;

	if( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	WEAPON *p = NULL;
	bool fastSwitch = gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f;

	if ( ( gpActiveSel == NULL ) || ( gpActiveSel == (WEAPON *) 1 ) || ( iSlot != gpActiveSel->iSlot ) )
	{
		p = GetFirstPos( iSlot );

		if ( p && fastSwitch ) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON *p2 = GetNextActivePos( p->iSlot, p->iSlotPos );
			if ( !p2 )
			{
				// only one active item in bucket, so change directly to weapon
				ServerCmd( p->szName );
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		if ( gpActiveSel )
			p = GetNextActivePos( gpActiveSel->iSlot, gpActiveSel->iSlotPos );
		if ( !p )
			p = GetFirstPos( iSlot );
	}

	if ( !p )  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if ( !fastSwitch )
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else
		gpActiveSel = p;
}

//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//
// AmmoX  -- Update the count of a known type of ammo
//
int CHudAmmo::MsgFunc_AmmoX( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo( iIndex, abs( iCount ) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoShort( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_SHORT();

	gWR.SetAmmo( iIndex, abs( iCount ) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_BYTE();
	READ_BYTE();

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_BYTE();

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_STRING();

	return 1;
}

int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		gpActiveSel = NULL;

	return 1;
}

int CHudAmmo::MsgFunc_ReloadDone( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	gHUD.m_bAutoReloadComplete = true;

	return 1;
}

void CHudAmmo::PlayerDied( void )
{
	gHUD.m_fPlayerDead = TRUE;
	gpActiveSel = NULL;
}

//
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type
//  numbers match a real ammo type.
//
int CHudAmmo::MsgFunc_CurWeapon( const char *pszName, int iSize, void *pbuf )
{
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	BEGIN_READ( pbuf, iSize );

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_BYTE();

	// detect if we're also on target
	if( iState > 1 )
	{
		fOnTarget = TRUE;
	}

	if( iId <= 0 )
	{
		SetCrosshair( 0, nullrc, 0, 0, 0 );
		return 0;
	}

	gHUD.m_fPlayerDead = FALSE;

	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if( iClip < -1 )
		pWeapon->iClip = abs( iClip );
	else
		pWeapon->iClip = iClip;

	if( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	if( !( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) && gHUD.m_iFOV < 90 )
	{
		// zoomed crosshairs
		if( fOnTarget && m_pWeapon->hZoomedAutoaim )
			SetCrosshair( m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255 );
		else
			SetCrosshair( m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255 );
	}

	m_fFade = 200.0f; //!!!
	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudAmmo::GetCurrentWeaponId( void )
{
	if( m_pWeapon )
		return m_pWeapon->iId;

	return -1;
}

extern char weaponnames[64][64];

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	WEAPON Weapon;

	memset( &Weapon, 0, sizeof( Weapon ) );

	Weapon.iAmmoType = (int)READ_CHAR();

	Weapon.iMax1 = READ_BYTE();
	if( Weapon.iMax1 == 255 )
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = READ_CHAR();
	Weapon.iMax2 = READ_BYTE();
	if( Weapon.iMax2 == 255 )
		Weapon.iMax2 = -1;

	Weapon.iSlot = READ_CHAR();
	Weapon.iSlotPos = READ_CHAR();
	Weapon.iId = READ_SHORT();
	Weapon.iFlags = READ_BYTE();
	Weapon.iClip = 0;
	Weapon.iClipMax = READ_BYTE();
	Weapon.iLastWeaponState = 0;

	if( Weapon.iId < 0 || Weapon.iId >= MAX_WEAPONS )
		return 0;
	if( Weapon.iSlot < 0 || Weapon.iSlot >= MAX_WEAPON_SLOTS + 1 )
		return 0;
	if( Weapon.iSlotPos < 0 || Weapon.iSlotPos >= MAX_WEAPON_POSITIONS + 1 )
		return 0;
	if( Weapon.iAmmoType < -1 || Weapon.iAmmoType >= MAX_AMMO_TYPES )
		return 0;
	if( Weapon.iAmmo2Type < -1 || Weapon.iAmmo2Type >= MAX_AMMO_TYPES )
		return 0;
	if( Weapon.iAmmoType != -1 && Weapon.iMax1 == 0 )
		return 0;
	if( Weapon.iAmmo2Type != -1 && Weapon.iMax2 == 0 )
		return 0;
	if( Weapon.iClipMax <= 0 )
		return 0;

	strncpy( Weapon.szName, weaponnames[Weapon.iId], sizeof( Weapon.szName ) );
	Weapon.szName[sizeof( Weapon.szName ) - 1] = 0;

	gWR.AddWeapon( &Weapon );

	return 1;
}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	gWR.SelectSlot( iSlot, FALSE, 1 );
}

void CHudAmmo::UserCmd_Slot1( void )
{
	SlotInput( 0 );
}

void CHudAmmo::UserCmd_Slot2( void )
{
	SlotInput( 1 );
}

void CHudAmmo::UserCmd_Slot3( void )
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4( void )
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5( void )
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6( void )
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7( void )
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8( void )
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9( void )
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10( void )
{
	SlotInput( 9 );
}

void CHudAmmo::UserCmd_Close( void )
{
	if( gHUD.m_Menu.m_fMenuDisplayed )
	{
		if( gHUD.m_Menu.CanCancel )
		{
			gHUD.m_Menu.m_fMenuDisplayed = 0;
			gHUD.m_Menu.m_iFlags &= ~HUD_ACTIVE;
		}
	}
	else if( gpActiveSel )
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
	}
	else
		ClientCmd( "escape" );
}

// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon( void )
{
	if( gHUD.m_fPlayerDead || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) )
		return;

	bool fastSwitch = gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f;

	if( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for( int loop = 0; loop <= 1; loop++ )
	{
		for( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if( wsp )
				{
					if( fastSwitch )
					{
						g_weaponselect = wsp->iId;
						ServerCmd( wsp->szName );
						gpActiveSel = NULL;
					}
					else
						gpActiveSel = wsp;

					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon( void )
{
	if( gHUD.m_fPlayerDead || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) )
		return;

	bool fastSwitch = gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f;

	if( !gpActiveSel || gpActiveSel == (WEAPON*) 1 )
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS - 1;
	int slot = MAX_WEAPON_SLOTS - 1;
	if( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}

	for( int loop = 0; loop <= 1; loop++ )
	{
		for( ; slot >= 0; slot-- )
		{
			for( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if( wsp )
				{
					if( fastSwitch )
					{
						g_weaponselect = wsp->iId;
						ServerCmd( wsp->szName );
						gpActiveSel = NULL;
					}
					else
						gpActiveSel = wsp;

					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS - 1;
		}

		slot = MAX_WEAPON_SLOTS - 1;
	}

	gpActiveSel = NULL;
}

ClipInfo *CHudAmmo::GetCurrentGun( WEAPON *pw )
{
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( pw->iId == ClipInfoArray[i].weapon_id )
			return &ClipInfoArray[i];
	}

	return NULL;
}

//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------
int CHudAmmo::Draw( float flTime )
{
	if( g_iVuser1z || ( !gHUD.m_iWeaponBits && !g_iWeaponBits2 ) )
		return 1;

	if( ( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL ) || g_iUser1 || g_iUser2 )
		return 1;

	if( !g_iAlive || g_iDeadFlag )
		return 1;

	if( !g_iTeamNumber || !g_iPlayerClass )
		return 1;

	if( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return 1;

	DrawWeaponList( flTime );

	if( !( m_iFlags & HUD_ACTIVE ) )
		return 0;

	WEAPON *pw = m_pWeapon;

	if( !pw )
		return 0;

	gHUD.m_iClipSize = pw->iClip;

	if( gEngfuncs.pDemoAPI->IsPlayingback() )
		gHUD.g_iClip = pw->iClip;

	if( pw->iAmmoType < 0 && pw->iAmmo2Type < 0 )
		return 0;

	if( m_fFade > 0 )
		m_fFade -= ( (float)gHUD.m_flTimeDelta * 20.0f );

	ClipInfo *currentGun = GetCurrentGun( pw );

	if( !currentGun )
		return 0;

	// Does weapon have any ammo at all?
	if( pw->iAmmoType > 0 )
	{
		int y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

		int clipHeight = currentGun->FullArea->bottom - currentGun->FullArea->top;
		int clipWidth = currentGun->FullArea->right - currentGun->FullArea->left;

		int ExtraClipWidth = currentGun->ExtraArea->right - currentGun->ExtraArea->left;
		int ExtraClipHeight = currentGun->ExtraArea->bottom - currentGun->ExtraArea->top;

		wrect_t rc = *currentGun->FullArea;

		if( currentGun->weapon_id == WEAPON_MG42 || currentGun->weapon_id == WEAPON_CAL30
			|| currentGun->weapon_id == WEAPON_MG34 || currentGun->weapon_id == WEAPON_WEBLEY )
		{
			// machine guns show the rounds left as a number
			int iNumberWidth = gHUD.GetSpriteRect( gHUD.m_MG_number_0 ).right - gHUD.GetSpriteRect( gHUD.m_MG_number_0 ).left;
			int k;
			int iNumber = gHUD.g_iClip;

			if( ShowHudElement( 3 ) )
			{
				int x = ScreenWidth - 30 - ( ExtraClipWidth + clipWidth );

				SPR_Set( currentGun->EmptySprite, 255, 255, 255 );
				SPR_DrawHoles( 0, x - 10, gHUD.m_iFontHeight + y - clipHeight, currentGun->EmptyArea );

				x += 26;

				if( iNumber > 99 )
				{
					k = iNumber / 100;
					SPR_Set( gHUD.GetSprite( gHUD.m_MG_number_0 + k ), 255, 255, 255 );
					SPR_DrawHoles( 0, x, y + 3, &gHUD.GetSpriteRect( gHUD.m_MG_number_0 + k ) );
				}

				x += iNumberWidth;

				if( iNumber > 9 )
				{
					k = ( iNumber % 100 ) / 10;
					SPR_Set( gHUD.GetSprite( gHUD.m_MG_number_0 + k ), 255, 255, 255 );
					SPR_DrawHoles( 0, x, y + 3, &gHUD.GetSpriteRect( gHUD.m_MG_number_0 + k ) );
				}

				x += iNumberWidth;

				k = iNumber % 10;
				SPR_Set( gHUD.GetSprite( gHUD.m_MG_number_0 + k ), 255, 255, 255 );
				SPR_DrawHoles( 0, x, y + 3, &gHUD.GetSpriteRect( gHUD.m_MG_number_0 + k ) );

				int fullclips = gWR.CountAmmo( pw->iAmmoType ) / pw->iClipMax;

				if( fullclips > 0 )
				{
					char buf[8];

					x = ScreenWidth - ExtraClipWidth - 15;

					SPR_Set( currentGun->ExtraSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - ExtraClipHeight, currentGun->ExtraArea );

					sprintf( buf, "x %d", fullclips );
					gHUD.DrawHudString( ExtraClipWidth / 2 + x, y, ExtraClipWidth / 2 + x + 64, buf, 255, 255, 255 );
				}
			}
		}
		else if( currentGun->weapon_id == WEAPON_HANDGRENADE || currentGun->weapon_id == WEAPON_STICKGRENADE
			|| currentGun->weapon_id == WEAPON_MORTAR )
		{
			HSPRITE sprite = currentGun->ExtraSprite;
			wrect_t *area = currentGun->ExtraArea;

			if( currentGun->weapon_id == WEAPON_HANDGRENADE && gHUD.m_bBritish )
			{
				sprite = BritGrenClipInfo.ExtraSprite;
				area = BritGrenClipInfo.ExtraArea;
			}

			int numGrens = gWR.CountAmmo( pw->iAmmoType );

			if( ShowHudElement( 3 ) && numGrens > 0 )
			{
				char buf[8];

				int x = ScreenWidth - ExtraClipWidth - 30;

				SPR_Set( sprite, 255, 255, 255 );
				SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - ExtraClipHeight, area );

				sprintf( buf, "x %d", numGrens );
				gHUD.DrawHudString( ExtraClipWidth + x, y, ExtraClipWidth + x + 64, buf, 255, 255, 255 );
			}
		}
		else if( currentGun->weapon_id >= WEAPON_BAZOOKA && currentGun->weapon_id <= WEAPON_MORTAR )
		{
			int numRockets = gWR.CountAmmo( pw->iAmmoType );
			int tubewidth = currentGun->FullArea->right - currentGun->FullArea->left;

			if( ShowHudElement( 3 ) )
			{
				int x = ScreenWidth - ( ExtraClipWidth + tubewidth ) - 30;

				SPR_Set( currentGun->EmptySprite, 255, 255, 255 );
				SPR_DrawHoles( 0, x, y - clipHeight + gHUD.m_iFontHeight, currentGun->EmptyArea );

				if( gHUD.g_iClip > 0 )
				{
					SPR_Set( currentGun->FullSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - clipHeight, currentGun->FullArea );
				}

				if( numRockets > 0 )
				{
					char buf[8];

					x += currentGun->FullArea->right - currentGun->FullArea->left;

					SPR_Set( currentGun->ExtraSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - ExtraClipHeight, currentGun->ExtraArea );

					sprintf( buf, "x %d", numRockets );
					gHUD.DrawHudString( ExtraClipWidth + x, y, ExtraClipWidth + x + 64, buf, 255, 255, 255 );
				}
			}
		}
		else
		{
			float last = 0.0f;
			wrect_t rc2;

			if( pw->iClip > 0 )
				last = currentGun->lastDrop;

			if( currentGun->weapon_id == WEAPON_ENFIELD )
			{
				// two clips of five rounds each
				int i_eclip = 5;

				if( gHUD.g_iClip <= 5 )
					i_eclip = gHUD.g_iClip;

				rc2 = *currentGun->FullArea;

				rc.top = (int)( (float)rc.top + ( 1.0f - ( (float)i_eclip * currentGun->subseqDrop + last ) ) * (float)clipHeight );
				rc2.top = (int)( (float)rc2.top + (float)clipHeight * ( 1.0f - ( last + currentGun->subseqDrop * (float)( gHUD.g_iClip - i_eclip ) ) ) );
			}
			else
			{
				rc.top = (int)( (float)clipHeight * ( 1.0f - ( last + (float)gHUD.g_iClip * currentGun->subseqDrop ) ) + (float)rc.top );
			}

			if( ShowHudElement( 3 ) )
			{
				int x = ScreenWidth - ( ExtraClipWidth + clipWidth ) - 45;
				int i_enfypos = clipHeight - 2;

				SPR_Set( currentGun->EmptySprite, 255, 255, 255 );
				SPR_DrawHoles( 0, x, y - clipHeight + gHUD.m_iFontHeight, currentGun->EmptyArea );

				if( currentGun->weapon_id == WEAPON_ENFIELD )
				{
					SPR_Set( currentGun->EmptySprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, y - clipHeight + gHUD.m_iFontHeight - i_enfypos, currentGun->EmptyArea );
				}

				if( rc.bottom > rc.top )
				{
					SPR_Set( currentGun->FullSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + rc.top + y - currentGun->EmptyArea->top - clipHeight, &rc );
				}

				if( currentGun->weapon_id == WEAPON_ENFIELD && rc2.bottom > rc2.top )
				{
					SPR_Set( currentGun->FullSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, x, gHUD.m_iFontHeight + rc2.top + y - currentGun->EmptyArea->top - clipHeight - i_enfypos, &rc2 );
				}

				int fullclips = gWR.CountAmmo( pw->iAmmoType ) / pw->iClipMax;

				if( fullclips > 0 )
				{
					char buf[8];

					SPR_Set( currentGun->ExtraSprite, 255, 255, 255 );
					SPR_DrawHoles( 0, ScreenWidth - ExtraClipWidth - 45, gHUD.m_iFontHeight + y - ExtraClipHeight, currentGun->ExtraArea );

					sprintf( buf, "x %d", fullclips );
					gHUD.DrawHudString( ScreenWidth - 45, y, ScreenWidth - 45 + 64, buf, 255, 255, 255 );
				}
			}
		}
	}

	// the MG42 barrel shows how much it is overheated
	if( currentGun->weapon_id == WEAPON_MG42 )
	{
		float cappedOverheat = g_flWeaponHeat;

		if( cappedOverheat > 100.0f )
			cappedOverheat = 100.0f;

		wrect_t barrel_area = *MGBarrelHUDArea;
		wrect_t newArea = *MGBarrel2HUDArea;

		int height = barrel_area.bottom - barrel_area.top;
		int barrelHeight = (int)( cappedOverheat * (float)height / 100.0f );
		int barrely = ( ScreenHeight - height ) / 2;

		newArea.top = newArea.bottom - barrelHeight;
		barrel_area.bottom -= barrelHeight;

		SPR_Set( MGBarrelHUD, 255, 255, 255 );
		SPR_DrawHoles( 0, ScreenWidth + MGBarrelHUDArea->left - MGBarrelHUDArea->right, barrely, &barrel_area );

		if( newArea.bottom - newArea.top > 0 )
		{
			SPR_Set( MGBarrel2HUD, 255, 255, 255 );
			SPR_DrawHoles( 0, ScreenWidth + MGBarrelHUDArea->left - MGBarrelHUDArea->right, barrely + height - barrelHeight, &newArea );
		}
	}

	return 1;
}

//
// Draws the weapon list on the left side of the screen (shown while selecting a weapon)
//
int CHudAmmo::DrawWeaponList( float flTime )
{
	if( !gpActiveSel )
		return 0;

	int y = ScreenHeight / 2 - 150;

	for( int iSlot = 0; iSlot < MAX_WEAPON_SLOTS; iSlot++ )
	{
		int numdrawn = 0;

		for( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
		{
			WEAPON *p = gWR.GetWeaponSlot( iSlot, iPos );

			if( !p || !p->iId )
				continue;

			y += 50;
			numdrawn++;

			int x = ScreenWidth + p->rcActive.left - 5 - p->rcActive.right;
			int b = ( gpActiveSel == p ) ? 255 : 80;

			HSPRITE hSprite = p->hActive;
			wrect_t *prc = &p->rcActive;

			switch( p->iId )
			{
			case WEAPON_FG42:
				if( g_iWeaponFlags & WPNSTATE_SCOPED )
				{
					hSprite = gWR.scoped_fg42.hActive;
					prc = &gWR.scoped_fg42.rcActive;
				}
				break;
			case WEAPON_M1CARBINE:
				if( gHUD.m_bParatrooper )
				{
					hSprite = gWR.folding_carbine.hActive;
					prc = &gWR.folding_carbine.rcActive;
				}
				break;
			case WEAPON_GERKNIFE:
				if( gHUD.m_bParatrooper )
				{
					hSprite = gWR.gravity_knife.hActive;
					prc = &gWR.gravity_knife.rcActive;
				}
				break;
			case WEAPON_ENFIELD:
				if( p->iLastWeaponState & WPNSTATE_SCOPED )
				{
					hSprite = gWR.scoped_enfield.hActive;
					prc = &gWR.scoped_enfield.rcActive;
				}
				break;
			case WEAPON_AMERKNIFE:
				if( gHUD.m_bBritish )
				{
					hSprite = gWR.brit_knife.hActive;
					prc = &gWR.brit_knife.rcActive;
				}
				break;
			case WEAPON_HANDGRENADE:
				if( gHUD.m_bBritish )
				{
					hSprite = gWR.brit_grenade.hActive;
					prc = &gWR.brit_grenade.rcActive;
				}
				break;
			default:
				if( p->iId == WEAPON_BINOC && g_iTeamNumber == 2 )
				{
					hSprite = gWR.ger_binoculars.hActive;
					prc = &gWR.ger_binoculars.rcActive;
				}
				break;
			}

			SPR_Set( hSprite, b, b, b );
			SPR_DrawHoles( 0, x, y, prc );
		}

		if( !numdrawn )
			y += 50;
	}

	return 1;
}

int CHudAmmo::DrawWList( float flTime )
{
	return 0;
}

/* =================================
	GetSpriteList

Finds and returns the matching
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t *GetSpriteList( client_sprite_t *pList, const char *psz, int iRes, int iCount )
{
	if( !pList )
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while( i-- )
	{
		if( p->iRes == iRes && !strcmp( psz, p->szName ))
			return p;
		p++;
	}

	return NULL;
}
