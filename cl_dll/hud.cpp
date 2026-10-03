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
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

#include "demo.h"
#include "demo_api.h"

#include "event_api.h"

#include "r_studioint.h"
#include "dod_shared.h"
#include "voice_status.h"

float g_fUser4;
int g_iVuser1x;
int g_iVuser1z;
int g_iMovetype;
int g_iEffects;
int g_iOnlyClientDraw;

float g_lastFOV = 0.0f;
float g_fStamina = 100.0f;

int g_iWeaponBits2;

extern engine_studio_api_t IEngineStudio;

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	  // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
team_info_t		g_TeamInfo[MAX_TEAMS + 1];				// for CHudScoreboard
pmodel_fx_t     g_PModelFxInfo[MAX_TEAMS + 1];

int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;

int iNumberOfTeamColors = 3;
int iTeamColors[3][3] =
{
	{ 128, 128, 128 }, // Spectators
	{ 0, 160, 0 }, // Allies
	{ 200, 0, 0 } // Axis
};

class CDoDVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex <= MAX_PLAYERS )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool CanShowSpeakerLabels()
	{
		return false;
	}
};

static CDoDVoiceStatusHelper g_VoiceStatusHelper;

cvar_t *hud_textmode;
float g_hud_text_color[3];

extern client_sprite_t *GetSpriteList( client_sprite_t *pList, const char *psz, int iRes, int iCount );

extern cvar_t *sensitivity;
extern cvar_t *cl_dmsmallmap;
extern cvar_t *cl_dmshowmarkers;
extern cvar_t *cl_dmshowplayers;
extern cvar_t *cl_dmshowflags;
extern cvar_t *cl_dmshowobjects;
extern cvar_t *cl_dmshowgrenades;
extern cvar_t *cl_numshotrubble;
extern cvar_t *cl_weatherdis;
cvar_t *cl_autoreload;

void ShutdownInput( void );

int __MsgFunc_Logo( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Logo( pszName, iSize, pbuf );
}

int __MsgFunc_ResetHUD( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ResetHUD( pszName, iSize, pbuf );
}

int __MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_YouDied( pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( g_iVuser1z )
		return gHUD.m_iFOV;

	g_lastFOV = READ_BYTE();

	if( !g_lastFOV )
	{
		gHUD.m_iFOV = 90;
		gHUD.m_flMouseSensitivity = 0.0;
		return 1;
	}

	gHUD.m_iFOV = READ_BYTE();

	if( gHUD.m_iFOV == 90 )
	{
		gHUD.m_flMouseSensitivity = 0.0;
		return 1;
	}
	
	float zoom_sens;

	if( gHUD.zoom_sensitivity_ratio )
		zoom_sens = gHUD.zoom_sensitivity_ratio->value;
	else
		zoom_sens = 1.0f;

	gHUD.m_flMouseSensitivity = zoom_sens * ( READ_BYTE() / 90.0f * sensitivity->value );
	return 1;
}

int __MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_HLTV( pszName, iSize, pbuf );
}

int __MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( READ_BYTE() )
		PlaySound( "common/wpn_select.wav", 0.5 );
	else
		PlaySound( "common/wpn_denyselect.wav", 0.5 );

	return 1;
}

int __MsgFunc_RoundState( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_iRoundState = READ_BYTE();
	return 1;
}

int __MsgFunc_TimeLeft( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_fRoundEndsTime = READ_BYTE() + gHUD.m_flTime;
	return 1;
}

int __MsgFunc_BloodPuff( const char *pszName, int iSize, void *pbuf )
{
	float origin[3];

	BEGIN_READ( pbuf, iSize );

	origin[0] = READ_COORD();
	origin[1] = READ_COORD();
	origin[2] = READ_COORD();

	ShouldShowBlood();
	EV_BloodPuff( origin );
	return 1;
}

int __MsgFunc_HandSignal( const char *pszName, int iSize, void *pbuf )
{
	return EV_HandSignalMsg( pszName, iSize, pbuf );
}

