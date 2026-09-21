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
#include "pmtrace.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_StatusBar, StatusValue )

#define STATUSBAR_ID_LINE		1

float *GetClientColor( int clientIndex );
extern float g_ColorYellow[3];
extern cvar_t *cl_identiconmode;
extern float g_lastFOV;
extern int g_iAlive;

int CHudStatusBar::Init( void )
{
	gHUD.AddHudElem( this );
	HOOK_MESSAGE( StatusValue );
	Reset();
	m_flNextUpdateTime = 0.0f;
	hud_centerid = CVAR_CREATE( "hud_centerid", "1", FCVAR_CLIENTDLL );

	memset( m_TargetTalking, 0, sizeof( m_TargetTalking ) );
	memset( m_szStatusText, 0, sizeof( m_szStatusText ) );
	return 1;
}

int CHudStatusBar::VidInit( void )
{
	Reset();
	m_flNextUpdateTime = 0.0f;

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

extern vec3_t v_angles;
extern int g_iDeadFlag;

bool CHudStatusBar::InDeathCamMode( void )
{
	if( g_iDeadFlag != DEAD_NO && g_iUser3 > 0 && g_iUser3 <= gEngfuncs.GetMaxClients() )
		return true;

	return false;
}

void CHudStatusBar::Think( void )
{
	cl_entity_t *localPlayer;
	cl_entity_t *killerEnt;
	cl_entity_t *hitPlayer;

	vec3_t start, view_ofs, forward, right, up, end;
	pmtrace_t tr;

	if( m_flNextUpdateTime <= gEngfuncs.GetClientTime() )
	{
		cl_entity_t *pTargetEntity = gEngfuncs.GetEntityByIndex( g_iUser2 );

		if( InDeathCamMode() )
		{
			killerEnt = gEngfuncs.GetEntityByIndex( g_iUser3 );

			if( killerEnt )
			{
				int iKillerIdx = killerEnt->index;

				if( iKillerIdx != gEngfuncs.GetLocalPlayer()->index && iKillerIdx > 0 && iKillerIdx <= gEngfuncs.GetMaxClients() )
				{
					m_iTargetIndex = iKillerIdx;
					m_iTargetTeam = g_PlayerExtraInfo[iKillerIdx].teamnumber;

					m_flNextUpdateTime = gEngfuncs.GetClientTime() + 0.1f;
					return;
				}
			}
		}
		else
		{
			localPlayer = gEngfuncs.GetLocalPlayer();
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( view_ofs );

			start = localPlayer->curstate.origin + view_ofs;

			AngleVectors( v_angles, forward, right, up );

			end = forward * 2048.0f + start;

			gEngfuncs.pEventAPI->EV_SetSolidPlayers( localPlayer->index - 1 );
			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( start, end, 2, -1, &tr );

			if( gEngfuncs.pEventAPI->EV_IndexFromTrace( &tr ) > 0 && gEngfuncs.pEventAPI->EV_IndexFromTrace( &tr ) <= gEngfuncs.GetMaxClients() )
			{
				hitPlayer = gEngfuncs.GetEntityByIndex( gEngfuncs.pEventAPI->EV_IndexFromTrace( &tr ) );
				cl_entity_t *localPlayerEnt = gEngfuncs.GetLocalPlayer();

				gEngfuncs.pfnGetPlayerInfo( hitPlayer->index, &g_PlayerInfoList[hitPlayer->index] );
				gEngfuncs.pfnGetPlayerInfo( localPlayerEnt->index, &g_PlayerInfoList[localPlayerEnt->index] );

				int iTargetIdx = hitPlayer->index;
				int iLocalTeam = g_PlayerExtraInfo[localPlayerEnt->index].teamnumber;
				int iTargetTeam = g_PlayerExtraInfo[iTargetIdx].teamnumber;

				if( iTargetTeam == iLocalTeam || iLocalTeam == 3 || g_iDeadFlag > DEAD_DYING )
				{
					m_iTargetIndex = iTargetIdx;
					m_iTargetTeam = iTargetTeam;
					m_flNextUpdateTime = gEngfuncs.GetClientTime() + 0.1f;
					return;
				}
			}
		}

		m_iTargetIndex = -1;
		m_iTargetTeam = 0;

		m_flNextUpdateTime = gEngfuncs.GetClientTime() + 0.1f;
	}
}

int CHudStatusBar::Draw( float fTime )
{
	char *playerName;
	float *color;
	char szStatusBar[128];
	int TextWidth;
	int y;
	float h;

	cvar_t *hud_centerid;

	if( !g_iUser1 )
	{
		if( g_iAlive )
		{
			if( m_iTargetIndex > 0 && m_iTargetIndex <= gEngfuncs.GetMaxClients() )
			{
				hud_player_info_t playerInfo;
				gEngfuncs.pfnGetPlayerInfo( m_iTargetIndex, &playerInfo );

				playerName = playerInfo.name;
				color = GetClientColor( m_iTargetIndex );

				if( m_iHealth > 0 )
				{
					sprintf( szStatusBar, "%s (%d)", playerName, m_iHealth );

					int iTextHeight;
					gEngfuncs.pfnDrawConsoleStringLen( szStatusBar, &TextWidth, &iTextHeight );
					h = ( float ) iTextHeight;
					int x = 4;

					if( hud_centerid && hud_centerid->value > 0.0f )
					{
						x = ( ScreenWidth - TextWidth ) / 2;
						if( x < 0 )
							x = 0;

						h = hud_centerid->value * h + ( h + ( float ) ( ScreenHeight / 2 ) );
						y = ( int ) h;
					}
					else
					{
						x = ScreenWidth / 2 - TextWidth / 2;
						y = ScreenHeight - 2 * ( int ) h;
					}

					if( color )
						DrawSetTextColor( color[0], color[1], color[2] );

					DrawConsoleString( x, y, szStatusBar );
				}
			}
		}
	}

	return 1;
}

// Message handler for StatusText message
// accepts two values:
//		byte: index into the status value array
//		short: value to store
int CHudStatusBar::MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iHealth = READ_BYTE();

	if( g_iUser1 )
		return 0;
		// vgui2
	return 1;
}

