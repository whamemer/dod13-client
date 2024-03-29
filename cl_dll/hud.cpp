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
#if USE_VGUI
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#endif

#include "demo.h"
#include "demo_api.h"

#include "dod_shared.h"
#include "pm_defs.h"

#include "r_studioint.h"
#include "r_efx.h"

#include "event_api.h"

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
team_info_t		g_TeamInfo[MAX_TEAMS + 1];
pmodel_fx_t g_PModelFxInfo[MAX_TEAMS + 1];
int		g_IsSpectator[MAX_PLAYERS+1];
int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;
int g_iVuser1x = 0;

extern engine_studio_api_t IEngineStudio;

static cvar_t *violence_hblood;

#if USE_VGUI
#include "vgui_ScorePanel.h"

class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
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

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;
#endif

cvar_t *hud_textmode;
float g_hud_text_color[3];

extern client_sprite_t *GetSpriteList( client_sprite_t *pList, const char *psz, int iRes, int iCount );

extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;
cvar_t *cl_viewbob = NULL;

void ShutdownInput( void );

//DECLARE_MESSAGE( m_Logo, Logo )
int __MsgFunc_Logo( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Logo( pszName, iSize, pbuf );
}

//DECLARE_MESSAGE( m_Logo, Logo )
int __MsgFunc_ResetHUD( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ResetHUD( pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}

int __MsgFunc_BloodPuff( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_BloodPuff( pszName, iSize, pbuf );
}

int __MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_CurMarker( pszName, iSize, pbuf );
}

int __MsgFunc_Frags( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Frags( pszName, iSize, pbuf );
}

int __MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_HLTV( pszName, iSize, pbuf );
}

int __MsgFunc_HandSignal( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_HandSignal( pszName, iSize, pbuf );
}

int __MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_MapMarker( pszName, iSize, pbuf );
}

int __MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ObjScore( pszName, iSize, pbuf );
}

int __MsgFunc_PClass( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PClass( pszName, iSize, pbuf );
}

int __MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PStatus( pszName, iSize, pbuf );
}

int __MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PTeam( pszName, iSize, pbuf );
}

int __MsgFunc_RoundState( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_RoundState( pszName, iSize, pbuf );
}

int __MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ScoreInfoLong( pszName, iSize, pbuf );
}

int __MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ScoreShort( pszName, iSize, pbuf );
}

int __MsgFunc_TimeLeft( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_TimeLeft( pszName, iSize, pbuf );
}

int __MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_UseSound( pszName, iSize, pbuf );
}

int __MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_WaveStatus( pszName, iSize, pbuf );
}

int __MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_WaveTime( pszName, iSize, pbuf );
}

int __MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_WideScreen( pszName, iSize, pbuf );
}

int __MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_YouDied( pszName, iSize, pbuf );
}

// TFFree Command Menu
void __CmdFunc_OpenCommandMenu( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
#endif
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
#endif
}

void __CmdFunc_CloseCommandMenu( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
#endif
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
#endif
}

void __CmdFunc_HideCommandMenu( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
#endif
}

void __CmdFunc_ShowCommandMenu( void )
{
#if USE_VGUI
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
#endif
}

// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
			return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
#endif
	return 0;
}

int __MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
#endif
	return 0;
}

int __MsgFunc_Feign( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
#endif
	return 0;
}

int __MsgFunc_Detpack( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
#endif
	return 0;
}

int __MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
#endif
	return 0;
}

#if USE_VGUI && !USE_NOVGUI_MOTD
int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}
#endif

int __MsgFunc_BuildSt( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
#endif
	return 0;
}

int __MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
#endif
	return 0;
}
 
int __MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
#endif
	return 0;
}

#if USE_VGUI && !USE_NOVGUI_SCOREBOARD
int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}
#endif

int __MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
#endif
	return 0;
}

