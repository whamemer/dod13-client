//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "dod_shared.h"

#define ANIM_WALK_SEQUENCE 3
#define ANIM_JUMP_SEQUENCE 6
#define ANIM_SWIM_1 8
#define ANIM_SWIM_2 9
#define ANIM_FIRST_DEATH_SEQUENCE 101
#define ANIM_LAST_DEATH_SEQUENCE 159
#define ANIM_FIRST_EMOTION_SEQUENCE 198
#define ANIM_LAST_EMOTION_SEQUENCE 207

//
// Override the StudioModelRender virtual member functions here to implement custom bone
// setup, blending, etc.
//

extern p_wpninfo_s *WpnInfo;
float g_flStartScaleTime;
int iPrevRenderState;
int iRenderStateChanged;

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

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

client_anim_state_t g_state;
client_anim_state_t g_clientstate;

int iNumBipodAnims;
int iNumSandbagAnims;
int iBipodAnims[64];
int iSandbagAnims[64];

void AddToBipodAnimTable( int seq )
{
	if( iNumBipodAnims <= 63 )
	{
		iBipodAnims[iNumBipodAnims] = seq;
		iNumBipodAnims++;
	}
}

void AddToSandbagAnimTable( int seq )
{
	if( iNumSandbagAnims <= 63 )
	{
		iSandbagAnims[iNumSandbagAnims] = seq;
		iNumSandbagAnims++;
	}
}

void BuildDeployAnimTable( studiohdr_t *pstudiohdr )
{
	mstudioseqdesc_t *pseqdesc;
	int i;

	if( !pstudiohdr )
		return;

	iNumBipodAnims = 0;
	iNumSandbagAnims = 0;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex );

	for( i = 0; i < pstudiohdr->numseq; ++i )
	{
		if( strncmp( pseqdesc[i].label, "bipod_", 6 ) == 0 )
			AddToBipodAnimTable( i );

		else if( strncmp( pseqdesc[i].label, "sandbag_", 8 ) == 0 )
			AddToSandbagAnimTable( i );

	}
}

bool IsBipodAnimation( int seq, studiohdr_t *pmodel )
{
	if( iNumBipodAnims == 0 )
		BuildDeployAnimTable( pmodel );

	for( int i = 0; i < iNumBipodAnims; ++i )
	{
		if( iBipodAnims[i] == seq )
			return true;
	}

	return false;
}

bool IsSandbagAnimation( int seq, studiohdr_t *pmodel )
{
	if( iNumSandbagAnims == 0 )
		BuildDeployAnimTable( pmodel );

	for( int i = 0; i < iNumSandbagAnims; ++i )
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
}