int __MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_Frags( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_PClass( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

int __MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf )
{
	return 0;
}

// This is called every time the DLL is loaded
void CHud::Init( void )
{
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( YouDied );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( HLTV );
	HOOK_MESSAGE( BloodPuff );
	HOOK_MESSAGE( HandSignal );
	HOOK_MESSAGE( UseSound );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( ScoreInfoLong );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );
	HOOK_MESSAGE( MapMarker );
	HOOK_MESSAGE( VGUIMenu );
	HOOK_MESSAGE( WaveTime );
	HOOK_MESSAGE( WaveStatus );
	HOOK_MESSAGE( WideScreen );
	HOOK_MESSAGE( Frags );
	HOOK_MESSAGE( ObjScore );
	HOOK_MESSAGE( PStatus );
	HOOK_MESSAGE( ScoreShort );
	HOOK_MESSAGE( PClass );
	HOOK_MESSAGE( PTeam );
	HOOK_MESSAGE( RoundState );
	HOOK_MESSAGE( CurMarker );
	HOOK_MESSAGE( TimeLeft );

	hud_takesshots = CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );
	max_rubble = CVAR_CREATE( "max_rubble", "240", 0 );
	cl_corpsestay = CVAR_CREATE( "cl_corpsestay", "10", FCVAR_ARCHIVE );

	CVAR_CREATE( "cl_dmsmallmap", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowmarkers", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowplayers", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowflags", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowobjects", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowgrenades", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_numshotrubble", "5", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_weatherdis", "1700", FCVAR_ARCHIVE );
	cl_autoreload = CVAR_CREATE( "cl_autoreload", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );

	_cl_minimap = CVAR_CREATE( "_cl_minimap", "2", FCVAR_ARCHIVE | FCVAR_USERINFO );
	_cl_minimapzoom = CVAR_CREATE( "_cl_minimapzoom", "1", FCVAR_ARCHIVE );
	zoom_sensitivity_ratio = CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", FCVAR_ARCHIVE );
	_ah = CVAR_CREATE( "_ah", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	cl_hudfont = CVAR_CREATE( "cl_hudfont", "1", FCVAR_ARCHIVE );
	hud_fastswitch = CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );

	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	r_drawentities = gEngfuncs.pfnGetCvarPointer( "r_drawentities" );
	cl_pitchdown = gEngfuncs.pfnGetCvarPointer( "cl_pitchdown" );
	cl_pitchup = gEngfuncs.pfnGetCvarPointer( "cl_pitchup" );
	crosshair = gEngfuncs.pfnGetCvarPointer( "crosshair" );
	developer = gEngfuncs.pfnGetCvarPointer( "developer" );

	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );

	m_iLogo = 0;
	m_iFOV = 0;
	m_iRes = 1;
	m_iSensLevel = 0;
	m_iRoundState = 1;
	i_Recoil = 0;
	m_iWaterLevel = 0;
	m_flTime = 1.0f;

	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;

	m_pSpriteList = NULL;

	if( m_pHudList )
	{
		HUDLIST *pList;
		while( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	m_Scope.Init();
	m_DoDCommon.Init();
	m_Icons.Init();
	m_DoDMap.Init();
	m_ObjectiveIcons.Init();
	m_PShooter.Init();
	m_CEnvModel.Init();
	m_Weather.Init();
	m_Ammo.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Train.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_Menu.Init();
	m_DoDCrossHair.Init();
	m_MortarHud.Init();
	m_VGUI2Print.Init();

	m_MOTD.Init();
	m_Scoreboard.Init();

	MsgFunc_ResetHUD( 0, 0, NULL );

	m_szTeamNames[NULL][NULL] = '\0';

	strcpy( m_szTeamNames[1], m_TextMessage.BufferedLocaliseTextString( "#Teamname_allies" ) );
	strcpy( m_szTeamNames[2], m_TextMessage.BufferedLocaliseTextString( "#Teamname_axis" ) );
	strcpy( m_szTeamNames[3], m_TextMessage.BufferedLocaliseTextString( "#Teamname_spectators" ) );
	strcpy( m_szTeamNames[4], m_TextMessage.BufferedLocaliseTextString( "#Teamname_british" ) );

	InitMapBounds();
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud::~CHud()
{
	delete[] m_rghSprites;
	delete[] m_rgrcRects;
	delete[] m_rgszSpriteNames;

	if( m_pHudList )
	{
		HUDLIST *pList;
		while( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	g_RubbleQueue.Update( m_flTime );
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud::GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for( int i = 0; i < m_iSpriteCount; i++ )
	{
		if( strncmp( SpriteName, m_rgszSpriteNames + ( i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud::VidInit( void )
{
	m_scrinfo.iSize = sizeof( SCREENINFO );
	gEngfuncs.pfnGetScreenInfo( &m_scrinfo );

	m_hsprLogo = 0;
	m_hsprCursor = 0;
	m_iRoundState = 1;
	m_iRes = ( ScreenWidth <= 639 ) ? 320 : 640;

	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;

	m_flPlaySprintSoundTime = 0.0f;
	m_fRoundEndsTime = 0.0f;

	SetFOV( 0 );
	g_lastFOV = 0.0f;

	m_flPitchRecoilAccumulator = 0.0f;
	m_flYawRecoilAccumulator = 0.0f;
	m_flRecoilTimeRemaining = 0.0f;

	if( m_pSpriteList )
	{
		if( m_iSpriteCountAllRes > 0 )
		{
			int validSprites = 0;
			int index = 0;
			client_sprite_t *p = m_pSpriteList;

			while( TRUE )
			{
				if( p->iRes == m_iRes )
				{
					validSprites++;
					char sz[256];
					sprintf( sz, "sprites/%s.spr", p->szSprite );
					m_rghSprites[index++] = gEngfuncs.pfnSPR_Load( sz );

					if( validSprites >= m_iSpriteCountAllRes )
						break;
				}
				else if( ++validSprites >= m_iSpriteCountAllRes )
				{
					break;
				}
				p++;
			}
		}
	}
	else
	{
		m_pSpriteList = gEngfuncs.pfnSPR_GetList( "sprites/hud.txt", &m_iSpriteCountAllRes );

		if( m_pSpriteList )
		{
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;

			for( int j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			m_rghSprites = new HSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;

			for( int i = 0; i < m_iSpriteCountAllRes; i++ )
			{
				if( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf( sz, "sprites/%s.spr", p->szSprite );
					m_rghSprites[index] = gEngfuncs.pfnSPR_Load( sz );
					m_rgrcRects[index] = p->rc;

					strncpy( m_rgszSpriteNames + ( index * MAX_SPRITE_NAME_LENGTH ), p->szName, MAX_SPRITE_NAME_LENGTH );
					index++;
				}
				p++;
			}
		}
	}

	m_HUD_number_0 = GetSpriteIndex( "number_0" );
	m_MG_number_0 = GetSpriteIndex( "mg_0" );

	if( m_HUD_number_0 != -1 )
		m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;
	else
		m_iFontHeight = 16;

	m_iFontEngineHeight = m_iFontHeight;

	m_Ammo.VidInit();
	m_Scope.VidInit();
	m_DoDCommon.VidInit();
	m_Icons.VidInit();
	m_DoDMap.VidInit();
	m_ObjectiveIcons.VidInit();
	m_CEnvModel.VidInit();
	m_PShooter.VidInit();
	m_Weather.VidInit();
	m_Train.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_StatusIcons.VidInit();
	m_DoDCrossHair.VidInit();
	m_MortarHud.VidInit();

	m_MOTD.VidInit();
	m_Scoreboard.VidInit();

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		g_PlayerExtraInfo[i].teamnumber = 3;
	}

	ClientSetSensitivity( 0 );
	memset( g_PModelFxInfo, 0, sizeof( g_PModelFxInfo ) );
}

int CHud::MsgFunc_Logo( const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

bool CHud::IsTeamPara( int team )
{
	if( team == 1 )
		return m_bAllieParatrooper;
	else
		return m_bAxisParatrooper;
}

extern DodClassInfo_t g_ClassInfo[28];
extern DodClassInfo_t g_ParaClassInfo[21];

char *CHud::GetPlayerClassName( int playerclass )
{
	if( ( playerclass - 10 ) > 10 )
	{
		if( !m_bAllieParatrooper )
			return g_ClassInfo[playerclass].classname;
	}
	else
	{
		if( !m_bAxisParatrooper )
			return g_ClassInfo[playerclass].classname;
	}

	return g_ParaClassInfo[playerclass].classname;
}

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out )
{
	int len, start, end;

	len = strlen( in );

	// scan backward for '.'
	end = len - 1;
	while( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;

	if( in[end] != '.' )		// no '.', copy to end
		end = len - 1;
	else 
		end--;					// Found ',', copy to left of '.'

	// Scan backward for '/'
	start = len - 1;
	while( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new string
	len = end - start + 1;

	// Copy partial string
	strlcpy( out, &in[start], len + 1 );
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[1024];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[100];

		// Active
		*(float *)&buf[i] = g_lastFOV;
		i += sizeof(float);

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( g_iVuser1z )
		return m_iFOV;

	int newfov = READ_BYTE();
	g_lastFOV = ( float ) newfov;

	int def_fov = 90;

	if( newfov == 0 )
		m_iFOV = def_fov;
	else
		m_iFOV = newfov;

	if( m_iFOV == def_fov )
	{
		m_flMouseSensitivity = 0.0f;
		return 1;
	}

	float zoom_sens = ( zoom_sensitivity_ratio ) ? zoom_sensitivity_ratio->value : 1.0f;
	float flEngineSens = ( sensitivity ) ? sensitivity->value : 1.0f;

	m_flMouseSensitivity = zoom_sens * ( ( float ) m_iFOV / ( float ) def_fov * flEngineSens );

	return 1;
}

int CHud::MsgFunc_RoundState( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iRoundState = READ_BYTE();
	return 1;
}

int CHud::MsgFunc_TimeLeft( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_fRoundEndsTime = READ_SHORT() + gHUD.m_flTime;
	return 1;
}

int CHud::MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int player = READ_BYTE();
	int data = READ_BYTE();

	if( gEngfuncs.IsSpectateOnly() )
	{
		if( data & 0x80 )
		{
			int health = data & 0x7F;

			if( player != 0 )
			{
				g_PlayerExtraInfo[player].health = health;
			}
			else
			{
				for( int i = 0; i < MAX_PLAYERS; i++ )
				{
					g_PlayerExtraInfo[i].health = health;
				}
			}
			return 1;
		}
		else
		{
			if( data == 0 )
			{
				data = 90;
			}

			if( player != 0 )
			{
				m_PlayerFOV[player] = data;
			}
			else
			{
				for( int i = 0; i < MAX_PLAYERS; i++ )
				{
					m_PlayerFOV[i] = data;
				}
			}
			return 1;
		}
	}
	else
	{
		gEngfuncs.Con_DPrintf( "Warning! Received unexpected HLTV user message.\n" );
		return 1;
	}
}

void CHud::AddHudElem( CHudBase *phudelem )
{
	HUDLIST *pdl, *ptemp;

	if( !phudelem )
		return;

	pdl = (HUDLIST *)malloc( sizeof(HUDLIST) );
	if( !pdl )
		ConsolePrint( "Cannot allocate memory!\n" );
		return;

	memset( pdl, 0, sizeof(HUDLIST) );
	pdl->p = phudelem;

	if( !m_pHudList )
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while( ptemp->pNext )
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

void ClientSetSensitivity( int level )
{
	gHUD.m_iSensLevel = level;
}

float CHud::GetSensitivity( void )
{
	if( gHUD.m_iSensLevel != 1 )
		return m_flMouseSensitivity;

	return 1.0f;
}

int CHud::GetCurrentWeaponId( void )
{
	return m_Ammo.GetCurrentWeaponId();
}

void CHud::SetWaterLevel( int level )
{
	m_iWaterLevel = level;
}

int CHud::GetWaterLevel( void )
{
	return m_iWaterLevel;
}

static cvar_t *violence_hblood;

bool ShouldShowBlood( void )
{
	if( !violence_hblood )
	{
		violence_hblood = gEngfuncs.pfnGetCvarPointer( "violence_hblood" );

		if( !violence_hblood )
			return false;
	}

	if( violence_hblood->value != 1.0f )
		return false;

	return true;
}

int EV_BloodPuffMsg( const char *pszName, int iSize, void *pbuf )
{
	float origin[3];

	BEGIN_READ( pbuf, iSize );

	origin[0] = READ_COORD();
	origin[1] = READ_COORD();
	origin[2] = READ_COORD();

	ShouldShowBlood();
	EV_BloodPuff( origin );
	return 1;
}

extern vec3_t v_origin;

void EV_BloodPuff( float *org )
{
	vec3_t velocity, origin;
	float dustscale, bloodscale;
	float flDist, scale;
	TEMPENTITY *pTemp;
	vec3_t closerOrigin, to_view;

	int modelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" );
	int modelBlood = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" );

	origin = Vector( org );
	velocity = Vector( 0.0f, 0.0f, 2.0f );

	dustscale = gEngfuncs.pfnRandomFloat( 0.7f, 0.8f );
	bloodscale = gEngfuncs.pfnRandomFloat( 0.4f, 0.45f );

	flDist = ( v_origin - origin ).Length();

	if( flDist >= 32.0f )
	{
		if( flDist <= 1200.0f )
		{
			if( flDist <= 600.0f )
				scale = 1.0f;
			else
				scale = ( flDist - 600.0f ) / 600.0f * 0.5f + 1.0f;
		}
		else
		{
			scale = 1.5f;
		}

		pTemp = gEngfuncs.pEfxAPI->R_TempSprite( &origin.x, &velocity.x, scale * dustscale, modelIndex, 4, 0, 0.15f, 30.0f, 256 );

		if( pTemp )
		{
			pTemp->entity.curstate.renderamt = 800;
			pTemp->entity.curstate.framerate = 45.0f;
			pTemp->entity.curstate.rendercolor.r = -76;
			pTemp->entity.curstate.rendercolor.g = -80;
			pTemp->entity.curstate.rendercolor.b = -108;
			pTemp->entity.angles.z = gEngfuncs.pfnRandomLong( 0, 90 );
		}

		to_view = v_origin - origin;
		VectorNormalize( &to_view.x );
		VectorMA( &origin.x, 2.0f, &to_view.x, &closerOrigin.x );

		pTemp = gEngfuncs.pEfxAPI->R_TempSprite( &closerOrigin.x, &velocity.x, scale * bloodscale, modelBlood, 4, 0, 0.35f, 30.0f, 256 );

		if( pTemp )
		{
			pTemp->entity.curstate.framerate = 20.0f;
			pTemp->entity.curstate.renderamt = 800;
			pTemp->entity.curstate.rendercolor.r = 120;
			pTemp->entity.curstate.rendercolor.g = 0;
			pTemp->entity.curstate.rendercolor.b = 0;
			pTemp->entity.angles.z = gEngfuncs.pfnRandomLong( 0, 90 );
		}
	}
}

extern char *s_HandSignalSubtitles[][3];

int EV_HandSignalMsg( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int clientIndex = READ_BYTE();
	int signal = READ_BYTE();

	if( clientIndex >= 1 && clientIndex <= MAX_PLAYERS && signal < HS_IDLE2 )
	{
		int teamId = g_PlayerExtraInfo[clientIndex].teamId;
		gEngfuncs.pfnGetPlayerInfo( clientIndex, &g_PlayerInfoList[clientIndex] );

		char pattern[268];
		sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[clientIndex].name, ": %s2" );

		char *sstr2 = s_HandSignalSubtitles[signal][0];

		if( teamId == 2 )
		{
			char *pszAxisStr = s_HandSignalSubtitles[signal][1];

			if( *pszAxisStr )
				sstr2 = pszAxisStr;
		}
		else if( teamId == 1 && gHUD.m_bBritish )
		{
			char *pszBritishStr = s_HandSignalSubtitles[signal][2];

			if( *pszBritishStr )
				sstr2 = pszBritishStr;
		}

		gHUD.m_SayText.SayTextPrint( pattern, 256, clientIndex, "#Handsignal", sstr2 );
		gHUD.m_Spectator.AddVoiceIconToPlayerEnt( clientIndex );
	}

	return 1;
}

void CHud::MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( READ_BYTE() )
		gEngfuncs.pfnPlaySoundByName( "common/wpn_select.wav", 0.5 );
	else
		gEngfuncs.pfnPlaySoundByName( "common/wpn_denyselect.wav", 0.5 );
}

char *CHud::GetTeamName( int team )
{
	if( team == 1 )
		return m_szTeamNames[1];
	
	if( team != 2 )
		return m_szTeamNames[3];

	return m_szTeamNames[2];
}

int CHud::GetMinimapZoomLevel( void )
{
	return _cl_minimapzoom->value;
}

int CHud::ZoomMinimap( void )
{
	_cl_minimapzoom->value = _cl_minimapzoom->value + 1.0f;

	if( _cl_minimapzoom->value >= 3.0f )
		_cl_minimapzoom->value = 0.0f;

	return _cl_minimapzoom->value;
}

int CHud::GetMinimapState( void )
{
	if( IEngineStudio.IsHardware() && m_iFOV == 90 )
		return GetMinimapState();
	else
		return 0;
}

void CHud::SetMinimapState( int state )
{
	if( IEngineStudio.IsHardware() )
		gEngfuncs.Cvar_SetValue( "_cl_minimap", state );
	else
		GetMinimapState();
}

void CHud::PlaySoundOnChan( char *name, float fVol, int chan )
{
	gEngfuncs.pfnPlaySoundByName( name, fVol );
}

void CHud::InitMapBounds( void )
{
	m_iMapWidth = ScreenWidth * 0.625f;
	m_iMapHeight = ScreenHeight * 0.625f;
	m_iMapX = ScreenWidth / 2 - ( ScreenWidth * 0.625f ) / 2;
	m_iMapY = ScreenHeight / 2 - ( ScreenHeight * 0.625f ) / 2;
	m_iSmallMapWidth = ScreenWidth * 0.24f;
	m_iSmallMapHeight = ScreenHeight * 0.24f;
	m_iSmallMapX = ScreenWidth - ScreenWidth * 0.24 - ScreenWidth / 640.0f + ScreenWidth / 640.0f + 0.5f;
	m_iSmallMapY = ScreenHeight / 480.0f + ScreenHeight / 480.0f + 0.5f;
}

void CHud::GetMapBounds( int &x, int &y, int &w, int &h )
{
	if( !IEngineStudio.IsHardware() || this->m_iFOV != 90 )
	{
		h = 0;
		w = 0;
		y = 0;
		x = 0;
		return;
	}

	if( GetMinimapState() == 1 )
	{
		x = m_iMapX;
		y = m_iMapY;
		w = m_iMapWidth;
		h = m_iMapHeight;
		return;
	}

	if( GetMinimapState() != 2 )
	{
		h = 0;
		w = 0;
		y = 0;
		x = 0;
		return;
	}

	x = m_iSmallMapX;
	y = m_iSmallMapY;

	if( g_iUser1 )
		y = ScreenHeight / 480.0f * 54.0f + 0.5f + m_iSmallMapY;

	w = m_iSmallMapWidth;
	h = m_iSmallMapHeight;
}

void CHud::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{
	gHUD.m_VGUI2Print.VGUI2HudPrint( charMsg, x, y, r, g, b );
}

bool CHud::IsInMGDeploy( void )
{
	if( g_iUser3 == 2 || g_iVuser1x == 2 )
		return true;

	return false;
}

bool CHud::IsProneDeployed( void )
{
	return ( g_iUser3 == 2 );
}

bool CHud::IsSandbagDeployed( void )
{
	return ( g_iVuser1x == 2 );
}

bool CHud::IsProne( void )
{
	if( g_iUser3 == 1 || g_iUser3 == 0 )
		return false;

	return true;
}

bool CHud::IsDucking( void )
{
	return ( gHUD.m_iKeyBits & IN_BACK );
}

extern int g_iDeadFlag;

bool CHud::IsInMortarDeploy( void )
{
	if( !g_iDeadFlag && g_iUser3 == 3 )
		return true;

	return false;
}

void CHud::SetMortarDeployTime( void )
{
	m_fMortarDeployTime = gEngfuncs.GetClientTime();
}

float CHud::GetMortarDeployTime( void )
{
	return m_fMortarDeployTime;
}

void CHud::SetMortarUnDeployTime( void )
{
	m_fMortarUnDeployTime = gEngfuncs.GetClientTime();
}

float CHud::GetMortarUnDeployTime( void )
{
	return m_fMortarUnDeployTime;
}

void CHud::PostMortarValue( float value )
{
	// Nothing.
}

extern int g_iWeaponFlags;

void CHud::GetWeaponRecoilAmount( int weaponId, float &flPitchRecoil, float &flYawRecoil )
{
	flPitchRecoil = 0.0f;
	flYawRecoil = 2.0f;

	switch( weaponId )
	{
	case WEAPON_COLT:
	case WEAPON_LUGER:
	case WEAPON_M1CARBINE:
	case WEAPON_WEBLEY:
		flPitchRecoil = 1.4f;
		flYawRecoil = 0.35f;
		break;
	case WEAPON_GARAND:
	case WEAPON_KAR:
		flPitchRecoil = 8.0f;
		flYawRecoil = 2.0f;
		break;
	case WEAPON_SCOPEDKAR:
		flPitchRecoil = 6.0f;
		flYawRecoil = 1.5f;
		break;
	case WEAPON_THOMPSON:
	case WEAPON_GREASEGUN:
		flPitchRecoil = 2.15f;
		flYawRecoil = 0.5375f;
		break;

	case WEAPON_MP44:
		flPitchRecoil = 5.0f;
		flYawRecoil = 1.25f;
		break;
	case WEAPON_SPRING:
		flPitchRecoil = 5.6f;
		flYawRecoil = 1.4f;
		break;
	case WEAPON_BAR:
	case WEAPON_BREN:
		flPitchRecoil = 6.72f;
		flYawRecoil = 1.3f;
		break;
	case WEAPON_MP40:
	case WEAPON_STEN:
		flPitchRecoil = 2.2f;
		flYawRecoil = 0.55f;
		break;
	case WEAPON_MG42:
	case WEAPON_CAL30:
	case WEAPON_MG34:
		flPitchRecoil = 20.0f;
		flYawRecoil = 5.0f;
		break;
	case WEAPON_FG42:
		flPitchRecoil = 5.3f;
		flYawRecoil = 1.325f;
		break;
	case WEAPON_K43:
		flPitchRecoil = 7.0f;
		flYawRecoil = 1.75f;
		break;
	case WEAPON_ENFIELD:
		if( ( g_iWeaponFlags & WPNSTATE_SCOPED ) != 0 )
		{
			flPitchRecoil = 6.0f;
			flYawRecoil = 1.5f;
		}
		else
		{
			flPitchRecoil = 8.0f;
			flYawRecoil = 2.0f;
		}
		break;
	case WEAPON_BAZOOKA:
	case WEAPON_PSCHRECK:
	case WEAPON_PIAT:
		flPitchRecoil = 10.0f;
		flYawRecoil = 2.5f;
		break;
	default:
		flPitchRecoil = 0.0f;
		flYawRecoil = 0.0f;
		flYawRecoil = 0.25f * flPitchRecoil;

		if( weaponId == WEAPON_BAR || weaponId == WEAPON_BREN )
			flYawRecoil = 1.3f;

		break;
	}

	if( ( gHUD.IsProneDeployed() || gHUD.IsSandbagDeployed() )
		&& ( weaponId == WEAPON_BAR || weaponId == WEAPON_MG42 || weaponId == WEAPON_MG34
			|| weaponId == WEAPON_FG42 || weaponId == WEAPON_BREN ) )
	{
		flPitchRecoil = 0.0f;
		flYawRecoil = 0.0f;
	}
	else
	{
		float fl = 1.0f;

		if( !gHUD.IsProne() || weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_MG34 )
		{
			if( ( gHUD.m_iKeyBits & IN_DUCK ) != 0 )
				fl = 0.5f;
			else
				return;
		}
		else
		{
			fl = 0.25f;
		}

		flPitchRecoil *= fl;
		flYawRecoil *= fl;
	}
}

void CHud::DoRecoil( int weapon_id )
{
	float flPitchRecoil = 0.0f;
	float flYawRecoil = 0.0f;

	GetWeaponRecoilAmount( weapon_id, flPitchRecoil, flYawRecoil );
	SetRecoilAmount( flPitchRecoil, flYawRecoil );
}

void CHud::SetRecoilAmount( float flPitchRecoil, float flYawRecoil )
{
	m_flPitchRecoilAccumulator = flPitchRecoil;

	float flRandomYaw = gEngfuncs.pfnRandomFloat( 0.8f, 1.1f ) * flYawRecoil;

	if( gEngfuncs.pfnRandomLong( 0, 1 ) > 0 )
		flRandomYaw = -flRandomYaw;

	m_flYawRecoilAccumulator = flRandomYaw;

	m_flRecoilTimeRemaining = 0.1f;
}

void CHud::PopRecoil( float frametime, float &flPitchRecoil, float &flYawRecoil )
{
	float flRecoilProportion;

	if( m_flRecoilTimeRemaining <= 0.0f )
	{
		flPitchRecoil = 0.0f;
		flYawRecoil = 0.0f;
	}
	else
	{
		if( frametime <= m_flRecoilTimeRemaining )
			m_flRecoilTimeRemaining = frametime;

		flRecoilProportion = m_flRecoilTimeRemaining / 0.1f;
		flPitchRecoil = m_flPitchRecoilAccumulator * flRecoilProportion;
		flYawRecoil = m_flYawRecoilAccumulator * flRecoilProportion;
		m_flRecoilTimeRemaining -= frametime;
	}
}