#if USE_VGUI
int __MsgFunc_SpecFade(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_SpecFade( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ResetFade(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ResetFade( pszName, iSize, pbuf );
	return 0;

}
#endif

int __MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{
#if USE_VGUI
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
#endif
	return 0;
}
 
// This is called every time the DLL is loaded
void CHud::Init( void )
{
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( YouDied );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( HLTV );
	HOOK_MESSAGE( Concuss );
	HOOK_MESSAGE( BloodPuff );
	HOOK_MESSAGE( HandSignal );
	HOOK_MESSAGE( UseSound );

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );

#if USE_VGUI && !USE_NOVGUI_MOTD
	HOOK_MESSAGE( MOTD );
#endif

#if USE_VGUI && !USE_NOVGUI_SCOREBOARD
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( ScoreInfoLong );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );
#endif

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );

#if USE_VGUI
	HOOK_MESSAGE( SpecFade );
	HOOK_MESSAGE( ResetFade );
	HOOK_MESSAGE( MapMarker );
#endif

	// VGUI Menus
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

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls whether or not to suicide immediately on TF class switch
	hud_takesshots = CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	hud_textmode = CVAR_CREATE ( "hud_textmode", "0", FCVAR_ARCHIVE );
	max_rubble = CVAR_CREATE( "max_rubble", "240", FCVAR_ARCHIVE );
	cl_corpsestay = CVAR_CREATE( "cl_corpsestay", "10", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmsmallmap", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowmarkers", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowplayers", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowflags", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowobjects", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_dmshowgrenades", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_numshotrubble", "5", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_weatherdis", "1700", FCVAR_ARCHIVE );

	m_iLogo = 0;
	m_iFOV = 0;
	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;
	m_iRoundState = 1;

	_cl_minimap = CVAR_CREATE( "_cl_minimap", "2", FCVAR_ARCHIVE | FCVAR_EXTDLL );
	_cl_minimapzoom = CVAR_CREATE( "_cl_minimapzoom", "1", FCVAR_ARCHIVE );
	zoom_sensitivity_ratio = CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_autowepswitch", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	_ah = CVAR_CREATE( "_ah", "1", FCVAR_ARCHIVE | FCVAR_EXTDLL );
	default_fov = CVAR_CREATE( "default_fov", "90", FCVAR_ARCHIVE );
	cl_hudfont = CVAR_CREATE( "cl_hudfont", "1", FCVAR_ARCHIVE );
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	cl_viewbob = CVAR_CREATE( "cl_viewbob", "0", FCVAR_ARCHIVE );

	m_pSpriteList = NULL;

	// Clear any old HUD list
	if( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	i_Recoil = 0;
	m_iWaterLevel = 0;
	m_flTime = 1.0;

	m_Scope.Init();
	m_Icons.Init();
	m_DoDMap.Init();
	m_ObjectiveIcons.Init();
	m_PShooter.Init();
	m_CEnvModel.Init();
	m_Weather.Init();
	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_DoDCrossHair.Init();
	m_MortarHud.Init();
	m_VGUI2Print.Init();
#if USE_VGUI
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);
#endif

#if !USE_VGUI || USE_NOVGUI_MOTD
	m_MOTD.Init();
#endif
#if !USE_VGUI || USE_NOVGUI_SCOREBOARD
	m_Scoreboard.Init();
