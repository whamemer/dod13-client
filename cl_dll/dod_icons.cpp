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
// dod_icons.cpp
//
// implementation of the CHudDodIcons class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "in_defs.h"
#include "event_api.h"
#include "screenfade.h"
#include "shake.h"
#include "pmtrace.h"
#include "ev_dod.h"
#include "com_weapons.h"

#include "dod_shared.h"

extern int i_dodmusic;

DECLARE_MESSAGE( m_Icons, Health )
DECLARE_MESSAGE( m_Icons, Object )
DECLARE_MESSAGE( m_Icons, ClientAreas )
DECLARE_MESSAGE( m_Icons, ClCorpse )

extern client_textmessage_t g_pCustomMessage;

void Credits( void )
{
	gHUD.m_Icons.StartDrawingCredits();
}

int CHudDodIcons::Init( void )
{
	HOOK_MESSAGE( Health );
	HOOK_MESSAGE( Object );
	HOOK_MESSAGE( ClientAreas );
	HOOK_MESSAGE( ClCorpse );

	m_iFlags |= HUD_ACTIVE;

	for( int i = 0; i < 128; i++ )
	{
		m_Areas[i].nStatus = -1;
		m_Areas[i].hSpr = 0;
	}

	gHUD.AddHudElem( this );

	gEngfuncs.pfnAddCommand( "credits", Credits );

	return 1;
}

void CHudDodIcons::Reset( void )
{
	screenfade_t sf;

	memset( &sf, 0, sizeof( sf ) );

	m_iHealth = 100;
	m_hObjectSpr = 0;

	sf.fader = 0;
	sf.fadeg = 0;
	sf.fadeb = 0;
	sf.fadealpha = 0;
	sf.fadeFlags = FFADE_OUT | FFADE_STAYOUT;

	gEngfuncs.pfnSetScreenFade( &sf );

	m_iFlags |= HUD_ACTIVE;
	m_fLastMapMarkerTime = 0.0f;

	i_dodmusic = 0;
}

void CHudDodIcons::PlayerDied( void )
{
	m_iHealth = 0;

	if( !g_iVuser1z )
	{
		gHUD.SetFOV( 0 );
		g_lastFOV = 0.0f;
		gHUD.m_flMouseSensitivity = 0.0f;
	}

	m_hObjectSpr = 0;
}

int CHudDodIcons::VidInit( void )
{
	int HUD_MG_Deploy_icon = gHUD.GetSpriteIndex( "icon_mg_deploy" );
	IconMGDeploy = gHUD.GetSprite( HUD_MG_Deploy_icon );
	IconMGDeployArea = &gHUD.GetSpriteRect( HUD_MG_Deploy_icon );

	int HUD_main_20 = gHUD.GetSpriteIndex( "hud_main" );
	MainHUD = gHUD.GetSprite( HUD_main_20 );
	MainHUDArea = &gHUD.GetSpriteRect( HUD_main_20 );

	int HUD_health_overlay = gHUD.GetSpriteIndex( "hud_health_overlay" );
	HealthOverlayHUD = gHUD.GetSprite( HUD_health_overlay );
	HealthOverlayHUDArea = &gHUD.GetSpriteRect( HUD_health_overlay );

	int HUD_objectives_20 = gHUD.GetSpriteIndex( "hud_objectives" );
	ObjectivesHUD = gHUD.GetSprite( HUD_objectives_20 );
	ObjectivesHUDArea = &gHUD.GetSpriteRect( HUD_objectives_20 );

	int HUD_staminabar_20 = gHUD.GetSpriteIndex( "hud_staminabar" );
	StaminaBarHUD = gHUD.GetSprite( HUD_staminabar_20 );
	StaminaBarHUDArea = &gHUD.GetSpriteRect( HUD_staminabar_20 );

	int HUD_heart_20 = gHUD.GetSpriteIndex( "hud_heart" );
	HeartHUD = gHUD.GetSprite( HUD_heart_20 );
	i_HeartFrame = 0;
	f_HeartSpeed = 0.07f;
	HeartHUDArea = &gHUD.GetSpriteRect( HUD_heart_20 );
	f_HeartTime = gHUD.m_flTime;

	int HUD_healthbar_20 = gHUD.GetSpriteIndex( "hud_healthbar" );
	HealthBarHUD = gHUD.GetSprite( HUD_healthbar_20 );
	HealthBarHUDArea = &gHUD.GetSpriteRect( HUD_healthbar_20 );

	int HUD_reinforcements_20 = gHUD.GetSpriteIndex( "hud_reinforcements" );
	ReinforcementsHUD = gHUD.GetSprite( HUD_reinforcements_20 );
	ReinforcementsHUDArea = &gHUD.GetSpriteRect( HUD_reinforcements_20 );

	m_iIconHeight = IconMGDeployArea->bottom - IconMGDeployArea->top;
	m_iIconWidth = IconMGDeployArea->right - IconMGDeployArea->left;

	int mapmarker = gHUD.GetSpriteIndex( "hud_mapmarker" );
	MapMarkerSprite = gHUD.GetSprite( mapmarker );
	MapMarkerArea = &gHUD.GetSpriteRect( mapmarker );

	int brithead = gHUD.GetSpriteIndex( "hint_brithead" );
	int ushead = gHUD.GetSpriteIndex( "hint_ushead" );
	int gerhead = gHUD.GetSpriteIndex( "hint_gerhead" );

	m_hHintHeads[0] = gHUD.GetSprite( brithead );
	m_rectHintHeads[0] = &gHUD.GetSpriteRect( brithead );
	m_hHintHeads[1] = gHUD.GetSprite( ushead );
	m_rectHintHeads[1] = &gHUD.GetSpriteRect( ushead );
	m_hHintHeads[2] = gHUD.GetSprite( gerhead );
	m_rectHintHeads[2] = &gHUD.GetSpriteRect( gerhead );

	int horiz = gHUD.GetSpriteIndex( "hint_horiz" );
	int vert = gHUD.GetSpriteIndex( "hint_vert" );

	m_hHorizHintBacking = gHUD.GetSprite( horiz );
	m_rectHorizHintBacking = &gHUD.GetSpriteRect( horiz );
	m_hVertHintBacking = gHUD.GetSprite( vert );
	m_rectVertHintBacking = &gHUD.GetSpriteRect( vert );

	m_iMsgX = 100;
	m_iMsgY = 100;
	m_flHintDieTime = 0.0f;
	m_iCreditName = -1;
	m_flCreditChangeTime = -1.0f;

	for( int i = 0; i < 128; i++ )
	{
		m_Areas[i].nStatus = -1;
		m_Areas[i].hSpr = 0;
	}

	m_hsprSelectedMarker = 0;
	m_flDrawSelectedMarkerTime = 0.0f;

	return 1;
}

