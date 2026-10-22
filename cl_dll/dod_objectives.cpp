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
// dod_objectives.cpp
//
// implementation of the CObjectiveIcons class
//

#include <math.h>

#include "hud.h"
#include "hud_safearea.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "view.h"

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
	CAP_ICON_ALLIES_TRUCK,
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

static const char szCPIcons[NUM_CAP_ICONS][64] =
{
	"sprites/obj_icons/icon_obj_neutral.spr",
	"sprites/obj_icons/icon_obj_allies.spr",
	"sprites/obj_icons/icon_obj_axis.spr",
	"sprites/obj_icons/icon_obj_88_neutral.spr",
	"sprites/obj_icons/icon_obj_88_allies.spr",
	"sprites/obj_icons/icon_obj_88_axis.spr",
	"sprites/obj_icons/icon_obj_bridge_neutral.spr",
	"sprites/obj_icons/icon_obj_bridge_allies.spr",
	"sprites/obj_icons/icon_obj_bridge_axis.spr",
	"sprites/obj_icons/icon_obj_radio_neutral.spr",
	"sprites/obj_icons/icon_obj_radio_allies.spr",
	"sprites/obj_icons/icon_obj_radio_axis.spr",
	"sprites/obj_icons/icon_obj_docs_neutral.spr",
	"sprites/obj_icons/icon_obj_docs_allies.spr",
	"sprites/obj_icons/icon_obj_docs_axis.spr",
	"sprites/obj_icons/icon_obj_truck_neutral.spr",
	"sprites/obj_icons/icon_obj_truck_allies.spr",
	"sprites/obj_icons/icon_obj_truck_axis.spr",
	"sprites/obj_icons/%s/icon_obj_custom1_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom1_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom1_axis.spr",
	"sprites/obj_icons/%s/icon_obj_custom2_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom2_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom2_axis.spr",
	"sprites/obj_icons/%s/icon_obj_custom3_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom3_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom3_axis.spr",
	"sprites/obj_icons/icon_obj_brit.spr",
	"sprites/obj_icons/%s/icon_obj_custom4_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom4_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom4_axis.spr",
	"sprites/obj_icons/%s/icon_obj_custom5_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom5_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom5_axis.spr",
	"sprites/obj_icons/%s/icon_obj_custom6_neutral.spr",
	"sprites/obj_icons/%s/icon_obj_custom6_allies.spr",
	"sprites/obj_icons/%s/icon_obj_custom6_axis.spr"
};

void CreatePickingRay( float fov, int mousex, int mousey, int screenwidth, int screenheight, const Vector &vecRenderOrigin, const Vector &vecRenderAngles, Vector &vecPickingRay )
{
	float dx, dy;
	float c_x, c_y;
	float dist;
	vec3_t vpn, vup, vright;

	c_x = (float)( screenwidth / 2 );
	c_y = (float)( screenheight / 2 );

	dx = (float)mousex - c_x;
	dy = c_y - (float)mousey;

	dist = c_x / (float)tan( M_PI * fov / 360.0 );

	AngleVectors( vecRenderAngles, vpn, vright, vup );

	vecPickingRay = vright * dx + vpn * dist + vup * dy;

	VectorNormalize( vecPickingRay );
}

void ScreenToWorld( int x, int y, Vector &pick )
{
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	if( player )
		CreatePickingRay( (float)gHUD.m_iFOV, x, y, ScreenWidth, ScreenHeight, player->origin, player->angles, pick );
}

// the ids of the icons that the server sent for the points and the map they are for,
// the sprites are loaded again when the video mode changes
static int s_iIconID[12][3];
static char s_szIconLevel[256];

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

	m_iconarea.left = 0;
	m_iconarea.right = 32;
	m_iconarea.top = 0;
	m_iconarea.bottom = 32;

	m_bWarmupMode = false;

	return 1;
}

extern bool b_StopDemo;

int CObjectiveIcons::VidInit( void )
{
	int HUD_icon_timer[11];
	char buf[256];
	int i;
	int HUD_timer_20;

	for( i = 0; i < 10; i++ )
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

	// It forgets the points here, but the server sends them only when the player joins
	// (InitHUD clears them now) and the icons were gone after a change of the resolution
	if( !b_StopDemo )
	{
		for( i = 0; i < 12; i++ )
		{
			if( !m_eControlPoints[i].valid )
				continue;

			for( int j = 0; j < 3; j++ )
			{
				if( s_iIconID[i][j] >= 0 && s_iIconID[i][j] < NUM_CAP_ICONS )
				{
					sprintf( buf, szCPIcons[s_iIconID[i][j]], s_szIconLevel );

					m_eControlPoints[i].m_iIcons[j] = LoadSprite( buf );
				}
			}
		}
	}

	m_nTimerStatus = -1;
	m_fTimerSeconds = 0.0f;

	SetWaveTime( 0.0f );

	return 1;
}

