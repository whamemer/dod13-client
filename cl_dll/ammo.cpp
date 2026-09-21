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

extern int g_iWeaponBits2;

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
	if( !p )
		return FALSE;

	// weapons with no max ammo can always be selected
	if( p->iMax1 == -1 )
		return TRUE;

	return ( p->iAmmoType == -1 ) || p->iClip > 0 || CountAmmo( p->iAmmoType ) 
		|| CountAmmo( p->iAmmo2Type ) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}

void WeaponsResource::LoadWeaponSprites( WEAPON *pWeapon )
{
	int i, iRes;

	iRes = GetSpriteRes( ScreenWidth, ScreenHeight );

	char sz[256];

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

	sprintf( sz, "sprites/%s.txt", pWeapon->szName );
	client_sprite_t *pList = SPR_GetList( sz, &i );

	if( !pList )
		return;

	int index = gHUD.GetSpriteIndex( pWeapon->szName );

	if( index > 0 )
		pWeapon->hActive = gHUD.m_rghSprites[index];
	else
		pWeapon->hActive = 0;

	pWeapon->rcActive = gHUD.m_rgrcRects[index];

	client_sprite_t *p;

	p = GetSpriteList( pList, "crosshair", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hCrosshair = SPR_Load( sz );
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = 0;

	p = GetSpriteList( pList, "autoaim", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hAutoaim = SPR_Load( sz );
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList( pList, "zoom", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hZoomedCrosshair = SPR_Load( sz );
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList( pList, "zoom_autoaim", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hZoomedAutoaim = SPR_Load( sz );
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList( pList, "weapon", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hInactive = SPR_Load( sz );
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList( pList, "weapon_s", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hActive = SPR_Load( sz );
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList( pList, "ammo", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hAmmo = SPR_Load( sz );
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList( pList, "ammo2", iRes, i );
	if( p )
	{
		sprintf( sz, "sprites/%s.spr", p->szSprite );
		pWeapon->hAmmo2 = SPR_Load( sz );
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo2 = 0;
}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource::GetFirstPos( int iSlot )
{
	WEAPON *pret = NULL;

	for( int i = 0; i < MAX_WEAPON_POSITIONS; i++ )
	{
		if ( rgSlots[iSlot][i] && HasAmmo( rgSlots[iSlot][i] ) )
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
	
	if ( !p || !gWR.HasAmmo( p ) )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}

int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE( m_Ammo, CurWeapon )	// Current weapon and clip
DECLARE_MESSAGE( m_Ammo, WeaponList )	// new weapon type
DECLARE_MESSAGE( m_Ammo, AmmoShort )
DECLARE_MESSAGE( m_Ammo, AmmoX )		// update known ammo type's count
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

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"5"

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

	CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );
	CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();
	gHR.Init();

	return 1;
}

void CHudAmmo::Reset( void )
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();

	//VidInit();
	wrect_t nullrc = {0,};
	SetCrosshair( 0, nullrc, 0, 0, 0 ); // reset crosshair
	m_pWeapon = NULL; // reset last weapon
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
	ClipInfoArray[10].extra_index = gHUD.GetSpriteIndex( "clip_stick_extra" );

	ClipInfoArray[11].weapon_id = WEAPON_HANDGRENADE;
	ClipInfoArray[11].full_index = gHUD.GetSpriteIndex( "clip_grenade_full" );
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

	for( int i = 0; i != 64; ++i )
	{
		if( ClipInfoArray[i].full_index >= 0 )
		{
			ClipInfoArray[i].FullSprite = gHUD.m_rghSprites[ClipInfoArray[i].full_index];
			ClipInfoArray[i].FullArea = &gHUD.m_rgrcRects[ClipInfoArray[i].full_index];
		}
		else
		{
			ClipInfoArray[i].FullSprite = 0;
			ClipInfoArray[i].FullArea = NULL;
		}

		if( ClipInfoArray[i].empty_index >= 0 )
		{
			ClipInfoArray[i].EmptySprite = gHUD.m_rghSprites[ClipInfoArray[i].empty_index];
			ClipInfoArray[i].EmptyArea = &gHUD.m_rgrcRects[ClipInfoArray[i].empty_index];
		}
		else
		{
			ClipInfoArray[i].EmptySprite = 0;
			ClipInfoArray[i].EmptyArea = NULL;
		}

		if( ClipInfoArray[i].extra_index >= 0 )
		{
			ClipInfoArray[i].ExtraSprite = gHUD.m_rghSprites[ClipInfoArray[i].extra_index];
			ClipInfoArray[i].ExtraArea = &gHUD.m_rgrcRects[ClipInfoArray[i].extra_index];
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

	m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	giBucketWidth = 12;
	giBucketHeight = 12;

	if( m_HUD_bucket0 != -1 )
	{
		ghsprBuckets = gHUD.GetSprite( m_HUD_bucket0 );
		giBucketWidth = gHUD.GetSpriteRect( m_HUD_bucket0 ).right - gHUD.GetSpriteRect( m_HUD_bucket0 ).left;
		giBucketHeight = gHUD.GetSpriteRect( m_HUD_bucket0 ).bottom - gHUD.GetSpriteRect( m_HUD_bucket0 ).top;
		gHR.iHistoryGap = gHUD.GetSpriteRect( m_HUD_bucket0 ).bottom - gHUD.GetSpriteRect( m_HUD_bucket0 ).top;
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

	if( gHUD.m_scrinfo.iWidth <= 639 )
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
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;
		gWR.iOldWeaponBits2 = g_iWeaponBits2;

		for( int i = MAX_WEAPONS-1; i > 0; i-- )
		{
			WEAPON *p = gWR.GetWeapon( i );

			if( p && p->iId )
			{
				bool bHasWeapon = false;

				if( p->iId < 32 )
					bHasWeapon = ( gHUD.m_iWeaponBits & ( 1 << p->iId ) ) != 0;
				else
					bHasWeapon = ( g_iWeaponBits2 & ( 1 << ( p->iId - 32 ) ) ) != 0;

				if( gHUD.m_iWeaponBits & ( 1 << p->iId ) )
					gWR.PickupWeapon( p );
				else
					gWR.DropWeapon( p );
			}
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

		PlaySound( "common/wpn_select.wav", 1 );
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

	if( iSlot > MAX_WEAPON_SLOTS )
		return;

	if( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	WEAPON *p = NULL;
	bool fastSwitch = CVAR_GET_FLOAT( "hud_fastswitch" ) != 0;

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
	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs( iCount ) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_WEAP, iIndex );

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	const char *szName = READ_STRING();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

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

	if( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if( ( iId == -1 ) && ( iClip == -1 ) )
		{
			PlayerDied();
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );

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

	if( ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) && gHUD.m_iFOV <= 89 )
	{
		if( gHUD.m_iFOV >= 90 )
		{
			if( iState > 1 && pWeapon->hZoomedAutoaim )
				SetCrosshair( pWeapon->hZoomedAutoaim, pWeapon->rcZoomedAutoaim, 255, 255, 255 );
			else
				SetCrosshair( pWeapon->hZoomedCrosshair, pWeapon->rcZoomedCrosshair, 255, 255, 255 );
		}
	}

	m_fFade = 200.0f; //!!!
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
extern char weaponnames[35][64];

int CHudAmmo::MsgFunc_WeaponList( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	WEAPON Weapon;

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
	Weapon.iLastWeaponState = 0;
	Weapon.iClipMax = READ_BYTE();

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

	if( ( Weapon.iAmmoType == -1 || Weapon.iMax1 ) && ( Weapon.iAmmo2Type == -1 || Weapon.iMax2 ) )
	{
		if( Weapon.iClipMax > 0 )
		{
			if( Weapon.iId >= 0 && Weapon.iId < WEAPON_GERPARAKNIFE )
			{
				strncpy( Weapon.szName, weaponnames[Weapon.iId], 128 );
			}
			else
			{
				sprintf( Weapon.szName, "weapon_unknown_%d", Weapon.iId );
			}

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
		PlaySound( "common/wpn_hudoff.wav", 1 );
	}
	else
		ClientCmd( "escape" );
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon( void )
{
	if( gHUD.m_fPlayerDead || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) )
		return;

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

				if( wsp && gWR.HasAmmo( wsp ) )
				{
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

				if( wsp && gWR.HasAmmo( wsp ) )
				{
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
	int height, barrelHeight, barrely;

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

	height = gHUD.m_scrinfo.iHeight;

	y = height - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

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
				x = gHUD.m_scrinfo.iWidth - ExtraClipWidth - 30;

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

			x = gHUD.m_scrinfo.iWidth - ( ExtraClipWidth + clipWidth ) - 30;

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
			x = gHUD.m_scrinfo.iWidth - clipWidth - 30;

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

//
// Draws the ammo bar on the hud
//
int DrawBar( int x, int y, int width, int height, float f )
{
	int r, g, b;

	if( f < 0 )
		f = 0;
	if( f > 1 )
		f = 1;

	if( f )
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if( w <= 0 )
			w = 1;
		UnpackRGB( r, g, b, RGB_GREENISH );
		FillRGBA( x, y, w, height, r, g, b, 255 );
		x += w;
		width -= w;
	}

	UnpackRGB( r, g, b, RGB_YELLOWISH );

	FillRGBA( x, y, width, height, r, g, b, 128 );

	return ( x + width );
}

void DrawAmmoBar( WEAPON *p, int x, int y, int width, int height )
{
	if( !p )
		return;

	if( p->iAmmoType != -1 )
	{
		if( !gWR.CountAmmo( p->iAmmoType ) )
			return;

		float f = (float)gWR.CountAmmo( p->iAmmoType ) / (float)p->iMax1;
		
		x = DrawBar( x, y, width, height, f );

		// Do we have secondary ammo too?
		if( p->iAmmo2Type != -1 )
		{
			f = (float)gWR.CountAmmo( p->iAmmo2Type ) / (float)p->iMax2;

			x += 5; //!!!

			DrawBar( x, y, width, height, f );
		}
	}
}

//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList( float flTime )
{
	int r, g, b, x, y, a, i;

	if( !gpActiveSel )
		return 0;

	int iActiveSlot;

	if( gpActiveSel == (WEAPON *) 1 )
		iActiveSlot = -1;	// current slot has no weapons
	else 
		iActiveSlot = gpActiveSel->iSlot;

	x = 10; //!!!
	y = 10; //!!!

	// Ensure that there are available choices in the active slot
	if( iActiveSlot > 0 )
	{
		if( !gWR.GetFirstPos( iActiveSlot ) )
		{
			gpActiveSel = (WEAPON *) 1;
			iActiveSlot = -1;
		}
	}

	// Draw top line
	for( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		int iWidth;

		UnpackRGB( r, g, b, RGB_YELLOWISH );

		if( iActiveSlot == i )
			a = 255;
		else
			a = 192;

		ScaleColors( r, g, b, 255 );
		SPR_Set( gHUD.GetSprite( m_HUD_bucket0 + i ), r, g, b );

		// make active slot wide enough to accomodate gun pictures
		if( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos( iActiveSlot );
			if( p )
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect( m_HUD_bucket0 + i ) );
		
		x += iWidth + 5;
	}

	a = 128; //!!!
	x = 10;

	// Draw all of the buckets
	for( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		y = giBucketHeight + 10;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos( i );
			int iWidth = giBucketWidth;
			if( p )
				iWidth = p->rcActive.right - p->rcActive.left;

			for( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				p = gWR.GetWeaponSlot( i, iPos );

				if( !p || !p->iId )
					continue;

				UnpackRGB( r, g, b, RGB_YELLOWISH );

				// if active, then we must have ammo.
				if( gpActiveSel == p )
				{
					SPR_Set( p->hActive, r, g, b );
					SPR_DrawAdditive( 0, x, y, &p->rcActive );

					SPR_Set( gHUD.GetSprite( m_HUD_selection ), r, g, b );
					SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect( m_HUD_selection ) );
				}
				else
				{
					// Draw Weapon if Red if no ammo
					if( gWR.HasAmmo( p ) )
						ScaleColors( r, g, b, 192 );
					else
					{
						UnpackRGB( r, g, b, RGB_REDISH );
						ScaleColors( r, g, b, 128 );
					}

					SPR_Set( p->hInactive, r, g, b );
					SPR_DrawAdditive( 0, x, y, &p->rcInactive );
				}

				// Draw Ammo Bar
				DrawAmmoBar( p, x + giABWidth / 2, y, giABWidth, giABHeight );
				
				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;
		}
		else
		{
			// Draw Row of weapons.
			UnpackRGB( r, g, b, RGB_YELLOWISH );

			for( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				WEAPON *p = gWR.GetWeaponSlot( i, iPos );

				if( !p || !p->iId )
					continue;

				if( gWR.HasAmmo( p ) )
				{
					UnpackRGB( r, g, b, RGB_YELLOWISH );
					a = 128;
				}
				else
				{
					UnpackRGB( r, g, b, RGB_REDISH );
					a = 96;
				}

				FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}

	return 1;
}

int CHudAmmo::DrawWeaponList( float flTime )
{
	int x, y, r, g, b;
	int height, width, iSlotHeight, averageHeight;
	int iSlot, numdrawn, iPos;

	WEAPON *pWpn;
	wrect_t *p_rcActive;
	HSPRITE hSprite;

	if( !gpActiveSel )
		return 0;

	height = gHUD.m_scrinfo.iHeight;
	width = gHUD.m_scrinfo.iWidth;

	averageHeight = height / 2 - 150;
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

			x = width + pWpn->rcActive.left - 5 - pWpn->rcActive.right;

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

		if( !numdrawn )
		{
			iSlotHeight += 50;
		}
	}

	return 1;
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
