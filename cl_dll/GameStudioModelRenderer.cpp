//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "dod_shared.h"

//
// Override the StudioModelRender virtual member functions here to implement custom bone
// setup, blending, etc.
//

extern p_wpninfo_s P_WpnInfo[];

void DoD_GetSequence( int *seq, int *gaitseq );
void DoD_GetOrientation( float *o, float *a );

extern "C" void DLLEXPORT HUD_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );

// the start of the shrinking of the models with kRenderFxExplode
float g_flStartScaleTime;
int iPrevRenderState;
int iRenderStateChanged;

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

//
// The state of the animation of a player
//
struct client_anim_state_t
{
	vec3_t origin;
	vec3_t angles;
	vec3_t realangles;
	float animtime;
	float frame;
	int sequence;
	int gaitsequence;
	float framerate;
	int m_fSequenceLoops;
	int m_fSequenceFinished;
	byte controller[4];
	byte blending[2];
	latchedvars_t lv;
};

static client_anim_state_t g_state;
static client_anim_state_t g_clientstate;

//
// The sequences of the deployed bipod and sandbag, they begin with "bipod_" and "sandbag_"
//
int iNumBipodAnims;
int iNumSandbagAnims;
int iBipodAnims[64];
int iSandbagAnims[64];

void AddToBipodAnimTable( int seq )
{
	if( iNumBipodAnims < 64 )
		iBipodAnims[iNumBipodAnims++] = seq;
}

void AddToSandbagAnimTable( int seq )
{
	if( iNumSandbagAnims < 64 )
		iSandbagAnims[iNumSandbagAnims++] = seq;
}

void BuildDeployAnimTable( void *pmodel )
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;
	mstudioseqdesc_t *pseqdesc;

	if( !pstudiohdr )
		return;

	iNumBipodAnims = 0;
	iNumSandbagAnims = 0;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)pstudiohdr + pstudiohdr->seqindex );

	for( int i = 0; i < pstudiohdr->numseq; i++, pseqdesc++ )
	{
		if( !memcmp( pseqdesc->label, "bipod_", 6 ) )
			AddToBipodAnimTable( i );
		else if( !memcmp( pseqdesc->label, "sandbag_", 8 ) )
			AddToSandbagAnimTable( i );
	}
}

bool IsBipodAnimation( int seq, void *pmodel )
{
	if( !iNumBipodAnims )
		BuildDeployAnimTable( pmodel );

	for( int i = 0; i < iNumBipodAnims; i++ )
	{
		if( iBipodAnims[i] == seq )
			return true;
	}

	return false;
}

bool IsSandbagAnimation( int seq, void *pmodel )
{
	if( !iNumSandbagAnims )
		BuildDeployAnimTable( pmodel );

	for( int i = 0; i < iNumSandbagAnims; i++ )
	{
		if( iSandbagAnims[i] == seq )
			return true;
	}

	return false;
}

// The renderer object, created on the stack.
CGameStudioModelRenderer g_StudioRenderer;
/*
====================
CGameStudioModelRenderer

====================
*/
CGameStudioModelRenderer::CGameStudioModelRenderer( void )
{
	/* Nothing. */
}

/*
====================
LookupAnimation

====================
*/
mstudioanim_t *CGameStudioModelRenderer::LookupAnimation( mstudioseqdesc_t *pseqdesc, int index )
{
	mstudioanim_t *panim = StudioGetAnim( m_pRenderModel, pseqdesc );

	if( index >= 0 && index < pseqdesc->numblends )
		panim += m_pStudioHeader->numbones * index;

	return panim;
}