#endif

	m_Menu.Init();

	MsgFunc_ResetHUD( 0, 0, NULL );

	m_iSensLevel = 0;
	m_szTeamNames[0][0] = 0;
	g_RubbleQueue.m_flDuration = 2.0f;
	
	char *teamallies = CHudTextMessage::BufferedLocaliseTextString( "#Teamname_allies" );
	strcpy( m_szTeamNames[1], teamallies );
	char *teamaxis = CHudTextMessage::BufferedLocaliseTextString( "#Teamname_axis" );
	strcpy( m_szTeamNames[2], teamaxis );
	char *teamspectators = CHudTextMessage::BufferedLocaliseTextString( "#Teamname_spectators" );
	strcpy( m_szTeamNames[3], teamspectators );
	char *teambritish = CHudTextMessage::BufferedLocaliseTextString( "#Teamname_british" );
	strcpy( m_szTeamNames[4], teambritish );

	gHUD.InitMapBounds();
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
	int j;
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo( &m_scrinfo );

	// ----------
	// Load Sprites
	// ---------
	//m_hsprFont = LoadSprite("sprites/%d_font.spr");

	m_hsprLogo = 0;	
	m_hsprCursor = 0;

	if( ScreenWidth < 640 )
		m_iRes = 320;
	else
		m_iRes = 640;

	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;
	m_flPlaySprintSoundTime = 0.0f;
	m_fRoundEndsTime = 0.0f;
	m_iFOV = 0;
	g_lastFOV = 0;
	m_flPitchRecoilAccumulator = 0.0f;
	m_flRecoilTimeRemaining = 0.0f;

	// Only load this once
	if( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList( "sprites/hud.txt", &m_iSpriteCountAllRes );

		if( m_pSpriteList )
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			for( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			// allocated memory for sprite handle arrays
 			m_rghSprites = new HSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;
			for( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf( sz, "sprites/%s.spr", p->szSprite );
					m_rghSprites[index] = SPR_Load( sz );
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					index++;
				}

				p++;
			}
		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;

		// count the number of sprites of the appropriate res
		m_iSpriteCount = 0;
		for( j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if( p->iRes == m_iRes )
				m_iSpriteCount++;
			p++;
		}

		delete[] m_rghSprites;
		delete[] m_rgrcRects;
		delete[] m_rgszSpriteNames;

		// allocated memory for sprite handle arrays
 		m_rghSprites = new HSPRITE[m_iSpriteCount];
		m_rgrcRects = new wrect_t[m_iSpriteCount];
		m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

		p = m_pSpriteList;
		int index = 0;
		for( j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rghSprites[index] = SPR_Load( sz );
				m_rgrcRects[index] = p->rc;
				strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	if( m_HUD_number_0 == -1 )
	{
		const char *msg = "There is something wrong with your game data! Please, reinstall\n";

		if( HUD_MessageBox( msg ) )
		{
			gEngfuncs.pfnClientCmd( "quit\n" );
		}

		return;
	}

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;
	//m_iFontEngineHeight = vgui2::surface( engineFont ) + 252;

	m_Ammo.VidInit();
	m_Scope.VidInit();
	m_Icons.VidInit();
	m_DoDMap.VidInit();
	m_ObjectiveIcons.VidInit();
	m_CEnvModel.VidInit();
	m_PShooter.VidInit();
	m_Weather.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	m_DoDCrossHair.VidInit();
	m_MortarHud.VidInit();
	m_Spectator.VidInit();
	m_VGUI2Print.VidInit();
#if USE_VGUI
	GetClientVoiceMgr()->VidInit();
#endif
#if !USE_VGUI || USE_NOVGUI_MOTD
	m_MOTD.VidInit();
#endif
#if !USE_VGUI || USE_NOVGUI_SCOREBOARD
	m_Scoreboard.VidInit();
#endif

	int entindex;
	int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

	/*do
	{
		iTeam->frags = 3;
		++iTeam;
	} while ( iTeam != g_PlayerExtraInfo[MAX_PLAYERS+1].teamnumber );*/

	m_iSensLevel = 0;
	memset( g_PModelFxInfo, 0, sizeof( g_PModelFxInfo ) );
}

int CHud::MsgFunc_Logo( const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

float g_lastFOV = 0.0;

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

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );

	// Terminate it
	out[len] = 0;
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

int CHud::MsgFunc_SetFOV( const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT( "default_fov" );

	g_lastFOV = newfov;

	if( newfov == 0 )
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if( m_iFOV == def_fov )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}

void CHud::AddHudElem( CHudBase *phudelem )
{
	HUDLIST *pdl, *ptemp;

	//phudelem->Think();

	if( !phudelem )
		return;

	pdl = (HUDLIST *)malloc( sizeof(HUDLIST) );
	if( !pdl )
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

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}

void CHud::GetAllPlayersInfo()
{
	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		GetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( g_PlayerInfoList[i].thisplayer )
		{
#if USE_VGUI
			if(gViewPort)
				gViewPort->m_pScoreBoard->m_iPlayerNum = i;
#endif
#if !USE_VGUI || USE_NOVGUI_SCOREBOARD
			m_Scoreboard.m_iPlayerNum = i;  // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
#endif
		}
	}
}

