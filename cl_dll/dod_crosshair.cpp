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
//  dod_crosshair.cpp - implementation of the CHudDoDCrossHair class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "dod_shared.h"
#include "com_model.h"

extern "C"
{
#include "pm_shared.h"
}

extern engine_studio_api_t IEngineStudio;

extern cvar_t *cl_dynamic_xhair;
extern cvar_s *cl_xhair_style;

DECLARE_MESSAGE( m_DoDCrossHair, ClanTimer );

float flBoltHideXHair;

int CHudDoDCrossHair::Init( void )
{
	HOOK_MESSAGE( ClanTimer );
	gHUD.AddHudElem( this );
	m_iYPos = gHUD.m_scrinfo.iHeight / 2;
	m_iXPos = gHUD.m_scrinfo.iWidth / 2;
	m_iFlags |= HUD_ACTIVE;
	m_fMoveTime = 0.0f;
	return 1;
}

int CHudDoDCrossHair::VidInit( void )
{
	int Horiz = gHUD.GetSpriteIndex( "cross_style_2_horiz" );
	int Vert = gHUD.GetSpriteIndex( "cross_style_2_vert" );
	int Dot = gHUD.GetSpriteIndex( "cross_style_2_dot" );

	if( Horiz >= 0 )
	{
		CrossSprite2horiz = gHUD.m_rghSprites[Horiz];
		CrossArea2horiz = &gHUD.m_rgrcRects[Horiz];
	}
	else
	{
		CrossSprite2horiz = 0;
		CrossArea2horiz = NULL;
	}

	if( Vert >= 0 )
	{
		CrossSprite2vert = gHUD.m_rghSprites[Vert];
		CrossArea2vert = &gHUD.m_rgrcRects[Vert];
	}
	else
	{
		CrossSprite2vert = 0;
		CrossArea2vert = NULL;
	}

	if( Dot >= 0 )
	{
		CrossSprite2dot = gHUD.m_rghSprites[Dot];
		CrossArea2dot = &gHUD.m_rgrcRects[Dot];
	}
	else
	{
		CrossSprite2dot = 0;
		CrossArea2dot = NULL;
	}

	flBoltHideXHair = 0.0f;
	m_fClanTimer = 0.0f;

	m_hCrosshair = gEngfuncs.pfnSPR_Load( "sprites/crosshairs.spr" );
	m_crosshairRect.left = 0;
	m_crosshairRect.right = 0;
	m_crosshairRect.top = 0;
	m_crosshairRect.bottom = 0;

	m_hCustomCrosshair = gEngfuncs.pfnSPR_Load( "sprites/customXHair.spr" );
	m_customCrosshairRect.left = 0;
	m_customCrosshairRect.right = 0;
	m_customCrosshairRect.top = 0;
	m_customCrosshairRect.bottom = 0;

	return 1;
}

int CHudDoDCrossHair::MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_fClanTimer = READ_BYTE() + gHUD.m_flTime;
	return 1;
}

extern int g_iWeaponFlags;
extern int g_ihidexhair;
extern int g_iDeadFlag;