void CObjectiveIcons::UpdateObjectiveIcons( void )
{
	for( int i = 0; i < 12; i++ )
	{
		if( m_eControlPoints[i].valid && m_eControlPoints[i].totaltime > 0.0f )
		{
			m_eControlPoints[i].animtime += gHUD.m_flTimeDelta;

			if( m_eControlPoints[i].animtime > m_eControlPoints[i].totaltime )
			{
				m_eControlPoints[i].totaltime = 0.0f;
				m_eControlPoints[i].animtime = 0.0f;
				m_eControlPoints[i].owner = m_eControlPoints[i].nextOwner;
			}
		}
	}
}

int CObjectiveIcons::MsgFunc_InitObj( const char *pszName, int iSize, void *pbuf )
{
	char iconName[256];
	char levelname[256];
	const char *levelnameLong;
	int length;
	char *p;
	int num;

	if( gEngfuncs.IsSpectateOnly() && m_eControlPoints[0].valid )
		return 1;

	// "maps/<name>.bsp" -> "<name>"
	levelnameLong = gEngfuncs.pfnGetLevelName();
	length = strlen( levelnameLong ) - 4;

	strncpy( levelname, levelnameLong, length );
	levelname[length] = 0;

	p = &levelname[5];
	memmove( levelname, p, strlen( p ) + 1 );

	ClearAllCapPoints();

	strlcpy( s_szIconLevel, levelname, sizeof( s_szIconLevel ) );

	for( int p = 0; p < 12; p++ )
		s_iIconID[p][0] = s_iIconID[p][1] = s_iIconID[p][2] = -1;

	BEGIN_READ( pbuf, iSize );

	num = READ_BYTE();

	for( int i = 0; i < num; i++ )
	{
		int entindex = READ_SHORT();
		int point = READ_BYTE();

		ChangeCapPoint( point, READ_BYTE(), 0 );
		SetVisible( point, READ_BYTE() );

		for( int j = 0; j < 3; j++ )
		{
			int id = READ_BYTE();

			if( id <= NUM_CAP_ICONS - 1 )
			{
				s_iIconID[point][j] = id;

				sprintf( iconName, szCPIcons[id], levelname );

				m_eControlPoints[point].m_iIcons[j] = LoadSprite( iconName );

				m_eControlPoints[point].m_rAreas[j].left = 0;
				m_eControlPoints[point].m_rAreas[j].right = 32;
				m_eControlPoints[point].m_rAreas[j].top = 0;
				m_eControlPoints[point].m_rAreas[j].bottom = 32;
			}
		}

		m_eControlPoints[point].m_rOrigin.x = READ_COORD();
		m_eControlPoints[point].m_rOrigin.y = READ_COORD();
		m_eControlPoints[point].m_rOrigin.z = 1.0f;

		m_eControlPoints[point].entindex = entindex;
	}

	return 1;
}

int CObjectiveIcons::MsgFunc_SetObj( const char *pszName, int iSize, void *pbuf )
{
	int point;
	int newowner;
	int timedCap;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	newowner = READ_BYTE();
	timedCap = READ_BYTE();

	ChangeCapPoint( point, newowner, timedCap );

	return 1;
}

int CObjectiveIcons::MsgFunc_StartProg( const char *pszName, int iSize, void *pbuf )
{
	int point;
	int newOwner;
	int capTime;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	newOwner = READ_BYTE();
	capTime = READ_BYTE();

	if( IsPointValid( point ) )
		StartCapProgress( point, newOwner, (float)capTime );

	return 1;
}

int CObjectiveIcons::MsgFunc_StartProgF( const char *pszName, int iSize, void *pbuf )
{
	int point;
	int newOwner;
	float capTime;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	newOwner = READ_BYTE();
	capTime = READ_COORD();

	if( IsPointValid( point ) )
		StartCapProgress( point, newOwner, capTime );

	return 1;
}

int CObjectiveIcons::MsgFunc_ProgUpdate( const char *pszName, int iSize, void *pbuf )
{
	int point;
	float time;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	time = READ_COORD();

	// rescale the progress to the new total time
	m_eControlPoints[point].animtime = m_eControlPoints[point].animtime / m_eControlPoints[point].totaltime * time;
	m_eControlPoints[point].totaltime = time;

	return 1;
}

