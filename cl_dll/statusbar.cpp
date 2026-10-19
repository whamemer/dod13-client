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
// statusbar.cpp
//
// generic text status bar, set by game dll
// runs across bottom of screen
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "event_api.h"
#include "entity_types.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "hltv.h"
#include "pm_shared.h"
#include "vgui/vgui_TeamFortressViewport.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_StatusBar, StatusValue )

float *GetClientColor( int clientIndex );

extern cvar_t *cl_identiconmode;
extern float g_lastFOV;
extern vec3_t v_angles;

int CHudStatusBar::Init( void )
{
	gHUD.AddHudElem( this );

	m_iFlags |= HUD_ACTIVE;

	HOOK_MESSAGE( StatusValue );

	Reset();

	m_flNextUpdateTime = 0.0f;

	hud_centerid = gEngfuncs.pfnRegisterVariable( "hud_centerid", "1", FCVAR_ARCHIVE );

	for( int i = 0; i < MAX_PLAYERS + 1; i++ )
		m_TargetTalking[i] = 0;

	return 1;
}

int CHudStatusBar::VidInit( void )
{
	m_flNextUpdateTime = 0.0f;

	Reset();

	m_StatusBarAllieModel = SPR_Load( "sprites/american.spr" );
	m_StatusBarBritishModel = SPR_Load( "sprites/british.spr" );
	m_StatusBarGermanModel = SPR_Load( "sprites/german.spr" );

	return 1;
}

void CHudStatusBar::Reset( void )
{
	m_iHealth = 100;
	m_iTargetIndex = -1;
	m_iTargetTeam = 0;
}

bool CHudStatusBar::InDeathCamMode( void )
{
	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( g_iUser2 );

	if( (unsigned int)( g_iDeadFlag - 1 ) <= 2 && g_iUser3 > 0 && gEngfuncs.GetMaxClients() >= g_iUser3 )
	{
		if( pEnt && g_iUser1 == OBS_CHASE_FREE )
			return pEnt->index == gEngfuncs.GetLocalPlayer()->index;
	}

	return false;
}

void CHudStatusBar::Think( void )
{
	if( m_flNextUpdateTime > gEngfuncs.GetClientTime() )
		return;

	if( gViewPort )
		gViewPort->UpdateSpectatorPanel();

	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( g_iUser2 );
	bool bFound = false;

	if( (unsigned int)( g_iDeadFlag - 1 ) <= 2 && g_iUser3 > 0 && gEngfuncs.GetMaxClients() >= g_iUser3 &&
		pEnt && g_iUser1 == OBS_CHASE_FREE && pEnt->index == gEngfuncs.GetLocalPlayer()->index )
	{
		// the death cam looks at the killer
		cl_entity_t *pKiller = gEngfuncs.GetEntityByIndex( g_iUser3 );

		if( pKiller && pKiller->index != gEngfuncs.GetLocalPlayer()->index && (unsigned int)( pKiller->index - 1 ) < MAX_PLAYERS )
		{
			m_iTargetIndex = pKiller->index;
			m_iTargetTeam = g_PlayerExtraInfo[pKiller->index].teamnumber;
			bFound = true;
		}
	}
	else
	{
		cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();
		vec3_t viewheight, forward, right, up, vecStart, vecEnd;
		pmtrace_t tr;

		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( viewheight );

		vecStart = Vector( pLocal->curstate.origin ) + viewheight;

		AngleVectors( v_angles, forward, right, up );

		vecEnd = vecStart + forward * 2048.0f;

		gEngfuncs.pEventAPI->EV_SetSolidPlayers( pLocal->index - 1 );
		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecStart, vecEnd, PM_STUDIO_BOX, -1, &tr );

		int iHit = gEngfuncs.pEventAPI->EV_IndexFromTrace( &tr );

		if( iHit > 0 && iHit <= gEngfuncs.GetMaxClients() )
		{
			cl_entity_t *pTarget = gEngfuncs.GetEntityByIndex( iHit );

			pLocal = gEngfuncs.GetLocalPlayer();

			gEngfuncs.pfnGetPlayerInfo( pTarget->index, &g_PlayerInfoList[pTarget->index] );
			gEngfuncs.pfnGetPlayerInfo( pLocal->index, &g_PlayerInfoList[pLocal->index] );

			int iLocalTeam = g_PlayerExtraInfo[pLocal->index].teamnumber;

			// the team mates only, the spectators and the dead see everyone
			if( g_PlayerExtraInfo[pTarget->index].teamnumber == iLocalTeam || iLocalTeam == 3 || g_iDeadFlag > 1 )
			{
				m_iTargetIndex = pTarget->index;
				m_iTargetTeam = g_PlayerExtraInfo[pTarget->index].teamnumber;
				bFound = true;
			}
		}
	}

	if( !bFound )
	{
		m_iTargetIndex = -1;
		m_iTargetTeam = 0;
	}

	m_flNextUpdateTime = gEngfuncs.GetClientTime() + 0.1f;
}