mstudioanim_t *CGameStudioModelRenderer::LookupAnimation( mstudioseqdesc_t *pseqdesc, int index )
{
	mstudioanim_t *panim = NULL;

	panim = StudioGetAnim( m_pRenderModel, pseqdesc );

	if( index < 0 )
		return panim;

	if( index > ( pseqdesc->numblends - 1 ) )
		return panim;

	panim += index * m_pStudioHeader->numbones;
	return panim;
}

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

	if( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq )
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;
	panim = StudioGetAnim( m_pRenderModel, pseqdesc );

	f = StudioEstimateFrame( pseqdesc );

	if( m_pPlayerInfo->gaitsequence == ANIM_WALK_SEQUENCE )
	{
		if( m_pCurrentEntity->curstate.blending[0] <= 26 )
		{
			m_pCurrentEntity->curstate.blending[0] = 0;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
		else
		{
			m_pCurrentEntity->curstate.blending[0] -= 26;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
	}

	if( pseqdesc->numblends == 9 )
	{
		float s = m_pCurrentEntity->curstate.blending[0];
		float t = m_pCurrentEntity->curstate.blending[1];

		if( s <= 127.0 )
		{
			s = ( s * 2.0 );

			if( t <= 127.0 )
			{
				t = ( t * 2.0 );

				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 1 );
				StudioCalcRotations( pos2, q2, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 3 );
				StudioCalcRotations( pos3, q3, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 4 );
				StudioCalcRotations( pos4, q4, pseqdesc, panim, f );
			}
			else
			{
				t = 2.0 * ( t - 127.0 );

				panim = LookupAnimation( pseqdesc, 3 );
				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 4 );
				StudioCalcRotations( pos2, q2, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 6 );
				StudioCalcRotations( pos3, q3, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 7 );
				StudioCalcRotations( pos4, q4, pseqdesc, panim, f );
			}
		}
		else
		{
			s = 2.0 * ( s - 127.0 );

			if( t <= 127.0 )
			{
				t = ( t * 2.0 );

				panim = LookupAnimation( pseqdesc, 1 );
				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 2 );
				StudioCalcRotations( pos2, q2, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 4 );
				StudioCalcRotations( pos3, q3, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 5 );
				StudioCalcRotations( pos4, q4, pseqdesc, panim, f );
			}
			else
			{
				t = 2.0 * ( t - 127.0 );

				panim = LookupAnimation( pseqdesc, 4 );
				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 5 );
				StudioCalcRotations( pos2, q2, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 7 );
				StudioCalcRotations( pos3, q3, pseqdesc, panim, f );
				panim = LookupAnimation( pseqdesc, 8 );
				StudioCalcRotations( pos4, q4, pseqdesc, panim, f );
			}
		}

		s /= 255.0;
		t /= 255.0;

		StudioSlerpBones( q, pos, q2, pos2, s );
		StudioSlerpBones( q3, pos3, q4, pos4, s );
		StudioSlerpBones( q, pos, q3, pos3, t );
	}
	else
	{
		StudioCalcRotations( pos, q, pseqdesc, panim, f );
	}

	if( m_fDoInterp && m_pCurrentEntity->latched.sequencetime && ( m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime ) && ( m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq ) )
	{
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s = m_pCurrentEntity->latched.prevseqblending[0];
		float t = m_pCurrentEntity->latched.prevseqblending[1];

		pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim( m_pRenderModel, pseqdesc );

		if( pseqdesc->numblends == 9 )
		{
			if( s <= 127.0 )
			{
				s = ( s * 2.0 );

				if( t <= 127.0 )
				{
					t = ( t * 2.0 );

					StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 1 );
					StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 3 );
					StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 4 );
					StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
				}
				else
				{
					t = 2.0 * ( t - 127.0 );

					panim = LookupAnimation( pseqdesc, 3 );
					StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 4 );
					StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 6 );
					StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 7 );
					StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
				}
			}
			else
			{
				s = 2.0 * ( s - 127.0 );

				if( t <= 127.0 )
				{
					t = ( t * 2.0 );

					panim = LookupAnimation( pseqdesc, 1 );
					StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 2 );
					StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 4 );
					StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 5 );
					StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
				}
				else
				{
					t = 2.0 * ( t - 127.0 );

					panim = LookupAnimation( pseqdesc, 4 );
					StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 5 );
					StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 7 );
					StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
					panim = LookupAnimation( pseqdesc, 8 );
					StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
				}
			}

			s /= 255.0;
			t /= 255.0;

			StudioSlerpBones( q1b, pos1b, q2, pos2, s );
			StudioSlerpBones( q3, pos3, q4, pos4, s );
			StudioSlerpBones( q1b, pos1b, q3, pos3, t );
		}
		else
		{
			StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
		}

		s = 1.0 - ( m_clTime - m_pCurrentEntity->latched.sequencetime ) / 0.2;
		StudioSlerpBones( q, pos, q1b, pos1b, s );
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = ( mstudiobone_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->boneindex );

	if( m_pPlayerInfo && ( m_pCurrentEntity->curstate.sequence < ANIM_FIRST_DEATH_SEQUENCE || 
		m_pCurrentEntity->curstate.sequence > ANIM_LAST_DEATH_SEQUENCE ) && ( m_pCurrentEntity->curstate.sequence 
		< ANIM_FIRST_EMOTION_SEQUENCE || m_pCurrentEntity->curstate.sequence > ANIM_LAST_EMOTION_SEQUENCE ) 
		&& m_pCurrentEntity->curstate.sequence != ANIM_SWIM_1 && m_pCurrentEntity->curstate.sequence != ANIM_SWIM_2 )
	{
		int copy = 1;

		if( m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq )
			m_pPlayerInfo->gaitsequence = 0;

		pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim( m_pRenderModel, pseqdesc );
		StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe );

		for( i = 0; i < m_pStudioHeader->numbones; i++ )
		{
			if( !strcmp( pbones[i].name, "Bip01 Spine" ) )
				copy = 0;
			else if( !strcmp( pbones[pbones[i].parent].name, "Bip01 Pelvis" ) )
				copy = 1;

			if( copy )
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
				ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_pbonetransform )[i] );
				MatrixCopy( ( *m_pbonetransform )[i], ( *m_plighttransform )[i] );
			}
			else
			{
				ConcatTransforms( ( *m_paliastransform ), bonematrix, ( *m_pbonetransform )[i] );
				ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_plighttransform )[i] );
			}

			StudioFxTransform( m_pCurrentEntity, ( *m_pbonetransform )[i] );
		}
		else
		{
			ConcatTransforms( ( *m_pbonetransform )[pbones[i].parent], bonematrix, ( *m_pbonetransform )[i] );
			ConcatTransforms( ( *m_plighttransform )[pbones[i].parent], bonematrix, ( *m_plighttransform )[i] );
		}
	}
}

