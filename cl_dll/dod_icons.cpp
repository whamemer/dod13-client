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
//  dod_icons.cpp - implementation of the CHudDodIcons class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "dod_shared.h"
#include "screenfade.h"
#include "shake.h"
#include "event_api.h"

float animTimer;
extern int i_dodmusic;

DECLARE_MESSAGE( m_Icons, Health )
DECLARE_MESSAGE( m_Icons, Object )
DECLARE_MESSAGE( m_Icons, ClientAreas )
DECLARE_MESSAGE( m_Icons, ClCorpse )

void __CmdFunc_Credits( void )
{
	gHUD.m_Icons.m_iCreditName = 0;
	gHUD.m_Icons.m_flCreditChangeTime = gHUD.m_flTime;
}

int CHudDodIcons::Init( void )
{
	HOOK_MESSAGE( Health );
	HOOK_MESSAGE( Object );
	HOOK_MESSAGE( ClientAreas );
	HOOK_MESSAGE( ClCorpse );

	m_iFlags |= HUD_ACTIVE;
	memset( m_Areas, 0, sizeof( m_Areas ) );
	gHUD.AddHudElem( this );

	HOOK_COMMAND( "credits", Credits )

	return 1;
}

void CHudDodIcons::Reset( void )
{
	screenfade_t sf;

	sf.fader = 0;
	sf.fadeg = 0;
	sf.fadeb = 0;
	sf.fadealpha = 0;
	sf.fadeFlags = FFADE_OUT | FFADE_STAYOUT;

	gEngfuncs.pfnSetScreenFade( &sf );

	m_iHealth = 100;
	m_hObjectSpr = 0;
	m_iFlags |= HUD_ACTIVE;
	m_fLastMapMarkerTime = 0.0f;
	i_dodmusic = 0;
}

extern float g_lastFOV;

void CHudDodIcons::PlayerDied( void )
{
	m_iHealth = 0;

	if( g_iVuser1z == 0 )
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
	HeartHUDArea = &gHUD.GetSpriteRect( HUD_heart_20 );
	i_HeartFrame = 0;
	f_HeartSpeed = 0.07f;
	f_HeartTime = gHUD.m_flTime;

	int HUD_healthbar_20 = gHUD.GetSpriteIndex( "hud_healthbar" );
	HealthBarHUD = gHUD.GetSprite( HUD_healthbar_20 );
	HealthBarHUDArea = &gHUD.GetSpriteRect( HUD_healthbar_20 );

	int HUD_reinforcements_20 = gHUD.GetSpriteIndex( "hud_reinforcements" );
	ReinforcementsHUD = gHUD.GetSprite( HUD_reinforcements_20 );
	ReinforcementsHUDArea = &gHUD.GetSpriteRect( HUD_reinforcements_20 );

	if( IconMGDeployArea )
	{
		m_iIconHeight = IconMGDeployArea->bottom - IconMGDeployArea->top;
		m_iIconWidth = IconMGDeployArea->right - IconMGDeployArea->left;
	}

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

	memset( m_Areas, 0, sizeof( m_Areas ) );

	m_hsprSelectedMarker = 0;
	m_flDrawSelectedMarkerTime = 0.0f;

	return 1;
}

int CHudDodIcons::MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iFlags |= HUD_ACTIVE;
	m_iHealth = READ_BYTE();
	return 1;
}

int CHudDodIcons::MsgFunc_Object( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_szObjectIcon = READ_STRING();
	m_iFlags |= HUD_ACTIVE;

	if( m_szObjectIcon && *m_szObjectIcon )
		m_hObjectSpr = gEngfuncs.pfnSPR_Load( m_szObjectIcon );
	else
		m_hObjectSpr = 0;

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
		if( spritefile && *spritefile )
		{
			m_Areas[area_index].hSpr = gEngfuncs.pfnSPR_Load( spritefile );
			m_Areas[area_index].nStatus = 0;
		}
		return 1;
	}

	if( status != 0 )
	{
		if( status == 1 )
		{
			m_Areas[area_index].nStatus = 1;
		}
		else if( status == 2 )
		{
			for( int i = 0; i < 128; ++i )
			{
				m_Areas[i].nStatus = 0;
			}
		}
		return 1;
	}

	m_Areas[area_index].nStatus = 0;
	return 1;
}

extern int g_iDeadFlag;
extern float g_fStamina;

extern bool ShowHudElement( int i_hudElement );