int CHudStatusBar::Draw( float fTime )
{
	if( !g_iUser1 && g_iAlive && (unsigned int)( m_iTargetIndex - 1 ) < MAX_PLAYERS )
	{
		hud_player_info_t info;
		char s[128];
		int width, height;
		int x, y;

		gEngfuncs.pfnGetPlayerInfo( m_iTargetIndex, &info );

		int iHealth = m_iHealth;
		float *color = GetClientColor( m_iTargetIndex );

		if( iHealth > 0 )
		{
			sprintf( s, "%s (%d)", info.name, iHealth );

			gEngfuncs.pfnDrawConsoleStringLen( s, &width, &height );

			// let user set status ID bar centering
			if( hud_centerid && hud_centerid->value > 0.0f )
			{
				x = Q_max( 2, ScreenWidth - width ) / 2;
				y = (int)( hud_centerid->value * height + ( height + ScreenHeight / 2 ) );
			}
			else
			{
				x = ScreenWidth / 2 - width / 2;
				y = ScreenHeight - 2 * height;
			}

			gEngfuncs.pfnDrawSetTextColor( color[0], color[1], color[2] );
			gEngfuncs.pfnDrawConsoleString( x, y, s );
		}
	}

	return 1;
}

// Message handler for StatusValue message
// accepts one value:
//		byte: the health of the target
int CHudStatusBar::MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iHealth = READ_BYTE();

	if( g_iUser1 && gViewPort )
		gViewPort->UpdateSpectatorPanel();

	return 1;
}

char *CHudStatusBar::GetTargetName( void )
{
	static hud_player_info_t info;

	if( (unsigned int)( m_iTargetIndex - 1 ) < MAX_PLAYERS )
	{
		gEngfuncs.pfnGetPlayerInfo( m_iTargetIndex, &info );
		return info.name;
	}

	return "";
}

void CHudStatusBar::UpdateWhosTalking( int entindex, qboolean bTalking )
{
	if( (unsigned int)entindex <= MAX_PLAYERS )
		m_TargetTalking[entindex] = bTalking;
}

void CHudStatusBar::CreateEntities( void )
{
	if( m_StatusBarAllieModel && m_StatusBarBritishModel && m_StatusBarGermanModel && !g_iUser1 && !( g_iEffects & EF_NODRAW ) && !g_iDeadFlag )
	{
		if( cl_identiconmode->value >= 2.0f )
			DrawEntitiesOverTeam();
		else if( cl_identiconmode->value >= 1.0f )
			DrawEntitiesOverTarget();
	}
}