void CGameStudioModelRenderer::StudioEstimateGait( entity_state_t *pplayer )
{
	float dt;
	vec3_t est_velocity;

	dt = ( m_clTime - m_clOldTime );
	dt = max( 0.0, dt );
	dt = min( 1.0, dt );

	if( dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount )
	{
		m_flGaitMovement = 0;
		return;
	}

	if( m_fGaitEstimation )
	{
		VectorSubtract( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity );
		VectorCopy( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin );
		m_flGaitMovement = est_velocity.Length();

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
		m_flGaitMovement = est_velocity.Length() * dt;
	}

	if( est_velocity[1] == 0 && est_velocity[0] == 0 )
	{
		float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - ( int ) ( flYawDiff / 360 ) * 360;

		if( flYawDiff > 180 )
			flYawDiff -= 360;

		if( flYawDiff < -180 )
			flYawDiff += 360;

		if( dt < 0.25 )
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - ( int ) ( m_pPlayerInfo->gaityaw / 360 ) * 360;

		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = ( atan2( est_velocity[1], est_velocity[0] ) * 180 / M_PI );

		if( m_pPlayerInfo->gaityaw > 180 )
			m_pPlayerInfo->gaityaw = 180;

		if( m_pPlayerInfo->gaityaw < -180 )
			m_pPlayerInfo->gaityaw = -180;
	}
}

void CGameStudioModelRenderer::StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch )
{
	float range = 45.0;

	*pBlend = ( *pPitch * 3 );

	if( *pBlend <= -range )
		*pBlend = 255;
	else if( *pBlend >= range )
		*pBlend = 0;
	else
		*pBlend = 255 * ( range - *pBlend ) / ( 2 * range );

	*pPitch = 0;
}

void CGameStudioModelRenderer::CalculatePitchBlend( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	int iBlend;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend( pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH] );

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = m_pCurrentEntity->curstate.blending[1];
	m_pCurrentEntity->latched.prevseqblending[1] = m_pCurrentEntity->curstate.blending[1];
}