/*
====================
StudioSetupBones

====================
*/
void CGameStudioModelRenderer::StudioSetupBones( void )
{
	int i;
	double f;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	static vec4_t q[MAXSTUDIOBONES];
	float bonematrix[3][4];

	static float pos2[MAXSTUDIOBONES][3];
	static vec4_t q2[MAXSTUDIOBONES];
	static float pos3[MAXSTUDIOBONES][3];
	static vec4_t q3[MAXSTUDIOBONES];
	static float pos4[MAXSTUDIOBONES][3];
	static vec4_t q4[MAXSTUDIOBONES];

	if( !m_pCurrentEntity->player )
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	if( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0 )
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	panim = StudioGetAnim( m_pRenderModel, pseqdesc );
	f = StudioEstimateFrame( pseqdesc );

	if( pseqdesc->numblends == 9 )
	{
		// the sequence has 3x3 blends: the torso (blending[0]) and the pitch (blending[1])
		float s, t;
		float b0 = m_pCurrentEntity->curstate.blending[0];
		float b1 = m_pCurrentEntity->curstate.blending[1];

		if( b0 > 127.0f )
		{
			s = ( b0 - 127.0f ) * 2.0f;

			if( b1 > 127.0f )
			{
				t = ( b1 - 127.0f ) * 2.0f;

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 5 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 7 ), f );
				StudioCalcRotations( pos4, q4, pseqdesc, LookupAnimation( pseqdesc, 8 ), f );
			}
			else
			{
				t = b1 * 2.0f;

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 1 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 2 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				StudioCalcRotations( pos4, q4, pseqdesc, LookupAnimation( pseqdesc, 5 ), f );
			}
		}
		else
		{
			s = b0 * 2.0f;

			if( b1 > 127.0f )
			{
				t = ( b1 - 127.0f ) * 2.0f;

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 3 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 6 ), f );
				StudioCalcRotations( pos4, q4, pseqdesc, LookupAnimation( pseqdesc, 7 ), f );
			}
			else
			{
				t = b1 * 2.0f;

				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 1 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 3 ), f );
				StudioCalcRotations( pos4, q4, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
			}
		}

		StudioSlerpBones( q, pos, q2, pos2, s / 255.0f );
		StudioSlerpBones( q3, pos3, q4, pos4, s / 255.0f );
		StudioSlerpBones( q, pos, q3, pos3, t / 255.0f );
	}
	else
	{
		StudioCalcRotations( pos, q, pseqdesc, panim, f );

		if( pseqdesc->numblends > 1 )
		{
			float s;
			float dadt;

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos2, q2, pseqdesc, panim, f );

			dadt = StudioEstimateInterpolant();
			s = 1.0 - ( m_pCurrentEntity->curstate.blending[1] * dadt + ( 1.0 - dadt ) * m_pCurrentEntity->latched.prevblending[1] ) / 255.0;

			StudioSlerpBones( q, pos, q2, pos2, s );
		}
	}

	if( m_fDoInterp && m_pCurrentEntity->latched.sequencetime &&
		( m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime ) &&
		( m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq ) )
	{
		// blend from last sequence
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s;

		pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim( m_pRenderModel, pseqdesc );

		if( pseqdesc->numblends == 9 )
		{
			float sa, t;
			float b0 = m_pCurrentEntity->latched.prevseqblending[0];
			float b1 = m_pCurrentEntity->latched.prevseqblending[1];
			int index;

			if( b0 > 127.0f )
			{
				sa = ( b0 - 127.0f ) * 2.0f;

				if( b1 > 127.0f )
				{
					t = ( b1 - 127.0f ) * 2.0f;

					StudioCalcRotations( pos1b, q1b, pseqdesc, LookupAnimation( pseqdesc, 4 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 5 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 7 ), m_pCurrentEntity->latched.prevframe );
					index = 8;
				}
				else
				{
					t = b1 * 2.0f;

					StudioCalcRotations( pos1b, q1b, pseqdesc, LookupAnimation( pseqdesc, 1 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 2 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 4 ), m_pCurrentEntity->latched.prevframe );
					index = 5;
				}
			}
			else
			{
				sa = b0 * 2.0f;

				if( b1 > 127.0f )
				{
					t = ( b1 - 127.0f ) * 2.0f;

					StudioCalcRotations( pos1b, q1b, pseqdesc, LookupAnimation( pseqdesc, 3 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 4 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 6 ), m_pCurrentEntity->latched.prevframe );
					index = 7;
				}
				else
				{
					t = b1 * 2.0f;

					StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 1 ), m_pCurrentEntity->latched.prevframe );
					StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 3 ), m_pCurrentEntity->latched.prevframe );
					index = 4;
				}
			}

			StudioCalcRotations( pos4, q4, pseqdesc, LookupAnimation( pseqdesc, index ), m_pCurrentEntity->latched.prevframe );

			StudioSlerpBones( q1b, pos1b, q2, pos2, sa / 255.0f );
			StudioSlerpBones( q3, pos3, q4, pos4, sa / 255.0f );
			StudioSlerpBones( q1b, pos1b, q3, pos3, t / 255.0f );
		}
		else
		{
			// clip prevframe
			StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

			if( pseqdesc->numblends > 1 )
			{
				panim += m_pStudioHeader->numbones;
				StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				s = m_pCurrentEntity->latched.prevseqblending[0] / 255.0;
				StudioSlerpBones( q1b, pos1b, q2, pos2, s );

				if( pseqdesc->numblends == 4 )
				{
					panim += m_pStudioHeader->numbones;
					StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

					panim += m_pStudioHeader->numbones;
					StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

					s = m_pCurrentEntity->latched.prevseqblending[0] / 255.0;
					StudioSlerpBones( q3, pos3, q4, pos4, s );

					s = m_pCurrentEntity->latched.prevseqblending[1] / 255.0;
					StudioSlerpBones( q1b, pos1b, q3, pos3, s );
				}
			}
		}

		s = 1.0 - ( m_clTime - m_pCurrentEntity->latched.sequencetime ) / 0.2;
		StudioSlerpBones( q, pos, q1b, pos1b, s );
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->boneindex );

	// calc gait animation, the legs of the player (the bones 0..7) are animated by the gait sequence
	if( ( m_pCurrentEntity->curstate.sequence < 19 || m_pCurrentEntity->curstate.sequence > 24 ) &&
		m_pPlayerInfo && m_pPlayerInfo->gaitsequence > 0 &&
		m_pCurrentEntity->curstate.sequence != 7 && m_pCurrentEntity->curstate.sequence != 8 )
	{
		bool bCopy = true;

		if( m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq )
			m_pPlayerInfo->gaitsequence = 0;

		pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim( m_pRenderModel, pseqdesc );
		StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe );

		for( i = 0; i < m_pStudioHeader->numbones; i++ )
		{
			if( i == 8 )
				bCopy = false;
			else if( i == 5 || i == 2 )
				bCopy = true;

			if( bCopy )
			{
				memcpy( pos[i], pos2[i], sizeof( pos[i] ) );
				memcpy( q[i], q2[i], sizeof( q[i] ) );
			}
		}
	}

	for( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		QuaternionMatrix( q[i], bonematrix );

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if( pbones[i].parent == -1 )
		{
			if( IEngineStudio.IsHardware() )
			{
				ConcatTransforms( (*m_protationmatrix), bonematrix, (*m_pbonetransform)[i] );

				// MatrixCopy should be faster...
				//ConcatTransforms( (*m_protationmatrix), bonematrix, (*m_plighttransform)[i] );
				MatrixCopy( (*m_pbonetransform)[i], (*m_plighttransform)[i] );
			}
			else
			{
				ConcatTransforms( (*m_paliastransform), bonematrix, (*m_pbonetransform)[i] );
				ConcatTransforms( (*m_protationmatrix), bonematrix, (*m_plighttransform)[i] );
			}

			// Apply client-side effects to the transformation matrix
			StudioFxTransform( m_pCurrentEntity, (*m_pbonetransform)[i] );
		}
		else
		{
			ConcatTransforms( (*m_pbonetransform)[pbones[i].parent], bonematrix, (*m_pbonetransform)[i] );
			ConcatTransforms( (*m_plighttransform)[pbones[i].parent], bonematrix, (*m_plighttransform)[i] );
		}
	}
}

