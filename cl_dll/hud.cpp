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
#include "hud_safearea.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "vgui/vgui_int.h"
#include "vgui/vgui_TeamFortressViewport.h"

#include "demo.h"
#include "demo_api.h"

#include "dod_shared.h"

#include "vgui/idodviewport.h"
#include "vgui/idodviewportmsgs.h"

#include "r_studioint.h"
#include "com_model.h"
#include "event_api.h"

extern engine_studio_api_t IEngineStudio;

int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;
int g_iVuser1x = 0;
int g_iVuser1z = 0;
int g_iMovetype = 0;
int g_iOnlyClientDraw = 0;
int g_iEffects = 0;

float g_lastFOV = 0.0f;

extern cvar_t *cl_weatherdis;
extern cvar_t *cl_numshotrubble;

cvar_t *cl_dmsmallmap;
cvar_t *cl_dmshowmarkers;
cvar_t *cl_dmshowplayers;
cvar_t *cl_dmshowflags;
cvar_t *cl_dmshowobjects;
cvar_t *cl_dmshowgrenades;

float g_fStamina = 0.0f;
int g_iWeaponBits2 = 0;

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll

pmodel_fx_t g_PModelFxInfo[MAX_PLAYERS+1];
team_info_t		g_TeamInfo[MAX_TEAMS + 1];
int		g_IsSpectator[MAX_PLAYERS+1];

#include "vgui/dodclientscoreboard.h"
#include "vgui/dodmenu.h"

// The helper of the voice status for DoD (CDoDVoiceStatusHelper of the original). voice_status of the SDK is used, so the helper
// keeps its interface (UpdateCursorState is not in the one of the original) and its value of GetAckIconHeight is a place on the
// screen: the original returns the distance from the bottom, 3 lines of the font and 6 pixels
class CDoDVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	// the color of the name of the speaker is the color of the team in ClientScheme.res
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex <= MAX_PLAYERS )
			DoD_GetTeamColor( g_PlayerExtraInfo[entindex].teamnumber, color[0], color[1], color[2] );
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - ( 3 * gHUD.m_iFontHeight + 6 );
	}

	// the speakers are not labeled while the scoreboard is shown
	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort )
			return !gViewPort->IsScoreBoardVisible();

		return false;
	}
};
static CDoDVoiceStatusHelper g_VoiceStatusHelper;

cvar_t *hud_textmode;
float g_hud_text_color[3];

extern client_sprite_t *GetSpriteList( client_sprite_t *pList, const char *psz, int iRes, int iCount );

extern cvar_t *sensitivity;

cvar_t *cl_autoreload = NULL;

cvar_t *cl_viewbob = NULL;

void ShutdownInput( void );

int EV_BloodPuffMsg( const char *pszName, int iSize, void *pbuf );
void EV_BloodPuff( float *org );
int EV_HandSignalMsg( const char *pszName, int iSize, void *pbuf );

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

int __MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_YouDied( pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_Damage( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Damage( pszName, iSize, pbuf );
}

int __MsgFunc_SetFOV( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_HLTV( pszName, iSize, pbuf );
}

int __MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_UseSound( pszName, iSize, pbuf );
}

int __MsgFunc_RoundState( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_RoundState( pszName, iSize, pbuf );
}

int __MsgFunc_TimeLeft( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_TimeLeft( pszName, iSize, pbuf );
}

int __MsgFunc_BloodPuff( const char *pszName, int iSize, void *pbuf )
{
	return EV_BloodPuffMsg( pszName, iSize, pbuf );
}

int __MsgFunc_HandSignal( const char *pszName, int iSize, void *pbuf )
{
	return EV_HandSignalMsg( pszName, iSize, pbuf );
}

// the command menu of commandmenu.txt (IViewPort::ShowCommandMenu and HideCommandMenu of the original)
void __CmdFunc_ShowCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

void __CmdFunc_HideCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	bool finished = gHUD.m_MOTD.HandleMOTDMessage(pszName, iSize, pbuf);
	if (finished)
	{
		if (gHUD.UseVguiMOTD() && gViewPort)
		{
			gViewPort->ShowMOTD();
			return 1;
		}

		gHUD.m_MOTD.m_bShow = true;
	}
	return 1;
}

