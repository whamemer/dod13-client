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
// dod_crosshair.cpp
//
// implementation of the CHudDoDCrossHair class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_studioint.h"
#include "pm_shared.h"

#include "dod_shared.h"

extern cvar_t *cl_dynamic_xhair;
extern cvar_t *cl_xhair_style;

extern engine_studio_api_t IEngineStudio;
extern p_wpninfo_s P_WpnInfo[];

DECLARE_MESSAGE( m_DoDCrossHair, ClanTimer )

float flBoltHideXHair = 0.0f;

int CHudDoDCrossHair::Init( void )
{
	HOOK_MESSAGE( ClanTimer );

	gHUD.AddHudElem( this );

	m_iYPos = ScreenHeight / 2;
	m_iFlags |= HUD_ACTIVE;
	m_fMoveTime = 0.0f;
	m_iXPos = ScreenWidth / 2;

	return 1;
}

int CHudDoDCrossHair::VidInit( void )
{
	int horiz = gHUD.GetSpriteIndex( "cross_style_2_horiz" );
	int vert = gHUD.GetSpriteIndex( "cross_style_2_vert" );
	int dot = gHUD.GetSpriteIndex( "cross_style_2_dot" );

	CrossSprite2horiz = gHUD.GetSprite( horiz );
	CrossArea2horiz = &gHUD.GetSpriteRect( horiz );
	CrossSprite2vert = gHUD.GetSprite( vert );
	CrossArea2vert = &gHUD.GetSpriteRect( vert );
	CrossSprite2dot = gHUD.GetSprite( dot );

	flBoltHideXHair = 0.0f;
	m_fClanTimer = 0.0f;

	CrossArea2dot = &gHUD.GetSpriteRect( dot );

	m_hCrosshair = SPR_Load( "sprites/crosshairs.spr" );
	m_crosshairRect.left = 0;
	m_crosshairRect.right = 0;
	m_crosshairRect.top = 0;
	m_crosshairRect.bottom = 0;

	m_hCustomCrosshair = SPR_Load( "sprites/customXHair.spr" );
	m_customCrosshairRect.left = 0;
	m_customCrosshairRect.right = 0;
	m_customCrosshairRect.top = 0;
	m_customCrosshairRect.bottom = 0;

	return 1;
}

void CHudDoDCrossHair::Reset( void )
{
	/* Nothing. */
}

int CHudDoDCrossHair::MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_fClanTimer = gHUD.m_flTime + READ_BYTE();

	return 1;
}

extern int g_iWeaponFlags;
extern int g_ihidexhair;

extern int g_iDeadFlag;

int CHudDoDCrossHair::Draw( float flTime )
{
	DrawClanTimer( flTime );

	if( !g_iUser1 )
	{
		if( !g_iUser2 && ShouldDrawCrossHair() )
		{
			if( (int)cl_xhair_style->value )
				DrawCustomCrossHair( (int)cl_xhair_style->value );
			else
				DrawDynamicCrossHair();
		}

		return 1;
	}

	if( g_iUser1 == OBS_ROAMING || g_iUser1 == OBS_IN_EYE )
		DrawSpectatorCrossHair();

	return 1;
}

void CHudDoDCrossHair::DrawDynamicCrossHair( void )
{
	int width = GetCrossHairWidth();
	int x = ScreenWidth / 2;
	int y = ScreenHeight / 2;

	SPR_Set( CrossSprite2dot, 255, 255, 255 );
	SPR_DrawHoles( 0, x - 1, y - 1, CrossArea2dot );

	int w = CrossArea2horiz->right - CrossArea2horiz->left;
	int h = CrossArea2horiz->bottom - CrossArea2horiz->top;

	SPR_Set( CrossSprite2horiz, 255, 255, 255 );
	SPR_DrawHoles( 0, x - width - w + 2, y - h / 2, CrossArea2horiz );
	SPR_DrawHoles( 0, x + width, y - h / 2, CrossArea2horiz );

	h = CrossArea2vert->bottom - CrossArea2vert->top;

	SPR_Set( CrossSprite2vert, 255, 255, 255 );
	SPR_DrawHoles( 0, x - 1, y - width - h + 2, CrossArea2vert );
	SPR_DrawHoles( 0, x - 1, y + width, CrossArea2vert );
}

void CHudDoDCrossHair::DrawCustomCrossHair( int style )
{
	wrect_t area;

	int iStyle = ( style > 16 ) ? 16 : style;
	int iIndex = iStyle - 1;

	m_customCrosshairRect.left = ( iIndex % 4 ) * 64;
	m_customCrosshairRect.top = ( iIndex / 4 ) * 64;
	m_customCrosshairRect.right = ( ( iIndex % 4 ) + 1 ) * 64;
	m_customCrosshairRect.bottom = ( ( iIndex / 4 ) + 1 ) * 64;

	area = m_customCrosshairRect;

	int w = area.right - area.left;
	int h = area.bottom - area.top;

	SPR_Set( m_hCustomCrosshair, 255, 255, 255 );
	SPR_DrawHoles( 0, ScreenWidth / 2 - ( w >> 1 ), ScreenHeight / 2 - ( h >> 1 ), &area );
}

