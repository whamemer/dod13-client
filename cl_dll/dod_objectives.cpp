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
//  dod_objectives.cpp - implementation of the CObjectiveIcons class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "dod_shared.h"
#include "pi_constant.h"

extern "C"
{
	void DLLEXPORT HUD_ChatInputPosition( int *x, int *y );
}

DECLARE_MESSAGE( m_ObjectiveIcons, InitObj )
DECLARE_MESSAGE( m_ObjectiveIcons, SetObj )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProg )
DECLARE_MESSAGE( m_ObjectiveIcons, StartProgF )
DECLARE_MESSAGE( m_ObjectiveIcons, ProgUpdate )
DECLARE_MESSAGE( m_ObjectiveIcons, CancelProg )
DECLARE_MESSAGE( m_ObjectiveIcons, TimerStatus )
DECLARE_MESSAGE( m_ObjectiveIcons, PlayersIn )

enum CP_Icons 
{
	CAP_ICON_NEUTRAL_FLAG = 0,
	CAP_ICON_ALLIES_FLAG,
	CAP_ICON_AXIS_FLAG,
	CAP_ICON_NEUTRAL_88,
	CAP_ICON_ALLIES_88,
	CAP_ICON_AXIS_88,
	CAP_ICON_NEUTRAL_BRIDGE,
	CAP_ICON_ALLIES_BRIDGE,
	CAP_ICON_AXIS_BRIDGE,
	CAP_ICON_NEUTRAL_RADIO,
	CAP_ICON_ALLIES_RADIO,
	CAP_ICON_AXIS_RADIO,
	CAP_ICON_NEUTRAL_DOCS,
	CAP_ICON_ALLIES_DOCS,
	CAP_ICON_AXIS_DOCS,
	CAP_ICON_NEUTRAL_TRUCK,
	CAP_ICON_ALLIES_TRUCK ,
	CAP_ICON_AXIS_TRUCK,
	CAP_ICON_NEUTRAL_CUSTOM1,
	CAP_ICON_ALLIES_CUSTOM1,
	CAP_ICON_AXIS_CUSTOM1,
	CAP_ICON_NEUTRAL_CUSTOM2,
	CAP_ICON_ALLIES_CUSTOM2,
	CAP_ICON_AXIS_CUSTOM2,
	CAP_ICON_NEUTRAL_CUSTOM3,
	CAP_ICON_ALLIES_CUSTOM3,
	CAP_ICON_AXIS_CUSTOM3,
	CAP_ICON_BRIT_FLAG,
	CAP_ICON_NEUTRAL_CUSTOM4,
	CAP_ICON_ALLIES_CUSTOM4,
	CAP_ICON_AXIS_CUSTOM4,
	CAP_ICON_NEUTRAL_CUSTOM5,
	CAP_ICON_ALLIES_CUSTOM5,
	CAP_ICON_AXIS_CUSTOM5,
	CAP_ICON_NEUTRAL_CUSTOM6,
	CAP_ICON_ALLIES_CUSTOM6,
	CAP_ICON_AXIS_CUSTOM6,
	NUM_CAP_ICONS
};

static const char szCPIcons[NUM_CAP_ICONS][64];

void CreatePickingRay( float fov, int mousex, int mousey, int screenwidth, int screenheight, 
	const vec3_t &vecRenderOrigin, const vec3_t &vecRenderAngles, vec3_t &vecPickingRay )
{
	float dx, dy, c_x, c_y, dist;
	vec3_t vpn, vup, vright;

	c_x = ( float ) screenwidth / 2.0f;
	c_y = ( float ) screenheight / 2.0f;

	dx = ( float ) mousex - c_x;
	dy = c_y - ( float ) mousey;

	dist = c_x / tan( M_PI * fov / 360.0f );

	AngleVectors( vecRenderAngles, vpn, vright, vup );

	vecPickingRay.x = dx * vright.x + dist * vpn.x + dy * vup.x;
	vecPickingRay.y = dx * vright.y + dist * vpn.y + dy * vup.y;
	vecPickingRay.z = dx * vright.z + dist * vpn.z + dy * vup.z;

	VectorNormalize( vecPickingRay );
}

