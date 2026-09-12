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
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "netadr.h"
#include "parsemsg.h"

extern "C"
{
#include "pm_shared.h"
}

#include <string.h>
#include "vcs_info.h"

#include "particleman.h"
#include "IParticleMan_Active.h"
#include "CMiniMem.h"

#include "cl_util.h"

cl_enginefunc_t gEngfuncs;
CHud gHUD;
mobile_engfuncs_t *gMobileEngfuncs = NULL;
IParticleMan *g_pParticleMan = NULL;

void InitInput( void );
void EV_HookEvents( void );
void IN_Commands( void );
void CL_LoadParticleMan( void );
void CL_UnloadParticleMan( void );

/*
========================== 
    Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C" 
{
int		DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int		DLLEXPORT HUD_VidInit( void );
void	DLLEXPORT HUD_Init( void );
void	DLLEXPORT HUD_Shutdown( void );
int		DLLEXPORT HUD_Redraw( float flTime, int intermission );
int		DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void	DLLEXPORT HUD_Reset ( void );
void	DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void	DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char	DLLEXPORT HUD_PlayerMoveTexture( char *name );
int		DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int		DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void	DLLEXPORT HUD_Frame( double time );
void	DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
void	DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
int		DLLEXPORT HUD_GetPlayerTeam( int iplayer );
int		DLLEXPORT HUD_MobilityInterface( mobile_engfuncs_t *gpMobileEngfuncs );
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch( hullnumber )
	{
	case 0:				// Normal player
		Vector( -16, -16, -36 ).CopyToArray(mins);
		Vector( 16, 16, 36 ).CopyToArray(maxs);
		iret = 1;
		break;
	case 1:				// Crouched player
		Vector( -16, -16, -18 ).CopyToArray(mins);
		Vector( 16, 16, 18 ).CopyToArray(maxs);
		iret = 1;
		break;
	case 2:				// Point based hull
		Vector( 0, 0, 0 ).CopyToArray(mins);
		Vector( 0, 0, 0 ).CopyToArray(maxs);
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	// int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}


void CL_UnloadParticleMan()
{
	if( g_pParticleMan )
	{
		delete g_pParticleMan;
		g_pParticleMan = NULL;
	}
}

void CL_LoadParticleMan()
{
	g_pParticleMan = new IParticleMan_Active();
	if( g_pParticleMan )
	{
		g_pParticleMan->SetUp( &gEngfuncs );
	}

	if( g_pParticleMan )
	{
		g_pParticleMan->SetUp( &gEngfuncs );
	}
	else
	{
		g_pParticleMan = NULL;
	}
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

	if( iVersion != CLDLL_INTERFACE_VERSION )
		return 0;

	// for now filterstuffcmd is last in the engine interface
	memcpy( &gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t) - sizeof( void * ) );

	if( gEngfuncs.pfnGetCvarPointer( "cl_filterstuffcmd" ) == 0 )
	{
		gEngfuncs.pfnFilteredClientCmd = gEngfuncs.pfnClientCmd;
	}
	else
	{
		gEngfuncs.pfnFilteredClientCmd = pEnginefuncs->pfnFilteredClientCmd;
	}

	EV_HookEvents();
	CL_LoadParticleMan();

	gEngfuncs.pfnRegisterVariable( "cl_game_build_commit", g_VCSInfo_Commit, 0 );
	gEngfuncs.pfnRegisterVariable( "cl_game_build_branch", g_VCSInfo_Branch, 0 );

	return 1;
}

/*
=================
HUD_GetRect

VGui stub
=================
*/
int *HUD_GetRect( void )
{
	static int extent[4];

	extent[0] = gEngfuncs.GetWindowCenterX() - ScreenWidth / 2;
	extent[1] = gEngfuncs.GetWindowCenterY() - ScreenHeight / 2;
	extent[2] = gEngfuncs.GetWindowCenterX() + ScreenWidth / 2;
	extent[3] = gEngfuncs.GetWindowCenterY() + ScreenHeight / 2;

	return extent;
}

/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

int DLLEXPORT HUD_VidInit( void )
{
	gHUD.VidInit();

	if( g_pParticleMan )
		g_pParticleMan->ResetParticles();

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init( void )
{
	InitInput();
	gHUD.Init();
}

/*
==========================
	HUD_Shutdown

===========================
*/

void DLLEXPORT HUD_Shutdown( void )
{
	if( g_pParticleMan )
	{
		CL_UnloadParticleMan();
	}
	auto miniMem = CMiniMem::Instance();
	if( miniMem )
	{
		miniMem->Reset();
		miniMem->Shutdown();
	}
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	gHUD.Redraw( time, intermission );

	return 1;
}

/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData( client_data_t *pcldata, float flTime )
{
	IN_Commands();

	return gHUD.UpdateClientData( pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	gEngfuncs.VGui_ViewportPaintBackground(HUD_GetRect());
}

/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus( int entindex, qboolean bTalking )
{

}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}

/*
==========================
HUD_GetPlayerTeam

==========================
*/

int DLLEXPORT HUD_GetPlayerTeam( int iplayer )
{
	if( iplayer <= MAX_PLAYERS && iplayer >= 1 )
		return g_PlayerExtraInfo[iplayer].teamId;

	return 0;
}

int DLLEXPORT HUD_MobilityInterface( mobile_engfuncs_t *gpMobileEngfuncs )
{
	if( gpMobileEngfuncs->version != MOBILITY_API_VERSION )
		return 1;
	gMobileEngfuncs = gpMobileEngfuncs;
	return 0;
}

bool HUD_MessageBox( const char *msg )
{
	gEngfuncs.Con_Printf( msg ); // just in case

	if( IsXashFWGS() )
	{
		gMobileEngfuncs->pfnSys_Warn( msg );
		return true;
	}

	// TODO: Load SDL2 and call ShowSimpleMessageBox

	return false;
}

bool IsXashFWGS()
{
	return gMobileEngfuncs != NULL;
}