int CHudDoDCrossHair::GetCrossHairWidth( void )
{
	if( cl_dynamic_xhair && cl_dynamic_xhair->value < 1.0f )
		return 4;

	unsigned int weaponId = gHUD.GetCurrentWeaponId();

	// grenades and the rocket launchers have a fixed crosshair
	const unsigned int fixedMask = ( 1u << WEAPON_HANDGRENADE ) | ( 1u << WEAPON_STICKGRENADE ) | ( 1u << WEAPON_STICKGRENADEX ) | ( 1u << WEAPON_HANDGRENADEX )
		| ( 1u << WEAPON_BAZOOKA ) | ( 1u << WEAPON_PSCHRECK ) | ( 1u << WEAPON_PIAT );

	if( weaponId <= 31 && ( ( 1u << weaponId ) & fixedMask ) )
		return 4;

	float flAccuracy = P_WpnInfo[weaponId].base_accuracy;

	if( gHUD.IsInMGDeploy() )
		flAccuracy = P_WpnInfo[weaponId].base_accuracy2;

	if( Length( gHUD.m_vecVelocity ) > 105.0f )
		flAccuracy += P_WpnInfo[weaponId].accuracy_penalty;

	int width = (int)( flAccuracy * 200.0f ) + 2;

	// the crosshair follows the accuracy a few pixels per frame
	if( width < m_iLastXHairWidth )
	{
		if( width < m_iLastXHairWidth - 2 )
			width = m_iLastXHairWidth - 3;

		m_iLastXHairWidth = width;
	}
	else if( width > m_iLastXHairWidth )
	{
		if( width > m_iLastXHairWidth + 2 )
			width = m_iLastXHairWidth + 3;

		m_iLastXHairWidth = width;
	}
	else
	{
		return m_iLastXHairWidth;
	}

	return width;
}

float CHudDoDCrossHair::GetCurrentWeaponAccuracy( void )
{
	int weaponId = gHUD.GetCurrentWeaponId();
	float flSpread = P_WpnInfo[weaponId].base_accuracy;

	if( gHUD.IsInMGDeploy() )
		flSpread = P_WpnInfo[weaponId].base_accuracy2;

	if( Length( gHUD.m_vecVelocity ) > 105.0f )
		return flSpread + P_WpnInfo[weaponId].accuracy_penalty;

	return flSpread;
}

bool CHudDoDCrossHair::ShouldDrawCrossHair( void )
{
	if( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return false;

	if( gHUD.m_Icons.m_iHealth <= 0 )
		return false;

	if( g_iDeadFlag || g_iVuser1z )
		return false;

	if( gHUD.GetMinimapState() == 1 )
		return false;

	unsigned int weaponId = gHUD.GetCurrentWeaponId();

	if( weaponId == (unsigned int)-1 || ( !gHUD.m_iWeaponBits && !g_iWeaponBits2 ) )
		return false;

	if( flBoltHideXHair > 0.0f || g_ihidexhair )
		return false;

	// knives and the spade
	if( weaponId <= WEAPON_GERKNIFE || weaponId == WEAPON_SPADE )
		return false;

	if( weaponId == WEAPON_MORTAR )
		return false;

	// FG42 scope
	if( weaponId == WEAPON_FG42 && gHUD.m_iFOV <= 89 )
		return false;

	// the bipod weapons only while deployed
	if( ( weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_MG34 ) && !gHUD.IsInMGDeploy() )
		return false;

	bool bScoped = false;

	if( weaponId == WEAPON_SPRING || weaponId == WEAPON_BINOC || weaponId == WEAPON_SCOPEDKAR )
		bScoped = true;
	else if( weaponId == WEAPON_ENFIELD )
		bScoped = ( g_iWeaponFlags & WPNSTATE_SCOPED ) != 0;

	// scoped weapons have their own overlay unless the renderer is software
	return ( !IEngineStudio.IsHardware() && bScoped && gHUD.m_iFOV <= 89 ) || !bScoped;
}

void CHudDoDCrossHair::DrawSpectatorCrossHair( void )
{
	if( (unsigned int)( gHUD.m_iFOV - 1 ) > 88 )
	{
		wrect_t area;

		m_crosshairRect.left = 24;
		m_crosshairRect.top = 0;
		m_crosshairRect.right = 48;
		m_crosshairRect.bottom = 24;

		area = m_crosshairRect;

		SPR_Set( m_hCrosshair, 255, 255, 255 );
		SPR_DrawHoles( 0, ScreenWidth / 2 - 12, ScreenHeight / 2 - 12, &area );
	}
}

void CHudDoDCrossHair::DrawClanTimer( float flTime )
{
	if( m_fClanTimer > gHUD.m_flTime )
	{
		float flRemaining = m_fClanTimer - gHUD.m_flTime;
		int mins = (int)flRemaining / 60;
		int secs = (int)flRemaining - mins * 60;

		char buf[256];

		if( secs > 1 || mins > 0 )
		{
			char smins[6];
			char ssecs[18];
			int flash;

			strcpy( buf, "#Clan_time_remaining" );

			if( secs <= 5 )
				flash = 0;
			else if( ( secs & 1 ) || secs > 20 )
				flash = 1;
			else
				flash = 0;

			sprintf( smins, "%d", mins );
			sprintf( ssecs, "%d", secs );

			gHUD.m_VGUI2Print.VGUI2HudPrintArgs( buf, smins, ssecs, NULL, NULL, -1, ScreenHeight / 3, 1.0f, (float)flash, (float)flash );
		}
		else
		{
			strcpy( buf, "#Clan_match_live" );

			gHUD.m_VGUI2Print.VGUI2HudPrintArgs( buf, NULL, NULL, NULL, NULL, -1, ScreenHeight / 3, 1.0f, 0.0f, 0.0f );
		}
	}
}
