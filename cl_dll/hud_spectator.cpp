//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"

#include "hltv.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "entity_types.h"

#include "com_model.h"
#include "demo_api.h"
#include "event_api.h"
#include "screenfade.h"

extern "C" int		iJumpSpectator;
extern "C" float	vJumpOrigin[3];
extern "C" float	vJumpAngles[3]; 

extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_cl_angles;
extern vec3_t v_sim_org;

const char *GetSpectatorLabel( int iMode )
{

}

void SpectatorMode( void )
{

}

void SpectatorToggleInset( void )
{

}

void SpectatorSpray( void )
{

}

void SpectatorHelp( void )
{

}

void SpectatorMenu( void )
{

}

void ToggleScores( void )
{

}

void SpectatorToggleDrawNames( void )
{

}

void SpectatorToggleDrawCone( void )
{

}

void SpectatorToggleDrawStatus( void )
{

}

void SpectatorToggleAutoDirector( void )
{

}

int CHudSpectator::Init( void )
{

}

void UTIL_StringToVector( float * pVector, const char *pString )
{

}

int UTIL_FindEntityInMap( const char *name, float *origin, float *angle )
{

}

void CHudSpectator::SetSpectatorStartPosition( void )
{

}

int CHudSpectator::VidInit( void )
{

}

int CHudSpectator::Draw( float flTime )
{

}

void CHudSpectator::DirectorMessage( int iSize, void *pbuf )
{

}

void CHudSpectator::FindNextPlayer( bool bReverse )
{

}

void CHudSpectator::FindPlayer( const char *name )
{

}

void CHudSpectator::HandleButtonsUp( int ButtonPressed )
{

}

void CHudSpectator::SetModes( int iMainMode, int iInsetMode )
{

}

bool CHudSpectator::IsActivePlayer( cl_entity_t *ent )
{

}

bool CHudSpectator::ParseOverviewFile( void )
{

}

void CHudSpectator::LoadMapSprites( void )
{

}

void CHudSpectator::DrawOverviewLayer( void )
{

}

void CHudSpectator::DrawOverviewEntities( void )
{

}

void CHudSpectator::DrawOverview( void )
{

}

void CHudSpectator::CheckOverviewEntities( void )
{

}

bool CHudSpectator::AddOverviewEntity( int type, struct cl_entity_s *ent, const char *modelname )
{

}

void CHudSpectator::DeathMessage( int victim )
{

}

bool CHudSpectator::AddOverviewEntityToList( HSPRITE sprite, cl_entity_t *ent, double killTime )
{

}

bool CHudSpectator::AddOverviewEntityToMap( HSPRITE sprite, cl_entity_t *ent, double killTime, Vector origin )
{

}

void CHudSpectator::CheckSettings( void )
{

}

int CHudSpectator::ToggleInset( bool allowOff )
{

}

void CHudSpectator::Reset( void )
{

}

void CHudSpectator::InitHUDData( void )
{

}

HSPRITE CHudSpectator::GetMarkerSPR( int marker )
{

}

void CHudSpectator::AddVoiceIconToPlayerEnt( int index )
{

}

void CHudSpectator::ClearVoiceIconFlags( void )
{

}

bool CHudSpectator::ShouldSetVoiceIcon( int index )
{

}