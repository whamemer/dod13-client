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

//extern HFont  _engineFont;
//HFont hudfont;

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "r_studioint.h"

#include "voice_status.h"

#include "demo.h"

float g_lastFOV;
float g_fStamina;
int g_iWeaponBits2;

int g_iTeamNumber;
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;

float g_fUser4 = 0.0f;

int g_iVuser1x = 0;
int g_iVuser1z = 0;

int g_iMovetype;
int g_iEffects;
int g_iOnlyClientDraw;

hud_player_info_s g_PlayerInfoList[MAX_PLAYERS + 1];
extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];
pmodel_fx_t g_PModelFxInfo[65];

extern engine_studio_api_t IEngineStudio;

class CDoDVoiceStatusHelper : public IVoiceStatusHelper
{
public:
    virtual void GetPlayerTextColor( int entindex, int *color )
    {

    }

    virtual int GetAckIconHeight( void )
    {

    }

    virtual bool CanShowSpeakerLabels( void )
    {

    }
};
static CDoDVoiceStatusHelper g_VoiceStatusHelper;

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

int __MsgFunc_Logo( const char *pszName, int iSize, void *pbuf )
{
    BEGIN_READ( pbuf, iSize );
    gHUD.m_iLogo = READ_BYTE();
    return 1;
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
        gHUD.m_flMouseSensitivity = 0.0f;
        return 1;
    }

    if( g_lastFOV == 90 )
    {
        gHUD.m_flMouseSensitivity = 0.0f;
        return 1;
    }

    float fl;

    if( gHUD.zoom_sensitivity_ratio )
        fl = gHUD.zoom_sensitivity_ratio->value;
    else
        fl = 1.0f;

    gHUD.m_flMouseSensitivity = fl * ( g_lastFOV / 90.0f * sensitivity->value );

    return 1;
}

int __MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf )
{
    BEGIN_READ( pbuf, iSize );

    if( READ_BYTE )
        gEngfuncs.pfnPlaySoundByName( "common/wpn_select.wav", 0.5f );
    else
        gEngfuncs.pfnPlaySoundByName( "common/wpn_denyselect.wav", 0.5f );

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
    gHUD.m_fRoundEndsTime = READ_SHORT() + gHUD.m_flTime;
    return 1;
}

int __MsgFunc_BloodPuff( const char *pszName, int iSize, void *pbuf )
{
    BEGIN_READ( pbuf, iSize );

    float org = READ_COORD();

    if( !violence_hblood )
    {
        violence_hblood = gEngfuncs.pfnGetCvarPointer( "violence_hblood" );

        if( !violence_hblood )
            return 1;
    }

    if( violence_hblood->value != 1.0f )
        return 1;

    EV_BloodPuff( org );
    return 1;
}

int __MsgFunc_HandSignal( const char *pszName, int iSize, void *pbuf )
{
    return EV_HandSignalMsg( pszName, iSize, pbuf );
}

void __CmdFunc_ShowCommandMenu( void )
{
    if( gViewPort )
    {
        gViewPort->ShowCommandMenu();
    }
}

void __CmdFunc_HideCommandMenu( void )
{
    if( gViewPort )
    {
        gViewPort->HideCommandMenu();
    }
}

int __MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf )
{
    if( gViewPort )
        return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
    return 0;
}

int __MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
    if( gViewPort )
        return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
    return 0;
}

int __MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_Frags( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_PClass( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf )
{

}

int __MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf )
{

}

void CHud::Init( void )
{

}

CHud::~CHud()
{

}

int CHud::GetSpriteIndex( const char *SpriteName )
{

}

void CHud::VidInit( void )
{

}

int CHud::MsgFunc_Logo( const char *pszName,  int iSize, void *pbuf )
{

}

bool IsTeamPara( int team )
{

}

char *GetPlayerClassName( int playerclass )
{

}

void COM_FileBase( const char *in, char *out )
{

}

int HUD_IsGame( const char *game )
{

}

float HUD_GetFOV( void )
{

}

int CHud::MsgFunc_SetFOV( const char *pszName,  int iSize, void *pbuf )
{

}

int CHud::MsgFunc_RoundState( const char *pszName,  int iSize, void *pbuf )
{

}

int CHud::MsgFunc_TimeLeft( const char *pszName,  int iSize, void *pbuf )
{

}

int CHud::MsgFunc_HLTV( const char *pszName,  int iSize, void *pbuf )
{

}

void CHud::AddHudElem( CHudBase *p )
{

}

void ClientSetSensitivity( int level )
{

}

float CHud::GetSensitivity( void )
{

}

int CHud::GetCurrentWeaponId( void )
{

}

void CHud::SetWaterLevel( int level )
{

}

int CHud::GetWaterLevel( void )
{

}

static cvar_t *violence_hblood;

bool ShouldShowBlood( void )
{

}

int EV_BloodPuffMsg( const char *pszName,  int iSize, void *pbuf )
{

}

extern vec3_t v_origin;

void EV_BloodPuff( float *org )
{

}

extern char *s_HandSignalSubtitles[][3];

int EV_HandSignalMsg( const char *pszName,  int iSize, void *pbuf )
{

}

int CHud::MsgFunc_UseSound( const char *pszName,  int iSize, void *pbuf )
{

}

char *CHud::GetTeamName( int )
{

}

int CHud::GetMinimapZoomLevel( void )
{

}

int CHud::ZoomMinimap( void )
{

}

int CHud::GetMinimapState( void )
{

}

void CHud::SetMinimapState( int state )
{

}

void CHud::PlaySoundOnChan( char *name, float fVol, int chan )
{

}

void CHud::InitMapBounds( void )
{

}

void CHud::GetMapBounds( const int *x, const int *y, const int *w, const int *h )
{

}

void CHud::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{

}

bool CHud::IsInMGDeploy( void )
{

}

bool CHud::IsProneDeployed( void )
{

}

bool CHud::IsSandbagDeployed( void )
{

}

bool CHud::IsProne( void )
{

}

bool CHud::IsDucking( void )
{

}

extern int g_iDeadFlag;

bool CHud::IsInMortarDeploy( void )
{

}

void CHud::SetMortarDeployTime( void )
{

}

float CHud::GetMortarDeployTime( void )
{

}

void CHud::SetMortarUnDeployTime( void )
{

}

float CHud::GetMortarUnDeployTime( void )
{

}

void CHud::PostMortarValue( float value )
{

}

extern int g_iWeaponFlags;

void CHud::GetWeaponRecoilAmount( int weaponId, const float *flPitchRecoil, const float *flYawRecoil )
{

}

void CHud::DoRecoil( int weapon_id )
{

}

void CHud::SetRecoilAmount( float flPitchRecoil, float flYawRecoil )
{

}

void CHud::PopRecoil( float frametime, const float *flPitchRecoil, const float *flYawRecoil )
{

}