int CHudDodIcons::MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int health = READ_BYTE();

	m_iFlags |= HUD_ACTIVE;
	m_iHealth = health;

	return 1;
}

int CHudDodIcons::MsgFunc_Object( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_szObjectIcon = READ_STRING();

	if( *m_szObjectIcon )
	{
		m_hObjectSpr = SPR_Load( m_szObjectIcon );
		m_iFlags |= HUD_ACTIVE;
	}
	else
	{
		m_iFlags |= HUD_ACTIVE;
		m_hObjectSpr = 0;
	}

	return 1;
}

int CHudDodIcons::MsgFunc_ClientAreas( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int area_index = READ_BYTE();
	int status = READ_BYTE();

	if( area_index > 128 )
		return 1;

	if( status > 253 )
	{
		char *spritefile = READ_STRING();

		if( *spritefile )
		{
			m_Areas[area_index].hSpr = SPR_Load( spritefile );
			m_Areas[area_index].nStatus = 0;
		}

		return 1;
	}

	if( status == 0 )
	{
		m_Areas[area_index].nStatus = 0;
	}
	else if( status == 1 )
	{
		m_Areas[area_index].nStatus = 1;
	}
	else if( status == 2 )
	{
		for( int i = 0; i < 128; i++ )
			m_Areas[i].nStatus = 0;
	}

	return 1;
}

extern int g_iDeadFlag;