// the icon of the team over the heads of all the team mates
void CHudStatusBar::DrawEntitiesOverTeam( void )
{
	cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();

	for( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( i );

		if( !pEnt )
			continue;

		if( pEnt->curstate.messagenum < pLocal->curstate.messagenum || ( pEnt->curstate.effects & EF_NODRAW ) || pEnt == pLocal || m_TargetTalking[i] )
			continue;

		int team = pEnt->curstate.team;

		if( team != pLocal->curstate.team || !pEnt->curstate.solid || g_PlayerExtraInfo[i].dead )
			continue;

		if( team == 1 )
			m_StatusBarHeadModel = gHUD.m_bBritish ? m_StatusBarBritishModel : m_StatusBarAllieModel;
		else
			m_StatusBarHeadModel = m_StatusBarGermanModel;

		memset( &m_TargetHeadModel[i], 0, sizeof( m_TargetHeadModel[i] ) );

		m_TargetHeadModel[i].curstate.rendermode = kRenderTransAdd;
		m_TargetHeadModel[i].curstate.renderamt = 127;
		m_TargetHeadModel[i].curstate.frame = 0.0f;
		m_TargetHeadModel[i].baseline.renderamt = 127;
		m_TargetHeadModel[i].curstate.renderfx = kRenderFxNoDissipation;
		m_TargetHeadModel[i].curstate.framerate = 1.0f;
		m_TargetHeadModel[i].model = (model_s *)gEngfuncs.GetSpritePointer( m_StatusBarHeadModel );
		m_TargetHeadModel[i].angles = Vector( 0.0f, 0.0f, 0.0f );
		m_TargetHeadModel[i].origin = Vector( 0.0f, 0.0f, 45.0f );
		m_TargetHeadModel[i].origin = Vector( pEnt->origin ) + m_TargetHeadModel[i].origin;

		float flDistance = ( m_TargetHeadModel[i].origin - Vector( pLocal->origin ) ).Length();

		// the icon grows with the distance, it stays small when zoomed
		if( flDistance >= 378.0f && g_lastFOV == 0.0f )
		{
			float flScale = flDistance / 175.0f / 10.0f;

			if( flScale > 2.0f )
				flScale = 2.0f;

			m_TargetHeadModel[i].curstate.scale = flScale;
		}
		else
			m_TargetHeadModel[i].curstate.scale = 0.2f;

		gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, &m_TargetHeadModel[i] );
	}
}

// the icon of the team over the head of the target
void CHudStatusBar::DrawEntitiesOverTarget( void )
{
	if( m_iTargetIndex < 0 )
		return;

	cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();
	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( m_iTargetIndex );

	if( !pEnt )
		return;

	if( pEnt->curstate.messagenum < pLocal->curstate.messagenum || ( pEnt->curstate.effects & EF_NODRAW ) || pEnt == pLocal )
		return;

	int team = pEnt->curstate.team;

	if( team != pLocal->curstate.team || m_TargetTalking[m_iTargetIndex] == 1 )
		return;

	if( team == 1 )
		m_StatusBarHeadModel = gHUD.m_bBritish ? m_StatusBarBritishModel : m_StatusBarAllieModel;
	else
		m_StatusBarHeadModel = m_StatusBarGermanModel;

	memset( &m_TargetHeadModel[0], 0, sizeof( m_TargetHeadModel[0] ) );

	m_TargetHeadModel[0].curstate.rendermode = kRenderTransAdd;
	m_TargetHeadModel[0].curstate.renderamt = 127;
	m_TargetHeadModel[0].curstate.renderfx = kRenderFxNoDissipation;
	m_TargetHeadModel[0].baseline.renderamt = 127;
	m_TargetHeadModel[0].curstate.frame = 0.0f;
	m_TargetHeadModel[0].curstate.framerate = 1.0f;
	m_TargetHeadModel[0].model = (model_s *)gEngfuncs.GetSpritePointer( m_StatusBarHeadModel );
	m_TargetHeadModel[0].angles = Vector( 0.0f, 0.0f, 0.0f );
	m_TargetHeadModel[0].origin = Vector( 0.0f, 0.0f, 45.0f );
	m_TargetHeadModel[0].origin = Vector( pEnt->origin ) + m_TargetHeadModel[0].origin;

	float flDistance = ( m_TargetHeadModel[0].origin - Vector( pLocal->origin ) ).Length();

	if( flDistance >= 378.0f && g_lastFOV == 0.0f )
	{
		float flScale = flDistance / 175.0f / 10.0f;

		if( flScale > 2.0f )
			flScale = 2.0f;

		m_TargetHeadModel[0].curstate.scale = flScale;
	}
	else
		m_TargetHeadModel[0].curstate.scale = 0.2f;

	gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, &m_TargetHeadModel[0] );
}