void ScreenToWorld( int x, int y, vec3_t &pick )
{
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	if( player )
		CreatePickingRay( gHUD.m_iFOV, x, y, ScreenWidth, ScreenHeight, player->origin, player->angles, pick );
}

int CObjectiveIcons::Init( void )
{
	HOOK_MESSAGE( InitObj );
	HOOK_MESSAGE( SetObj );
	HOOK_MESSAGE( StartProg );
	HOOK_MESSAGE( StartProgF );
	HOOK_MESSAGE( ProgUpdate );
	HOOK_MESSAGE( CancelProg );
	HOOK_MESSAGE( TimerStatus );
	HOOK_MESSAGE( PlayersIn );

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	m_iconarea.top = 0;
	m_iconarea.left = 0;
	m_iconarea.bottom = 32;
	m_iconarea.right = 32;
	m_bWarmupMode = false;
	return 1;
}

extern bool b_StopDemo;

int CObjectiveIcons::VidInit( void )
{
	int HUD_icon_timer[11];
	char buf[28];
	int HUD_timer_20;

	for( int i = 0; i < 10; i++ )
	{
		sprintf( buf, "timer_num_%d", i );
		HUD_icon_timer[i] = gHUD.GetSpriteIndex( buf );

		m_TimerIcons[i] = gHUD.GetSprite( HUD_icon_timer[i] );
		m_TimerAreas[i] = &gHUD.GetSpriteRect( HUD_icon_timer[i] );
	}

	HUD_icon_timer[10] = gHUD.GetSpriteIndex( "timer_num_blank" );
	m_TimerIcons[10] = gHUD.GetSprite( HUD_icon_timer[10] );
	m_TimerAreas[10] = &gHUD.GetSpriteRect( HUD_icon_timer[10] );

	HUD_timer_20 = gHUD.GetSpriteIndex( "hud_timer" );
	TimerHUD = gHUD.GetSprite( HUD_timer_20 );
	TimerHUDArea = &gHUD.GetSpriteRect( HUD_timer_20 );

	if( !b_StopDemo )
		ClearAllCapPoints();

	m_nTimerStatus = -1;
	m_fTimerSeconds = 0.0f;

	SetWaveTime( 0.0f );

	return 1;
}

void CObjectiveIcons::UpdateObjectiveIcons( void )
{
	float flTimeDelta = gHUD.m_flTimeDelta;

	for( int i = 0; i < 12; i++ )
	{
		if( m_eControlPoints[i].valid )
		{
			float flTotalTime = m_eControlPoints[i].totaltime;

			if( flTotalTime > 0.0f )
			{
				m_eControlPoints[i].animtime += flTimeDelta;

				if( m_eControlPoints[i].animtime > flTotalTime )
				{
					m_eControlPoints[i].owner = m_eControlPoints[i].nextOwner;
					m_eControlPoints[i].totaltime = 0.0f;
					m_eControlPoints[i].animtime = 0.0f;
				}
			}
		}
	}
}