void CGameStudioModelRenderer::CalculateYawBlend( entity_state_t *pplayer )
{
	float flYaw;

	StudioEstimateGait( pplayer );

	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	flYaw = fmod( flYaw, 360.0f );

	if( flYaw < -180 )
		flYaw = flYaw + 360;
	else if( flYaw > 180 )
		flYaw = flYaw - 360;

	float maxyaw = 120.0;

	if( flYaw > maxyaw )
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if( flYaw < -maxyaw )
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	float blend_yaw = ( flYaw / 90.0 ) * 128.0 + 127.0;

	blend_yaw = 255.0 - bound( 0.0, blend_yaw, 255.0 );

	m_pCurrentEntity->curstate.blending[0] = ( int ) ( blend_yaw );
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

	if( m_pCurrentEntity->angles[YAW] < -0 )
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];
}

void CGameStudioModelRenderer::StudioProcessGait( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;

	CalculateYawBlend( pplayer );
	CalculatePitchBlend( pplayer );

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + pplayer->gaitsequence;

	if( pseqdesc->linearmovement[0] > 0 )
		m_pPlayerInfo->gaitframe += ( m_flGaitMovement / pseqdesc->linearmovement[0] ) * pseqdesc->numframes;
	else
	{
		float dt = bound( 0.0, ( m_clTime - m_clOldTime ), 1.0 );
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;
	}

	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - ( int ) ( m_pPlayerInfo->gaitframe / pseqdesc->numframes ) * pseqdesc->numframes;

	if( m_pPlayerInfo->gaitframe < 0 )
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

void CGameStudioModelRenderer::SavePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if( !ent )
		return;

	st = &g_state;
	st->angles = ent->curstate.angles;
	st->origin = ent->curstate.origin;
	st->realangles = ent->angles;
	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy( st->blending, ent->curstate.blending, 2 );
	memcpy( st->controller, ent->curstate.controller, 4 );

	st->lv = ent->latched;
}

void GetSequenceInfo( void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = ( studiohdr_t * )pmodel;

	if( !pstudiohdr )
		return;

	mstudioseqdesc_t *pseqdesc;

	if( pev->sequence >= pstudiohdr->numseq )
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex ) + ( int ) pev->sequence;

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

int GetSequenceFlags( void *pmodel, client_anim_state_t *pev )
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = ( studiohdr_t * ) pmodel;

	if( !pstudiohdr || pev->sequence >= pstudiohdr->numseq )
		return 0;

	mstudioseqdesc_t *pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex ) + ( int ) pev->sequence;

	return pseqdesc->flags;
}

float StudioFrameAdvance( client_anim_state_t *st, float framerate, float flInterval )
{
	if( flInterval == 0.0 )
	{
		flInterval = ( gEngfuncs.GetClientTime() - st->animtime );

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
			st->frame -= ( int ) ( st->frame / 256.0 ) * 256.0;
		else
			st->frame = ( st->frame < 0.0 ) ? 0 : 255;

		st->m_fSequenceFinished = TRUE;
	}

	return flInterval;
}

extern void DoD_GetSequence( int *seq, int *gaitseq );
extern void DoD_GetOrientation( float *o, float *a );

void CGameStudioModelRenderer::SetupClientAnimation( entity_state_t *pplayer )
{
	static double oldtime;
	double curtime, dt;

	client_anim_state_t *st;
	float fr, gs;

	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if( !ent )
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = bound( 0.0, ( curtime - oldtime ), 1.0 );

	oldtime = curtime;
	st = &g_clientstate;

	st->framerate = 1.0;

	int oldseq = st->sequence;
	DoD_GetSequence( &st->sequence, &st->gaitsequence );
	DoD_GetOrientation( ( float * ) &st->origin, ( float * ) &st->angles );
	VectorCopy( st->angles, st->realangles );

	if( st->sequence != oldseq )
	{
		st->frame = 0.0;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		memcpy( st->lv.prevseqblending, st->blending, 2 );
		memcpy( st->lv.prevcontroller, st->controller, 4 );
	}

	void *pmodel = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( ent->model );

	if( !pmodel )
		return;


	GetSequenceInfo( pmodel, st, &fr, &gs );
	st->m_fSequenceLoops = ( ( GetSequenceFlags( pmodel, st ) & STUDIO_LOOPING ) != 0 );
	StudioFrameAdvance( st, fr, dt );

	ent->angles = st->realangles;

	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy( ent->curstate.blending, st->blending, 2 );
	memcpy( ent->curstate.controller, st->controller, 4 );

	ent->latched = st->lv;
}