void CHud::GetWeaponRecoilAmount( int weapon_id, float *flPitchRecoil, float *flYawRecoil )
{
	*flPitchRecoil = 0.0f;
	*flYawRecoil = 2.0f;

	bool b;

	switch( weapon_id )
	{
		case WEAPON_COLT:
		case WEAPON_LUGER:
		case WEAPON_SCOPEDKAR:
			*flPitchRecoil = 6.0f;
			m_flPitchRecoilAccumulator = 1.5f;

		case WEAPON_THOMPSON:
		case WEAPON_MP44:
			*flPitchRecoil = 5.0f;
			m_flPitchRecoilAccumulator = 1.25f;

		case WEAPON_SPRING:
			*flPitchRecoil = 5.6f;
			m_flPitchRecoilAccumulator = 1.4f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_KAR:
			*flPitchRecoil = 8.0f;
			m_flPitchRecoilAccumulator = 2.0f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_BAR:
		case WEAPON_MP40:
		case WEAPON_MG42:
		case WEAPON_CAL30:
		case WEAPON_M1CARBINE:
		case WEAPON_MG34:
			*flPitchRecoil = 20.0f;
			m_flPitchRecoilAccumulator = 5.0f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_GREASEGUN:
			*flPitchRecoil = 2.15f;
			m_flPitchRecoilAccumulator = 0.5f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_FG42:
			*flPitchRecoil = 5.3f;
			m_flPitchRecoilAccumulator = 1.325f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_K43:
			*flPitchRecoil = 7.0f;
			m_flPitchRecoilAccumulator = 1.75f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_ENFIELD:
			if( ( g_iWeaponFlags & 1 ) != 0 )
			{
				*flPitchRecoil = 6.0f;
				m_flPitchRecoilAccumulator = 1.5f;
				b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;
			}
			else
			{
				*flPitchRecoil = 8.0f;
				m_flPitchRecoilAccumulator = 2.0f;
				b = false;
			}

			*flYawRecoil = m_flPitchRecoilAccumulator;

			if( ( g_iUser3 == OBS_CHASE_FREE || g_iVuser1x == OBS_CHASE_FREE ) && ( ( weapon_id - WEAPON_MG42 ) <= 1 || weapon_id == WEAPON_FG42 || b || weapon_id == WEAPON_MG34 ) )
			{
				*flPitchRecoil = 0.0f;
				*flYawRecoil = 0.0f;
			}
			else
			{
				if( ( g_iUser3 - OBS_CHASE_LOCKED ) > 1 || weapon_id == WEAPON_MG42 || weapon_id == WEAPON_CAL30 || weapon_id == WEAPON_MG34 )
				{
					if( ( gHUD.m_iKeyBits & 4 ) == 0 )
						return;

					m_flYawRecoilAccumulator = 0.5f; 
				}
				else
				{
					m_flYawRecoilAccumulator = 0.25f;
				}
				*flPitchRecoil = *flPitchRecoil * m_flYawRecoilAccumulator;
				*flYawRecoil = *flYawRecoil * m_flYawRecoilAccumulator;
			}
			return;

		case WEAPON_STEN:
			*flPitchRecoil = 2.2f;
			m_flPitchRecoilAccumulator = 0.55f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_BREN:
			*flPitchRecoil = 6.72f;
			m_flPitchRecoilAccumulator = 1.3f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_WEBLEY:
			*flPitchRecoil = 1.4f;
			m_flPitchRecoilAccumulator = 0.35f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		case WEAPON_BAZOOKA:
		case WEAPON_PSCHRECK:
		case WEAPON_PIAT:
			*flPitchRecoil = 10.0f;
			m_flPitchRecoilAccumulator = 2.5f;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;

		default:
			*flPitchRecoil = 0.0f;
			*flYawRecoil = 0.0f;
			m_flPitchRecoilAccumulator = 0.25f * *flPitchRecoil;
			b = weapon_id == WEAPON_BAR || weapon_id == WEAPON_BREN;
			*flYawRecoil = m_flPitchRecoilAccumulator;

			if( b )
				m_flPitchRecoilAccumulator = 1.3f;
			
			*flYawRecoil = m_flPitchRecoilAccumulator;

			if( ( g_iUser3 == OBS_CHASE_FREE || g_iVuser1x == OBS_CHASE_FREE ) && ( ( weapon_id - WEAPON_MG42 ) <= 1 || weapon_id == WEAPON_FG42 || b || weapon_id == WEAPON_MG34 ) )
			{
				*flPitchRecoil = 0.0f;
				*flYawRecoil = 0.0f;
			}
			else
			{
				if( ( g_iUser3 - OBS_CHASE_LOCKED ) > 1 || weapon_id == WEAPON_MG42 || weapon_id == WEAPON_CAL30 || weapon_id == WEAPON_MG34 )
				{
					if( ( gHUD.m_iKeyBits & 4 ) == 0 )
						return;

					m_flYawRecoilAccumulator = 0.5f; 
				}
				else
				{
					m_flYawRecoilAccumulator = 0.25f;
				}
				*flPitchRecoil = *flPitchRecoil * m_flYawRecoilAccumulator;
				*flYawRecoil = *flYawRecoil * m_flYawRecoilAccumulator;
			}
			return;
	}
}