int CObjectiveIcons::MsgFunc_InitObj( const char *pszName, int iSize, void *pbuf )
{
	char iconName[256];
	char levelname[284];
	const char *levelnameLong;
	int length;

	if( !gEngfuncs.IsSpectateOnly() || !m_eControlPoints[0].valid )
	{
		levelnameLong = gEngfuncs.pfnGetLevelName();

		if( levelnameLong && *levelnameLong )
		{
			length = strlen( levelnameLong ) - 4;
			strncpy( levelname, levelnameLong, length );
			levelname[length] = 0;

			if( strstr( levelname, "maps/" ) != NULL )
			{
				char *p = levelname + 5;
				memmove( levelname, p, strlen( p ) + 1 );
			}
		}

		ClearAllCapPoints();

		BEGIN_READ( pbuf, iSize );

		int num = READ_BYTE();

		if( num > 0 )
		{
			for( int i = 0; i < num; i++ )
			{
				int entindex = READ_SHORT();
				int point = READ_BYTE();
				int newowner = READ_BYTE();

				m_eControlPoints[point].valid = true;
				m_eControlPoints[point].entindex = entindex;

				ChangeCapPoint( point, newowner, 0 );

				m_eControlPoints[point].requiredplayers = READ_BYTE();

				for( int j = 0; j < 3; j++ )
				{
					int id = READ_BYTE();

					if( id < NUM_CAP_ICONS )
					{
						sprintf( iconName, szCPIcons[id], levelname );

						m_eControlPoints[point].m_iIcons[j] = gHUD.GetSpriteIndex( iconName );
						m_eControlPoints[point].m_rAreas[j] = gHUD.GetSpriteRect( m_eControlPoints[point].m_iIcons[j] );
					}
				}

				m_eControlPoints[point].m_rOrigin.x = READ_COORD();
				m_eControlPoints[point].m_rOrigin.y = READ_COORD();
				m_eControlPoints[point].m_rOrigin.z = READ_COORD();

				SetVisible( point, 1 );
			}
		}
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_SetObj( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	int newowner = READ_BYTE();
	int timedCap = READ_BYTE();

	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	pPoint->visible = 1;

	if( timedCap > 0 && pPoint->owner < 0 )
	{
		pPoint->owner = newowner;
		pPoint->nextOwner = -1;
		pPoint->totaltime = 0.0f;
		pPoint->animtime = 0.0f;
	}
	else
	{
		if( newowner != pPoint->owner )
		{
			pPoint->animtime = 0.5f;
			pPoint->totaltime = 0.0f;
			pPoint->owner = newowner;
		}
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_StartProg( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	int newOwner = READ_BYTE();
	float time = READ_BYTE();

	if( point < 0 || point >= 12 )
		return 1;

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->nextOwner = newOwner;
		pPoint->totaltime = 0.0f;
		pPoint->animtime = time;
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_StartProgF( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	int newOwner = READ_BYTE();
	float time = READ_COORD();

	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->nextOwner = newOwner;
		pPoint->totaltime = 0.0f;
		pPoint->animtime = time;
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_ProgUpdate( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	float newTime = READ_COORD();

	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->animtime > 0.0f )
		pPoint->totaltime = ( pPoint->totaltime / pPoint->animtime ) * newTime;

	pPoint->animtime = newTime;

	return 1;
}

int CObjectiveIcons::MsgFunc_CancelProg( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	int owner = READ_BYTE();

	if( point < 0 || point >= 12 )
		return 1;

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->owner = owner;
		pPoint->nextOwner = -1;
		pPoint->totaltime = 0.0f;
		pPoint->animtime = 0.0f;
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_TimerStatus( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_nTimerStatus = READ_BYTE();
	m_fTimerSeconds = READ_COORD();

	return 1;
}

int CObjectiveIcons::MsgFunc_PlayersIn( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int point = READ_BYTE();
	int team = READ_BYTE();
	int numplayers = READ_BYTE();
	int required = READ_BYTE();

	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->occupyingteam = team;
		pPoint->numplayers = numplayers;
		pPoint->requiredplayers = required;
	}

	return 1;
}

void CObjectiveIcons::ClearAllCapPoints( void )
{
	for( int i = 0; i < 12; i++ )
	{
		control_point_t *pPoint = &m_eControlPoints[i];

		pPoint->valid = 0;
		pPoint->visible = 1;
		pPoint->owner = -1;
		pPoint->nextOwner = -1;
		pPoint->animtime = 0.0f;
		pPoint->totaltime = 0.0f;
		pPoint->numplayers = 0;
		pPoint->requiredplayers = 0;
		pPoint->occupyingteam = 0;

		for( int j = 0; j < 3; j++ )
		{
			pPoint->m_iIcons[j] = 0;

			pPoint->m_rAreas[j].left = 0;
			pPoint->m_rAreas[j].top = 0;
			pPoint->m_rAreas[j].right = 0;
			pPoint->m_rAreas[j].bottom = 0;
		}
	}
}

void CObjectiveIcons::ChangeCapPoint( int point, int newowner, int timedCap )
{
	if( point < 0 || point >= 12 )
		return;

	control_point_t *pPoint = &m_eControlPoints[point];

	pPoint->visible = 1;

	if( timedCap > 0 && pPoint->owner < 0 )
	{
		pPoint->owner = newowner;
		pPoint->nextOwner = -1;
		pPoint->totaltime = 0.0f;
		pPoint->animtime = 0.0f;
	}
	else
	{
		if( pPoint->owner != newowner )
		{
			pPoint->animtime = 0.5f;
			pPoint->numplayers = 0;
			pPoint->owner = newowner;
		}
	}
}

void CObjectiveIcons::SetVisible( int point, int visible )
{
	m_eControlPoints[point].visible = visible;
}

void CObjectiveIcons::StartCapProgress( int point, int newOwner, float time )
{
	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->numplayers = 0;
		pPoint->occupyingteam = newOwner;
		pPoint->totaltime = time;
	}
}

void CObjectiveIcons::CancelCapProgress( int point, int owner )
{
	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->occupyingteam = -1;
		pPoint->numplayers = 0;
		pPoint->totaltime = 0.0f;
		pPoint->owner = owner;
	}
}

void CObjectiveIcons::SetNumPlayersInArea( int point, int team, int numplayers, int required )
{
	IsPointValid( point );

	control_point_t *pPoint = &m_eControlPoints[point];

	if( pPoint->visible )
	{
		pPoint->occupyingteam = team;
		pPoint->numplayers = numplayers;
		pPoint->requiredplayers = required;
	}
}

void CObjectiveIcons::Think( void )
{
	if( m_nTimerStatus == 1 )
		m_fTimerSeconds -= gHUD.m_flTimeDelta;

	float flWaveTimeLeft = GetWaveTime() + 1.0f - gHUD.m_flTime;

	if( flWaveTimeLeft < -2.0f )
		SetWaveStatus( 0 );
}

extern bool ShowHudElement( int i_hudElement );
extern int g_iDeadFlag;
int i_iObjBottomY;

int CObjectiveIcons::Draw( float flTime )
{
	UpdateObjectiveIcons();

	bool displayHide = ( gHUD.m_iHideHUDDisplay & 4 ) != 0;

	if( g_iVuser1z )
		return 1;

	int bPip = 0;

	if( g_iUser1 )
		bPip = ( gHUD.m_Spectator.m_pip->value != 0.0f );

	if( m_bWarmupMode )
	{
		int height = ScreenHeight;
		int warmupY = ( height / 2 ) - ( ( height / 480.0f * 32.0f ) + 0.5f );
		int warmupX = ( ScreenWidth / 640.0f * 10.0f ) + 0.5f;

		gHUD.m_VGUI2Print.DrawVGUI2String( "clan_warmup_mode", warmupX, warmupY, 1.0f, 1.0f, 1.0f );
	}

	int x = 10;

	if( ShowHudElement( 4 ) && m_nTimerStatus >= 0 )
	{
		int y, digitY;
		if( g_iUser1 )
		{
			y = ( gHUD.m_scrinfo.iHeight / 480.0f * 54.0f ) + 0.5f;
			digitY = y + 6;
		}
		else
		{
			digitY = 8;
			y = 2;
		}

		gEngfuncs.pfnSPR_Set( TimerHUD, 255, 255, 255 );
		gEngfuncs.pfnSPR_DrawHoles( 0, 0, y, TimerHUDArea );

		int minutes = ( int ) m_fTimerSeconds / 60;
		int seconds = ( int ) m_fTimerSeconds % 60;

		if( minutes < 0 ) minutes = 0;
		if( minutes > 99 ) minutes = 99;

		DrawDigit( minutes / 10, 10, digitY );
		DrawDigit( minutes % 10, 24, digitY );
		DrawDigit( seconds / 10, 44, digitY );
		DrawDigit( seconds % 10, 58, digitY );

		x = 82;
	}

	if( ShowHudElement( 5 ) )
	{
		if( bPip )
		{
			float scaleX = ScreenWidth / 640.0f;
			int pipLeft = ( gHUD.m_Spectator.m_OverviewData.insetWindowX * scaleX ) + 0.5f;
			int pipWidth = ( gHUD.m_Spectator.m_OverviewData.insetWindowWidth * scaleX ) + 0.5f;
			x = pipLeft + pipWidth + ( ( scaleX + scaleX ) + 0.5f );
		}

		float scaleY = ScreenHeight / 480.0f;
		int y = ( g_iUser1 ? ( scaleY * 54.0f ) : ( scaleY + scaleY ) ) + 0.5f;

		int iconW = m_iconarea.right - m_iconarea.left + 2;
		int mapState = gHUD.GetMinimapState();

		for( int i = 0; i < 12; i++ )
		{
			control_point_t *pPoint = &m_eControlPoints[i];

			if( !pPoint->visible )
				continue;

			int owner = pPoint->owner;
			int nextOwner = pPoint->nextOwner;

			if( owner > 2 )
				continue;

			HSPRITE icon = pPoint->m_iIcons[owner];
			wrect_t area = pPoint->m_rAreas[owner];

			int x0 = x;

			if( pPoint->totaltime > 0.0f )
			{
				HSPRITE newIcon = pPoint->m_iIcons[nextOwner];
				wrect_t newArea = pPoint->m_rAreas[nextOwner];

				if( mapState == 1 )
				{
					x0 = pPoint->m_iMapXPos;
					y = pPoint->m_iMapYPos;
				}

				if( newIcon )
				{
					int width = newArea.right - newArea.left;
					float percent = pPoint->animtime / pPoint->totaltime;
					int delta = percent * width;

					newArea.right = newArea.left + delta;

					gEngfuncs.pfnSPR_Set( newIcon, 255, 255, 255 );
					gEngfuncs.pfnSPR_DrawHoles( 0, x0, y - 1, &newArea );

					x0 += delta;
				}
			}
			else
			{
				if( mapState == 1 )
				{
					x0 = pPoint->m_iMapXPos;
					y = pPoint->m_iMapYPos;
				}
			}

			if( icon && mapState == 1 )
			{
				gEngfuncs.pfnSPR_Set( icon, 255, 255, 255 );
				gEngfuncs.pfnSPR_DrawHoles( 0, x0, y - 1, &area );
			}
			else if( icon && mapState == 2 )
			{
				cl_entity_t *ent = gEngfuncs.GetEntityByIndex( pPoint->entindex );
				if( ent )
				{
					ent->baseline.iuser1 = 1;
					Vector p_origin = pPoint->m_rOrigin;
					float clientTime = gEngfuncs.GetClientTime();
					gHUD.m_Spectator.AddOverviewEntityToMap( icon, ent, clientTime - 1.0f, &p_origin );
				}
			}

			if( pPoint->numplayers > 0 && pPoint->requiredplayers > 0 )
			{
				char buf[16];
				sprintf( buf, "%d/%d", pPoint->numplayers, pPoint->requiredplayers );

				int r = ( pPoint->occupyingteam == 1 ) ? 13 : 174;
				int g = ( pPoint->occupyingteam == 1 ) ? 110 : 11;
				int b = 11;

				int textFlags = ( pPoint->occupyingteam == 1 ) ? 13 : 11;
				gHUD.DrawHudString( x0 + 4, y + 20, 1000, buf, r, g, textFlags );
			}

			x += iconW;
		}
	}

	if( !displayHide && !g_iDeadFlag && ShowHudElement( 2 ) )
	{
		int y = ScreenHeight - 31;
		float wavetime = GetWaveTime() + 1.0f - gHUD.m_flTime;

		if( wavetime >= 0.0f )
		{
			int minutes = ( int ) wavetime / 60;
			int seconds = ( int ) wavetime % 60;

			if( minutes > 99 ) minutes = 99;

			DrawDigit( minutes / 10, 138, y );
			DrawDigit( minutes % 10, 152, y );
			DrawDigit( seconds / 10, 170, y );
			DrawDigit( seconds % 10, 184, y );
		}
	}

	return 1;
}

void CObjectiveIcons::DrawDigit( int digit, int x, int y )
{
	wrect_t *pArea;

	if( digit > 9 )
	{
		gEngfuncs.pfnSPR_Set( m_TimerIcons[10], 255, 255, 255 );
		pArea = m_TimerAreas[10];
	}
	else
	{
		gEngfuncs.pfnSPR_Set( m_TimerIcons[digit], 255, 255, 255 );
		pArea = m_TimerAreas[digit];
	}

	gEngfuncs.pfnSPR_DrawHoles( 0, x, y, pArea );
}

void CObjectiveIcons::CalcIconLocations( void )
{
	vec3_t org, screen;
	int mapx, mapy, mapw, maph;

	for( int i = 0; i < 12; i++ )
	{
		control_point_t *pPoint = &m_eControlPoints[i];

		if( !pPoint->valid )
			continue;

		org = pPoint->m_rOrigin;

		if( gHUD.m_Spectator.m_OverviewData.rotated )
		{
			vec3_t newOrigin;
			float centerX = gHUD.m_Spectator.m_OverviewData.origin.x;
			float centerY = gHUD.m_Spectator.m_OverviewData.origin.y;

			newOrigin.x = pPoint->m_rOrigin.y - centerY + centerX;
			newOrigin.y = centerY - ( pPoint->m_rOrigin.x - centerX );
			newOrigin.z = pPoint->m_rOrigin.z;

			org = newOrigin;
		}

		if( gEngfuncs.pTriAPI->WorldToScreen( org, screen ) == 1 )
		{
			float xratio = ( screen.x + 1.0f ) * 0.5f;
			float yratio = ( 1.0f - screen.y ) * 0.5f;

			gHUD.GetMapBounds( mapx, mapy, mapw, maph );

			int iconWidth = pPoint->m_rAreas[0].right - pPoint->m_rAreas[0].left;
			int iconHeight = pPoint->m_rAreas[0].bottom - pPoint->m_rAreas[0].top;

			pPoint->m_iMapXPos = mapx + ( int ) ( mapw * xratio ) - ( iconWidth / 2 );
			pPoint->m_iMapYPos = mapy + ( int ) ( maph * yratio ) - ( iconHeight / 2 );
		}
		else
		{
			pPoint->m_iMapXPos = -100;
			pPoint->m_iMapYPos = -100;
		}
	}
}

void DLLEXPORT HUD_ChatInputPosition( int *x, int *y )
{
	if( !g_iUser1 && !gEngfuncs.IsSpectateOnly() )
	{
		int timerHeight = gHUD.m_ObjectiveIcons.TimerHUDArea->bottom - gHUD.m_ObjectiveIcons.TimerHUDArea->top;
		*y += timerHeight + 6;
		return;
	}

	if( gHUD.m_Spectator.m_pip->value != 0.0f )
	{
		float scaleY = ScreenHeight / 480.0f;
		float calculatedY = ( gHUD.m_Spectator.m_OverviewData.insetWindowHeight + 5 ) * scaleY;
		*y = ( int ) ( calculatedY + 0.5f );
		return;
	}

	if( !ShowHudElement( 5 ) )
	{
		float scaleY = ScreenHeight / 480.0f;
		float calculatedY = scaleY * 54.0f;
		*y = ( int ) ( calculatedY + 0.5f );
		return;
	}

	*y = i_iObjBottomY + 4;
}