int CHudDoDCrossHair::Draw( float flTime )
{
	int value;

	DrawClanTimer( flTime );

	if( !g_iUser1 )
	{
		if( !g_iUser2 && ShouldDrawCrossHair() )
		{
			value = ( int ) cl_xhair_style->value;

			if( value )
				DrawCustomCrossHair( value );
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
	int h, b;
	int frame, iHalfWidth;
	int x, y;
	int iHalfHeight, CrossHairWidth;

	CrossHairWidth = GetCrossHairWidth();

	iHalfWidth = gHUD.m_scrinfo.iWidth / 2;
	iHalfHeight = gHUD.m_scrinfo.iHeight / 2;
	frame = 0;

	gEngfuncs.pfnSPR_Set( CrossSprite2dot, 255, 255, 255 );
	x = iHalfWidth - 1;
	y = iHalfHeight - 1;
	gEngfuncs.pfnSPR_DrawHoles( frame, x, y, CrossArea2dot );

	if( CrossArea2horiz )
	{
		b = CrossArea2horiz->right - CrossArea2horiz->left;
		h = CrossArea2horiz->bottom - CrossArea2horiz->top;

		gEngfuncs.pfnSPR_Set( CrossSprite2horiz, 255, 255, 255 );
		y = iHalfHeight - h / 2;

		x = iHalfWidth - CrossHairWidth - b + 2;
		gEngfuncs.pfnSPR_DrawHoles( frame, x, y, CrossArea2horiz );

		x = CrossHairWidth + iHalfWidth;
		gEngfuncs.pfnSPR_DrawHoles( frame, x, y, CrossArea2horiz );
	}

	if( CrossArea2vert )
	{
		b = CrossArea2vert->right - CrossArea2vert->left;
		h = CrossArea2vert->bottom - CrossArea2vert->top;

		gEngfuncs.pfnSPR_Set( CrossSprite2vert, 255, 255, 255 );
		x = iHalfWidth - 1;

		y = iHalfHeight - CrossHairWidth - h + 2;
		gEngfuncs.pfnSPR_DrawHoles( frame, x, y, CrossArea2vert );

		y = CrossHairWidth + iHalfHeight;
		gEngfuncs.pfnSPR_DrawHoles( frame, x, y, CrossArea2vert );
	}
}

void CHudDoDCrossHair::DrawCustomCrossHair( int style )
{
	if( style > 16 )
	{
		style = 16;
	}
	else if( style < 1 )
	{
		style = 1;
	}

	int iIndex = style - 1;

	int column = iIndex % 4;
	int row = iIndex / 4;

	int iLeft = column * 64;
	int iTop = row * 64;
	int iRight = ( column + 1 ) * 64;
	int iBottom = ( row + 1 ) * 64;

	m_customCrosshairRect.left = iLeft;
	m_customCrosshairRect.top = iTop;
	m_customCrosshairRect.right = iRight;
	m_customCrosshairRect.bottom = iBottom;

	wrect_t area;
	area.left = iLeft;
	area.top = iTop;
	area.right = iRight;
	area.bottom = iBottom;

	int w = iRight - iLeft;
	int h = iBottom - iTop;

	HSPRITE sprite = m_hCustomCrosshair;
	gEngfuncs.pfnSPR_Set( sprite, 255, 255, 255 );

	int x = ( gHUD.m_scrinfo.iWidth / 2 ) - ( w / 2 );
	int y = ( gHUD.m_scrinfo.iHeight / 2 ) - ( h / 2 );

	gEngfuncs.pfnSPR_DrawHoles( 0, x, y, &area );
}

const unsigned int DISALLOW_DYNAMIC_XHAIR = ( 1 << WEAPON_HANDGRENADE ) | ( 1 << WEAPON_STICKGRENADE ) | 
									( 1 << WEAPON_STICKGRENADEX ) | ( 1 << WEAPON_HANDGRENADEX ) | 
									( 1 << WEAPON_BAZOOKA ) | ( 1 << WEAPON_PSCHRECK ) | ( 1 << WEAPON_PIAT );

int CHudDoDCrossHair::GetCrossHairWidth( void )
{
	int iIdealWidth;
	int iWeaponId;
	float accuracy;

	if( cl_dynamic_xhair && cl_dynamic_xhair->value < 1.0f )
		return 4;

	unsigned int CurrentWeaponId = gHUD.GetCurrentWeaponId();

	if( CurrentWeaponId > WEAPON_PIAT || ( ( WEAPON_AMERKNIFE << CurrentWeaponId ) & DISALLOW_DYNAMIC_XHAIR ) == 0 )
	{
		iWeaponId = CurrentWeaponId;

		float accuracy = GetCurrentWeaponAccuracy();
		iIdealWidth = ( int ) ( accuracy * 200.0f ) + 2;

		if( iIdealWidth < m_iLastXHairWidth )
		{
			if( iIdealWidth < ( m_iLastXHairWidth - 2 ) )
				m_iLastXHairWidth -= 3;
			else
				m_iLastXHairWidth -= 2;
		}
		else if( iIdealWidth > m_iLastXHairWidth )
		{
			if( iIdealWidth > ( m_iLastXHairWidth + 2 ) )
				m_iLastXHairWidth += 3;
			else
				m_iLastXHairWidth += 2;
		}
	}
	else
		m_iLastXHairWidth = 4;

	return m_iLastXHairWidth;
}

extern p_wpninfo_s WpnInfo[];

float CHudDoDCrossHair::GetCurrentWeaponAccuracy( void )
{
	int weaponId;
	float flSpread;

	weaponId = gHUD.GetCurrentWeaponId();
	flSpread = WpnInfo[weaponId].base_accuracy;

	if( gHUD.IsInMGDeploy() )
		flSpread = WpnInfo[weaponId].base_accuracy2;

	if( Length( gHUD.m_vecVelocity ) > 105.0f )
		return flSpread + WpnInfo[weaponId].accuracy_penalty;

	return flSpread;
}

extern int g_iWeaponBits2;

bool CHudDoDCrossHair::ShouldDrawCrossHair( void )
{
	int weaponId;
	bool bSniperRifle;

	if( ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) ) != 0 )
		return false;

	if( gHUD.m_Icons.m_iHealth <= 0 || g_iDeadFlag || g_iVuser1z || gHUD.GetMinimapState() == 1 )
		return false;

	weaponId = gHUD.GetCurrentWeaponId();

	if( weaponId == -1 || ( !gHUD.m_iWeaponBits && !g_iWeaponBits2 ) )
		return false;

	if( flBoltHideXHair > 0.0f || g_ihidexhair )
		return false;

	if( weaponId == WEAPON_AMERKNIFE || weaponId == WEAPON_GERKNIFE ||
		weaponId == WEAPON_SPADE || weaponId == WEAPON_MORTAR )
	{
		return false;
	}

	if( weaponId == WEAPON_FG42 && gHUD.m_iFOV <= 89 )
		return false;

	bool bIsMachineGun = ( weaponId == WEAPON_MG42 ||
		weaponId == WEAPON_CAL30 ||
		weaponId == WEAPON_MG34 );

	if( bIsMachineGun )
	{
		if( !gHUD.IsInMGDeploy() )
			return false;
	}

	bSniperRifle = false;

	if( weaponId == WEAPON_SPRING || weaponId == WEAPON_BINOC || weaponId == WEAPON_SCOPEDKAR )
	{
		bSniperRifle = true;
	}
	else if( weaponId == WEAPON_ENFIELD )
	{
		bSniperRifle = ( ( g_iWeaponFlags & 1 ) != 0 );
	}

	if( bSniperRifle )
	{
		if( IEngineStudio.IsHardware() && gHUD.m_iFOV <= 89 )
			return false;
	}

	return true;
}