int CObjectiveIcons::MsgFunc_CancelProg( const char *pszName, int iSize, void *pbuf )
{
	int point;
	int owner;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	owner = READ_BYTE();

	CancelCapProgress( point, owner );

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
	int point;
	int team;
	int num;
	int required;

	BEGIN_READ( pbuf, iSize );

	point = READ_BYTE();
	team = READ_BYTE();
	num = READ_BYTE();
	required = READ_BYTE();

	SetNumPlayersInArea( point, team, num, required );

	return 1;
}

void CObjectiveIcons::ClearAllCapPoints( void )
{
	for( int i = 0; i < 12; i++ )
	{
		m_eControlPoints[i].valid = false;
		m_eControlPoints[i].owner = -1;
		m_eControlPoints[i].nextOwner = -1;
		m_eControlPoints[i].animtime = 0.0f;
		m_eControlPoints[i].numplayers = 0;
		m_eControlPoints[i].requiredplayers = 0;
		m_eControlPoints[i].occupyingteam = 0;
		m_eControlPoints[i].visible = 1;

		for( int j = 0; j < 3; j++ )
		{
			m_eControlPoints[i].m_iIcons[j] = 0;
			m_eControlPoints[i].m_rAreas[j].left = 0;
			m_eControlPoints[i].m_rAreas[j].right = 0;
			m_eControlPoints[i].m_rAreas[j].top = 0;
			m_eControlPoints[i].m_rAreas[j].bottom = 0;
		}
	}
}

void CObjectiveIcons::ChangeCapPoint( int point, int newowner, int timedCap )
{
	m_eControlPoints[point].valid = true;

	if( timedCap > 0 && m_eControlPoints[point].owner < 0 )
	{
		m_eControlPoints[point].owner = newowner;
		m_eControlPoints[point].nextOwner = -1;
		m_eControlPoints[point].animtime = 0.0f;
		m_eControlPoints[point].totaltime = 0.0f;
	}
	else if( newowner != m_eControlPoints[point].nextOwner )
	{
		m_eControlPoints[point].totaltime = 0.5f;
		m_eControlPoints[point].animtime = 0.0f;
		m_eControlPoints[point].nextOwner = newowner;
	}
}

void CObjectiveIcons::SetVisible( int point, int visible )
{
	m_eControlPoints[point].visible = visible;
}

void CObjectiveIcons::StartCapProgress( int point, int newOwner, float time )
{
	if( m_eControlPoints[point].valid )
	{
		m_eControlPoints[point].animtime = 0.0f;
		m_eControlPoints[point].nextOwner = newOwner;
		m_eControlPoints[point].totaltime = time;
	}
}

void CObjectiveIcons::CancelCapProgress( int point, int owner )
{
	if( m_eControlPoints[point].valid )
	{
		m_eControlPoints[point].nextOwner = -1;
		m_eControlPoints[point].animtime = 0.0f;
		m_eControlPoints[point].totaltime = 0.0f;
		m_eControlPoints[point].owner = owner;
	}
}

void CObjectiveIcons::SetNumPlayersInArea( int point, int team, int num, int required )
{
	if( m_eControlPoints[point].valid )
	{
		m_eControlPoints[point].occupyingteam = team;
		m_eControlPoints[point].numplayers = num;
		m_eControlPoints[point].requiredplayers = required;
	}
}

void CObjectiveIcons::Think( void )
{
	if( m_nTimerStatus == 1 )
		m_fTimerSeconds -= gHUD.m_flTimeDelta;

	if( GetWaveTime() + 1.0f - gHUD.m_flTime < -2.0f )
		SetWaveStatus( 0 );
}

extern int g_iDeadFlag;

int i_iObjBottomY;