/*
====================
StudioEstimateGait

====================
*/
void CGameStudioModelRenderer::StudioEstimateGait( entity_state_t *pplayer )
{
	float dt;
	vec3_t est_velocity;
	bool bProne;

	// the gait sequences 15 and 16 and the sequences 17 and 18 are the prone ones
	bProne = ( m_pPlayerInfo->gaitsequence == 15 || m_pPlayerInfo->gaitsequence == 16 ||
		m_pCurrentEntity->curstate.sequence == 17 || m_pCurrentEntity->curstate.sequence == 18 );

	dt = (float)( m_clTime - m_clOldTime );

	if( dt < 0 )
		dt = 0;
	else if( dt > 1.0 )
		dt = 1;

	if( dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount )
	{
		m_flGaitMovement = 0;
		return;
	}

	// VectorAdd( pplayer->velocity, pplayer->prevgaitorigin, est_velocity );
	if( m_fGaitEstimation )
	{
		VectorSubtract( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity );
		VectorCopy( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin );
		m_flGaitMovement = Length( est_velocity );

		if( dt <= 0 || m_flGaitMovement / dt < 5 )
		{
			m_flGaitMovement = 0;
			est_velocity[0] = 0;
			est_velocity[1] = 0;
		}
	}
	else
	{
		VectorCopy( pplayer->velocity, est_velocity );
		m_flGaitMovement = Length( est_velocity ) * dt;
	}

	// the player with a deployed sandbag looks where the view points
	if( IsSandbagAnimation( m_pCurrentEntity->curstate.sequence, IEngineStudio.Mod_Extradata( m_pCurrentEntity->model ) ) )
	{
		m_pPlayerInfo->gaityaw = m_pCurrentEntity->angles[YAW];
		return;
	}

	if( est_velocity[1] == 0 && est_velocity[0] == 0 )
	{
		if( ( m_pCurrentEntity->curstate.sequence < 19 || m_pCurrentEntity->curstate.sequence > 24 ) && !bProne )
		{
			float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
			float flYaw;

			flYawDiff = flYawDiff - (int)( flYawDiff / 360 ) * 360;

			if( flYawDiff > 180 )
				flYawDiff -= 360;

			if( flYawDiff < -180 )
				flYawDiff += 360;

			flYaw = (float)fmod( m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw, 360 );

			if( flYaw < -180 )
				flYaw += 360;
			else if( flYaw > 180 )
				flYaw -= 360;

			// how fast the legs turn after the torso
			if( flYaw > -5 && flYaw < 5 )
				m_pCurrentEntity->baseline.fuser1 = 0.05f;
			else if( flYaw < -90 || flYaw > 90 )
				m_pCurrentEntity->baseline.fuser1 = 3.5f;
			else
				m_pCurrentEntity->baseline.fuser1 = 0.0f;

			if( dt < 0.25 )
				flYawDiff *= dt * m_pCurrentEntity->baseline.fuser1;
			else
				flYawDiff *= dt;

			if( abs( (int)flYawDiff ) < 0.1 )
				flYawDiff = 0;

			m_pPlayerInfo->gaityaw += flYawDiff;
			m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)( m_pPlayerInfo->gaityaw / 360 ) * 360;

			m_flGaitMovement = 0;
		}
		else
		{
			m_pPlayerInfo->gaityaw = m_pCurrentEntity->angles[YAW];
		}
	}
	else
	{
		// moving
		if( !bProne )
		{
			m_pPlayerInfo->gaityaw = (float)( atan2( est_velocity[1], est_velocity[0] ) * 180 / M_PI );

			if( m_pPlayerInfo->gaityaw > 180 )
				m_pPlayerInfo->gaityaw = 180;

			if( m_pPlayerInfo->gaityaw < -180 )
				m_pPlayerInfo->gaityaw = -180;
		}
	}
}