int CHudDodIcons::Draw( float flTime )
{
	DrawCredits( flTime );

	if( ( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL ) || g_iDeadFlag || g_iUser1 || g_iUser2 || g_iVuser1z )
		return 1;

	int r, g, b, x, y, a;
	wrect_t *area;
	HSPRITE sprite;
	wrect_t IconArea;

	UnpackRGB( r, g, b, 0xFFFFFF );
	a = 100;
	ScaleColors( r, g, b, a );

	UnpackRGB( r, g, b, 0xFFFFFF );

	if( m_hObjectSpr )
	{
		IconArea.left = 0;
		IconArea.right = 64;
		IconArea.top = 0;
		IconArea.bottom = 64;

		SPR_Set( m_hObjectSpr, r, g, b );

		x = m_iIconWidth;
		y = (int)( m_iIconHeight + ScreenHeight / 2 - m_iIconHeight * 1.5 );

		SPR_DrawAdditive( 0, x, y, &IconArea );
	}

	y = MainHUDArea->top + ScreenHeight - MainHUDArea->bottom;

	if( ShowHudElement( 1 ) )
	{
		wrect_t bar_area;
		int barHeight, hOverlay, diff;

		SPR_Set( MainHUD, r, g, b );
		SPR_DrawHoles( 0, 0, y, MainHUDArea );

		SPR_Set( StaminaBarHUD, r, g, b );

		bar_area = *StaminaBarHUDArea;
		barHeight = (int)( ( bar_area.bottom - bar_area.top ) * g_fStamina / 100.0f );
		bar_area.top = bar_area.bottom - barHeight;

		SPR_DrawHoles( 0, 8, ScreenHeight - barHeight, &bar_area );

		y = ScreenHeight + MainHUDArea->top - MainHUDArea->bottom;

		bar_area.top = HealthBarHUDArea->top;
		bar_area.left = HealthBarHUDArea->left;
		bar_area.right = HealthBarHUDArea->right;

		bar_area.bottom = bar_area.top + (int)( ( HealthBarHUDArea->bottom - bar_area.top ) * m_iHealth / 100.0f ) + 1;
		diff = HealthBarHUDArea->bottom - bar_area.bottom;

		SPR_Set( HealthBarHUD, r, g, b );
		SPR_DrawHoles( 0, 99, y + diff + 28, &bar_area );

		bar_area.top = HealthOverlayHUDArea->top;
		bar_area.left = HealthOverlayHUDArea->left;
		bar_area.right = HealthOverlayHUDArea->right;

		hOverlay = (int)( ( HealthOverlayHUDArea->bottom - bar_area.top ) * ( 1.0f - m_iHealth / 100.0f ) );
		bar_area.bottom = bar_area.top + hOverlay + 1;

		if( bar_area.top < bar_area.bottom )
		{
			SPR_Set( HealthOverlayHUD, r, g, b );
			SPR_DrawHoles( 0, 54, y + 27, &bar_area );
		}
	}

	y -= 64;

	if( g_iVuser1x > 0 )
	{
		sprite = IconMGDeploy;
		area = IconMGDeployArea;

		if( sprite && area )
		{
			SPR_Set( sprite, 220, 220, 220 );
			SPR_DrawAdditive( 0, 10, y + area->top - area->bottom, area );

			y = area->top + y - area->bottom;
		}
	}

	IconArea.left = 0;
	IconArea.right = 64;
	IconArea.top = 0;
	IconArea.bottom = 64;

	for( int i = 0; i < 128; i++ )
	{
		if( m_Areas[i].nStatus > 0 )
		{
			sprite = m_Areas[i].hSpr;

			if( sprite )
			{
				SPR_Set( sprite, r, g, b );
				SPR_DrawAdditive( 0, 10, y + IconArea.top - IconArea.bottom, &IconArea );
			}
		}
	}

	if( ShowHudElement( 2 ) )
	{
		SPR_Set( ReinforcementsHUD, r, g, b );
		SPR_DrawHoles( 0, 113, ScreenHeight + ReinforcementsHUDArea->top - ReinforcementsHUDArea->bottom, ReinforcementsHUDArea );
	}

	if( ShowHudElement( 6 ) )
	{
		float time = gEngfuncs.GetClientTime();

		if( m_fLastMapMarkerTime != 0.0f && m_fLastMapMarkerTime + 1.0f > time )
		{
			int a;
			int r = 255;
			int g = 255;
			int b = 255;
			float fade = 1.0f - ( time - m_fLastMapMarkerTime );

			a = (int)( fade * 255.0f );
			ScaleColors( r, g, b, a );

			int x = MapMarkerArea->left + ScreenWidth - XRES( 2 ) - MapMarkerArea->right;
			int y = ScreenHeight / 2 + 2 * ( MapMarkerArea->top - MapMarkerArea->bottom );

			SPR_Set( MapMarkerSprite, r, g, b );
			SPR_DrawAdditive( 0, x, y, MapMarkerArea );
		}
	}

	if( m_flDrawSelectedMarkerTime > gEngfuncs.GetClientTime() && m_hsprSelectedMarker )
	{
		int x = ScreenWidth;
		int y = ScreenHeight / 2;
		wrect_t rect;

		SPR_Set( m_hsprSelectedMarker, 255, 255, 255 );

		rect.left = 0;
		rect.right = 64;
		rect.top = 0;
		rect.bottom = 64;

		SPR_DrawHoles( 0, x - XRES( 2 ) - 96, y - 64, &rect );

		gHUD.m_VGUI2Print.DrawVGUI2StringReverse( "Dod_current_marker", ScreenWidth - XRES( 2 ), y, 1.0f, 1.0f, 1.0f );
	}

	return 1;
}