char *CHudStatusBar::GetTargetName( void )
{
	if( m_iTargetIndex > 0 && m_iTargetIndex <= gEngfuncs.GetMaxClients() )
	{
		hud_player_info_t info;

		gEngfuncs.pfnGetPlayerInfo( m_iTargetIndex, &info );

		return info.name;
	}

	return "";
}

extern int g_iEffects;

void CHudStatusBar::CreateEntities( void )
{
	if( m_StatusBarAllieModel && m_StatusBarBritishModel && m_StatusBarGermanModel )
	{
		if( !g_iUser1 && ( g_iEffects & EF_NODRAW ) == 0 && !g_iDeadFlag )
		{
			if( cl_identiconmode && cl_identiconmode->value >= 2.0f )
			{
				DrawEntitiesOverTeam();
			}
			else if( cl_identiconmode && cl_identiconmode->value >= 1.0f )
			{
				DrawEntitiesOverTarget();
			}
		}
	}
}


void CHudStatusBar::DrawEntitiesOverTeam( void )
{
	cl_entity_t *localPlayer;
	cl_entity_t *pClient;
	cl_entity_t *pEnt;
	vec3_t vecDist;
	float f_scaleMult;

	localPlayer = gEngfuncs.GetLocalPlayer();
	if( !localPlayer )
		return;

	for( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		pClient = gEngfuncs.GetEntityByIndex( i );
		if( !pClient )
			continue;

		if( pClient->curstate.messagenum >= localPlayer->curstate.messagenum && pClient->curstate.effects >= 0 &&
			pClient != localPlayer && !m_TargetTalking[i] )
		{
			int team = pClient->curstate.team;

			if( team == localPlayer->curstate.team && pClient->curstate.solid && !g_PlayerExtraInfo[i].dead )
			{
				if( team == 1 )
				{
					if( gHUD.m_bBritish )
						m_StatusBarHeadModel = m_StatusBarBritishModel;
					else
						m_StatusBarHeadModel = m_StatusBarAllieModel;
				}
				else
				{
					m_StatusBarHeadModel = m_StatusBarGermanModel;
				}

				pEnt = &m_TargetHeadModel[i];
				memset( pEnt, 0, sizeof( cl_entity_t ) );

				pEnt->curstate.rendermode = kRenderTransAdd;
				pEnt->curstate.renderamt = 127;
				pEnt->curstate.frame = 0.0f;
				pEnt->baseline.renderamt = 127;
				pEnt->curstate.renderfx = kRenderFxNoDissipation;
				pEnt->curstate.framerate = 1.0f;

				pEnt->model = (model_s *)gEngfuncs.GetSpritePointer( m_StatusBarHeadModel );

				pEnt->angles = { 0.0f, 0.0f, 0.0f };

				pEnt->origin[0] = pClient->origin[0];
				pEnt->origin[1] = pClient->origin[1];
				pEnt->origin[2] = pClient->origin[2] + 45.0f;

				vecDist.x = pEnt->origin[0] - localPlayer->origin[0];
				vecDist.y = pEnt->origin[1] - localPlayer->origin[1];
				vecDist.z = pEnt->origin[2] - localPlayer->origin[2];

				float flDistance = vecDist.Length();

				if( flDistance >= 378.0f && g_lastFOV == 0.0f )
				{
					f_scaleMult = flDistance / 175.0f / 10.0f;

					if( f_scaleMult > 2.0f )
						f_scaleMult = 2.0f;

					pEnt->curstate.scale = f_scaleMult;
				}
				else
				{
					pEnt->curstate.scale = 0.2f;
				}

				gEngfuncs.CL_CreateVisibleEntity( 0, pEnt );
			}
		}
	}
}