void CHud::DoRecoil( int weapon_id )
{
	float flPitchRecoil, flYawRecoil[4];
	float flRecoil;

	flPitchRecoil = m_flPitchRecoilAccumulator;

	flRecoil = gEngfuncs.pfnRandomFloat( 0.8f, 1.1f ) * flYawRecoil[0];

	CHud::GetWeaponRecoilAmount( weapon_id, &flPitchRecoil, flYawRecoil );

	if( gEngfuncs.pfnRandomLong( 0, 1 ) > 0 )
		flRecoil = -flRecoil;

	gHUD.VidInit();
	
	m_flRecoilTimeRemaining = 0.1f;
}

int CHud::GetCurrentWeaponId( void )
{
	return gHUD.m_Ammo.GetCurrentWeaponId();
}

void CHud::GetMapBounds( int *x, int *y, int *w, int *h )
{
	if( !IEngineStudio.IsHardware() || m_iFOV != 90 )
	{
		*h = 0;
		*w = 0;
		*y = 0;
		*x = 0;

		return;
	}

	if( GetMinimapState() == 1 )
	{
		*x = m_iMapX;
		*y = m_iMapY;
		*w = m_iMapWidth;
		*h = m_iMapHeight;
	}

	if( GetMinimapState() != 2 )
	{
		*h = 0;
		*w = 0;
		*y = 0;
		*x = 0;

		return;
	}

	*x = m_iSmallMapX;
	*y = m_iSmallMapY;

	if( g_iUser1 )
	{
		*y = gHUD.m_scrinfo.iHeight / 480.0f * 0.54f + 0.5f + m_iSmallMapY;
	}

	*w = m_iSmallMapWidth;
	*h = m_iSmallMapHeight;
}

int CHud::GetMinimapState( void )
{
	if( IEngineStudio.IsHardware() && m_iFOV == 90 )
	{
		SetMinimapState( 12 );
		return 1;
	}
	else
		return 0;
}

int CHud::GetMinimapZoomLevel( void )
{
	return _cl_minimapzoom->value;
}

float CHud::GetMortarDeployTime( void )
{
	SetMortarDeployTime();

	return 1.0f;
}

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

char *CHud::GetTeamName( int team )
{
	if( team == 1 )
		return m_szTeamNames[1];

	if( team != 2 )
		return m_szTeamNames[3];

	return m_szTeamNames[2];
}

int CHud::GetWaterLevel( void )
{
	return m_iWaterLevel;
}

void CHud::InitMapBounds( void )
{
	int iWidth, iHeight;

	iWidth = gHUD.m_scrinfo.iWidth;
	iHeight = gHUD.m_scrinfo.iHeight;

	m_iMapWidth = iWidth * 0.625;
	m_iMapHeight = iHeight * 0.625;
	m_iMapX = iWidth / 2 - m_iMapWidth / 2;
	m_iMapY = iHeight / 2 - m_iMapHeight / 2;
	m_iSmallMapWidth = iWidth * 0.24;
	m_iSmallMapHeight = iHeight * 0.24;
	m_iSmallMapX = iWidth - m_iSmallMapWidth - iWidth / 640.0 + iWidth / 640.0 + 0.5;
	m_iSmallMapY = iHeight / 480.0 + iHeight / 480.0 + 0.5;
}