int CObjectiveIcons::Draw( float flTime )
{
	bool displayHide;
	int x;

	UpdateObjectiveIcons();

	if( g_iVuser1z )
		return 1;

	displayHide = false;

	if( g_iUser1 )
		displayHide = gHUD.m_Spectator.m_pip->value != 0.0f;

	x = 10;

	if( m_bWarmupMode )
	{
		// at the left edge, in the middle of the screen
		CSafeAnchor anchor( SAFE_LEFT );

		gHUD.m_VGUI2Print.DrawVGUI2String( "clan_warmup_mode", XRES( 10 ), ScreenHeight / 2 - YRES( 32 ), 1.0f, 1.0f, 1.0f );
	}

	// the round timer
	if( ShowHudElement( 4 ) && m_nTimerStatus >= 0 )
	{
		int y;
		int minutes, seconds;

		if( g_iUser1 )
			y = YRES( 54 );
		else
			y = 2;

		gEngfuncs.pfnSPR_Set( TimerHUD, 255, 255, 255 );
		gEngfuncs.pfnSPR_DrawHoles( 0, 0, y, TimerHUDArea );

		minutes = (int)m_fTimerSeconds / 60;
		seconds = (int)m_fTimerSeconds % 60;

		if( minutes < 0 )
			minutes = 0;
		if( minutes > 99 )
			minutes = 99;
		if( seconds < 0 )
			seconds = 0;

		DrawDigit( minutes / 10, 10, y + 6 );
		DrawDigit( minutes % 10, 24, y + 6 );
		DrawDigit( seconds / 10, 44, y + 6 );
		DrawDigit( seconds % 10, 58, y + 6 );

		x = 82;
	}

	// the objective icons
	if( ShowHudElement( 5 ) )
	{
		int y;
		int iconW;
		int mapState;

		if( displayHide )
			x = XRES( gHUD.m_Spectator.m_OverviewData.insetWindowX ) + XRES( gHUD.m_Spectator.m_OverviewData.insetWindowWidth ) + XRES( 2 );

		if( g_iUser1 )
			y = YRES( 54 );
		else
			y = YRES( 2 );

		iconW = m_iconarea.right - m_iconarea.left + 2;
		mapState = gHUD.GetMinimapState();

		// the row is at the top left corner, over the big map the icons sit on its control points in the middle of the screen
		CSafeAnchor iconsAnchor( mapState == 1 ? SAFE_NONE : SAFE_LEFT | SAFE_TOP );

		for( int i = 0; i < 12; i++ )
		{
			HSPRITE icon = 0;
			wrect_t area = { 0, 0, 0, 0 };
			HSPRITE newIcon = 0;
			wrect_t newArea = { 0, 0, 0, 0 };
			int owner, nextOwner;
			int x0;
			int height;

			if( !m_eControlPoints[i].visible )
				continue;

			owner = m_eControlPoints[i].owner;
			nextOwner = m_eControlPoints[i].nextOwner;

			if( owner < 0 || owner > 2 )
				continue;

			icon = m_eControlPoints[i].m_iIcons[owner];
			area = m_eControlPoints[i].m_rAreas[owner];

			if( m_eControlPoints[i].totaltime > 0.0f )
			{
				newIcon = m_eControlPoints[i].m_iIcons[nextOwner];
				newArea = m_eControlPoints[i].m_rAreas[nextOwner];
			}

			if( mapState == 1 )
			{
				// the icons sit over the control points of the minimap
				x = m_eControlPoints[i].m_iMapXPos;
				y = m_eControlPoints[i].m_iMapYPos;
			}
			else if( icon && mapState == 2 )
			{
				// the icons are drawn by the overview
				cl_entity_t *ent = gEngfuncs.GetEntityByIndex( m_eControlPoints[i].entindex );

				if( ent )
				{
					ent->baseline.iuser1 = 1;

					gHUD.m_Spectator.AddOverviewEntityToMap( icon, ent, gEngfuncs.GetClientTime() - 1.0, m_eControlPoints[i].m_rOrigin );
				}
			}

			x0 = x;

			// the point is changing its owner, slide the new icon in
			if( newIcon )
			{
				int width = newArea.right - newArea.left;
				float percent = m_eControlPoints[i].animtime / m_eControlPoints[i].totaltime;
				int delta = (int)( percent * width );

				newArea.right = newArea.left + delta;
				area.left = area.right - width + delta;

				x0 = x + delta;

				if( newArea.left < newArea.right )
				{
					gEngfuncs.pfnSPR_Set( newIcon, 255, 255, 255 );
					gEngfuncs.pfnSPR_DrawHoles( 0, x, y - 1, &newArea );
				}
			}

			height = area.bottom - area.top;

			if( icon )
			{
				gEngfuncs.pfnSPR_Set( icon, 255, 255, 255 );
				gEngfuncs.pfnSPR_DrawHoles( 0, x0, y - 1, &area );

				i_iObjBottomY = y - 1 + area.top + area.bottom;
			}

			// players in the capture area
			if( m_eControlPoints[i].numplayers > 0 && m_eControlPoints[i].requiredplayers > 0 )
			{
				char buf[16];
				int r, g, b;

				sprintf( buf, "%d/%d", m_eControlPoints[i].numplayers, m_eControlPoints[i].requiredplayers );

				if( m_eControlPoints[i].occupyingteam == 1 )
				{
					r = 13;
					g = 110;
					b = 13;
				}
				else
				{
					r = 174;
					g = 11;
					b = 11;
				}

				gHUD.DrawHudString( x + 4, y + height + 4, 1000, buf, r, g, b );
			}

			x += iconW;
		}
	}

	// the reinforcement timer
	if( !( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL ) && !g_iDeadFlag && ShowHudElement( 2 ) )
	{
		float wavetime;
		int y;
		int minutes, seconds;

		// at the bottom left corner
		CSafeAnchor anchor( SAFE_LEFT | SAFE_BOTTOM );

		wavetime = GetWaveTime() + 1.0f - gHUD.m_flTime;

		y = ScreenHeight - 31;

		minutes = 0;
		seconds = 0;

		if( wavetime >= 0.0f )
		{
			minutes = (int)wavetime / 60;
			seconds = (int)wavetime % 60;

			if( minutes > 99 )
				minutes = 99;
		}

		DrawDigit( minutes / 10, 138, y );
		DrawDigit( minutes % 10, 152, y );
		DrawDigit( seconds / 10, 170, y );
		DrawDigit( seconds % 10, 184, y );
	}

	return 1;
}