void CHudStatusBar::DrawEntitiesOverTarget( void )
{
	cl_entity_t *localPlayer;
	cl_entity_t *pClient;
	cl_entity_t *pEnt;
	vec3_t vecDist;
	float f_scaleMult;

	if( m_iTargetIndex >= 0 )
	{
		localPlayer = gEngfuncs.GetLocalPlayer();
		pClient = gEngfuncs.GetEntityByIndex( m_iTargetIndex );

		if( pClient )
		{
			if( pClient->curstate.messagenum >= localPlayer->curstate.messagenum &&
				pClient->curstate.effects >= 0 &&
				pClient != localPlayer )
			{
				int team = pClient->curstate.team;

				if( team == localPlayer->curstate.team && m_TargetTalking[m_iTargetIndex] != 1 )
				{
					if( team == 1 )
					{
						if( gHUD.m_bBritish )
							m_StatusBarHeadModel = m_StatusBarBritishModel;
						else
							m_StatusBarHeadModel = m_StatusBarAllieModel;
					}
					else
					{
						m_StatusBarHeadModel = m_StatusBarGermanModel;
					}

					pEnt = &m_TargetHeadModel[0];
					memset( pEnt, 0, sizeof( cl_entity_t ) );

					pEnt->curstate.rendermode = kRenderTransAdd;
					pEnt->curstate.renderamt = 127;
					pEnt->curstate.renderfx = kRenderFxNoDissipation;
					pEnt->baseline.renderamt = 127;
					pEnt->curstate.frame = 0.0f;
					pEnt->curstate.framerate = 1.0f;

					pEnt->model = (model_s *)gEngfuncs.GetSpritePointer( m_StatusBarHeadModel );

					pEnt->angles = { 0.0f, 0.0f, 0.0f };

					pEnt->origin.x = pClient->origin.x;
					pEnt->origin.y = pClient->origin.y;
					pEnt->origin.z = pClient->origin.z + 45.0f;

					vecDist.x = pEnt->origin.x - localPlayer->origin.x;
					vecDist.y = pEnt->origin.y - localPlayer->origin.y;
					vecDist.z = pEnt->origin.z - localPlayer->origin.z;

					float flDistance = vecDist.Length();

					if( flDistance >= 378.0f && g_lastFOV == 0.0f )
					{
						f_scaleMult = flDistance / 175.0f / 10.0f;
						if( f_scaleMult > 2.0f )
						{
							f_scaleMult = 2.0f;
						}
						pEnt->curstate.scale = f_scaleMult;
					}
					else
					{
						pEnt->curstate.scale = 0.2f;
					}

					gEngfuncs.CL_CreateVisibleEntity( 0, pEnt );
				}
			}
		}
	}
}