void CGameStudioModelRenderer::RestorePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if( !ent )
		return;

	st = &g_clientstate;
	st->angles = ent->curstate.angles;
	st->origin = ent->curstate.origin;
	st->realangles = ent->angles;
	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy( st->blending, ent->curstate.blending, 2 );
	memcpy( st->controller, ent->curstate.controller, 4 );

	st->lv = ent->latched;

	st = &g_state;

	ent->angles = st->realangles;

	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy( ent->curstate.blending, st->blending, 2 );
	memcpy( ent->curstate.controller, st->controller, 4 );

	ent->latched = st->lv;
}

extern pmodel_fx_t g_PModelFxInfo[65];

int CGameStudioModelRenderer::StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	alight_t lighting;
	vec3_t dir, orig_angles;
	cl_entity_t *ent, saveent;
	model_t *pweaponmodel, *pnewweaponmodel;
	pmodel_fx_t *pInfo;

	ent = IEngineStudio.GetCurrentEntity();
	m_pCurrentEntity = ent;

	IEngineStudio.GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	IEngineStudio.GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	IEngineStudio.GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	m_nPlayerIndex = pplayer->number - 1;

	if( m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients() )
		return 0;

	m_pRenderModel = IEngineStudio.SetupPlayerModel( m_nPlayerIndex );

	if( !m_pRenderModel )
		return 0;

	m_pStudioHeader = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( m_pRenderModel );
	IEngineStudio.StudioSetHeader( m_pStudioHeader );
	IEngineStudio.SetRenderModel( m_pRenderModel );
	int numseq = m_pStudioHeader->numseq;

	if( ent->curstate.sequence >= numseq || ent->curstate.sequence < 0 )
		ent->curstate.sequence = 0;

	if( pplayer->sequence >= numseq || pplayer->sequence < 0 )
		pplayer->sequence = 0;

	if( ent->curstate.gaitsequence >= numseq || ent->curstate.gaitsequence < 0 )
		ent->curstate.gaitsequence = 0;

	if( pplayer->gaitsequence >= numseq || pplayer->gaitsequence < 0 )
		pplayer->gaitsequence = 0;


	if( pplayer->gaitsequence )
	{
		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );
		orig_angles = ent->angles;
		StudioProcessGait( pplayer );
		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = NULL;
		StudioSetUpTransform( 0 );
		ent->angles = orig_angles;
	}
	else
	{
		memset( ent->curstate.controller, 127, sizeof( ent->curstate.controller ) );
		memcpy( ent->latched.prevcontroller, ent->curstate.controller, sizeof( ent->curstate.controller ) );
		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );
		CalculateYawBlend( pplayer );
		CalculatePitchBlend( pplayer );
		m_pPlayerInfo->gaitsequence = 0;
		StudioSetUpTransform( 0 );
	}

	if( ( flags & STUDIO_RENDER ) != 0 && !IEngineStudio.StudioCheckBBox() )
		return 0;

	*m_pModelsDrawn++;
	*m_pStudioModelCount++;

	if( m_pStudioHeader->numbodyparts )
	{
		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		StudioSetupBones();
		StudioSaveBones();

		m_pPlayerInfo->renderframe = m_nFrameCount;
		m_pPlayerInfo = NULL;
	}

	if( ( flags & STUDIO_EVENTS ) != 0 )
	{
		StudioCalcAttachments();
		StudioClientEvents();

		if( ent->index > 0 )
		{
			cl_entity_t *pPlayerEnt = gEngfuncs.GetEntityByIndex( ent->index );
			memcpy( pPlayerEnt->attachment, ent->attachment, sizeof( ent->attachment ) );
		}
	}

	if( ( flags & STUDIO_RENDER ) != 0 )
	{
		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight( ent, &lighting );
		IEngineStudio.StudioEntityLight( &lighting );
		IEngineStudio.StudioSetupLighting( &lighting );

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		m_nTopColor = m_pPlayerInfo->topcolor;

		if( m_nTopColor < 0 ) 
			m_nTopColor = 0;

		else if( m_nTopColor > 360 ) 
			m_nTopColor = 360;

		m_nBottomColor = m_pPlayerInfo->bottomcolor;

		if( m_nBottomColor < 0 )        
			m_nBottomColor = 0;

		else if( m_nBottomColor > 360 ) 
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors( m_nTopColor, m_nBottomColor );

		StudioRenderModel();
		m_pPlayerInfo = NULL;

		if( pplayer->weaponmodel )
		{
			saveent = *ent;

			pweaponmodel = IEngineStudio.GetModelByIndex( pplayer->weaponmodel );
			pnewweaponmodel = pweaponmodel;

			if( pweaponmodel )
			{
				model_t *pSwappedModel = SwapPWpnModels( ent, pweaponmodel );

				if( pSwappedModel )
					pnewweaponmodel = pSwappedModel;
			}

			m_pStudioHeader = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( pnewweaponmodel );
			IEngineStudio.StudioSetHeader( m_pStudioHeader );

			if( ent->player )
			{
				pInfo = &g_PModelFxInfo[ent->index];
				if( pInfo->bAnim )
				{
					if( pplayer->sequence == pInfo->iAnimSeq )
					{
						if( pplayer->frame >= pInfo->iAnimFrame )
						{
							ent->curstate.frame = pInfo->iAnimFrame;
							ent->curstate.sequence = pInfo->iAnimTargetSeq;
						}
					}
					pInfo->bAnim = false;
				}
				else
				{
					ent->curstate.frame = 0.0f;
					ent->curstate.sequence = 0;
				}

				ent->player = 0;
			}

			StudioMergeBones( pnewweaponmodel );
			IEngineStudio.StudioSetupLighting( &lighting );
			StudioCalcAttachments();
			ent->curstate.animtime = gEngfuncs.GetClientTime() + 1.0f;
			StudioClientEvents();
			m_pStudioHeader = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( pnewweaponmodel );
			IEngineStudio.StudioSetHeader( m_pStudioHeader );
			StudioRenderModel();

			*ent = saveent;
		}
	}

	return 1;
}

