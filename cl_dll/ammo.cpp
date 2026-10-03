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
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>

#include "dod_shared.h"
#include "demo_api.h"
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
	char *name;
	int index;

	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( rgWeapons[i].iId )
			LoadWeaponSprites( &rgWeapons[i] );
	}

	name = "weapon_scopedfg42";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		scoped_fg42.hActive = gHUD.GetSprite( index );
		scoped_fg42.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		scoped_fg42.hActive = 0;
	}

	name = "weapon_fcarb";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		folding_carbine.hActive = gHUD.GetSprite( index );
		folding_carbine.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		folding_carbine.hActive = 0;
	}

	name = "weapon_paraknife";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		gravity_knife.hActive = gHUD.GetSprite( index );
		gravity_knife.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		gravity_knife.hActive = 0;
	}

	name = "weapon_scopedenfield";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		scoped_enfield.hActive = gHUD.GetSprite( index );
		scoped_enfield.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		scoped_enfield.hActive = 0;
	}

	name = "weapon_britknife";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		brit_knife.hActive = gHUD.GetSprite( index );
		brit_knife.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		brit_knife.hActive = 0;
	}

	name = "weapon_britgrenade";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		brit_grenade.hActive = gHUD.GetSprite( index );
		brit_grenade.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		brit_grenade.hActive = 0;
	}

	name = "weapon_gerbinoculars";
	index = gHUD.GetSpriteIndex( name );
	if( index >= 0 )
	{
		ger_binoculars.hActive = gHUD.GetSprite( index );
		ger_binoculars.rcActive = gHUD.m_rgrcRects[index];
	}
	else
	{
		ger_binoculars.hActive = 0;
	}
}