bool CHud::IsDucking( void )
{
	return ( gHUD.m_iKeyBits & 4 ) != false;
}

bool CHud::IsInMGDeploy( void )
{
	if( g_iUser3 != OBS_CHASE_FREE )
		return g_iVuser1x == OBS_CHASE_FREE;
	return true;
}

bool CHud::IsInMortarDeploy( void )
{
	if( !g_iDeadFlag )
		return g_iUser3 == OBS_ROAMING;

	return false;
}

bool CHud::IsProne( void )
{
	return ( g_iUser3 - 1 ) <= OBS_CHASE_LOCKED;
}

bool CHud::IsProneDeployed( void )
{
	return g_iUser3 == OBS_CHASE_FREE;
}

bool CHud::IsSandbagDeployed( void )
{
	return g_iVuser1x == OBS_CHASE_FREE;
}

bool CHud::IsTeamPara( int team )
{
	if( team == 1 )
		return m_bAllieParatrooper;
	else
		return m_bAxisParatrooper;;
}

int CHud::MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{

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

int CHud::MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( READ_BYTE() )
		PlaySound( "common/wpn_select.wav", 1 );
	else
		PlaySound( "common/wpn_denyselect.wav", 1 );
	
	return 1;
}

void CHud::PlaySoundOnChan( char *name, float fVol, int chan )
{
	PlaySound( name, fVol );
}

void CHud::PopRecoil( float frametime, float *flPitchRecoil, float *flYawRecoil )
{
	if( m_flRecoilTimeRemaining < 0.0f )
	{
		*flPitchRecoil = 0.0f;
		*flYawRecoil = 0.0f;
	}
	else
	{
		if( frametime < m_flRecoilTimeRemaining )
			m_flRecoilTimeRemaining = frametime;
		
		float flRecoilProportion = m_flRecoilTimeRemaining / 0.1;
		*flPitchRecoil = m_flPitchRecoilAccumulator * flRecoilProportion;
		*flYawRecoil = m_flYawRecoilAccumulator * flRecoilProportion;
		m_flRecoilTimeRemaining = m_flRecoilTimeRemaining - frametime;
	}
}

void CHud::PostMortarValue( float value )
{
	// TODO: WHAMER: need vgui2 :(
}

void CHud::SetMinimapState( int state )
{
	if( IEngineStudio.IsHardware() )
		gEngfuncs.Cvar_SetValue( "_cl_minimap", state );
	else
		SetMinimapState( 0 );
}

void CHud::SetMortarDeployTime( void )
{
	gEngfuncs.GetClientTime();
}

void CHud::SetMortarUnDeployTime( void )
{
	gEngfuncs.GetClientTime();
}

void CHud::SetRecoilAmount( float flPitchRecoil, float flYawRecoil )
{
	m_flPitchRecoilAccumulator = flPitchRecoil;

	float fl = gEngfuncs.pfnRandomFloat( 0.8f, 1.1f ) * flYawRecoil;

	if( gEngfuncs.pfnRandomLong( 0, 1 ) > 0 )
		fl = -fl;

	DoRecoil( fl );
	m_flRecoilTimeRemaining = 0.1f;
}

void CHud::SetWaterLevel( int level )
{
	m_iWaterLevel = level;
}

void CHud::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{
	// TODO: WHAMER: need vgui2 :(
}

int CHud::ZoomMinimap( void )
{
	if( _cl_minimapzoom->value + 1.0f > 3.0f )
		_cl_minimapzoom->value = 0.0f;

	return _cl_minimapzoom->value;
}

void ClientSetSensitivity( int level )
{
	gHUD.m_iSensLevel = level;
}