void CObjectiveIcons::DrawDigit( int digit, int x, int y )
{
	wrect_t *area;

	if( (unsigned int)digit > 9 )
		digit = 10;

	area = m_TimerAreas[digit];

	gEngfuncs.pfnSPR_Set( m_TimerIcons[digit], 255, 255, 255 );
	gEngfuncs.pfnSPR_DrawHoles( 0, x, y, area );
}

void CObjectiveIcons::CalcIconLocations( void )
{
	for( int i = 0; i < 12; i++ )
	{
		vec3_t screen, org;

		if( !m_eControlPoints[i].valid )
			continue;

		org = m_eControlPoints[i].m_rOrigin;

		if( gHUD.m_Spectator.m_OverviewData.rotated )
		{
			vec3_t newOrigin;

			newOrigin.x = org.y - gHUD.m_Spectator.m_OverviewData.origin[1] + gHUD.m_Spectator.m_OverviewData.origin[0];
			newOrigin.y = gHUD.m_Spectator.m_OverviewData.origin[1] - ( org.x - gHUD.m_Spectator.m_OverviewData.origin[0] );
			newOrigin.z = org.z;

			org = newOrigin;
		}

		int result = gEngfuncs.pTriAPI->WorldToScreen( org, screen );

		if( result == 1 )
		{
			// behind the camera
			m_eControlPoints[i].m_iMapXPos = -100;
			m_eControlPoints[i].m_iMapYPos = -100;
		}
		else
		{
			int mapx, mapy, mapw, maph;
			float xratio, yratio;

			org.x = ( screen.x + 1.0f ) * ScreenWidth * 0.5f;
			org.y = ( 1.0f - screen.y ) * ScreenHeight * 0.5f;
			org.z = 0.0f;

			gHUD.GetMapBounds( mapx, mapy, mapw, maph );

			xratio = org.x / ScreenWidth;
			yratio = org.y / ScreenHeight;

			m_eControlPoints[i].m_iMapXPos = mapx + (int)( mapw * xratio ) - ( m_eControlPoints[i].m_rAreas[0].right - m_eControlPoints[i].m_rAreas[0].left ) / 2;
			m_eControlPoints[i].m_iMapYPos = mapy + (int)( maph * yratio ) - ( m_eControlPoints[i].m_rAreas[0].bottom - m_eControlPoints[i].m_rAreas[0].top ) / 2;
		}
	}
}

extern "C" void DLLEXPORT HUD_ChatInputPosition( int *x, int *y )
{
	int height;

	// the chat input sits under the timer
	if( !g_iUser1 && !gEngfuncs.IsSpectateOnly() )
	{
		*y += gHUD.m_ObjectiveIcons.TimerHUDArea->bottom - gHUD.m_ObjectiveIcons.TimerHUDArea->top + 6;
		return;
	}

	if( gHUD.m_Spectator.m_pip->value != 0.0f )
	{
		height = gHUD.m_Spectator.m_OverviewData.insetWindowHeight + 5;
		*y = YRES( height );
		return;
	}

	if( !ShowHudElement( 5 ) )
	{
		*y = YRES( 54 );
		return;
	}

	// under the objective icons
	*y = i_iObjBottomY + 4;
}
