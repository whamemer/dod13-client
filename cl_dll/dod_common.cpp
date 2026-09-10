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
//  dod_common.cpp - implementation of the CHudDoDCommon class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "event_api.h"
#include "dod_shared.h"
#include "ref_params.h"

DECLARE_MESSAGE( m_DoDCommon, GameRules )
DECLARE_MESSAGE( m_DoDCommon, CameraView )
DECLARE_MESSAGE( m_DoDCommon, ResetSens )

bool b_cutscene_return;
int i_lastspectoggle;
float fl_DeadTargetSwitch;
extern int i_dodmusic, g_iDeadFlag, g_iVuser1x, g_iUser3, g_iinjump, g_ionground, g_iMovetype;
int g_ihidexhair;
float i_ProneCounter;

void __CmdFunc_ClientProne( void )
{
	cl_entity_t *player;

	if( g_iDeadFlag || g_iVuser1x == 2 )
		return;

	if( ( gHUD.m_iKeyBits & IN_ATTACK ) == 0 )
	{
		if( gHUD.m_iKeyBits & IN_DUCK )
		{
			if( g_iUser3 <= 0 )
				return;
		}

		else if( g_iUser3 == 0 && gHUD.GetWaterLevel() > 1 )
		{
			return;
		}

		if( g_iMovetype != 5 && gHUD.m_flTime > i_ProneCounter )
		{
			gEngfuncs.pfnServerCmd( "sprone" );

			if( g_iUser3 <= 1 )
			{
				i_ProneCounter = gHUD.m_flTime + 1.5f;

				if( g_iUser3 == 0 )
				{
					player = gEngfuncs.GetLocalPlayer();

					gEngfuncs.pEventAPI->EV_PlaySound( player->index, player->origin, CHAN_BODY, "player/goprone.wav", 1.0f,
						ATTN_NORM, 0, 100 );
				}
			}
		}
	}
}

int CHudDoDCommon::Init( void )
{
	HOOK_MESSAGE( GameRules );
	HOOK_MESSAGE( CameraView );
	HOOK_MESSAGE( ResetSens );

	HOOK_COMMAND( "prone", ClientProne );

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
	byte a, b;
	bool warmupMode;

	BEGIN_READ( pbuf, iSize );

	a = READ_BYTE();
	b = READ_BYTE();

	gHUD.m_bAllieParatrooper = ( a & 1 ) != false;
	gHUD.m_bAllieInfiniteLives = ( a & 2 ) != false;
	gHUD.m_bBritish = ( a & 4 ) != false;

	gHUD.m_bAxisParatrooper = ( b & 1 ) != false;
	gHUD.m_bAxisInfiniteLives = ( b & 2 ) != false;

	warmupMode = ( b & 4 ) != false;
	gHUD.m_ObjectiveIcons.m_bWarmupMode = warmupMode;

	return 1;
}

int CHudDoDCommon::MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf )
{
	gHUD.m_iSensLevel = 0;
	return 1;
}

int CHudDoDCommon::Draw( float flTime )
{
	if( g_iinjump && g_ionground )
		g_iinjump = 0;

	if( g_iVuser1z )
		b_cutscene_return = 1;

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

	if( g_iUser1 && g_iUser2 )
	{
		if( g_PlayerExtraInfo[g_iUser2].dead )
		{
			if( fl_DeadTargetSwitch == 0.0f )
			{
				fl_DeadTargetSwitch = gHUD.m_flTime + 4.0f;
			}
			else if( gHUD.m_flTime > fl_DeadTargetSwitch )
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

	if( b_cutscene_return && !g_iVuser1z )
	{
		gHUD.m_Scope.SetScope( WEAPON_NONE );
		b_cutscene_return = 0;
	}

	return 1;
}

int CHudDoDCommon::VidInit( void )
{
	b_cutscene_return = 0;
	g_iinjump = 0;
	g_ihidexhair = 0;
	i_ProneCounter = 0.0f;
	return 1;
}

float DoDGunGoOnOffScreen( float v_modeloffscreen, ref_params_s *pparams )
{
	int weaponId;
	int fov;
	bool bMoving;

	weaponId = gHUD.GetCurrentWeaponId();

	if( weaponId == -1 || g_iUser3 == 3 )
		return 0.0f;

	if( gEngfuncs.IsSpectateOnly() || g_iUser1 || g_iUser2 )
		return 0.0f;

	bMoving = false;

	if( ( ( gHUD.m_iKeyBits & IN_RUN ) != 0 && ( gHUD.m_iKeyBits & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) != 0 ) 
		|| i_ProneCounter > gHUD.m_flTime )
		bMoving = true;

	else if( gHUD.GetWaterLevel() == 3 || ( gHUD.GetWaterLevel() > 0 && g_iUser3 == 1 ) )
	{
		if( weaponId != WEAPON_AMERKNIFE && weaponId != WEAPON_GERKNIFE && weaponId != WEAPON_SPADE )
			bMoving = true;
	}

	if( !bMoving )
	{
		if( ( g_iUser3 != 0 && ( gHUD.m_iKeyBits & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_LEFT | IN_RIGHT) ) 
			!= 0 ) || ( !g_ionground && g_iinjump ) || g_iMovetype == 5 )
			bMoving = true;
	}

	if( !bMoving )
	{
		if( v_modeloffscreen >= 3.0f )
			v_modeloffscreen -= 3.0f;

		g_ihidexhair = 0;
	}
	else
	{
		if( !gHUD.IsInMGDeploy() )
		{
			if( v_modeloffscreen <= 54.0f )
				v_modeloffscreen += 1.0f;

			g_ihidexhair = 1;
		}
	}

	fov = gHUD.m_iFOV;

	if( fov != 90 )
	{
		if( fov == 0 )
			return v_modeloffscreen;

		return 10.0f;
	}

	return v_modeloffscreen;
}

extern void DoD_ReadCameraSettings( const char *Targetname );

int CHudDoDCommon::MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf )
{
	const char *Targername = READ_STRING();
	BEGIN_READ( pbuf, iSize );
	DoD_ReadCameraSettings( Targername );
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
	bool displayHide = true;
	bool b_case;

	if( !g_iUser1 )
		displayHide = ( g_iUser2 != 0 );

	switch( i_hudElement )
	{
	case 1:
		b_case = ( cl_hud_health->value > 0.0f && !displayHide );
		break;
	case 2:
		b_case = false;
		if( cl_hud_reinforcements->value > 0.0f && gHUD.m_bInfiniteLives )
			b_case = !displayHide;
		break;
	case 3:
		b_case = ( cl_hud_ammo->value > 0.0f && !displayHide );
		break;
	case 4:
		b_case = ( cl_hud_objtimer->value > 0.0f );
		break;
	case 5:
		b_case = ( cl_hud_objectives->value > 0.0f );
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