int CHudDodIcons::Draw( float flTime )
{
	wrect_t IconArea, bar_area, rect;
	int barHeight, hOverlay, diff;

	wrect_t *area;
	HSPRITE sprite;

	float time;
	int a, b;
	float fade;
	int x, y;

	DrawCredits( flTime );

	if( ( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL ) == 0 && !g_iDeadFlag && !g_iUser1 && !g_iUser2 && !g_iVuser1z )
	{
		int g_color = 255;
		unsigned long ulRGBColor = 0xFFFFFFFF;
		UnpackRGB( a, g_color, b, ulRGBColor );

		if( m_hObjectSpr )
		{
			IconArea.top = 0;
			IconArea.left = 0;
			IconArea.bottom = 64;
			IconArea.right = 64;

			gEngfuncs.pfnSPR_Set( m_hObjectSpr, 255, 255, 255 );

			float y_pos = ( ( float ) m_iIconHeight + ( float ) ScreenHeight / 2.0f ) - ( float ) m_iIconHeight * 1.5f;
			gEngfuncs.pfnSPR_DrawAdditive( 0, m_iIconWidth, ( int ) y_pos, &IconArea );
		}

		int y_hud_base = MainHUDArea->top + ScreenHeight - MainHUDArea->bottom;

		if( ShowHudElement( 1 ) )
		{
			gEngfuncs.pfnSPR_Set( MainHUD, 255, 255, 255 );
			gEngfuncs.pfnSPR_DrawHoles( 0, 0, y_hud_base, MainHUDArea );

			gEngfuncs.pfnSPR_Set( StaminaBarHUD, 255, 255, 255 );
			bar_area = *StaminaBarHUDArea;
			barHeight = ( int ) ( ( float ) ( bar_area.bottom - bar_area.top ) * g_fStamina / 100.0f );
			int y_stamina_top = ScreenHeight - barHeight;
			bar_area.top = bar_area.bottom - barHeight;
			gEngfuncs.pfnSPR_DrawHoles( 0, 8, y_stamina_top, &bar_area );

			y_hud_base = ScreenHeight + MainHUDArea->top - MainHUDArea->bottom;
			bar_area.top = HealthBarHUDArea->top;
			bar_area.left = HealthBarHUDArea->left;
			bar_area.right = HealthBarHUDArea->right;

			hOverlay = ( int ) ( ( float ) ( HealthBarHUDArea->bottom - bar_area.top ) * ( float ) m_iHealth / 100.0f );
			diff = ( HealthBarHUDArea->bottom - bar_area.top ) - hOverlay - 1;
			bar_area.bottom = bar_area.top + hOverlay + 1;

			gEngfuncs.pfnSPR_Set( HealthBarHUD, 255, 255, 255 );
			gEngfuncs.pfnSPR_DrawHoles( 0, 99, y_hud_base + diff + 28, &bar_area );

			bar_area.top = HealthOverlayHUDArea->top;
			bar_area.left = HealthOverlayHUDArea->left;
			bar_area.right = HealthOverlayHUDArea->right;

			float overlay_h = ( float ) ( HealthOverlayHUDArea->bottom - bar_area.top ) * ( 1.0f - ( float ) m_iHealth / 100.0f );
			bar_area.bottom = bar_area.top + ( int ) overlay_h + 1;

			if( bar_area.top < bar_area.bottom )
			{
				gEngfuncs.pfnSPR_Set( HealthOverlayHUD, 255, 255, 255 );
				gEngfuncs.pfnSPR_DrawHoles( 0, 54, y_hud_base + 27, &bar_area );
			}
		}

		int y_icon_stack = y_hud_base - 64;

		if( g_iVuser1x > 0 && IconMGDeploy && IconMGDeployArea )
		{
			gEngfuncs.pfnSPR_Set( IconMGDeploy, 220, 220, 220 );
			gEngfuncs.pfnSPR_DrawAdditive( 0, 10, y_icon_stack + IconMGDeployArea->top - IconMGDeployArea->bottom, IconMGDeployArea );
			y_icon_stack = IconMGDeployArea->top + y_icon_stack - IconMGDeployArea->bottom;
		}

		for( int i = 0; i < 128; i++ )
		{
			if( m_Areas[i].nStatus > 0 )
			{
				sprite = m_Areas[i].hSpr;
				if( sprite )
				{
					area = &gHUD.GetSpriteRect( gHUD.GetSpriteIndex( "hud_main" ) );

					if( area )
					{
						gEngfuncs.pfnSPR_Set( sprite, 255, 255, 255 );
						gEngfuncs.pfnSPR_DrawAdditive( 0, 10, y_icon_stack + area->top - area->bottom, area );
					}
				}
			}
		}

		if( ShowHudElement( 2 ) )
		{
			gEngfuncs.pfnSPR_Set( ReinforcementsHUD, 255, 255, 255 );
			int y_reinf = ScreenHeight + ReinforcementsHUDArea->top - ReinforcementsHUDArea->bottom;
			gEngfuncs.pfnSPR_DrawHoles( 0, 113, y_reinf, ReinforcementsHUDArea );
		}

		if( ShowHudElement( 6 ) )
		{
			time = gEngfuncs.GetClientTime();
			if( m_fLastMapMarkerTime != 0.0f && ( m_fLastMapMarkerTime + 1.0f ) > time )
			{
				a = 255;
				b = 255;
				int b_marker = 255;
				fade = ( 1.0f - ( time - m_fLastMapMarkerTime ) ) * 255.0f;

				unsigned long ulFadeRGB = 0xFFFFFFFF;
				UnpackRGB( a, b, b_marker, ulFadeRGB );

				float x_scale = ( float ) ScreenWidth / 640.0f + ( float ) ScreenWidth / 640.0f + 0.5f;
				x = MapMarkerArea->left + ScreenWidth - ( int ) x_scale - MapMarkerArea->right;
				y = ScreenHeight / 2 + 2 * ( MapMarkerArea->top - MapMarkerArea->bottom );

				gEngfuncs.pfnSPR_Set( MapMarkerSprite, a, b, b_marker );
				gEngfuncs.pfnSPR_DrawAdditive( 0, x, y, MapMarkerArea );
			}
		}

		time = m_flDrawSelectedMarkerTime;
		if( time > gEngfuncs.GetClientTime() && m_hsprSelectedMarker )
		{
			gEngfuncs.pfnSPR_Set( m_hsprSelectedMarker, 255, 255, 255 );

			rect.left = 0;
			rect.right = 64;
			rect.top = 0;
			rect.bottom = 64;

			float x_scale = ( float ) ScreenWidth / 640.0f + ( float ) ScreenWidth / 640.0f + 0.5f;
			b = ( int ) x_scale;
			x = ScreenWidth - b - 96;
			y = ScreenHeight / 2 - 64;

			gEngfuncs.pfnSPR_DrawHoles( 0, x, y, &rect );

			gHUD.m_VGUI2Print.DrawVGUI2StringReverse( "Dod_current_marker", ScreenWidth - b, ScreenHeight / 2, 1.0f, 1.0f, 1.0f );
		}
	}

	return 1;
}

