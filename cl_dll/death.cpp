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

#include <string.h>

#include "vgui/vgui_TeamFortressViewport.h"

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

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

float g_ColorBlue[3]	= { 0.6f, 0.8f, 1.0f };
float g_ColorRed[3]	= { 1.0f, 0.25f, 0.25f };
float g_ColorGreen[3]	= { 0.6f, 1.0f, 0.6f };
float g_ColorYellow[3]	= { 1.0f, 0.7f, 0.0f };

float g_ColorDoDGreen[3]	= { 0.3f, 0.4f, 0.3f };

float *GetClientColor( int clientIndex )
{
	switch( g_PlayerExtraInfo[clientIndex].teamnumber )
	{
	case 2:	return g_ColorRed;
	case 3: return g_ColorYellow;
	case 4: return g_ColorGreen;
	default: return g_ColorDoDGreen;
	}
}

float *GetTeamColor( int teamIndex )
{
	switch( teamIndex )
	{
	case 1:	return g_ColorDoDGreen;
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	default: return g_ColorGreen;
	}
}

int CHudDeathNotice::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	hud_deathnotice_time = CVAR_CREATE( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );

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
	int i;
	int x, y;
	int mapX, mapY, mapWidth, mapHeight;

	gHUD.GetMapBounds( mapX, mapY, mapWidth, mapHeight );

	if( gHUD.GetMinimapState() == 2 )
	{
		// under the minimap
		y = mapHeight + mapY + (int)( (float)ScreenHeight / 480.0f * 2.0f + 0.5f );
	}
	else
	{
		y = 20;

		// under the bar of the spectator
		if( g_iUser1 )
			y = (int)( (float)ScreenHeight / 480.0f * 42.0f + 0.5f ) + 20;
	}

	i = 0;

	while( rgDeathNoticeList[i].iId != 0 )
	{
		if( rgDeathNoticeList[i].flDisplayTime < flTime )
		{
			// display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof(DeathNoticeItem) * ( MAX_DEATHNOTICES - i ) );
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = Q_min( rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME );

		// Only draw if the viewport will let me
		if( gViewPort && gViewPort->AllowedToPrintText() )
		{
			// Draw the death notice
			int id = ( rgDeathNoticeList[i].iId == -1 ) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			wrect_t rect = gHUD.GetSpriteRect( id );

			x = ScreenWidth - ConsoleStringLen( rgDeathNoticeList[i].szVictim ) - ( rect.right - rect.left );

			if( !rgDeathNoticeList[i].iSuicide )
			{
				x -= ( 5 + ConsoleStringLen( rgDeathNoticeList[i].szKiller ) );

				// Draw killers name
				if( rgDeathNoticeList[i].KillerColor )
					DrawSetTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );

				x = 5 + DrawConsoleString( x, y, rgDeathNoticeList[i].szKiller );
			}

			// Draw death weapon
			SPR_Set( gHUD.GetSprite( id ), 255, 255, 255 );
			SPR_DrawHoles( 0, x, y, &rect );

			// Draw victims name (if it was a player that was killed)
			if( rgDeathNoticeList[i].iNonPlayerKill == FALSE )
			{
				if( rgDeathNoticeList[i].VictimColor )
					DrawSetTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );

				DrawConsoleString( x + ( rect.right - rect.left ), y, rgDeathNoticeList[i].szVictim );
			}

			y += rect.bottom - rect.top;
		}

		if( ++i >= MAX_DEATHNOTICES )
			break;
	}

	return 1;
}

char deathicons[44][32] =
{
	"",
	"d_amerknife",
	"d_gerknife",
	"d_colt",
	"d_luger",
	"d_garand",
	"d_scopedkar",
	"d_thompson",
	"d_mp44",
	"d_spring",
	"d_kar",
	"d_bar",
	"d_mp40",
	"d_grenade",
	"d_stick",
	"d_stick",
	"d_grenade",
	"d_mg42",
	"d_30cal",
	"d_spade",
	"d_m1carbine",
	"d_mg34",
	"d_greasegun",
	"d_fg42",
	"d_k43",
	"d_enfield",
	"d_sten",
	"d_bren",
	"d_webley",
	"d_bazooka",
	"d_pschreck",
	"d_piat",
	"d_mortar",
	"d_binoculars",
	"d_satchel",
	"d_scopedfg42",
	"d_fcarbine",
	"d_bayonet",
	"d_scopedenfield",
	"d_britgrenade",
	"d_britknife",
	"d_mortar",
	"d_garandbutt",
	"d_enfbayonet"
};

// This message handler may be better off elsewhere
int CHudDeathNotice::MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	int i;
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );

	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int weapon = READ_BYTE();

	char killedwith[32];

	if( (unsigned int)( weapon - 1 ) <= 42 )
		strcpy( killedwith, deathicons[weapon] );
	else
		strcpy( killedwith, "d_world" );

	gHUD.m_Scoreboard.DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage( victim );

	for( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if( rgDeathNoticeList[i].iId == 0 )
			break;
	}

	if( i == MAX_DEATHNOTICES )
	{
		// move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList + 1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	gEngfuncs.pfnGetPlayerInfo( killer, &g_PlayerInfoList[killer] );

	// 255 (not a player) is out of the list
	if( victim <= MAX_PLAYERS )
		gEngfuncs.pfnGetPlayerInfo( victim, &g_PlayerInfoList[victim] );

	// Get the Killer's name
	char *killer_name = g_PlayerInfoList[killer].name;

	if( !killer_name )
	{
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strlcpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
	}

	// Get the Victim's name
	char *victim_name = NULL;

	// If victim is 255, the killer killed a specific, non-player object
	if( victim <= MAX_PLAYERS )
		victim_name = g_PlayerInfoList[victim].name;

	if( !victim_name )
	{
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strlcpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
	}

	// Is it a non-player object kill?
	if( victim == 255 )
	{
		rgDeathNoticeList[i].iNonPlayerKill = TRUE;

		// Store the object's name in the Victim slot (skip the d_ bit)
		strcpy( rgDeathNoticeList[i].szVictim, killedwith + 2 );
	}
	else
	{
		if( killer == 0 || killer == victim )
			rgDeathNoticeList[i].iSuicide = TRUE;
		else if( victim && g_PlayerExtraInfo[victim].teamnumber == g_PlayerExtraInfo[killer].teamnumber )
			rgDeathNoticeList[i].iTeamKill = TRUE;
	}

	// Find the sprite in the list
	rgDeathNoticeList[i].iId = gHUD.GetSpriteIndex( killedwith );

	if( hud_deathnotice_time )
		DEATHNOTICE_DISPLAY_TIME = (int)hud_deathnotice_time->value;
	else
		DEATHNOTICE_DISPLAY_TIME = 6;

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
		// record the death notice in the console
		if( rgDeathNoticeList[i].iSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			if( victim == killer )
				ConsolePrint( " killed self" );
			else
				ConsolePrint( " died" );
		}
		else
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );

			if( rgDeathNoticeList[i].iTeamKill )
				ConsolePrint( " killed his teammate " );
			else
				ConsolePrint( " killed " );

			ConsolePrint( rgDeathNoticeList[i].szVictim );
			ConsolePrint( " with " );
			ConsolePrint( killedwith + 2 ); // skip over the "d_" part
		}

		ConsolePrint( "\n" );
	}

	return 1;
}