/*
====================
StudioPlayerBlend

====================
*/
void CGameStudioModelRenderer::StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch )
{
	// calc up/down pointing
	*pBlend = (int)( *pPitch * 3 );

	if( *pBlend <= -45 )
	{
		*pBlend = 255;
		*pPitch = 0;
	}
	else if( *pBlend >= 45 )
	{
		*pBlend = 0;
		*pPitch = 0;
	}
	else
	{
		*pBlend = (int)( ( 45.0 - *pBlend ) * 255.0 / 90.0 );
		*pPitch = 0;
	}
}

/*
====================
CalculatePitchBlend

====================
*/
void CGameStudioModelRenderer::CalculatePitchBlend( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	int iBlend;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	// calc player pitch
	StudioPlayerBlend( pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH] );

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = m_pCurrentEntity->curstate.blending[1];
	m_pCurrentEntity->latched.prevseqblending[1] = m_pCurrentEntity->curstate.blending[1];
}

/*
====================
CalculateYawBlend

====================
*/
void CGameStudioModelRenderer::CalculateYawBlend( entity_state_t *pplayer )
{
	float flYaw;
	float blend_yaw;

	StudioEstimateGait( pplayer );

	// calc side to side turning
	if( m_pPlayerInfo->gaitsequence != 15 && m_pPlayerInfo->gaitsequence != 16 &&
		m_pCurrentEntity->curstate.sequence != 17 && m_pCurrentEntity->curstate.sequence != 18 )
	{
		flYaw = (float)fmod( m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw, 360 );

		if( flYaw < -180 )
			flYaw += 360;
		else if( flYaw > 180 )
			flYaw -= 360;

		if( m_flGaitMovement != 0.0 )
		{
			if( flYaw > 120 )
			{
				m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
				m_flGaitMovement = -m_flGaitMovement;
				flYaw = flYaw - 180;
			}
			else if( flYaw < -120 )
			{
				m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
				m_flGaitMovement = -m_flGaitMovement;
				flYaw = flYaw + 180;
			}
		}

		// adjust torso
		blend_yaw = ( flYaw / 90.0f ) * 128.0f + 127.0f;

		if( blend_yaw > 255.0f )
			blend_yaw = 255.0f;

		if( blend_yaw < 0.0f )
			blend_yaw = 0.0f;

		m_pCurrentEntity->curstate.blending[0] = (int)( 255.0f - blend_yaw );
		m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
		m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

		m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

		if( m_pCurrentEntity->angles[YAW] < 0 )
			m_pCurrentEntity->angles[YAW] += 360;

		m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];
	}
}

/*
====================
StudioProcessGait

====================
*/
void CGameStudioModelRenderer::StudioProcessGait( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	float dt;

	CalculateYawBlend( pplayer );
	CalculatePitchBlend( pplayer );

	dt = (float)( m_clTime - m_clOldTime );

	if( dt < 0 )
		dt = 0;
	else if( dt > 1.0 )
		dt = 1;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + pplayer->gaitsequence;

	// calc gait frame
	if( pseqdesc->linearmovement[0] > 0 )
	{
		m_pPlayerInfo->gaitframe += ( m_flGaitMovement / pseqdesc->linearmovement[0] ) * pseqdesc->numframes;
	}
	else
	{
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;
	}

	// do modulo
	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)( m_pPlayerInfo->gaitframe / pseqdesc->numframes ) * pseqdesc->numframes;

	if( m_pPlayerInfo->gaitframe < 0 )
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

/*
====================
SavePlayerState

====================
*/
void CGameStudioModelRenderer::SavePlayerState( entity_state_t *pplayer )
{
	cl_entity_t *pent = IEngineStudio.GetCurrentEntity();

	if( !pent )
		return;

	VectorCopy( pent->curstate.angles, g_state.angles );
	VectorCopy( pent->curstate.origin, g_state.origin );
	VectorCopy( pent->angles, g_state.realangles );
	g_state.sequence = pent->curstate.sequence;
	g_state.gaitsequence = pplayer->gaitsequence;
	g_state.animtime = pent->curstate.animtime;
	g_state.frame = pent->curstate.frame;
	g_state.framerate = pent->curstate.framerate;
	memcpy( g_state.blending, pent->curstate.blending, sizeof( g_state.blending ) );
	memcpy( g_state.controller, pent->curstate.controller, sizeof( g_state.controller ) );
	g_state.lv = pent->latched;
}

/*
====================
GetSequenceInfo

====================
*/
void GetSequenceInfo( void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;
	mstudioseqdesc_t *pseqdesc;

	if( !pstudiohdr )
		return;

	if( pev->sequence >= pstudiohdr->numseq || pev->sequence < 0 )
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)( (byte *)pstudiohdr + pstudiohdr->seqindex ) + (int)pev->sequence;

	if( pseqdesc->numframes > 1 )
	{
		*pflFrameRate = 256 * pseqdesc->fps / ( pseqdesc->numframes - 1 );
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0] * pseqdesc->linearmovement[0] + pseqdesc->linearmovement[1] * pseqdesc->linearmovement[1] + pseqdesc->linearmovement[2] * pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / ( pseqdesc->numframes - 1 );
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

