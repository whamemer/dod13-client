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
// dod_common.cpp
//
// implementation of CHudDoDCommon class and the HUD helpers shared between DoD elements
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "in_defs.h"
#include "event_api.h"
#include "pm_defs.h"
#include "com_weapons.h"
#include "vgui/idodviewport.h"

#include "dod_shared.h"
#include "dod_camera.h"

DECLARE_MESSAGE( m_DoDCommon, GameRules )
DECLARE_MESSAGE( m_DoDCommon, CameraView )
DECLARE_MESSAGE( m_DoDCommon, ResetSens )

bool b_cutscene_return;
int i_lastspectoggle;
float fl_DeadTargetSwitch;

extern int i_dodmusic;
extern int g_iDeadFlag;
extern int g_iVuser1x;
extern int g_iUser3;
extern int g_iinjump;
extern int g_ionground;

int g_ihidexhair;

float i_ProneCounter;

// the viewport of the original is a VGUI2 one, here it can be missing
static bool IsWidescreenVisible( void )
{
	return gDoDViewPortInterface && gDoDViewPortInterface->isWidescreenVisible();
}

void ClientProne( void )
{
	cl_entity_t *player;

	if( g_iDeadFlag || g_iVuser1x == SANDBAG_DEPLOYED )
		return;

	if( gHUD.m_iKeyBits & IN_ATTACK )
		return;

	if( gHUD.m_iKeyBits & IN_DUCK )
	{
		if( g_iUser3 <= 0 )
			return;
	}
	else if( !g_iUser3 && gHUD.GetWaterLevel() > 1 )
	{
		return;
	}

	if( g_iMovetype == MOVETYPE_FLY || gHUD.m_flTime <= i_ProneCounter )
		return;

	gEngfuncs.pfnServerCmd( "sprone" );

	if( g_iUser3 <= 1 )
	{
		i_ProneCounter = gHUD.m_flTime + 1.5f;

		if( !g_iUser3 )
		{
			player = gEngfuncs.GetLocalPlayer();

			gEngfuncs.pEventAPI->EV_PlaySound( player->index, player->origin, CHAN_VOICE, "player/goprone.wav", 1.0f, ATTN_NORM, 0, 100 );
		}
	}
}

int CHudDoDCommon::Init( void )
{
	HOOK_MESSAGE( GameRules );
	HOOK_MESSAGE( CameraView );
	HOOK_MESSAGE( ResetSens );

	gEngfuncs.pfnAddCommand( "prone", ClientProne );

	gHUD.i_specmenutoggle = 1;

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem( this );

	return 1;
}

void CHudDoDCommon::InitHUDData( void )
{
	i_dodmusic = 1;
}

int CHudDoDCommon::MsgFunc_GameRules( const char *pszName, int iSize, void *pbuf )
{
	byte a;
	byte b;
	bool warmupMode;

	BEGIN_READ( pbuf, iSize );

	a = READ_BYTE();
	b = READ_BYTE();

	gHUD.m_bAllieParatrooper = ( a & 1 ) != 0;
	gHUD.m_bAllieInfiniteLives = ( a & 2 ) != 0;
	gHUD.m_bBritish = ( a & 4 ) != 0;

	gHUD.m_bAxisParatrooper = ( b & 1 ) != 0;
	gHUD.m_bAxisInfiniteLives = ( b & 2 ) != 0;

	warmupMode = ( b & 4 ) != 0;

	gHUD.m_ObjectiveIcons.SetWarmupMode( warmupMode );

	if( gHUD.m_bBritish && gDoDViewPortInterface )
		gDoDViewPortInterface->updateTeamMenu();

	return 1;
}

int CHudDoDCommon::MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf )
{
	ClientSetSensitivity( 0 );

	return 1;
}

int CHudDoDCommon::Draw( float flTime )
{
	if( g_iinjump && g_ionground )
		g_iinjump = 0;

	if( g_iVuser1z )
		b_cutscene_return = true;

	if( g_iTeamNumber == 1 )
	{
		gHUD.m_bParatrooper = gHUD.m_bAllieParatrooper;
		gHUD.m_bInfiniteLives = gHUD.m_bAllieInfiniteLives;
	}
	else if( g_iTeamNumber == 2 )
	{
		gHUD.m_bParatrooper = gHUD.m_bAxisParatrooper;
		gHUD.m_bInfiniteLives = gHUD.m_bAxisInfiniteLives;
	}

	// the spectated player is dead: go to the next one after a while
	if( g_iUser1 && g_iUser2 )
	{
		if( g_PlayerExtraInfo[g_iUser2].dead )
		{
			if( fl_DeadTargetSwitch == 0.0f || gHUD.m_flTime <= fl_DeadTargetSwitch )
			{
				if( fl_DeadTargetSwitch == 0.0f )
					fl_DeadTargetSwitch = gHUD.m_flTime + 4.0f;
			}
			else
			{
				gEngfuncs.pfnClientCmd( "+attack;wait;-attack" );
				fl_DeadTargetSwitch = 0.0f;
			}
		}
		else
		{
			fl_DeadTargetSwitch = 0.0f;
		}
	}

	// the cutscene is over
	if( b_cutscene_return && !g_iVuser1z )
	{
		gHUD.m_Scope.SetScope( 0 );
		b_cutscene_return = false;
	}

	// the widescreen bars need a wider view
	if( IsWidescreenVisible() )
	{
		g_lastFOV = 115.0f;
		gHUD.SetFOV( 115 );
	}
	else if( !IsWidescreenVisible() && gHUD.m_iFOV == 115 )
	{
		g_lastFOV = 0.0f;
		gHUD.SetFOV( 90 );
	}

	if( IsWidescreenVisible() && !g_iUser1 && !g_iVuser1z )
		gDoDViewPortInterface->setWidescreenVisible( false );

	return 1;
}

