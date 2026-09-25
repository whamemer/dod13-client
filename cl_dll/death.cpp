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
// death notice
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "dod_shared.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_DeathNotice, DeathMsg )

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH * 2];
	char szVictim[MAX_PLAYER_NAME_LENGTH * 2];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

float g_ColorBlue[3]	 = { 0.6, 0.8, 1.0 };
float g_ColorRed[3]		 = { 1.0, 0.25, 0.25 };
float g_ColorGreen[3]	 = { 0.6, 1.0, 0.6 };
float g_ColorYellow[3]	 = { 1.0, 0.7, 0.0 };
float g_ColorGrey[3]	 = { 0.8, 0.8, 0.8 };
float g_ColorDoDGreen[3] = { 0.4, 0.7, 0.4 };

float *GetClientColor( int clientIndex )
{
	switch( g_PlayerExtraInfo[clientIndex].teamnumber )
	{
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	case 4: return g_ColorGreen;
	case 0:
	case 1:
	default: return g_ColorDoDGreen;
	}

	return NULL;
}

float *GetTeamColor( int teamIndex )
{
	switch( teamIndex )
	{
	case 1: return g_ColorDoDGreen;
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	default: return g_ColorGreen;
	}

	return NULL;
}

int CHudDeathNotice::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	CVAR_CREATE( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );

	return 1;
}

void CHudDeathNotice::InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}

int CHudDeathNotice::VidInit( void )
{
	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );

	return 1;
}

int CHudDeathNotice::Draw( float flTime )
{
	int mapX, mapWidth;
	int mapY, mapHeight;
	int yPos = 20;

	gHUD.GetMapBounds( mapX, mapY, mapWidth, mapHeight );
	if( gHUD.GetMinimapState() == 2 )
	{
		float scaleY = ( float ) gHUD.m_scrinfo.iHeight / 480.0f;
		yPos = mapHeight + mapY + ( int ) ( scaleY + scaleY + 0.5f );
	}
	else if( g_iUser1 )
	{
		float scaleY = ( float ) gHUD.m_scrinfo.iHeight / 480.0f;
		yPos = ( int ) ( scaleY * 42.0f + 0.5f ) + 20;
	}

	for( int i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if( rgDeathNoticeList[i].flDisplayTime < flTime )
		{
			// display time has expired, remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof( DeathNoticeItem ) * ( MAX_DEATHNOTICES - i ) );
			i--;
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min( rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME );


		int id = ( rgDeathNoticeList[i].iId == -1 ) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;

		int victimLen = ConsoleStringLen( rgDeathNoticeList[i].szVictim );
		wrect_t *pRect = &gHUD.m_rgrcRects[id];
		int spriteX = gHUD.m_scrinfo.iWidth - victimLen - ( pRect->right - pRect->left ) - pRect->right;

		if( !rgDeathNoticeList[i].iSuicide )
		{
			int killerLen = ConsoleStringLen( rgDeathNoticeList[i].szKiller );
			int killerX = spriteX - killerLen - 5;

			if( rgDeathNoticeList[i].KillerColor )
				DrawSetTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );

			gEngfuncs.pfnDrawConsoleString( killerX, yPos, rgDeathNoticeList[i].szKiller );
		}

		HSPRITE hSprite = ( id >= 0 ) ? gHUD.m_rghSprites[id] : 0;

		gEngfuncs.pfnSPR_Set( hSprite, 255, 255, 255 );
		gEngfuncs.pfnSPR_DrawHoles( 0, spriteX, yPos, pRect );

		if( rgDeathNoticeList[i].iNonPlayerKill == FALSE )
		{
			if( rgDeathNoticeList[i].VictimColor )
			{
				DrawSetTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );
			}

			int victimX = spriteX + ( pRect->right - pRect->left );
			gEngfuncs.pfnDrawConsoleString( victimX, yPos, rgDeathNoticeList[i].szVictim );
		}

		yPos += ( pRect->bottom - pRect->top );
	}

	return 1;
}

char deathicons[44][32];

// This message handler may be better off elsewhere
int CHudDeathNotice::MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );

	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int weaponId = READ_BYTE();

	char killedwith[60];

	if( weaponId >= WEAPON_AMERKNIFE && weaponId < 44 )
	{
		strlcpy( killedwith, deathicons[weaponId], sizeof( killedwith ) );
	}
	else
	{
		strcpy( killedwith, "d_world" );
	}

	gHUD.m_Scoreboard.DeathMsg( killer, victim );
	gHUD.m_Spectator.DeathMessage( victim );

	int i;

	for( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if( i == MAX_DEATHNOTICES )
	{
		memmove( rgDeathNoticeList, rgDeathNoticeList + 1, sizeof( DeathNoticeItem ) * ( MAX_DEATHNOTICES - 1 ) );
		i = MAX_DEATHNOTICES - 1;
	}

	gEngfuncs.pfnGetPlayerInfo( killer, &g_PlayerInfoList[killer] );
	gEngfuncs.pfnGetPlayerInfo( victim, &g_PlayerInfoList[victim] );

	// Get the Killer's name
	const char *killer_name = g_PlayerInfoList[killer].name;
	if( !killer_name || !*killer_name )
	{
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strlcpy( rgDeathNoticeList[i].szKiller, killer_name, 32 );
	}

	// Get the Victim's name
	const char *victim_name = "";
	if( ( ( signed char ) victim ) != -1 )
		victim_name = g_PlayerInfoList[victim].name;

	if( victim == 255 || !victim_name || !*victim_name )
	{
		rgDeathNoticeList[i].szVictim[0] = 0;

		if( victim == 255 )
		{
			rgDeathNoticeList[i].iNonPlayerKill = TRUE;
			strlcpy( rgDeathNoticeList[i].szVictim, killedwith + 2, 32 );
		}
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strlcpy( rgDeathNoticeList[i].szVictim, victim_name, 32 );
	}

	if( ( ( signed char ) victim ) != -1 )
	{
		if( killer && killer != victim )
		{
			if( g_PlayerExtraInfo[victim].teamnumber == g_PlayerExtraInfo[killer].teamnumber )
				rgDeathNoticeList[i].iTeamKill = TRUE;
		}
		else
		{
			rgDeathNoticeList[i].iSuicide = TRUE;
		}
	}

	rgDeathNoticeList[i].iId = gHUD.GetSpriteIndex( killedwith );

	if( hud_deathnotice_time )
		DEATHNOTICE_DISPLAY_TIME = hud_deathnotice_time->value;
	else
		DEATHNOTICE_DISPLAY_TIME = 6.0f;

	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	if( rgDeathNoticeList[i].iNonPlayerKill )
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed a " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
		ConsolePrint( "\n" );
	}
	else
	{
		if( rgDeathNoticeList[i].iSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );
			if( !strcmp( killedwith, "d_world" ) )
				ConsolePrint( " died" );
			else
				ConsolePrint( " killed self" );
		}
		else if( rgDeathNoticeList[i].iTeamKill )
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed his teammate " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}
		else
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}

		if( *killedwith && strcmp( killedwith, "d_world" ) && !rgDeathNoticeList[i].iTeamKill )
		{
			ConsolePrint( " with " );
			ConsolePrint( killedwith + 2 );
		}

		ConsolePrint( "\n" );
	}

	return 1;
}