void CHudDoDCrossHair::DrawSpectatorCrossHair( void )
{
	int fov;
	int w, h;
	HSPRITE sprite;
	wrect_t area;

	fov = gHUD.m_iFOV;

	if( fov > 89 )
	{
		m_crosshairRect.left = 24;
		m_crosshairRect.top = 0;
		m_crosshairRect.right = 48;
		m_crosshairRect.bottom = 24;

		area = m_crosshairRect;
		sprite = m_hCrosshair;
		gEngfuncs.pfnSPR_Set( sprite, 255, 255, 255 );

		w = area.right - area.left;
		h = area.bottom - area.top;

		int x = ( gHUD.m_scrinfo.iWidth / 2 ) - ( w / 2 );
		int y = ( gHUD.m_scrinfo.iHeight / 2 ) - ( h / 2 );

		gEngfuncs.pfnSPR_DrawHoles( 0, x, y, &area );
	}
}

void CHudDoDCrossHair::DrawClanTimer( float flTime )
{
	char buf[256];
	int time_remaining2, time_mins, time_secs;
	int x, y;
	int r, g, b;

	if( m_fClanTimer > gHUD.m_flTime )
	{
		float flTimeLeft = m_fClanTimer - gHUD.m_flTime;
		time_remaining2 = ( int ) flTimeLeft;

		time_mins = time_remaining2 / 60;
		time_secs = time_remaining2 - ( time_mins * 60 );

		y = gHUD.m_scrinfo.iHeight / 3;
		x = -1;

		if( time_secs > 1 || time_mins > 0 )
		{
			char mins[6];
			char secs[18];

			strcpy( buf, "#Clan_match_start" );

			r = 255;
			g = 255;
			b = 255;

			if( time_secs <= 5 )
			{
				r = 255; g = 255; b = 255;
			}
			else if( ( time_secs & 1 ) != 0 || time_secs > 20 )
			{
				r = 0; g = 0; b = 0;
			}

			sprintf( mins, "%d", time_mins );
			sprintf( secs, "%d", time_secs );

			float flColorTrigger = ( float ) r;

			gHUD.m_VGUI2Print.VGUI2HudPrintArgs( buf, mins, secs, NULL, NULL, x, y, 1.0f, flColorTrigger, flColorTrigger );
		}
		else
		{
			strcpy( buf, "#Clan_match_live" );

			gHUD.m_VGUI2Print.VGUI2HudPrintArgs( buf, NULL, NULL, NULL, NULL, x, y, 1.0f, 0.0f, 0.0f );
		}
	}
}