int __MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_ScoreInfo( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_ScoreInfoLong( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_TeamScore( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_MapMarker( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_WaveTime( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_WaveStatus( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_WideScreen( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_ScoreShort( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_Frags( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_Frags( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_ObjScore( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_PStatus( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_PClass( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_PClass( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_PTeam( pszName, iSize, pbuf );

	return 0;
}

int __MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf )
{
	if( gDoDViewPortMsgs )
		return gDoDViewPortMsgs->MsgFunc_CurMarker( pszName, iSize, pbuf );

	return 0;
}

// This is called every time the DLL is loaded
void CHud::Init( void )
{
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( YouDied );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( Damage );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( HLTV );
	HOOK_MESSAGE( BloodPuff );
	HOOK_MESSAGE( HandSignal );
	HOOK_MESSAGE( UseSound );

	HOOK_COMMAND( "+commandmenu", ShowCommandMenu );
	HOOK_COMMAND( "-commandmenu", HideCommandMenu );

	// The buttons of the command menu close it with this command
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );

	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( ScoreInfoLong );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );
	HOOK_MESSAGE( MapMarker );

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

	hud_takesshots = CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	max_rubble = CVAR_CREATE( "max_rubble", "240", 0 );
	cl_corpsestay = CVAR_CREATE( "cl_corpsestay", "10", FCVAR_ARCHIVE );
	cl_dmsmallmap = CVAR_CREATE( "cl_dmsmallmap", "1", FCVAR_ARCHIVE );
	cl_dmshowmarkers = CVAR_CREATE( "cl_dmshowmarkers", "1", FCVAR_ARCHIVE );
	cl_dmshowplayers = CVAR_CREATE( "cl_dmshowplayers", "1", FCVAR_ARCHIVE );
	cl_dmshowflags = CVAR_CREATE( "cl_dmshowflags", "1", FCVAR_ARCHIVE );
	cl_dmshowobjects = CVAR_CREATE( "cl_dmshowobjects", "1", FCVAR_ARCHIVE );
	cl_dmshowgrenades = CVAR_CREATE( "cl_dmshowgrenades", "1", FCVAR_ARCHIVE );
	cl_numshotrubble = CVAR_CREATE( "cl_numshotrubble", "5", FCVAR_ARCHIVE );
	cl_weatherdis = CVAR_CREATE( "cl_weatherdis", "1700", FCVAR_ARCHIVE );
	_cl_minimap = CVAR_CREATE( "_cl_minimap", "2", FCVAR_ARCHIVE | FCVAR_USERINFO );
	_cl_minimapzoom = CVAR_CREATE( "_cl_minimapzoom", "1", FCVAR_ARCHIVE );
	zoom_sensitivity_ratio = CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", FCVAR_ARCHIVE );
	_ah = CVAR_CREATE( "_ah", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	cl_hudfont = CVAR_CREATE( "cl_hudfont", "1", FCVAR_ARCHIVE );
	hud_fastswitch = CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress

	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	r_drawentities = gEngfuncs.pfnGetCvarPointer( "r_drawentities" );
	cl_pitchdown = gEngfuncs.pfnGetCvarPointer( "cl_pitchdown" );
	cl_pitchup = gEngfuncs.pfnGetCvarPointer( "cl_pitchup" );
	crosshair = gEngfuncs.pfnGetCvarPointer( "crosshair" );
	developer = gEngfuncs.pfnGetCvarPointer( "developer" );

	m_iFOV = 0;
	m_iLogo = 0;
	m_bAutoReloadComplete = true;

	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;

	m_iRoundState = 1;

	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_autoreload = CVAR_CREATE( "cl_autoreload", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );

	hud_textmode = CVAR_CREATE ( "hud_textmode", "0", FCVAR_ARCHIVE );
	m_pAllowHD = CVAR_CREATE ( "hud_allow_hd", "0", FCVAR_ARCHIVE );
	cl_viewbob = CVAR_CREATE( "cl_viewbob", "1", FCVAR_ARCHIVE );
	m_pCvarMOTDVGUI = CVAR_CREATE("cl_motd_vgui", "1", FCVAR_ARCHIVE);
	m_pCvarScoreboardVGUI = CVAR_CREATE("cl_scoreboard_vgui", "1", FCVAR_ARCHIVE);

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

	m_iClipSize = 0;
	i_Recoil = 0;
	m_iWaterLevel = 0;

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

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
	m_DoDMap.Init();	// the original calls it twice
	m_MortarHud.Init();
	m_VGUI2Print.Init();

	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);

	m_MOTD.Init();
	m_Scoreboard.Init();

	// the safe area of the screen: the elements at an edge are moved from it
	HUD_SafeAreaInit();

	m_Icons.m_iSafeAnchor = SAFE_LEFT | SAFE_BOTTOM;
	m_ObjectiveIcons.m_iSafeAnchor = SAFE_LEFT | SAFE_TOP;
	m_Ammo.m_iSafeAnchor = SAFE_RIGHT | SAFE_BOTTOM;
	m_SayText.m_iSafeAnchor = SAFE_LEFT | SAFE_BOTTOM;
	m_StatusBar.m_iSafeAnchor = SAFE_BOTTOM;
	m_DeathNotice.m_iSafeAnchor = SAFE_RIGHT | SAFE_TOP;
	m_StatusIcons.m_iSafeAnchor = SAFE_LEFT;
	m_Train.m_iSafeAnchor = SAFE_BOTTOM;

	MsgFunc_ResetHUD( 0, 0, NULL );

	m_iSensLevel = 0;
	m_szTeamNames[0][0] = 0;
	g_RubbleQueue.m_flDuration = 2.0f;

	strlcpy( m_szTeamNames[1], CHudTextMessage::BufferedLocaliseTextString( "#Teamname_allies" ), MAX_TEAMNAME_SIZE );
	strlcpy( m_szTeamNames[2], CHudTextMessage::BufferedLocaliseTextString( "#Teamname_axis" ), MAX_TEAMNAME_SIZE );
	strlcpy( m_szTeamNames[3], CHudTextMessage::BufferedLocaliseTextString( "#Teamname_spectators" ), MAX_TEAMNAME_SIZE );
	strlcpy( m_szTeamNames[4], CHudTextMessage::BufferedLocaliseTextString( "#Teamname_british" ), MAX_TEAMNAME_SIZE );

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

	g_RubbleQueue.~Queue();
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

	m_iRoundState = 1;
	m_bAutoReloadComplete = true;

	// a1ba: don't break the loading order here and
	// don't cause memory leak but check
	// maximum HUD sprite resolution we have
	m_iMaxRes = 640;
	client_sprite_t *pSpriteList = m_pSpriteList ? m_pSpriteList :
		SPR_GetList( "sprites/hud.txt", &m_iSpriteCountAllRes );
	if( pSpriteList )
	{
		for( int i = 0; i < m_iSpriteCountAllRes; i++ )
		{
			if( m_iMaxRes < pSpriteList[i].iRes )
				m_iMaxRes = pSpriteList[i].iRes;
		}
	}

	m_iRes = GetSpriteRes( ScreenWidth, ScreenHeight );

	m_bAllieParatrooper = false;
	m_bAllieInfiniteLives = true;
	m_bAxisParatrooper = false;
	m_bAxisInfiniteLives = true;
	m_bParatrooper = false;
	m_bInfiniteLives = true;
	m_bBritish = false;
	m_flPlaySprintSoundTime = 0;
	m_fRoundEndsTime = 0;
	m_iFOV = 0;
	g_lastFOV = 0;
	m_fMortarDeployTime = 0;
	m_flPitchRecoilAccumulator = 0;
	m_flYawRecoilAccumulator = 0;
	m_flRecoilTimeRemaining = 0;

	// Only load this once
	if( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = pSpriteList;

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
					strlcpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );
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
				strlcpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );
	m_MG_number_0 = GetSpriteIndex( "mg_0" );

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
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	m_DoDCrossHair.VidInit();
	m_MortarHud.VidInit();
	GetClientVoiceMgr()->VidInit();
	m_Spectator.VidInit();
	m_VGUI2Print.VidInit();

	m_MOTD.VidInit();
	m_Scoreboard.VidInit();

	m_iSensLevel = 0;
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

	return m_bAxisParatrooper;
}

char *CHud::GetPlayerClassName( int playerclass )
{
	char *result;
	int team;

	// the classes 10..20 are Axis
	team = ( playerclass >= 10 && playerclass <= 20 ) ? 2 : 1;

	if( IsTeamPara( team ) )
		result = g_ParaClassInfo[playerclass].classname;
	else
		result = g_ClassInfo[playerclass].classname;

	return result;
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

int CHud::MsgFunc_SetFOV( const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if( g_iVuser1z )
		return GetFOV();

	int newfov = READ_BYTE();
	int def_fov = 90;

	g_lastFOV = newfov;

	if( newfov == 0 )
		SetFOV( def_fov );
	else
		SetFOV( newfov );

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
		float zoom_sens = zoom_sensitivity_ratio ? zoom_sensitivity_ratio->value : 1.0f;

		m_flMouseSensitivity = zoom_sens * ( (float)newfov / (float)def_fov * sensitivity->value );
	}

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

	m_fRoundEndsTime = m_flTime + READ_SHORT();

	return 1;
}

int CHud::MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int player = READ_BYTE();
	int data = READ_BYTE();

	if( !gEngfuncs.IsSpectateOnly() )
	{
		gEngfuncs.Con_DPrintf( "Warning! Received unexpected HLTV user message.\n" );
		return 1;
	}

	if( data & 0x80 )
	{
		data &= 0x7F;

		if( player )
		{
			g_PlayerExtraInfo[player].health = data;
		}
		else
		{
			for( int i = 0; i < MAX_PLAYERS; i++ )
				g_PlayerExtraInfo[i].health = data;
		}
	}
	else
	{
		if( !data )
			data = 90;

		if( player )
		{
			m_PlayerFOV[player] = data;
		}
		else
		{
			for( int i = 0; i < MAX_PLAYERS; i++ )
				m_PlayerFOV[i] = data;
		}
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

void ClientSetSensitivity( int level )
{
	gHUD.m_iSensLevel = level;
}

float CHud::GetSensitivity( void )
{
	if( m_iSensLevel == 1 )
		return 1.0f;

	return m_flMouseSensitivity;
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

static cvar_t *violence_hblood = NULL;

bool ShouldShowBlood( void )
{
	if( !violence_hblood )
	{
		violence_hblood = gEngfuncs.pfnGetCvarPointer( "violence_hblood" );

		if( !violence_hblood )
			return false;
	}

	return violence_hblood->value == 1.0f;
}

int EV_BloodPuffMsg( const char *pszName, int iSize, void *pbuf )
{
	float origin[3];

	BEGIN_READ( pbuf, iSize );

	origin[0] = READ_COORD();
	origin[1] = READ_COORD();
	origin[2] = READ_COORD();

	if( ShouldShowBlood() )
		EV_BloodPuff( origin );

	return 1;
}

extern vec3_t v_origin;

void EV_BloodPuff( float *org )
{
	vec3_t velocity( 0, 0, 2 );
	vec3_t origin( org );
	int modelindex;
	int lifetime;
	float dustscale;
	float bloodscale;
	float flDist;
	float scale;
	TEMPENTITY *pTemp;
	vec3_t closerOrigin;
	vec3_t to_view;

	modelindex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" );
	lifetime = 30;

	dustscale = gEngfuncs.pfnRandomFloat( 0.7f, 0.8f );
	bloodscale = gEngfuncs.pfnRandomFloat( 0.4f, 0.45f );

	flDist = ( v_origin - origin ).Length();

	// too close to the view, nothing is drawn
	if( flDist >= 32.0f )
	{
		// the farther the puff is the bigger it is, to be seen
		if( flDist > 1200.0f )
			scale = 1.5f;
		else if( flDist > 600.0f )
			scale = ( flDist - 600.0f ) / 600.0f * 0.5f + 1.0f;
		else
			scale = 1.0f;

		pTemp = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, scale * dustscale, modelindex, kRenderTransAlpha, kRenderFxNone, 0.15f, (float)lifetime, FTENT_SPRANIMATE );

		// The temp entities can be exhausted
		if( pTemp )
		{
			pTemp->entity.curstate.renderamt = 800;
			pTemp->entity.curstate.framerate = 45.0f;
			pTemp->entity.curstate.rendercolor.r = 180;
			pTemp->entity.curstate.rendercolor.g = 176;
			pTemp->entity.curstate.rendercolor.b = 148;
			pTemp->entity.angles[2] = (float)gEngfuncs.pfnRandomLong( 0, 90 );
		}

		to_view = v_origin - origin;
		VectorNormalize( to_view );

		// the blood is a bit closer to the view than the dust
		VectorMA( origin, 2.0f, to_view, closerOrigin );

		modelindex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" );

		pTemp = gEngfuncs.pEfxAPI->R_TempSprite( closerOrigin, velocity, bloodscale * scale, modelindex, kRenderTransAlpha, kRenderFxNone, 0.35f, (float)lifetime, FTENT_SPRANIMATE );

		if( pTemp )
		{
			pTemp->entity.curstate.framerate = 20.0f;
			pTemp->entity.curstate.renderamt = 800;
			pTemp->entity.curstate.rendercolor.r = 120;
			pTemp->entity.curstate.rendercolor.g = 0;
			pTemp->entity.curstate.rendercolor.b = 0;
			pTemp->entity.angles[2] = (float)gEngfuncs.pfnRandomLong( 0, 90 );
		}
	}
}

extern char *s_HandSignalSubtitles[25][3];

int EV_HandSignalMsg( const char *pszName, int iSize, void *pbuf )
{
	int entindex;
	int signal;

	BEGIN_READ( pbuf, iSize );

	entindex = READ_BYTE();
	signal = READ_BYTE();

	// the original lets the signal 25 through, there is no such row in the table
	if( entindex >= 1 && entindex <= MAX_PLAYERS && signal < 25 )
	{
		int team;
		char pattern[268];
		char *subtitle;

		team = g_PlayerExtraInfo[entindex].teamId;

		gEngfuncs.pfnGetPlayerInfo( entindex, &g_PlayerInfoList[entindex] );

		sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[entindex].name, ": %s2" );

		// the signals have an own text for the axis and for the british
		subtitle = s_HandSignalSubtitles[signal][0];

		if( team == 2 )
		{
			if( s_HandSignalSubtitles[signal][1] && *s_HandSignalSubtitles[signal][1] )
				subtitle = s_HandSignalSubtitles[signal][1];
		}
		else if( team == 1 && gHUD.m_bBritish )
		{
			if( s_HandSignalSubtitles[signal][2] && *s_HandSignalSubtitles[signal][2] )
				subtitle = s_HandSignalSubtitles[signal][2];
		}

		gHUD.m_SayText.SayTextPrint( pattern, 256, entindex, (char *)"#Handsignal", subtitle, NULL, NULL );
		gHUD.m_Spectator.AddVoiceIconToPlayerEnt( entindex );
	}

	return 1;
}

int CHud::MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int success = READ_BYTE();

	if( success )
		PlaySound( "common/wpn_select.wav", 0.5f );
	else
		PlaySound( "common/wpn_denyselect.wav", 0.5f );

	return 1;
}

char *CHud::GetTeamName( int team )
{
	if( team == 1 )
		return m_szTeamNames[1];

	if( team == 2 )
		return m_szTeamNames[2];

	return m_szTeamNames[3];
}

int CHud::GetMinimapZoomLevel( void )
{
	return (int)_cl_minimapzoom->value;
}

int CHud::ZoomMinimap( void )
{
	_cl_minimapzoom->value = _cl_minimapzoom->value + 1.0f;

	if( _cl_minimapzoom->value >= 3.0f )
		_cl_minimapzoom->value = 0.0f;

	return (int)_cl_minimapzoom->value;
}

int CHud::GetMinimapState( void )
{
	if( IEngineStudio.IsHardware() && m_iFOV == 90 )
		return (int)_cl_minimap->value;

	return 0;
}

void CHud::SetMinimapState( int state )
{
	if( IEngineStudio.IsHardware() )
		gEngfuncs.Cvar_SetValue( "_cl_minimap", (float)state );
	else
		_cl_minimap->value = 0;
}

void CHud::PlaySoundOnChan( char *name, float fVol, int chan )
{
	PlaySound( name, fVol );
}

void CHud::InitMapBounds( void )
{
	m_iMapBoundsWidth = ScreenWidth;
	m_iMapBoundsHeight = ScreenHeight;

	m_iMapWidth = (int)( ScreenWidth * 0.625 );
	m_iMapHeight = (int)( ScreenHeight * 0.625 );
	m_iMapX = ScreenWidth / 2 - m_iMapWidth / 2;
	m_iMapY = ScreenHeight / 2 - m_iMapHeight / 2;

	m_iSmallMapWidth = (int)( ScreenWidth * 0.24 );
	m_iSmallMapHeight = (int)( ScreenHeight * 0.24 );
	m_iSmallMapX = ScreenWidth - m_iSmallMapWidth - (int)( ScreenWidth / 640.0 + ScreenWidth / 640.0 + 0.5 );
	m_iSmallMapY = (int)( ScreenHeight / 480.0 + ScreenHeight / 480.0 + 0.5 );
}

void CHud::GetMapBounds( int &x, int &y, int &w, int &h )
{
	// the screen of the HUD changes without VidInit: hud_scale takes effect at once, and the bounds made for the old size would be in
	// the wrong pixels (the map view is scaled by the ratio of the screen to the HUD)
	if( m_iMapBoundsWidth != ScreenWidth || m_iMapBoundsHeight != ScreenHeight )
		InitMapBounds();

	int state = GetMinimapState();

	if( state == 1 )
	{
		x = m_iMapX;
		y = m_iMapY;
		w = m_iMapWidth;
		h = m_iMapHeight;
	}
	else if( state == 2 )
	{
		x = m_iSmallMapX;
		y = m_iSmallMapY;

		if( g_iUser1 )
			y += (int)( ScreenHeight / 480.0 * 54.0 + 0.5 );

		w = m_iSmallMapWidth;
		h = m_iSmallMapHeight;
	}
	else
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
}

// The rectangle of the map in the pixels of the HUD, the minimap moved from the corner by the safe area. The overlays of the map
// (icons, the kills under it) use GetMapBounds and are moved with it by their anchor.
void CHud::GetMapViewBounds( int &x, int &y, int &w, int &h )
{
	int dx = 0, dy = 0;

	GetMapBounds( x, y, w, h );

	// the big map is in the middle of the screen
	if( GetMinimapState() == 2 )
		HUD_SafeAreaOffset( SAFE_RIGHT | SAFE_TOP, dx, dy );

	x += dx;
	y += dy;
}

void CHud::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{
	m_VGUI2Print.VGUI2HudPrint( charMsg, x, y, r, g, b );
}

extern int g_iWeaponFlags;

bool CHud::IsInMGDeploy( void )
{
	if( g_iUser3 == STANCE_PRONE_DEPLOYED )
		return true;

	return g_iVuser1x == SANDBAG_DEPLOYED;
}

bool CHud::IsProneDeployed( void )
{
	return g_iUser3 == STANCE_PRONE_DEPLOYED;
}

bool CHud::IsSandbagDeployed( void )
{
	return g_iVuser1x == SANDBAG_DEPLOYED;
}

bool CHud::IsProne( void )
{
	return ( unsigned int )( g_iUser3 - 1 ) <= 1;
}

bool CHud::IsDucking( void )
{
	return ( gHUD.m_iKeyBits & IN_DUCK ) != 0;
}

bool CHud::IsInMortarDeploy( void )
{
	if( g_iDeadFlag )
		return false;

	return g_iUser3 == STANCE_MORTAR;
}

// the original keeps both mortar times in m_fMortarDeployTime
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
	m_fMortarDeployTime = gEngfuncs.GetClientTime();
}

float CHud::GetMortarUnDeployTime( void )
{
	return m_fMortarDeployTime;
}

void CHud::PostMortarValue( float value )
{
	if( gDoDViewPortInterface )
		gDoDViewPortInterface->PostMortarValue( value );
}

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
		break;
	case WEAPON_GARAND:
	case WEAPON_KAR:
		flPitchRecoil = 8.0f;
		break;
	case WEAPON_SCOPEDKAR:
		flPitchRecoil = 6.0f;
		break;
	case WEAPON_THOMPSON:
	case WEAPON_GREASEGUN:
		flPitchRecoil = 2.15f;
		break;
	case WEAPON_MP44:
		flPitchRecoil = 5.0f;
		break;
	case WEAPON_SPRING:
		flPitchRecoil = 5.6f;
		break;
	case WEAPON_BAR:
	case WEAPON_BREN:
		flPitchRecoil = 6.72f;
		break;
	case WEAPON_MP40:
	case WEAPON_STEN:
		flPitchRecoil = 2.2f;
		break;
	case WEAPON_MG42:
	case WEAPON_CAL30:
	case WEAPON_MG34:
		flPitchRecoil = 20.0f;
		break;
	case WEAPON_FG42:
		flPitchRecoil = 5.3f;
		break;
	case WEAPON_K43:
		flPitchRecoil = 7.0f;
		break;
	case WEAPON_ENFIELD:
		if( ( g_iWeaponFlags & WPNSTATE_SCOPED ) != 0 )
			flPitchRecoil = 6.0f;
		else
			flPitchRecoil = 8.0f;
		break;
	case WEAPON_BAZOOKA:
	case WEAPON_PSCHRECK:
	case WEAPON_PIAT:
		flPitchRecoil = 10.0f;
		break;
	default:
		flPitchRecoil = 0.0f;
		break;
	}

	flYawRecoil = flPitchRecoil * 0.25f;

	if( weaponId == WEAPON_BAR || weaponId == WEAPON_BREN )
		flYawRecoil = 1.3f;

	if( IsInMGDeploy() && ( weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_FG42
		|| weaponId == WEAPON_BAR || weaponId == WEAPON_BREN || weaponId == WEAPON_MG34 ) )
	{
		flPitchRecoil = 0.0f;
		flYawRecoil = 0.0f;
	}
	else
	{
		float flScale;

		if( !IsProne() || weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_MG34 )
		{
			if( ( gHUD.m_iKeyBits & IN_DUCK ) == 0 )
				return;

			flScale = 0.5f;
		}
		else
		{
			flScale = 0.25f;
		}

		flPitchRecoil *= flScale;
		flYawRecoil *= flScale;
	}
}

void CHud::DoRecoil( int weapon_id )
{
	float flPitchRecoil;
	float flYawRecoil;

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
	float flRemaining = m_flRecoilTimeRemaining;
	float flRecoilProportion;

	if( flRemaining <= 0.0f )
	{
		flPitchRecoil = 0.0f;
		flYawRecoil = 0.0f;
	}
	else
	{
		if( frametime <= flRemaining )
			flRemaining = frametime;

		flRecoilProportion = flRemaining / 0.1f;
		flPitchRecoil = m_flPitchRecoilAccumulator * flRecoilProportion;
		flYawRecoil = flRecoilProportion * m_flYawRecoilAccumulator;
		m_flRecoilTimeRemaining -= frametime;
	}
}

void CHud::GetAllPlayersInfo()
{
	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		GetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( g_PlayerInfoList[i].thisplayer )
		{
			m_Scoreboard.m_iPlayerNum = i;  // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
		}
	}
}

bool CHud::UseVguiMOTD()
{
	return m_pCvarMOTDVGUI && m_pCvarMOTDVGUI->value;
}

bool CHud::UseVguiScoreBoard()
{
	return m_pCvarScoreboardVGUI && m_pCvarScoreboardVGUI->value;
}