void CGameStudioModelRenderer::StudioFxTransform( cl_entity_t *ent, float transform[3][4] )
{
	switch( ent->curstate.renderfx )
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
	{
		if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			int axis = gEngfuncs.pfnRandomLong( 0, 1 );

			if( axis == 1 )
				axis = 2;

			VectorScale( transform[axis], gEngfuncs.pfnRandomFloat( 1, 1.484 ), transform[axis] );
		}
		else if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			float offset;

			offset = gEngfuncs.pfnRandomFloat( -10, 10 );
			transform[gEngfuncs.pfnRandomLong( 0, 2 )][3] += offset;
		}

		break;
	}
	case kRenderFxExplode:
	{
		if( iRenderStateChanged )
		{
			g_flStartScaleTime = m_clTime;
			iRenderStateChanged = FALSE;
		}

		float flTimeDelta = m_clTime - g_flStartScaleTime;

		if( flTimeDelta > 0 )
		{
			float flScale = 0.001;

			if( flTimeDelta <= 2.0 )
				flScale = 1.0 - ( flTimeDelta / 2.0 );

			for( int i = 0; i < 3; i++ )
			{
				for( int j = 0; j < 3; j++ )
					transform[i][j] *= flScale;
			}
		}

		break;
	}
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
	if ( version != STUDIO_INTERFACE_VERSION )
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