/*
====================
GetSequenceFlags

====================
*/
int GetSequenceFlags( void *pmodel, client_anim_state_t *pev )
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;

	if( !pstudiohdr || pev->sequence >= pstudiohdr->numseq || pev->sequence < 0 )
		return 0;

	return ( (mstudioseqdesc_t *)( (byte *)pstudiohdr + pstudiohdr->seqindex ) + (int)pev->sequence )->flags;
}

/*
====================
StudioFrameAdvance

====================
*/
float StudioFrameAdvance( client_anim_state_t *st, float framerate, float flInterval )
{
	if( flInterval == 0.0 )
	{
		flInterval = gEngfuncs.GetClientTime() - st->animtime;

		if( flInterval <= 0.001 )
		{
			st->animtime = gEngfuncs.GetClientTime();
			return 0.0;
		}
	}

	if( !st->animtime )
		flInterval = 0.0;

	st->frame += flInterval * framerate * st->framerate;
	st->animtime = gEngfuncs.GetClientTime();

	if( st->frame < 0.0 || st->frame >= 256.0 )
	{
		if( st->m_fSequenceLoops )
			st->frame -= (int)( st->frame / 256.0 ) * 256.0;
		else
			st->frame = ( st->frame < 0.0 ) ? 0 : 255;

		st->m_fSequenceFinished = TRUE;	// just in case it wasn't caught in GetEvents
	}

	return flInterval;
}

/*
====================
SetupClientAnimation

====================
*/
void CGameStudioModelRenderer::SetupClientAnimation( entity_state_t *pplayer )
{
	static double oldtime;
	double curtime, dt;
	int oldseq;
	void *pmodel;
	float flFrameRate, flGroundSpeed;
	cl_entity_t *pent = IEngineStudio.GetCurrentEntity();

	if( !pent )
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = curtime - oldtime;

	if( dt < 0 )
		dt = 0;
	else if( dt > 1.0 )
		dt = 1.0;

	oldtime = curtime;

	oldseq = g_clientstate.sequence;
	g_clientstate.framerate = 1.0;

	DoD_GetSequence( &g_clientstate.sequence, &g_clientstate.gaitsequence );
	DoD_GetOrientation( g_clientstate.origin, g_clientstate.angles );
	VectorCopy( g_clientstate.angles, g_clientstate.realangles );

	if( g_clientstate.sequence != oldseq )
	{
		g_clientstate.frame = 0;
		g_clientstate.lv.prevsequence = oldseq;
		g_clientstate.lv.sequencetime = g_clientstate.animtime;
		memcpy( g_clientstate.lv.prevseqblending, g_clientstate.blending, sizeof( g_clientstate.blending ) );
		memcpy( g_clientstate.lv.prevcontroller, g_clientstate.controller, sizeof( g_clientstate.controller ) );
	}

	pmodel = IEngineStudio.Mod_Extradata( pent->model );

	flFrameRate = 0;
	flGroundSpeed = 0;
	GetSequenceInfo( pmodel, &g_clientstate, &flFrameRate, &flGroundSpeed );

	g_clientstate.m_fSequenceLoops = ( GetSequenceFlags( pmodel, &g_clientstate ) & STUDIO_LOOPING ) != 0;

	StudioFrameAdvance( &g_clientstate, flFrameRate, (float)dt );

	VectorCopy( g_clientstate.realangles, pent->angles );
	VectorCopy( g_clientstate.angles, pent->curstate.angles );
	VectorCopy( g_clientstate.origin, pent->curstate.origin );
	pent->curstate.sequence = g_clientstate.sequence;
	pplayer->gaitsequence = g_clientstate.gaitsequence;
	pent->curstate.frame = g_clientstate.frame;
	pent->curstate.animtime = g_clientstate.animtime;
	pent->curstate.framerate = g_clientstate.framerate;
	memcpy( pent->curstate.blending, g_clientstate.blending, sizeof( g_clientstate.blending ) );
	memcpy( pent->curstate.controller, g_clientstate.controller, sizeof( g_clientstate.controller ) );
	pent->latched = g_clientstate.lv;
}

/*
====================
RestorePlayerState

====================
*/
void CGameStudioModelRenderer::RestorePlayerState( entity_state_t *pplayer )
{
	cl_entity_t *pent = IEngineStudio.GetCurrentEntity();

	if( !pent )
		return;

	VectorCopy( pent->curstate.angles, g_clientstate.angles );
	VectorCopy( pent->curstate.origin, g_clientstate.origin );
	VectorCopy( pent->angles, g_clientstate.realangles );
	g_clientstate.sequence = pent->curstate.sequence;
	g_clientstate.gaitsequence = pplayer->gaitsequence;
	g_clientstate.animtime = pent->curstate.animtime;
	g_clientstate.frame = pent->curstate.frame;
	g_clientstate.framerate = pent->curstate.framerate;
	memcpy( g_clientstate.blending, pent->curstate.blending, sizeof( g_clientstate.blending ) );
	memcpy( g_clientstate.controller, pent->curstate.controller, sizeof( g_clientstate.controller ) );
	g_clientstate.lv = pent->latched;

	VectorCopy( g_state.angles, pent->curstate.angles );
	VectorCopy( g_state.origin, pent->curstate.origin );
	VectorCopy( g_state.realangles, pent->angles );
	pent->curstate.sequence = g_state.sequence;
	pplayer->gaitsequence = g_state.gaitsequence;
	pent->curstate.animtime = g_state.animtime;
	pent->curstate.frame = g_state.frame;
	pent->curstate.framerate = g_state.framerate;
	memcpy( pent->curstate.blending, g_state.blending, sizeof( g_state.blending ) );
	memcpy( pent->curstate.controller, g_state.controller, sizeof( g_state.controller ) );
	pent->latched = g_state.lv;
}

