//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined( GAMESTUDIOMODELRENDERER_H )
#define GAMESTUDIOMODELRENDERER_H

/*
====================
CGameStudioModelRenderer

====================
*/
class CGameStudioModelRenderer : public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer( void );

	// Set up model bone positions (the players are blended by the 9 blends of the sequences)
	virtual void StudioSetupBones( void );

	// Estimate gait frame for player
	virtual void StudioEstimateGait( entity_state_t *pplayer );

	// Process movement of player
	virtual void StudioProcessGait( entity_state_t *pplayer );

	// Draws the player and its weapon
	virtual int StudioDrawPlayer( int flags, entity_state_t *pplayer );

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );

	virtual void CalculateYawBlend( entity_state_t *pplayer );
	virtual void CalculatePitchBlend( entity_state_t *pplayer );

	void SavePlayerState( entity_state_t *pplayer );
	void SetupClientAnimation( entity_state_t *pplayer );
	void RestorePlayerState( entity_state_t *pplayer );

	// Animation of the blend of a sequence
	mstudioanim_t *LookupAnimation( mstudioseqdesc_t *pseqdesc, int index );

	// Replaces the model of the weapon in the hands of the player (bipod, sandbag, reload with the left hand)
	model_t *SwapPWpnModels( cl_entity_t *vplayer, model_t *pweaponmodel );
	void AnimatePWpnModels( cl_entity_t *vplayer );

	// Fires the events of the sequence
	void StudioClientEvents( void );

private:
	// unused in the original
	int m_nPlayerGaitSequences[32];
};

#endif // GAMESTUDIOMODELRENDERER_H