model_t *CGameStudioModelRenderer::SwapPWpnModels( cl_entity_t *vplayer, model_t *pweaponmodel )
{
	mstudioseqdesc_t *pseqdesc;
	pmodel_fx_t *pInfo;
	float fl_EstFrame;
	int modelindex;
	bool bBipodDeployed, bSandbagDeployed, bSwitch;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;
	fl_EstFrame = StudioEstimateFrame( pseqdesc );

	if( ( m_pCurrentEntity->index - 1 ) > 0x3E )
		return pweaponmodel;

	pInfo = &g_PModelFxInfo[m_pCurrentEntity->index];
	studiohdr_t *pStudioHdr = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( m_pCurrentEntity->model );
	int sequence = m_pCurrentEntity->curstate.sequence;

	bBipodDeployed = IsBipodAnimation( sequence, pStudioHdr );

	if( !bBipodDeployed )
		bSandbagDeployed = IsSandbagAnimation( sequence, pStudioHdr );
	else
		bSandbagDeployed = false;

	bSwitch = pInfo->bSwitch;
	int iSwitchSeq = pInfo->iSwitchSeq;

	for( int i = 0; i < 41; ++i )
	{
		sequence = m_pCurrentEntity->curstate.sequence;

		if( bBipodDeployed )
		{
			if( bSwitch && ( pInfo->iSwitchFrame == -1 || pInfo->iSwitchFrame > fl_EstFrame ) )
			{
				if( sequence == iSwitchSeq )
				{
					if( WpnInfo[i].pronereload[0] != '\0' )
					{
						if( strcasecmp( pweaponmodel->name, WpnInfo[i].bipoddown ) == 0 ||
							strcasecmp( pweaponmodel->name, WpnInfo[i].bipodup ) == 0 )
						{
							gEngfuncs.CL_LoadModel( WpnInfo[i].pronereload, &modelindex );
							return IEngineStudio.GetModelByIndex( modelindex );
						}
					}
					continue;
				}
			}
			else if( sequence == iSwitchSeq )
			{
				if( strcasecmp( pweaponmodel->name, WpnInfo[i].pronereload ) == 0 )
				{
					gEngfuncs.CL_LoadModel( WpnInfo[i].bipodup, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}
				continue;
			}

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( strcasecmp( pweaponmodel->name, WpnInfo[i].pronereload ) == 0 )
			{
				gEngfuncs.CL_LoadModel( WpnInfo[i].bipoddown, &modelindex );
				return IEngineStudio.GetModelByIndex( modelindex );
			}
		}

		else if( bSandbagDeployed )
		{
			if( bSwitch && ( pInfo->iSwitchFrame == -1 || pInfo->iSwitchFrame > fl_EstFrame ) )
			{
				if( sequence == iSwitchSeq )
				{
					if( WpnInfo[i].sandbagreload[0] != '\0' )
					{
						if( strcasecmp( pweaponmodel->name, WpnInfo[i].bipodup ) == 0 ||
							strcasecmp( pweaponmodel->name, WpnInfo[i].bipoddown ) == 0 )
						{
							gEngfuncs.CL_LoadModel( WpnInfo[i].sandbagreload, &modelindex );
							return IEngineStudio.GetModelByIndex( modelindex );
						}
					}
					continue;
				}
			}
			else if( sequence == iSwitchSeq )
			{
				if( strcasecmp( pweaponmodel->name, WpnInfo[i].sandbagreload ) == 0 )
				{
					gEngfuncs.CL_LoadModel( WpnInfo[i].bipoddown, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}
				continue;
			}

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( strcasecmp( pweaponmodel->name, WpnInfo[i].sandbagreload ) == 0 )
			{
				gEngfuncs.CL_LoadModel( WpnInfo[i].bipoddown, &modelindex );
				return IEngineStudio.GetModelByIndex( modelindex );
			}
		}
		else
		{
			if( bSwitch && ( pInfo->iSwitchFrame == -1 || pInfo->iSwitchFrame > fl_EstFrame ) )
			{
				if( sequence == iSwitchSeq )
				{
					if( WpnInfo[i].pmodel[0] != '\0' )
					{
						if( strcasecmp( pweaponmodel->name, WpnInfo[i].pmodel ) == 0 )
						{
							gEngfuncs.CL_LoadModel( WpnInfo[i].leftreload, &modelindex );
							return IEngineStudio.GetModelByIndex( modelindex );
						}
					}
					continue;
				}
			}
			else if( sequence == iSwitchSeq )
			{
				if( strcasecmp( pweaponmodel->name, WpnInfo[i].leftreload ) == 0 )
				{
					gEngfuncs.CL_LoadModel( WpnInfo[i].pmodel, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}
				continue;
			}

			pInfo->bSwitch = false;
			pInfo->iSwitchFrame = -1;
			bSwitch = false;

			if( WpnInfo[i].leftreload != '\0' )
			{
				if( strcasecmp( pweaponmodel->name, WpnInfo[i].leftreload ) == 0 )
				{
					gEngfuncs.CL_LoadModel( WpnInfo[i].pmodel, &modelindex );
					return IEngineStudio.GetModelByIndex( modelindex );
				}
			}
		}
	}

	return pweaponmodel;
}

void CGameStudioModelRenderer::AnimatePWpnModels( cl_entity_t *vplayer )
{
	// Nothing.
}

extern "C" void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);

void CGameStudioModelRenderer::StudioClientEvents( void )
{
	mstudioseqdesc_t *pseqdesc;
	mstudioevent_t *pevent;
	int *piFiredEvent;
	float flStart, flEnd;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	if( !pseqdesc->numevents )
		return;

	pevent = ( mstudioevent_t * ) ( ( byte * ) m_pStudioHeader + pseqdesc->eventindex );
	flEnd = StudioEstimateFrame( pseqdesc );
	flStart = flEnd - m_pCurrentEntity->curstate.framerate * gHUD.m_flTimeDelta * pseqdesc->fps;

	if( m_pCurrentEntity->latched.sequencetime == m_pCurrentEntity->curstate.animtime )
	{
		if( !( ( pseqdesc->flags & 1 ) != 0 ) )
			flStart = -0.01f;
	}

	if( m_pCurrentEntity->curstate.sequence != m_pCurrentEntity->prevstate.sequence )
	{
		m_pCurrentEntity->baseline.iuser1 = 0;
		m_pCurrentEntity->baseline.iuser2 = 0;
		m_pCurrentEntity->baseline.iuser3 = 0;
		m_pCurrentEntity->baseline.iuser4 = 0;
	}

	for( int i = 0; i < pseqdesc->numevents; ++i )
	{
		if( pevent[i].event <= 4999 )
			continue;

		piFiredEvent = NULL;

		if( i == 0 )     
			piFiredEvent = &m_pCurrentEntity->baseline.iuser1;
		else if( i == 1 ) 
			piFiredEvent = &m_pCurrentEntity->baseline.iuser2;
		else if( i == 2 ) 
			piFiredEvent = &m_pCurrentEntity->baseline.iuser3;
		else if( i == 3 ) 
			piFiredEvent = &m_pCurrentEntity->baseline.iuser4;

		if( piFiredEvent && m_pCurrentEntity->player )
		{
			if( flEnd >= pevent[i].frame && !( *piFiredEvent ) )
			{
				HUD_StudioEvent( &pevent[i], m_pCurrentEntity );
				*piFiredEvent = 1;
			}
		}
		else
		{
			if( pevent[i].frame > flStart && flEnd >= pevent[i].frame )
			{
				if( m_pCurrentEntity->player )
				{
					gEngfuncs.Con_DPrintf( "firing event %d\n", ( int ) pevent[i].frame );
				}

				HUD_StudioEvent( &pevent[i], m_pCurrentEntity );
			}
		}
	}
}