void CHudDodIcons::DrawMarkerIcon( HSPRITE pSpr )
{
	m_hsprSelectedMarker = pSpr;
	m_flDrawSelectedMarkerTime = gEngfuncs.GetClientTime() + 1.0f;
}

extern int HUD_GetWeaponAnim();

void ShowHideWeapons( cl_entity_t *player )
{
	if( !player )
		return;

	int iAnimIndex = -1;
	int iCurrentWeaponId = gHUD.GetCurrentWeaponId();

	if( iCurrentWeaponId == WEAPON_THOMPSON )
	{
		iAnimIndex = THOMPSON_PRONE_BACK;
	}

	if( player->curstate.iuser3 <= 0 )
	{
		int iBodyParam = ( iCurrentWeaponId == WEAPON_THOMPSON ) ? 0 : -1;

		gEngfuncs.pEventAPI->EV_WeaponAnimation( iBodyParam, THOMPSON_DRAW );
	}
	else if( ( gHUD.m_iKeyBits & IN_ATTACK ) != 0 )
	{
		HUD_GetWeaponAnim();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnimIndex, 2 );
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

char *szCreditNames[14];

void CHudDodIcons::StartDrawingCredits( void )
{
	m_iCreditName = 0;
	m_flCreditChangeTime = gHUD.m_flTime;
}

extern client_textmessage_t	g_pCustomMessage;

void CHudDodIcons::DrawCredits( float flTime )
{
	if( m_iCreditName >= 0 && gHUD.m_flTime > m_flCreditChangeTime )
	{
		int iNextCredit = m_iCreditName + 1;
		m_iCreditName = iNextCredit;

		if( iNextCredit <= 13 )
		{
			g_pCustomMessage.pName = "asdf";
			g_pCustomMessage.pMessage = szCreditNames[iNextCredit];

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

extern void CreateCorpse( vec3_t vOrigin, vec3_t vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody );

int CHudDodIcons::MsgFunc_ClCorpse( const char *pszName, int iSize, void *pbuf )
{
	vec3_t vOrigin, vAngles;
	float flAnimTime;
	int iSequence, iBody, iTeam, iIndex;
	char *pModel;

	BEGIN_READ( pbuf, iSize );

	pModel = READ_STRING();

	vOrigin.x = READ_COORD();
	vOrigin.y = READ_COORD();
	vOrigin.z = READ_COORD();

	vAngles.x = READ_ANGLE();
	vAngles.y = READ_ANGLE();
	vAngles.z = READ_ANGLE();

	iTeam = READ_BYTE();
	iIndex = READ_SHORT();

	iSequence = READ_BYTE();
	flAnimTime = 0.0f;
	iBody = 0;

	CreateCorpse( vOrigin, vAngles, pModel, flAnimTime, iTeam, iIndex );

	return 1;
}