int CHudDoDCommon::VidInit( void )
{
	b_cutscene_return = false;
	g_iinjump = 0;
	g_ihidexhair = 0;
	i_ProneCounter = 0.0f;

	return 1;
}

float DoDGunGoOnOffScreen( float v_modeloffscreen, ref_params_s *pparams )
{
	int weaponId;
	bool bMoving;
	bool bOffscreen;
	float offscreen;
	int fov;

	weaponId = gHUD.GetCurrentWeaponId();

	if( weaponId == -1 )
		return 0.0f;

	if( g_iUser3 == STANCE_MORTAR )
		return 0.0f;

	if( gEngfuncs.IsSpectateOnly() || g_iUser1 || g_iUser2 )
		return 0.0f;

	// sprinting or just went prone / stood up
	bMoving = ( gHUD.m_iKeyBits & IN_RUN ) && ( gHUD.m_iKeyBits & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) );
	bOffscreen = bMoving || i_ProneCounter > gHUD.m_flTime;

	// under water only the melee weapons stay
	if( !bOffscreen && ( gHUD.GetWaterLevel() == 3 || ( gHUD.GetWaterLevel() > 0 && g_iUser3 == STANCE_PRONE ) ) )
	{
		if( weaponId != WEAPON_AMERKNIFE && weaponId != WEAPON_GERKNIFE && weaponId != WEAPON_SPADE )
			bOffscreen = true;
	}

	if( !bOffscreen )
	{
		if( ( !g_iUser3 || !( gHUD.m_iKeyBits & ( IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_MOVELEFT | IN_MOVERIGHT ) ) )
			&& ( g_ionground || !g_iinjump ) && g_iMovetype != MOVETYPE_FLY )
		{
			// the weapon comes back to the screen
			offscreen = 0.0f;

			if( v_modeloffscreen <= 0.0f )
				offscreen = v_modeloffscreen;
			else if( v_modeloffscreen >= 3.0f )
				offscreen = v_modeloffscreen - 3.0f;

			g_ihidexhair = 0;
		}
		else
		{
			bOffscreen = true;
		}
	}

	if( bOffscreen )
	{
		offscreen = v_modeloffscreen;

		if( !gHUD.IsInMGDeploy() )
		{
			if( (int)offscreen <= 54 )
				offscreen += 1.0f;

			g_ihidexhair = 1;
		}
	}

	fov = gHUD.m_iFOV;

	if( fov != 90 )
	{
		if( !fov )
			return offscreen;

		return 10.0f;
	}

	return offscreen;
}

int CHudDoDCommon::MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf )
{
	const char *Targetname;

	BEGIN_READ( pbuf, iSize );

	Targetname = READ_STRING();

	DoD_ReadCameraSettings( Targetname );

	m_iFlags |= HUD_ACTIVE;

	return 1;
}

extern cvar_t *cl_hud_objectives;
extern cvar_t *cl_hud_objtimer;
extern cvar_t *cl_hud_reinforcements;
extern cvar_t *cl_hud_health;
extern cvar_t *cl_hud_ammo;

bool ShowHudElement( int i_hudElement )
{
	bool displayHide = g_iUser1 != 0 || g_iUser2 != 0;
	bool b_case;

	switch( i_hudElement )
	{
	case 1:
		b_case = cl_hud_health->value > 0.0f && !displayHide;
		break;
	case 2:
		b_case = false;

		if( cl_hud_reinforcements->value > 0.0f && gHUD.m_bInfiniteLives )
			b_case = !displayHide;
		break;
	case 3:
		b_case = cl_hud_ammo->value > 0.0f && !displayHide;
		break;
	case 4:
		b_case = cl_hud_objtimer->value > 0.0f;
		break;
	case 5:
		b_case = cl_hud_objectives->value > 0.0f;
		break;
	case 6:
		b_case = false;

		if( !displayHide )
			b_case = cl_hud_objectives != NULL;
		break;
	case 7:
		b_case = true;
		break;
	default:
		b_case = false;
		break;
	}

	return b_case;
}