/*
====================
StudioDrawPlayer

====================
*/
int CGameStudioModelRenderer::StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	alight_t lighting;
	vec3_t dir;

	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();
	IEngineStudio.GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	IEngineStudio.GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	IEngineStudio.GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	m_nPlayerIndex = pplayer->number - 1;

	if( m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients() )
		return 0;

	m_pRenderModel = IEngineStudio.SetupPlayerModel( m_nPlayerIndex );

	if( m_pRenderModel == NULL )
		return 0;

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( m_pRenderModel );
	IEngineStudio.StudioSetHeader( m_pStudioHeader );
	IEngineStudio.SetRenderModel( m_pRenderModel );

	if( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0 )
		m_pCurrentEntity->curstate.sequence = 0;

	if( pplayer->sequence >= m_pStudioHeader->numseq || pplayer->sequence < 0 )
		pplayer->sequence = 0;

	if( m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.gaitsequence < 0 )
		m_pCurrentEntity->curstate.gaitsequence = 0;

	if( pplayer->gaitsequence >= m_pStudioHeader->numseq || pplayer->gaitsequence < 0 )
		pplayer->gaitsequence = 0;

	if( pplayer->gaitsequence )
	{
		vec3_t orig_angles;

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		VectorCopy( m_pCurrentEntity->angles, orig_angles );

		StudioProcessGait( pplayer );

		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = NULL;

		StudioSetUpTransform( 0 );
		VectorCopy( orig_angles, m_pCurrentEntity->angles );
	}
	else
	{
		m_pCurrentEntity->curstate.controller[0] = 127;
		m_pCurrentEntity->curstate.controller[1] = 127;
		m_pCurrentEntity->curstate.controller[2] = 127;
		m_pCurrentEntity->curstate.controller[3] = 127;
		m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
		m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
		m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
		m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		CalculatePitchBlend( pplayer );
		CalculateYawBlend( pplayer );

		m_pPlayerInfo->gaitsequence = 0;

		StudioSetUpTransform( 0 );
	}

	if( flags & STUDIO_RENDER )
	{
		// see if the bounding box lets us trivially reject, also sets
		if( !IEngineStudio.StudioCheckBBox() )
			return 0;

		( *m_pModelsDrawn )++;
		( *m_pStudioModelCount )++; // render data cache cookie

		if( m_pStudioHeader->numbodyparts == 0 )
			return 1;
	}

	m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );
	StudioSetupBones();
	StudioSaveBones();
	m_pPlayerInfo->renderframe = m_nFrameCount;

	m_pPlayerInfo = NULL;

	if( flags & STUDIO_EVENTS )
	{
		StudioCalcAttachments();
		StudioClientEvents();

		// copy attachments into global entity array
		if( m_pCurrentEntity->index > 0 )
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex( m_pCurrentEntity->index );

			memcpy( ent->attachment, m_pCurrentEntity->attachment, sizeof( vec3_t ) * 4 );
		}
	}

	if( flags & STUDIO_RENDER )
	{
		lighting.plightvec = dir;

		IEngineStudio.StudioDynamicLight( m_pCurrentEntity, &lighting );

		IEngineStudio.StudioEntityLight( &lighting );

		// model and frame independant
		IEngineStudio.StudioSetupLighting( &lighting );

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		// get remap colors
		m_nTopColor = m_pPlayerInfo->topcolor;
		m_nBottomColor = m_pPlayerInfo->bottomcolor;

		if( m_nTopColor < 0 )
			m_nTopColor = 0;

		if( m_nTopColor > 360 )
			m_nTopColor = 360;

		if( m_nBottomColor < 0 )
			m_nBottomColor = 0;

		if( m_nBottomColor > 360 )
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors( m_nTopColor, m_nBottomColor );

		StudioRenderModel();

		m_pPlayerInfo = NULL;

		if( pplayer->weaponmodel )
		{
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex( pplayer->weaponmodel );

			if( pweaponmodel )
			{
				model_t *pswapmodel = SwapPWpnModels( m_pCurrentEntity, pweaponmodel );

				if( pswapmodel )
					pweaponmodel = pswapmodel;
			}

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( pweaponmodel );
			IEngineStudio.StudioSetHeader( m_pStudioHeader );

			if( m_pCurrentEntity->player )
			{
				pmodel_fx_t *pInfo = &g_PModelFxInfo[m_pCurrentEntity->index];

				if( pInfo->bAnim )
				{
					// the weapon plays another sequence from the frame of the event of the player model
					if( pplayer->sequence == pInfo->iAnimSeq )
					{
						if( pplayer->frame >= pInfo->iAnimFrame )
						{
							m_pCurrentEntity->curstate.frame = pInfo->iAnimFrame;
							m_pCurrentEntity->curstate.sequence = pInfo->iAnimTargetSeq;
							pInfo->bAnim = false;
						}
					}
					else
					{
						pInfo->bAnim = false;
					}
				}
				else
				{
					m_pCurrentEntity->curstate.frame = 0;
					m_pCurrentEntity->curstate.sequence = 0;
				}

				// the weapon is not a player
				m_pCurrentEntity->player = 0;
			}

			StudioMergeBones( pweaponmodel );
			IEngineStudio.StudioSetupLighting( &lighting );

			StudioCalcAttachments();

			m_pCurrentEntity->curstate.animtime = gEngfuncs.GetClientTime() + 1.0;

			StudioClientEvents();

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( pweaponmodel );
			IEngineStudio.StudioSetHeader( m_pStudioHeader );

			StudioRenderModel();

			*m_pCurrentEntity = saveent;
		}
	}

	return 1;
}

