//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined( GAMESTUDIOMODELRENDERER_H )
#define GAMESTUDIOMODELRENDERER_H

#ifdef _WIN32
#include "string.h"
#define strcasecmp _stricmp
#else
#include "strings.h"
#endif

/*
====================
CGameStudioModelRenderer

====================
*/
class CGameStudioModelRenderer : public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer( void );

	virtual void StudioSetupBones( void );
	virtual void StudioEstimateGait( entity_state_t *pplayer );
	virtual void StudioProcessGait( entity_state_t *pplayer );
	virtual int StudioDrawPlayer( int flags, entity_state_t *pplayer );
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );
	virtual void StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );
	virtual void CalculateYawBlend( entity_state_t *pplayer );
	virtual void CalculatePitchBlend( entity_state_t *pplayer );

private:
	void SavePlayerState( entity_state_t *pplayer );
	void SetupClientAnimation( entity_state_t *pplayer );
	void RestorePlayerState( entity_state_t *pplayer );
	mstudioanim_t *LookupAnimation( mstudioseqdesc_t *pseqdesc, int index );
	model_t *SwapPWpnModels( cl_entity_t *vplayer, model_t *pweaponmodel );
	void AnimatePWpnModels( cl_entity_t *vplayer );
	void StudioClientEvents( void );

private:
	int m_nPlayerGaitSequences[MAX_CLIENTS];
};

#endif // GAMESTUDIOMODELRENDERER_H