int WeaponsResource::CountAmmo( int iId ) 
{ 
	if( iId < 0 )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource::HasAmmo( WEAPON *p )
{
	return 1;
}

void WeaponsResource::LoadWeaponSprites( struct WEAPON *pWeapon )
{
	if( !pWeapon )
		return;

	int iRes = gHUD.m_iRes;

	pWeapon->hActive = 0;
	pWeapon->hInactive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	pWeapon->rcActive.left = 0;
	pWeapon->rcActive.right = 0;
	pWeapon->rcActive.top = 0;
	pWeapon->rcActive.bottom = 0;

	pWeapon->rcInactive.left = 0;
	pWeapon->rcInactive.right = 0;
	pWeapon->rcInactive.top = 0;
	pWeapon->rcInactive.bottom = 0;

	pWeapon->rcAmmo.left = 0;
	pWeapon->rcAmmo.right = 0;
	pWeapon->rcAmmo.top = 0;
	pWeapon->rcAmmo.bottom = 0;

	pWeapon->rcAmmo2.left = 0;
	pWeapon->rcAmmo2.right = 0;
	pWeapon->rcAmmo2.top = 0;
	pWeapon->rcAmmo2.bottom = 0;

	int HUD_weapon_s = gHUD.GetSpriteIndex( pWeapon->szName );
	HSPRITE hSpr = 0;

	if( HUD_weapon_s >= 0 )
	{
		hSpr = gHUD.GetSprite( HUD_weapon_s );
		pWeapon->rcActive = gHUD.GetSpriteRect( HUD_weapon_s );
	}

	pWeapon->hActive = hSpr;
}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource::GetFirstPos( int iSlot )
{
	if( iSlot < 0 || iSlot >= DOD_MAX_SLOTS )
		return NULL;

	for( int i = 0; i < DOD_MAX_POSITIONS; i++ )
	{
		if( rgSlots[iSlot][i] != NULL )
			return rgSlots[iSlot][i];
	}

	return NULL;
}

WEAPON *WeaponsResource::GetNextActivePos( int iSlot, int iSlotPos )
{
	WEAPON *result = NULL;

	while( iSlotPos <= 11 && iSlot <= 4 )
	{
		result = gWR.rgSlots[iSlot][++iSlotPos];

		if( result != NULL )
			return result;
	}

	return NULL;
}

int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

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
	m_fFade = 0.0f;
	m_iFlags |= HUD_ACTIVE;
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
	ClipInfoArray[0].subseqDrop = 0.107843140f;

	ClipInfoArray[1].weapon_id = WEAPON_LUGER;
	ClipInfoArray[1].full_index = gHUD.GetSpriteIndex( "clip_luger_full" );
	ClipInfoArray[1].empty_index = gHUD.GetSpriteIndex( "clip_luger_empty" );
	ClipInfoArray[1].extra_index = gHUD.GetSpriteIndex( "clip_luger_extra" );
	ClipInfoArray[1].lastDrop = 0.189075630f;
	ClipInfoArray[1].subseqDrop = 0.079800002f;

	ClipInfoArray[2].weapon_id = WEAPON_GARAND;
	ClipInfoArray[2].full_index = gHUD.GetSpriteIndex( "clip_garand_full" );
	ClipInfoArray[2].empty_index = gHUD.GetSpriteIndex( "clip_garand_empty" );
	ClipInfoArray[2].extra_index = gHUD.GetSpriteIndex( "clip_garand_extra" );
	ClipInfoArray[2].lastDrop = 0.125000000f;
	ClipInfoArray[2].subseqDrop = 0.104166660f;

	ClipInfoArray[3].weapon_id = WEAPON_SCOPEDKAR;
	ClipInfoArray[3].full_index = gHUD.GetSpriteIndex( "clip_kar_full" );
	ClipInfoArray[3].empty_index = gHUD.GetSpriteIndex( "clip_kar_empty" );
	ClipInfoArray[3].extra_index = gHUD.GetSpriteIndex( "clip_kar_extra" );
	ClipInfoArray[3].lastDrop = 0.069767445f;
	ClipInfoArray[3].subseqDrop = 0.186046510f;

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
	ClipInfoArray[6].subseqDrop = 0.186046510f;

	ClipInfoArray[7].weapon_id = WEAPON_KAR;
	ClipInfoArray[7].full_index = gHUD.GetSpriteIndex( "clip_kar_full" );
	ClipInfoArray[7].empty_index = gHUD.GetSpriteIndex( "clip_kar_empty" );
	ClipInfoArray[7].extra_index = gHUD.GetSpriteIndex( "clip_kar_extra" );
	ClipInfoArray[7].lastDrop = 0.069767445f;
	ClipInfoArray[7].subseqDrop = 0.186046510f;

	ClipInfoArray[12].weapon_id = WEAPON_NONE;
	ClipInfoArray[12].full_index = -1;
	ClipInfoArray[12].empty_index = -1;
	ClipInfoArray[12].extra_index = -1;

	ClipInfoArray[8].weapon_id = WEAPON_BAR;
	ClipInfoArray[8].full_index = gHUD.GetSpriteIndex( "clip_bar_full" );
	ClipInfoArray[8].empty_index = gHUD.GetSpriteIndex( "clip_bar_empty" );
	ClipInfoArray[8].extra_index = gHUD.GetSpriteIndex( "clip_bar_extra" );
	ClipInfoArray[8].lastDrop = 0.243478250f;
	ClipInfoArray[8].subseqDrop = 0.034782607f;

	ClipInfoArray[9].weapon_id = WEAPON_MP40;
	ClipInfoArray[9].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[9].empty_index = gHUD.GetSpriteIndex( "clip_mp40_empty" );
	ClipInfoArray[9].extra_index = gHUD.GetSpriteIndex( "clip_mp40_extra" );
	ClipInfoArray[9].lastDrop = 0.052000001f;
	ClipInfoArray[9].subseqDrop = 0.030075189f;

	ClipInfoArray[10].weapon_id = WEAPON_STICKGRENADE;
	ClipInfoArray[10].full_index = gHUD.GetSpriteIndex( "clip_stick_full" );
	ClipInfoArray[10].empty_index = -1;
	ClipInfoArray[10].extra_index = gHUD.GetSpriteIndex( "clip_stick_extra" );

	ClipInfoArray[11].weapon_id = WEAPON_HANDGRENADE;
	ClipInfoArray[11].full_index = gHUD.GetSpriteIndex( "clip_grenade_full" );
	ClipInfoArray[11].empty_index = -1;
	ClipInfoArray[11].extra_index = gHUD.GetSpriteIndex( "clip_grenade_extra" );

	ClipInfoArray[13].weapon_id = WEAPON_MG42;
	ClipInfoArray[13].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[13].empty_index = gHUD.GetSpriteIndex( "clip_mg42_empty" );
	ClipInfoArray[13].extra_index = gHUD.GetSpriteIndex( "clip_mg42_extra" );
	ClipInfoArray[13].lastDrop = 0.023000000f;
	ClipInfoArray[13].subseqDrop = 0.030999999f;

	ClipInfoArray[14].weapon_id = WEAPON_CAL30;
	ClipInfoArray[14].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[14].empty_index = gHUD.GetSpriteIndex( "clip_30cal_empty" );
	ClipInfoArray[14].extra_index = gHUD.GetSpriteIndex( "clip_30cal_extra" );
	ClipInfoArray[14].lastDrop = 0.023000000f;
	ClipInfoArray[14].subseqDrop = 0.030999999f;

	ClipInfoArray[15].weapon_id = WEAPON_M1CARBINE;
	ClipInfoArray[15].full_index = gHUD.GetSpriteIndex( "clip_m1carbine_full" );
	ClipInfoArray[15].empty_index = gHUD.GetSpriteIndex( "clip_m1carbine_empty" );
	ClipInfoArray[15].extra_index = gHUD.GetSpriteIndex( "clip_m1carbine_extra" );
	ClipInfoArray[15].lastDrop = 0.151515160f;
	ClipInfoArray[15].subseqDrop = 0.045454547f;

	ClipInfoArray[16].weapon_id = WEAPON_MG34;
	ClipInfoArray[16].full_index = gHUD.GetSpriteIndex( "clip_mp40_full" );
	ClipInfoArray[16].empty_index = gHUD.GetSpriteIndex( "clip_mg34_empty" );
	ClipInfoArray[16].extra_index = gHUD.GetSpriteIndex( "clip_mg34_extra" );
	ClipInfoArray[16].lastDrop = 0.023000000f;
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
	ClipInfoArray[18].lastDrop = 0.243478250f;
	ClipInfoArray[18].subseqDrop = 0.034782607f;

	ClipInfoArray[19].weapon_id = WEAPON_K43;
	ClipInfoArray[19].full_index = gHUD.GetSpriteIndex( "clip_k43_full" );
	ClipInfoArray[19].empty_index = gHUD.GetSpriteIndex( "clip_k43_empty" );
	ClipInfoArray[19].extra_index = gHUD.GetSpriteIndex( "clip_k43_extra" );
	ClipInfoArray[19].lastDrop = 0.138888900f;
	ClipInfoArray[19].subseqDrop = 0.074074075f;

	ClipInfoArray[20].weapon_id = WEAPON_ENFIELD;
	ClipInfoArray[20].full_index = gHUD.GetSpriteIndex( "clip_enfield_full" );
	ClipInfoArray[20].empty_index = gHUD.GetSpriteIndex( "clip_enfield_empty" );
	ClipInfoArray[20].extra_index = gHUD.GetSpriteIndex( "clip_enfield_extra" );
	ClipInfoArray[20].lastDrop = 0.069767445f;
	ClipInfoArray[20].subseqDrop = 0.186046510f;

	ClipInfoArray[21].weapon_id = WEAPON_STEN;
	ClipInfoArray[21].full_index = gHUD.GetSpriteIndex( "clip_sten_full" );
	ClipInfoArray[21].empty_index = gHUD.GetSpriteIndex( "clip_sten_empty" );
	ClipInfoArray[21].extra_index = gHUD.GetSpriteIndex( "clip_sten_extra" );
	ClipInfoArray[21].lastDrop = 0.038461540f;
	ClipInfoArray[21].subseqDrop = 0.030769231f;

	ClipInfoArray[22].weapon_id = WEAPON_BREN;
	ClipInfoArray[22].full_index = gHUD.GetSpriteIndex( "clip_bren_full" );
	ClipInfoArray[22].empty_index = gHUD.GetSpriteIndex( "clip_bren_empty" );
	ClipInfoArray[22].extra_index = gHUD.GetSpriteIndex( "clip_bren_extra" );
	ClipInfoArray[22].lastDrop = 0.093750000f;
	ClipInfoArray[22].subseqDrop = 0.020833334f;

	ClipInfoArray[23].weapon_id = WEAPON_WEBLEY;
	ClipInfoArray[23].full_index = gHUD.GetSpriteIndex( "clip_webley_full" );
	ClipInfoArray[23].empty_index = gHUD.GetSpriteIndex( "clip_webley_full" );
	ClipInfoArray[23].extra_index = gHUD.GetSpriteIndex( "clip_webley_extra" );
	ClipInfoArray[23].lastDrop = 0.000000000f;
	ClipInfoArray[23].subseqDrop = 0.000000000f;

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

	for( int idx = WEAPON_WEBLEY; idx < MAX_WEAPONS; idx++ )
	{
		ClipInfoArray[idx].weapon_id = 0;
		ClipInfoArray[idx].full_index = -1;
		ClipInfoArray[idx].empty_index = -1;
		ClipInfoArray[idx].extra_index = -1;
		ClipInfoArray[idx].lastDrop = 0.0f;
		ClipInfoArray[idx].subseqDrop = 0.0f;
	}

	for( int i = 0; i < MAX_WEAPONS; ++i )
	{
		int fIdx = ClipInfoArray[i].full_index;
		if( fIdx >= 0 )
		{
			ClipInfoArray[i].FullSprite = gHUD.m_rghSprites[fIdx];
			ClipInfoArray[i].FullArea = &gHUD.m_rgrcRects[fIdx];
		}
		else
		{
			ClipInfoArray[i].FullSprite = 0;
			ClipInfoArray[i].FullArea = NULL;
		}

		int eIdx = ClipInfoArray[i].empty_index;
		if( eIdx >= 0 )
		{
			ClipInfoArray[i].EmptySprite = gHUD.m_rghSprites[eIdx];
			ClipInfoArray[i].EmptyArea = &gHUD.m_rgrcRects[eIdx];
		}
		else
		{
			ClipInfoArray[i].EmptySprite = 0;
			ClipInfoArray[i].EmptyArea = NULL;
		}

		int exIdx = ClipInfoArray[i].extra_index;
		if( exIdx >= 0 )
		{
			ClipInfoArray[i].ExtraSprite = gHUD.m_rghSprites[exIdx];
			ClipInfoArray[i].ExtraArea = &gHUD.m_rgrcRects[exIdx];
		}
		else
		{
			ClipInfoArray[i].ExtraSprite = 0;
			ClipInfoArray[i].ExtraArea = NULL;
		}
	}

	BritGrenClipInfo.weapon_id = WEAPON_HANDGRENADE;
	int iBritGrenExtra = gHUD.GetSpriteIndex( "clip_britgrenade_extra" );
	if( iBritGrenExtra >= 0 )
	{
		BritGrenClipInfo.extra_index = iBritGrenExtra;
		BritGrenClipInfo.ExtraSprite = gHUD.m_rghSprites[iBritGrenExtra];
		BritGrenClipInfo.ExtraArea = &gHUD.m_rgrcRects[iBritGrenExtra];
	}

	int HUD_mgbarrel_20 = gHUD.GetSpriteIndex( "hud_barrel" );
	if( HUD_mgbarrel_20 >= 0 )
	{
		MGBarrelHUD = gHUD.m_rghSprites[HUD_mgbarrel_20];
		MGBarrelHUDArea = &gHUD.m_rgrcRects[HUD_mgbarrel_20];
	}

	int HUD_mgbarrel2_20 = gHUD.GetSpriteIndex( "hud_barrelo" );
	if( HUD_mgbarrel2_20 >= 0 )
	{
		MGBarrel2HUD = gHUD.m_rghSprites[HUD_mgbarrel2_20];
		MGBarrel2HUDArea = &gHUD.m_rgrcRects[HUD_mgbarrel2_20];
	}

	gWR.LoadAllWeaponSprites();

	int idxFG42 = gHUD.GetSpriteIndex( "weapon_scopedfg42" );
	if( idxFG42 >= 0 )
	{
		gWR.scoped_fg42.hActive = gHUD.m_rghSprites[idxFG42];
		gWR.scoped_fg42.rcActive = gHUD.m_rgrcRects[idxFG42];
	}

	int idxFCarb = gHUD.GetSpriteIndex( "weapon_fcarb" );
	if( idxFCarb >= 0 )
	{
		gWR.folding_carbine.hActive = gHUD.m_rghSprites[idxFCarb];
		gWR.folding_carbine.rcActive = gHUD.m_rgrcRects[idxFCarb];
	}

	int idxKnife = gHUD.GetSpriteIndex( "weapon_paraknife" );
	if( idxKnife >= 0 )
	{
		gWR.gravity_knife.hActive = gHUD.m_rghSprites[idxKnife];
		gWR.gravity_knife.rcActive = gHUD.m_rgrcRects[idxKnife];
	}

	int idxEnfield = gHUD.GetSpriteIndex( "weapon_scopedenfield" );
	if( idxEnfield >= 0 )
	{
		gWR.scoped_enfield.hActive = gHUD.m_rghSprites[idxEnfield];
		gWR.scoped_enfield.rcActive = gHUD.m_rgrcRects[idxEnfield];
	}

	int idxBKnife = gHUD.GetSpriteIndex( "weapon_britknife" );
	if( idxBKnife >= 0 )
	{
		gWR.brit_knife.hActive = gHUD.m_rghSprites[idxBKnife];
		gWR.brit_knife.rcActive = gHUD.m_rgrcRects[idxBKnife];
	}

	int idxBGren = gHUD.GetSpriteIndex( "weapon_britgrenade" );
	if( idxBGren >= 0 )
	{
		gWR.brit_grenade.hActive = gHUD.m_rghSprites[idxBGren];
		gWR.brit_grenade.rcActive = gHUD.m_rgrcRects[idxBGren];
	}

	int idxBino = gHUD.GetSpriteIndex( "weapon_gerbinoculars" );
	if( idxBino >= 0 )
	{
		gWR.ger_binoculars.hActive = gHUD.m_rghSprites[idxBino];
		gWR.ger_binoculars.rcActive = gHUD.m_rgrcRects[idxBino];
	}

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

	if( gHUD.m_iWeaponBits != gWR.iOldWeaponBits || gWR.iOldWeaponBits2 != g_iWeaponBits2 )
	{
		gWR.Reset();
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;
		gWR.iOldWeaponBits2 = g_iWeaponBits2;

		for( int i = MAX_WEAPONS - 1; i > 0; i-- )
		{
			WEAPON *p = gWR.GetWeapon( i );

			if( p && p->iId )
			{
				bool bHasWeapon = false;

				if( p->iId < 32 )
					bHasWeapon = ( gHUD.m_iWeaponBits & ( 1 << p->iId ) ) != 0;
				else
					bHasWeapon = ( g_iWeaponBits2 & ( 1 << ( p->iId - 32 ) ) ) != 0;

				if( bHasWeapon )
					gWR.PickupWeapon( p );
			}
		}
	}

	if( m_pWeapon )
		m_pWeapon->iLastWeaponState = g_iWeaponFlags;

	if( !gpActiveSel )
		return;

	if( gHUD.m_iKeyBits & IN_ATTACK )
	{
		if( gpActiveSel != ( WEAPON * ) 1 )
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
	if( !fAdvance && gHUD.m_Menu.m_fMenuDisplayed && iDirection == 1 )
	{
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );
		return;
	}

	if( iSlot < 0 || iSlot >= DOD_MAX_SLOTS )
		return;

	if( gHUD.m_fPlayerDead || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) )
		return;

	bool fastSwitch = ( gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f );

	if( gpActiveSel > ( WEAPON * ) 1 )
	{
		int iActiveSlot = gpActiveSel->iSlot;

		if( iActiveSlot == iSlot )
		{
			WEAPON *pNextWeapon = GetNextActivePos( iSlot, gpActiveSel->iSlotPos );

			if( !pNextWeapon )
				pNextWeapon = GetFirstPos( iActiveSlot );

			if( pNextWeapon )
			{
				gpActiveSel = pNextWeapon;
				return;
			}
		}
	}

	WEAPON *pWeapon = GetFirstPos( iSlot );

	if( !pWeapon )
	{
		if( !fastSwitch )
		{
			gpActiveSel = ( WEAPON * ) ( !fastSwitch );
		}
		return;
	}

	if( !fastSwitch )
	{
		gpActiveSel = pWeapon;
		return;
	}

	if( GetNextActivePos( pWeapon->iSlot, pWeapon->iSlotPos ) != NULL )
	{
		gpActiveSel = pWeapon;
		return;
	}

	ServerCmd( pWeapon->szName );
	g_weaponselect = pWeapon->iId;
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

	if( iIndex < 0 || iIndex >= MAX_AMMO_TYPES )
		return 1;

	gWR.SetAmmo( iIndex, abs( iCount ) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoShort( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_SHORT();

	if( iIndex < 0 || iIndex >= MAX_AMMO_TYPES )
		return 1;

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
	gHUD.m_bAutoReloadComplete = 1;
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
	wrect_t nullrc = {0,};

	BEGIN_READ( pbuf, iSize );

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_BYTE();

	if( iId <= 0 )
	{
		SetCrosshair( 0, nullrc, 0, 0, 0 );
		return 0;
	}

	gHUD.m_fPlayerDead = FALSE;
	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if( !pWeapon )
		return 1;

	if( iClip < -1 )
	{
		pWeapon->iClip = -iClip;
		if( !iState )
			return 1;
	}
	else
	{
		pWeapon->iClip = iClip;
		if( !iState )
			return 1;
	}

	m_pWeapon = pWeapon;

	if( !( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) && gHUD.m_iFOV <= 89 )
	{
		if( iState > 1 && pWeapon->hZoomedAutoaim )
			SetCrosshair( pWeapon->hZoomedAutoaim, pWeapon->rcZoomedAutoaim, 255, 255, 255 );
		else
			SetCrosshair( pWeapon->hZoomedCrosshair, pWeapon->rcZoomedCrosshair, 255, 255, 255 );
	}

	m_fFade = 200.0f;
	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudAmmo::GetCurrentWeaponId( void )
{
	if( m_pWeapon )
		return m_pWeapon->iId;
	else
		return WEAPON_NONE;
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
extern char weaponnames[MAX_WEAPONS][64];

int CHudAmmo::MsgFunc_WeaponList( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	WEAPON Weapon;
	memset( &Weapon, 0, sizeof( WEAPON ) );

	Weapon.iAmmoType = ( int ) READ_CHAR();

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
	Weapon.iLastWeaponState = 0;
	Weapon.iClipMax = READ_BYTE();

	if( Weapon.iId < 0 || Weapon.iId >= MAX_WEAPONS )
		return 0;
	if( Weapon.iSlot < 0 || Weapon.iSlot >= DOD_MAX_SLOTS )
		return 0;
	if( Weapon.iSlotPos < 0 || Weapon.iSlotPos >= DOD_MAX_POSITIONS )
		return 0;
	if( Weapon.iAmmoType < -1 || Weapon.iAmmoType >= MAX_AMMO_TYPES )
		return 0;
	if( Weapon.iAmmo2Type < -1 || Weapon.iAmmo2Type >= MAX_AMMO_TYPES )
		return 0;

	if( ( Weapon.iAmmoType == -1 || Weapon.iMax1 > 0 ) && ( Weapon.iAmmo2Type == -1 || Weapon.iMax2 > 0 ) )
	{
		if( Weapon.iClipMax > 0 )
		{
			strncpy( Weapon.szName, weaponnames[Weapon.iId], 128 );
			Weapon.szName[127] = '\0';
			gWR.AddWeapon( &Weapon );
			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	gWR.SelectSlot(iSlot, FALSE, 1);
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

	bool bFastSwitch = ( gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f );

	int slot = 0;
	int pos = 0;

	if( gpActiveSel > ( WEAPON * ) 1 )
	{
		slot = gpActiveSel->iSlot;
		pos = gpActiveSel->iSlotPos + 1;
	}
	else if( m_pWeapon )
	{
		slot = m_pWeapon->iSlot;
		pos = m_pWeapon->iSlotPos + 1;
	}

	for( int loop = 0; loop <= 1; loop++ )
	{
		for( ; slot < DOD_MAX_SLOTS - 1; slot++ )
		{
			for( ; pos < DOD_MAX_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if( wsp != NULL )
				{
					if( bFastSwitch )
					{
						ServerCmd( wsp->szName );
						g_weaponselect = wsp->iId;
						gpActiveSel = NULL;
					}
					else
					{
						gpActiveSel = wsp;
					}
					return;
				}
			}
			pos = 0;
		}

		slot = 0;
		pos = 0;
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon( void )
{
	if( gHUD.m_fPlayerDead || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) )
		return;

	bool bFastSwitch = ( gHUD.hud_fastswitch && gHUD.hud_fastswitch->value != 0.0f );
	int slot = DOD_MAX_SLOTS - 2;
	int pos = 11;

	if( gpActiveSel > ( WEAPON * ) 1 )
	{
		slot = gpActiveSel->iSlot;
		pos = gpActiveSel->iSlotPos - 1;
	}
	else if( m_pWeapon )
	{
		slot = m_pWeapon->iSlot;
		pos = m_pWeapon->iSlotPos - 1;
	}

	for( int loop = 0; loop <= 1; loop++ )
	{
		for( ; slot >= 0; slot-- )
		{
			for( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if( wsp != NULL )
				{
					if( bFastSwitch )
					{
						ServerCmd( wsp->szName );
						g_weaponselect = wsp->iId;
						gpActiveSel = NULL;
					}
					else
					{
						gpActiveSel = wsp;
					}
					return;
				}
			}
			pos = 11;
		}

		slot = DOD_MAX_SLOTS - 2;
		pos = 11;
	}

	gpActiveSel = NULL;
}

ClipInfo *CHudAmmo::GetCurrentGun( WEAPON *pw )
{
	ClipInfo *Clip;

	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( pw->iId == Clip->weapon_id )
			return &Clip[i];

		Clip++;
	}

	return NULL;
}

//-------------------------------------------------------------------------
// Drawing code
// 
//-------------------------------------------------------------------------

extern bool ShowHudElement( int i_hudElement );
extern int g_iAlive, g_iDeadFlag;

int CHudAmmo::Draw( float flTime )
{
	int x, y, r, g, b;
	int clipHeight, clipWidth, ExtraClipWidth, ExtraClipHeight;
	int numGrens, i_eclip, fullclips, remainder;
	int barrelHeight, barrely;

	WEAPON *pw;
	ClipInfo currentGun;
	wrect_t rc, *area;
	float cappedOverheat;
	char buf[8];
	HSPRITE sprite;

	r = 255; g = 255; b = 255;

	if( g_iVuser1z || ( !gHUD.m_iWeaponBits && !g_iWeaponBits2 ) )
		return 1;

	int iFlags = gHUD.m_iHideHUDDisplay;

	if( ( iFlags & HIDEHUD_WEAPONS ) != 0 || g_iUser1 || g_iUser2 || !g_iAlive || g_iDeadFlag || !g_iTeamNumber || !g_iPlayerClass )
		return 1;

	if( ( iFlags & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) != 0 )
		return 1;

	DrawWeaponList( flTime );

	if( !( m_iFlags & HUD_ACTIVE ) )
		return 0;

	pw = m_pWeapon;

	if( !pw )
		return 0;

	gHUD.m_iClipSize = pw->iClip;

	if( gEngfuncs.pDemoAPI->IsPlayingback() )
		gHUD.g_iClip = pw->iClip;

	if( pw->iAmmoType < 0 && pw->iAmmo2Type < 0 )
		return 0;

	if( m_fFade > 0.0f )
		m_fFade -= 20.0f * gHUD.m_flTimeDelta;

	currentGun = *GetCurrentGun( pw );

	if( currentGun.weapon_id == WEAPON_NONE )
		return 0;

	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

	if( pw->iAmmoType <= 0 )
		return 0;

	area = currentGun.FullArea;
	clipHeight = area->bottom - area->top;
	clipWidth = area->right - area->left;

	ExtraClipWidth = currentGun.ExtraArea->right - currentGun.ExtraArea->left;
	ExtraClipHeight = currentGun.ExtraArea->bottom - currentGun.ExtraArea->top;

	if( currentGun.weapon_id != WEAPON_MG42 &&
		currentGun.weapon_id != WEAPON_CAL30 &&
		currentGun.weapon_id != WEAPON_MG34 &&
		currentGun.weapon_id != WEAPON_WEBLEY )
	{
		if( currentGun.weapon_id == WEAPON_HANDGRENADE ||
			currentGun.weapon_id == WEAPON_STICKGRENADE ||
			currentGun.weapon_id == WEAPON_MORTAR )
		{
			sprite = currentGun.ExtraSprite;
			wrect_t *grenArea = currentGun.ExtraArea;

			if( currentGun.weapon_id == WEAPON_HANDGRENADE && gHUD.m_bBritish )
			{
				sprite = BritGrenClipInfo.ExtraSprite;
				grenArea = BritGrenClipInfo.ExtraArea;
			}

			numGrens = gWR.CountAmmo( pw->iAmmoType );

			if( ShowHudElement( 3 ) && numGrens > 0 )
			{
				x = ScreenWidth - ExtraClipWidth - 30;

				gEngfuncs.pfnSPR_Set( sprite, r, g, b );
				gEngfuncs.pfnSPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - ExtraClipHeight, grenArea );

				sprintf( buf, "x %d", numGrens );
				gHUD.DrawHudString( ExtraClipWidth + x, y, ExtraClipWidth + x + 64, buf, r, g, b );
			}
		}

		else if( currentGun.weapon_id == WEAPON_BAZOOKA ||
			currentGun.weapon_id == WEAPON_PSCHRECK ||
			currentGun.weapon_id == WEAPON_PIAT )
		{
			numGrens = gWR.CountAmmo( pw->iAmmoType );

			x = ScreenWidth - ( ExtraClipWidth + clipWidth ) - 30;

			if( ShowHudElement( 3 ) )
			{
				gEngfuncs.pfnSPR_Set( currentGun.EmptySprite, r, g, b );
				gEngfuncs.pfnSPR_DrawHoles( 0, x, y - clipHeight + gHUD.m_iFontHeight, currentGun.EmptyArea );

				if( gHUD.g_iClip > 0 )
				{
					gEngfuncs.pfnSPR_Set( currentGun.FullSprite, r, g, b );
					gEngfuncs.pfnSPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - clipHeight, currentGun.FullArea );
				}

				if( numGrens > 0 )
				{
					i_eclip = clipWidth + x;

					gEngfuncs.pfnSPR_Set( currentGun.ExtraSprite, r, g, b );
					gEngfuncs.pfnSPR_DrawHoles( 0, i_eclip, gHUD.m_iFontHeight + y - ExtraClipHeight, currentGun.ExtraArea );
					sprintf( buf, "x %d", numGrens );
					gHUD.DrawHudString( ExtraClipWidth + i_eclip, y, ExtraClipWidth + i_eclip + 64, buf, r, g, b );
				}
			}
		}
	}
	else
	{
		if( ShowHudElement( 3 ) )
		{
			x = ScreenWidth - clipWidth - 30;

			fullclips = pw->iClip / 10;
			remainder = pw->iClip % 10;

			sprite = gHUD.GetSprite( currentGun.weapon_id );
			rc = gHUD.GetSpriteRect( currentGun.weapon_id );

			gEngfuncs.pfnSPR_Set( currentGun.EmptySprite, r, g, b );
			gEngfuncs.pfnSPR_DrawHoles( 0, x, y - clipHeight + gHUD.m_iFontHeight, currentGun.EmptyArea );

			if( gHUD.g_iClip > 0 )
			{
				gEngfuncs.pfnSPR_Set( currentGun.FullSprite, r, g, b );
				gEngfuncs.pfnSPR_DrawHoles( 0, x, gHUD.m_iFontHeight + y - clipHeight, currentGun.FullArea );
			}

			if( g_flWeaponHeat > 0.0f )
			{
				cappedOverheat = g_flWeaponHeat;

				if( cappedOverheat > 1.0f )
					cappedOverheat = 1.0f;

				barrelHeight = clipHeight;
				barrely = y - barrelHeight + gHUD.m_iFontHeight;

				sprite = gHUD.GetSprite( currentGun.weapon_id + 100 );
				rc = gHUD.GetSpriteRect( currentGun.weapon_id + 100 );

				int iHeatVisualHeight = ( int ) ( barrelHeight * cappedOverheat );
				rc.top = rc.bottom - iHeatVisualHeight;

				gEngfuncs.pfnSPR_Set( sprite, 255, ( int ) ( 255 * ( 1.0f - cappedOverheat ) ), 0 );
				gEngfuncs.pfnSPR_DrawHoles( 0, x - 15, barrely + ( barrelHeight - iHeatVisualHeight ), &rc );
			}

			numGrens = gWR.CountAmmo( pw->iAmmoType );

			if( numGrens > 0 )
			{
				i_eclip = clipWidth + x + 5;

				gEngfuncs.pfnSPR_Set( currentGun.ExtraSprite, r, g, b );
				gEngfuncs.pfnSPR_DrawHoles( 0, i_eclip, gHUD.m_iFontHeight + y - ExtraClipHeight, currentGun.ExtraArea );

				sprintf( buf, "x %d", numGrens );
				gHUD.DrawHudString( ExtraClipWidth + i_eclip, y, ExtraClipWidth + i_eclip + 64, buf, r, g, b );
			}
		}
	}

	return 0;
}

int CHudAmmo::DrawWeaponList( float flTime )
{
	int x, y, r, g, b;
	int width, iSlotHeight, averageHeight;
	int iSlot, numdrawn, iPos;

	WEAPON *pWpn;
	wrect_t *p_rcActive;
	HSPRITE hSprite;

	if( !gpActiveSel )
		return 0;

	averageHeight = ScreenHeight / 2 - 150;
	iSlotHeight = averageHeight;

	for( iSlot = 0; iSlot < 5; iSlot++ )
	{
		numdrawn = 0;

		for( iPos = 0; iPos < 12; iPos++ )
		{
			pWpn = gWR.GetWeaponSlot( iSlot, iPos );

			if( !pWpn || !pWpn->iId )
				continue;

			iSlotHeight += 50;
			numdrawn++;

			x = ScreenWidth + pWpn->rcActive.left - 5 - pWpn->rcActive.right;

			r = g = b = ( gpActiveSel == pWpn ) ? 255 : 80;

			hSprite = pWpn->hActive;
			p_rcActive = &pWpn->rcActive;

			switch( pWpn->iId )
			{
			case WEAPON_FG42:
				if( g_iWeaponFlags & 1 )
				{
					hSprite = gWR.scoped_fg42.hActive;
					p_rcActive = &gWR.scoped_fg42.rcActive;
				}
				break;
			case WEAPON_M1CARBINE:
				if( gHUD.m_bParatrooper )
				{
					hSprite = gWR.folding_carbine.hActive;
					p_rcActive = &gWR.folding_carbine.rcActive;
				}
				break;
			case WEAPON_GERKNIFE:
				if( gHUD.m_bParatrooper )
				{
					hSprite = gWR.gravity_knife.hActive;
					p_rcActive = &gWR.gravity_knife.rcActive;
				}
				break;
			case WEAPON_ENFIELD:
				if( pWpn->iLastWeaponState & WPNSTATE_SCOPED )
				{
					hSprite = gWR.scoped_enfield.hActive;
					p_rcActive = &gWR.scoped_enfield.rcActive;
				}
				break;
			case WEAPON_AMERKNIFE:
				if( gHUD.m_bBritish )
				{
					hSprite = gWR.brit_knife.hActive;
					p_rcActive = &gWR.brit_knife.rcActive;
				}
				break;
			case WEAPON_HANDGRENADE:
				if( gHUD.m_bBritish )
				{
					hSprite = gWR.brit_grenade.hActive;
					p_rcActive = &gWR.brit_grenade.rcActive;
				}
				break;
			default:
				if( pWpn->iId == WEAPON_BINOC && g_iTeamNumber == 2 )
				{
					hSprite = gWR.ger_binoculars.hActive;
					p_rcActive = &gWR.ger_binoculars.rcActive;
				}
				break;
			}

			gEngfuncs.pfnSPR_Set( hSprite, r, g, b );
			gEngfuncs.pfnSPR_DrawHoles( 0, x, iSlotHeight, p_rcActive );
		}

		averageHeight = y;

		if( !numdrawn )
			averageHeight = y + 50;

		iSlotHeight += 52;

		if( iSlotHeight == 260 )
			return 1;
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