/*
====================
StudioFxTransform

====================
*/
void CGameStudioModelRenderer::StudioFxTransform( cl_entity_t *ent, float transform[3][4] )
{
	switch( ent->curstate.renderfx )
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
		if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			int axis = gEngfuncs.pfnRandomLong( 0, 1 );

			// Choose between x & z
			if( axis == 1 )
				axis = 2;

			VectorScale( transform[axis], gEngfuncs.pfnRandomFloat( 1, 1.484 ), transform[axis] );
		}
		else if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			float offset;
			int axis = gEngfuncs.pfnRandomLong( 0, 1 );

			// Choose between x & z
			if( axis == 1 )
				axis = 2;

			offset = gEngfuncs.pfnRandomFloat( -10, 10 );
			transform[gEngfuncs.pfnRandomLong( 0, 2 )][3] += offset;
		}
		break;
	case kRenderFxExplode:
		{
			if( iRenderStateChanged )
			{
				g_flStartScaleTime = (float)m_clTime;
				iRenderStateChanged = false;
			}

			// Make the model continue to shrink
			float flTimeDelta = (float)( m_clTime - g_flStartScaleTime );

			if( flTimeDelta > 0 )
			{
				float flScale = 0.001f;

				// Goes almost all away
				if( flTimeDelta <= 2.0f )
					flScale = 1.0f - ( flTimeDelta / 2.0f );

				for( int i = 0; i < 3; i++ )
				{
					for( int j = 0; j < 3; j++ )
						transform[i][j] *= flScale;
				}
			}
		}
		break;
	}
}

////////////////////////////////////
// Hooks to class implementation
////////////////////////////////////

/*
====================
R_StudioDrawPlayer

====================
*/
int R_StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	return g_StudioRenderer.StudioDrawPlayer( flags, pplayer );
}

/*
====================
R_StudioDrawModel

====================
*/
int R_StudioDrawModel( int flags )
{
	return g_StudioRenderer.StudioDrawModel( flags );
}

/*
====================
R_StudioInit

====================
*/
void R_StudioInit( void )
{
	g_StudioRenderer.Init();
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

/*
====================
HUD_GetStudioModelInterface

Export this function for the engine to use the studio renderer class to render objects.
====================
*/
extern "C" int DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	if( version != STUDIO_INTERFACE_VERSION )
		return 0;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy( &IEngineStudio, pstudio, sizeof( IEngineStudio ) );

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return 1;
}