void CHudDodIcons::DrawMarkerIcon( HSPRITE pSpr )
{
	m_hsprSelectedMarker = pSpr;
	m_flDrawSelectedMarkerTime = gEngfuncs.GetClientTime() + 1.0f;
}

void ShowHideWeapons( cl_entity_t *player )
{
	int AnimBack = -1;
	int AnimNormal;
	int weaponId = gHUD.GetCurrentWeaponId();

	AnimNormal = ( weaponId == WEAPON_THOMPSON ) - 1;

	if( weaponId == WEAPON_THOMPSON )
		AnimBack = 6;

	if( player->curstate.iuser3 <= 0 )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( AnimNormal, 2 );
	}
	else if( gHUD.m_iKeyBits & IN_FORWARD )
	{
		HUD_GetWeaponAnim();

		gEngfuncs.pEventAPI->EV_WeaponAnimation( AnimBack, 2 );
	}
}

void CHudDodIcons::MapMarkerPosted( void )
{
	m_fLastMapMarkerTime = gEngfuncs.GetClientTime();
}

void CHudDodIcons::ActivateHintBacking( int team, float flTime )
{
	if( team == 2 )
		m_iHintTeam = 2;
	else
		m_iHintTeam = !gHUD.m_bBritish;

	m_flHintDieTime = flTime + gHUD.m_flTime;
}

void CHudDodIcons::GetHintMessageLocation( int &x, int &y )
{
	x = m_iMsgX;
	y = m_iMsgY;
}

char *szCreditNames[14] =
{
	"",
	"Matt Boone",
	"Tim Holt",
	"Jakob Jungels",
	"John Morello II",
	"Brandon Russell",
	"Travis Smith",
	"Kelly Thornton",
	"Special Thanks to",
	"All the mappers, artists and voice actors who worked on Day of Defeat",
	"Everyone at Valve Software",
	"Our patient and loving families",
	"The DoD community - many thanks for your continued support",
	"We couldn't have done it without you all!"
};

void CHudDodIcons::StartDrawingCredits( void )
{
	m_iCreditName = 0;
	m_flCreditChangeTime = gHUD.m_flTime;
}

void CHudDodIcons::DrawCredits( float flTime )
{
	if( m_iCreditName >= 0 && gHUD.m_flTime > m_flCreditChangeTime )
	{
		m_iCreditName++;

		if( m_iCreditName <= 13 )
		{
			g_pCustomMessage.pName = "asdf";
			g_pCustomMessage.pMessage = szCreditNames[m_iCreditName];
			g_pCustomMessage.effect = 1;
			g_pCustomMessage.r1 = 18;
			g_pCustomMessage.g1 = 115;
			g_pCustomMessage.b1 = 11;
			g_pCustomMessage.a1 = 255;
			g_pCustomMessage.r2 = 0;
			g_pCustomMessage.g2 = 60;
			g_pCustomMessage.b2 = 0;
			g_pCustomMessage.a2 = 255;
			g_pCustomMessage.x = gEngfuncs.pfnRandomFloat( 0.2f, 0.4f );
			g_pCustomMessage.y = gEngfuncs.pfnRandomFloat( 0.4f, 0.6f );
			g_pCustomMessage.fadein = 0.7f;
			g_pCustomMessage.fadeout = 0.7f;
			g_pCustomMessage.fxtime = 0.07f;
			g_pCustomMessage.holdtime = 3.0f;

			gHUD.m_Message.MessageAdd( &g_pCustomMessage );

			m_flCreditChangeTime = gHUD.m_flTime + 5.0f;
		}
		else
		{
			m_iCreditName = -1;
		}
	}
}

int CHudDodIcons::MsgFunc_ClCorpse( const char *pszName, int iSize, void *pbuf )
{
	char szModel[64];
	vec3_t vOrigin;
	vec3_t vAngles;
	float flAnimTime;
	int iSequence;
	int iBody;
	int iTeam;
	char *pModel;

	BEGIN_READ( pbuf, iSize );

	pModel = READ_STRING();

	vOrigin.x = READ_COORD();
	vOrigin.y = READ_COORD();
	vOrigin.z = READ_COORD();

	vAngles.x = READ_ANGLE();
	vAngles.y = READ_ANGLE();
	vAngles.z = READ_ANGLE();

	iSequence = READ_BYTE();
	iBody = READ_SHORT();
	iTeam = READ_BYTE();

	flAnimTime = 0.0f;

	safe_snprintf( szModel, sizeof( szModel ), "models/player/%s/%s.mdl", pModel, pModel );
	szModel[63] = 0;

	CreateCorpse( vOrigin, vAngles, szModel, flAnimTime, iSequence, iBody );

	return 1;
}