void EV_BloodPuff( float *org )
{
	vec3_t origin, velocity, to_view, closerOrigin;
	struct event_args_s *args;

	VectorCopy( args->origin, origin );
	VectorCopy( args->velocity, velocity );

	origin = org;

	double m_dNormalize = VectorNormalize( origin );
	
	float fl;

	if( m_dNormalize > 32.0 )
	{
		if( m_dNormalize < 1200.0 )
		{
			if( m_dNormalize < 600.0 )
			{
				fl = 1.0f;
			}
			else
			{
				fl = ( ( m_dNormalize - 600.0 ) / 600.0 * 0.5 + 1.0 );
			}
		}
		else
		{
			fl = 1.5;
		}

		float flRandsd = fl * gEngfuncs.pfnRandomFloat( 0.7, 0.8 );

		TEMPENTITY *pShotDust;
		
		pShotDust = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, flRandsd, gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" ), kRenderTransAlpha, kRenderFxNone, 0.15f, 30.0f, FTENT_SPRANIMATE );

		pShotDust->entity.curstate.renderamt = 800;
		pShotDust->entity.curstate.framerate = 45.0f;
		pShotDust->entity.curstate.rendercolor.r = -76;
		pShotDust->entity.curstate.rendercolor.g = -80;
		pShotDust->entity.curstate.rendercolor.b = -108;
		pShotDust->entity.angles.z = gEngfuncs.pfnRandomLong( 0, 90 );

		to_view = origin;
		VectorNormalize( to_view );
		VectorMA( origin, 2.0f, to_view, closerOrigin );

		float flRandbn = fl * gEngfuncs.pfnRandomFloat( 0.4, 0.45 );

		TEMPENTITY *pBloodNarrow;

		pBloodNarrow = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, flRandbn, gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" ), kRenderTransAlpha, kRenderFxNone, 0.35f, 30.0f, FTENT_FADEOUT );

		pBloodNarrow->entity.curstate.renderamt = 800;
		pBloodNarrow->entity.curstate.framerate = 20.0f;
		pBloodNarrow->entity.curstate.rendercolor.r = 120;
		pBloodNarrow->entity.curstate.rendercolor.g = 0;
		pBloodNarrow->entity.curstate.rendercolor.b = 0;
		pBloodNarrow->entity.angles.z = gEngfuncs.pfnRandomLong( 0, 90 );
	}
}

int EV_BloodPuffMsg( const char *pszName, int iSize, void *pbuf )
{
	vec3_t origin;
	
	*origin = READ_COORD();

	if( violence_hblood )
	{
		violence_hblood = gEngfuncs.pfnGetCvarPointer( "violence_hblood" );

		if( !violence_hblood )
			return 1;
	}

	if( violence_hblood->value != 1.0f )
		return 1;
	
	EV_BloodPuff( origin );
}

int EV_HandSignalMsg( const char *pszName, int iSize, void *pbuf )
{
	char sz[256];
	int index = READ_BYTE();

	BEGIN_READ( pbuf, iSize );

	if( index < 64 && index > 0 && index < 25 )
	{
		gEngfuncs.pfnGetPlayerInfo( index, &g_PlayerInfoList[index] );
		sprintf( sz, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[index].name, ": %s2" );

		if( g_PlayerExtraInfo[index].teamId == 2 )
		{
			if( s_HandSignalSubtitles[index][1] )
			{
				/* gHUD.m_SayText.SayTextPrint( sz, 256, index, "#Handsignal", s_HandSignalSubtitles[index][1], 0, 0 );
				&gHUD.m_Spectator.AddVoiceIconToPlayerEnt( index ); */ // TODO: WHAMER: need vgui2 :(
			}
		}
		else if( g_PlayerExtraInfo[index].teamId == 1 && gHUD.m_bBritish )
		{
			if( s_HandSignalSubtitles[index][2] )
			{
				/* gHUD.m_SayText.SayTextPrint( sz, 256, index, "#Handsignal", s_HandSignalSubtitles[index][2], 0, 0 );
				&gHUD.m_Spectator.AddVoiceIconToPlayerEnt( index ); */ // TODO: WHAMER: need vgui2 :(
			}
		}

		/* gHUD.m_SayText.SayTextPrint( sz, 256, index, "#Handsignal", s_HandSignalSubtitles[index][0], 0, 0 );
		&gHUD.m_Spectator.AddVoiceIconToPlayerEnt( index ); */ // TODO: WHAMER: need vgui2 :(
	}

	return 1;
}

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