/*
====================
SwapPWpnModels

Replaces the third person model of the weapon: the bipod and sandbag variants and the models with the left hand reload
====================
*/
model_t *CGameStudioModelRenderer::SwapPWpnModels( cl_entity_t *vplayer, model_t *pweaponmodel )
{
	mstudioseqdesc_t *pseqdesc;
	pmodel_fx_t *pInfo;
	void *pmodel;
	float fl_EstFrame;
	bool bSwitch, bPending;
	bool bBipodDeployed, bSandbagDeployed;
	int iSwitchSeq;
	int modelindex;
	int seq;
	int i;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;
	fl_EstFrame = StudioEstimateFrame( pseqdesc );

	if( m_pCurrentEntity->index < 1 || m_pCurrentEntity->index > 63 )
		return pweaponmodel;

	pInfo = &g_PModelFxInfo[m_pCurrentEntity->index];
	pmodel = IEngineStudio.Mod_Extradata( m_pCurrentEntity->model );

	bBipodDeployed = IsBipodAnimation( m_pCurrentEntity->curstate.sequence, pmodel );
	bSandbagDeployed = bBipodDeployed ? false : IsSandbagAnimation( m_pCurrentEntity->curstate.sequence, pmodel );

	bSwitch = pInfo->bSwitch;
	iSwitchSeq = pInfo->iSwitchSeq;

	for( i = 0; i < 41; i++ )
	{
		seq = m_pCurrentEntity->curstate.sequence;

		// the model switch of the sequence was not reached yet
		bPending = bSwitch && ( pInfo->iSwitchFrame == -1 || pInfo->iSwitchFrame > fl_EstFrame );

		if( bBipodDeployed )
		{
			if( bPending && seq == iSwitchSeq )
			{
				if( !P_WpnInfo[i].pronereload[0] )
					continue;

				if( !stricmp( pweaponmodel->name, P_WpnInfo[i].bipoddown ) || !stricmp( pweaponmodel->name, P_WpnInfo[i].bipodup ) )
				{
					gEngfuncs.CL_LoadModel( P_WpnInfo[i].pronereload, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}

				continue;
			}

			if( !bPending && seq == iSwitchSeq )
				continue;

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( !P_WpnInfo[i].bipoddown[0] )
				continue;

			if( !stricmp( pweaponmodel->name, P_WpnInfo[i].pronereload ) || !stricmp( pweaponmodel->name, P_WpnInfo[i].bipodup ) )
			{
				gEngfuncs.CL_LoadModel( P_WpnInfo[i].bipoddown, &modelindex );
				return IEngineStudio.GetModelByIndex( modelindex );
			}
		}
		else if( bSandbagDeployed )
		{
			if( bPending && seq == iSwitchSeq )
			{
				if( !P_WpnInfo[i].sandbagreload[0] )
					continue;

				if( !stricmp( pweaponmodel->name, P_WpnInfo[i].bipodup ) || !stricmp( pweaponmodel->name, P_WpnInfo[i].bipoddown ) )
				{
					gEngfuncs.CL_LoadModel( P_WpnInfo[i].sandbagreload, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}

				continue;
			}

			if( !bPending && seq == iSwitchSeq )
				continue;

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( !P_WpnInfo[i].bipoddown[0] )
				continue;

			if( !stricmp( pweaponmodel->name, P_WpnInfo[i].sandbagreload ) || !stricmp( pweaponmodel->name, P_WpnInfo[i].bipodup ) )
			{
				gEngfuncs.CL_LoadModel( P_WpnInfo[i].bipoddown, &modelindex );
				return IEngineStudio.GetModelByIndex( modelindex );
			}
		}
		else
		{
			if( bPending && seq == iSwitchSeq )
			{
				if( P_WpnInfo[i].pmodel[0] && !stricmp( pweaponmodel->name, P_WpnInfo[i].pmodel ) )
				{
					gEngfuncs.CL_LoadModel( P_WpnInfo[i].leftreload, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}

				continue;
			}

			if( !bPending && seq == iSwitchSeq )
				continue;

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( P_WpnInfo[i].leftreload[0] && !stricmp( pweaponmodel->name, P_WpnInfo[i].leftreload ) )
			{
				gEngfuncs.CL_LoadModel( P_WpnInfo[i].pmodel, &modelindex );
				return IEngineStudio.GetModelByIndex( modelindex );
			}
		}
	}

	return pweaponmodel;
}

/*
====================
AnimatePWpnModels

====================
*/
void CGameStudioModelRenderer::AnimatePWpnModels( cl_entity_t *vplayer )
{
	/* Nothing. */
}

/*
====================
StudioClientEvents

The events of the sequence of the model, the first four events of a player are fired only once
====================
*/
void CGameStudioModelRenderer::StudioClientEvents( void )
{
	mstudioseqdesc_t *pseqdesc;
	mstudioevent_t *pevent;
	float f, flStart;
	int *pFired;
	int i;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	if( !pseqdesc->numevents )
		return;

	pevent = (mstudioevent_t *)( (byte *)m_pStudioHeader + pseqdesc->eventindex );

	f = StudioEstimateFrame( pseqdesc );
	flStart = f - m_pCurrentEntity->curstate.framerate * gHUD.m_flTimeDelta * pseqdesc->fps;

	// the sequence has just started
	if( m_pCurrentEntity->latched.sequencetime == m_pCurrentEntity->curstate.animtime )
		flStart = ( pseqdesc->flags & STUDIO_LOOPING ) ? flStart : -0.01f;

	if( m_pCurrentEntity->curstate.sequence != m_pCurrentEntity->prevstate.sequence )
	{
		m_pCurrentEntity->baseline.iuser1 = 0;
		m_pCurrentEntity->baseline.iuser2 = 0;
		m_pCurrentEntity->baseline.iuser3 = 0;
		m_pCurrentEntity->baseline.iuser4 = 0;
	}

	for( i = 0; i < pseqdesc->numevents; i++ )
	{
		// the events of the engine are not handled here
		if( pevent[i].event < 5000 )
			continue;

		switch( i )
		{
		case 0:
			pFired = &m_pCurrentEntity->baseline.iuser1;
			break;
		case 1:
			pFired = &m_pCurrentEntity->baseline.iuser2;
			break;
		case 2:
			pFired = &m_pCurrentEntity->baseline.iuser3;
			break;
		case 3:
			pFired = &m_pCurrentEntity->baseline.iuser4;
			break;
		default:
			pFired = NULL;
			break;
		}

		if( pFired && m_pCurrentEntity->player )
		{
			if( f >= pevent[i].frame && !*pFired )
			{
				HUD_StudioEvent( &pevent[i], m_pCurrentEntity );
				*pFired = 1;
			}
		}
		else if( pevent[i].frame > flStart && f >= pevent[i].frame )
		{
			if( m_pCurrentEntity->player )
				gEngfuncs.Con_DPrintf( "firing event %d\n", pevent[i].frame );

			HUD_StudioEvent( &pevent[i], m_pCurrentEntity );
		}
	}
}
