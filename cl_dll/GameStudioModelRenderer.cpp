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
	mstudioanim_t *panim;

	panim = StudioGetAnim( m_pRenderModel, pseqdesc );

	if( index >= 0 && index < pseqdesc->numblends )
		panim += m_pStudioHeader->numbones * index;

	return panim;
}

void CGameStudioModelRenderer::StudioSetupBones( void )
{
	static float pos[MAXSTUDIOBONES][3];
	static float q[MAXSTUDIOBONES][4];
	static float pos2[MAXSTUDIOBONES][3];
	static float q2[MAXSTUDIOBONES][4];
	static float pos3[MAXSTUDIOBONES][3];
	static float q3[MAXSTUDIOBONES][4];
	static float pos4[MAXSTUDIOBONES][3];
	static float q4[MAXSTUDIOBONES][4];

	int i, copy;
	double f;
	float s, t, dadt;

	cl_entity_t *m_pCurrentEntity = m_pCurrentEntity;

	if( !m_pCurrentEntity->player )
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	studiohdr_t *m_pStudioHeader = m_pStudioHeader;
	int sequence = m_pCurrentEntity->curstate.sequence;

	if( sequence >= m_pStudioHeader->numseq || sequence < 0 )
	{
		m_pCurrentEntity->curstate.sequence = 0;
		sequence = 0;
	}

	mstudioseqdesc_t *pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + sequence;
	mstudioanim_t *panim = StudioGetAnim( m_pRenderModel, pseqdesc );
	f = StudioEstimateFrame( pseqdesc );

	if( pseqdesc->numblends == 9 )
	{
		float flBlendX = m_pCurrentEntity->curstate.blending[0];
		float flBlendY = m_pCurrentEntity->curstate.blending[1];
		mstudioanim_t *panimTarget = NULL;

		if( flBlendX > 127.0f )
		{
			s = ( flBlendX - 127.0f ) + ( flBlendX - 127.0f );

			if( flBlendY > 127.0f )
			{
				t = ( flBlendY - 127.0f ) + ( flBlendY - 127.0f );

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 5 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 7 ), f );
				panimTarget = LookupAnimation( pseqdesc, 8 );
			}
			else
			{
				t = flBlendY + flBlendY;

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 1 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 2 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				panimTarget = LookupAnimation( pseqdesc, 5 );
			}
		}
		else
		{
			s = flBlendX + flBlendX;

			if( flBlendY <= 127.0f )
			{
				t = flBlendY + flBlendY;

				StudioCalcRotations( pos, q, pseqdesc, panim, f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 1 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 3 ), f );
				panimTarget = LookupAnimation( pseqdesc, 4 );
			}
			else
			{
				t = ( flBlendY - 127.0f ) + ( flBlendY - 127.0f );

				StudioCalcRotations( pos, q, pseqdesc, LookupAnimation( pseqdesc, 3 ), f );
				StudioCalcRotations( pos2, q2, pseqdesc, LookupAnimation( pseqdesc, 4 ), f );
				StudioCalcRotations( pos3, q3, pseqdesc, LookupAnimation( pseqdesc, 6 ), f );
				panimTarget = LookupAnimation( pseqdesc, 7 );
			}
		}

		StudioCalcRotations( pos4, q4, pseqdesc, panimTarget, f );

		float flWeightX = s / 255.0f;
		StudioSlerpBones( q, pos, q2, pos2, flWeightX );
		StudioSlerpBones( q3, pos3, q4, pos4, flWeightX );

		float flWeightY = t / 255.0f;
		StudioSlerpBones( q, pos, q3, pos3, flWeightY );
	}
	else
	{
		StudioCalcRotations( pos, q, pseqdesc, panim, f );

		if( pseqdesc->numblends > 1 )
		{
			mstudioanim_t *panim1 = LookupAnimation( pseqdesc, 1 );
			StudioCalcRotations( pos2, q2, pseqdesc, panim1, f );

			dadt = StudioEstimateInterpolant();
			float flWeightX = 1.0f - ( ( m_pCurrentEntity->curstate.blending[0] * dadt +
				( 1.0f - dadt ) * m_pCurrentEntity->latched.prevblending[0] ) ) / 255.0f;

			StudioSlerpBones( q, pos, q2, pos2, flWeightX );
		}
	}

	if( m_fDoInterp )
	{
		if( m_pCurrentEntity->latched.sequencetime != 0.0f && m_pCurrentEntity->latched.sequencetime + 0.2f > m_clTime )
		{
			int prevsequence = m_pCurrentEntity->latched.prevsequence;

			if( prevsequence < m_pStudioHeader->numseq )
			{
				byte prevBlendX = m_pCurrentEntity->latched.prevseqblending[0];
				byte prevBlendY = m_pCurrentEntity->latched.prevseqblending[1];

				mstudioseqdesc_t *pOldSeqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + prevsequence;
				mstudioanim_t *pOldAnim = StudioGetAnim( m_pRenderModel, pOldSeqdesc );
				float flPrevFrame = m_pCurrentEntity->latched.prevframe;

				if( pOldSeqdesc->numblends == 9 )
				{
					int index = 0;

					if( prevBlendX > 127.0f )
					{
						s = ( prevBlendX - 127.0f ) + ( prevBlendX - 127.0f );

						if( prevBlendY > 127.0f )
						{
							t = ( prevBlendY - 127.0f ) + ( prevBlendY - 127.0f );
							StudioCalcRotations( pos3, q3, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 4 ), flPrevFrame );
							StudioCalcRotations( pos2, q2, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 5 ), flPrevFrame );
							StudioCalcRotations( pos4, q4, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 7 ), flPrevFrame );
							index = 8;
						}
						else
						{
							t = prevBlendY + prevBlendY;
							StudioCalcRotations( pos3, q3, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 1 ), flPrevFrame );
							StudioCalcRotations( pos2, q2, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 2 ), flPrevFrame );
							StudioCalcRotations( pos4, q4, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 4 ), flPrevFrame );
							index = 5;
						}
					}
					else
					{
						s = prevBlendX + prevBlendX;

						if( prevBlendY > 127.0f )
						{
							t = ( prevBlendY - 127.0f ) + ( prevBlendY - 127.0f );
							StudioCalcRotations( pos3, q3, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 3 ), flPrevFrame );
							StudioCalcRotations( pos2, q2, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 4 ), flPrevFrame );
							StudioCalcRotations( pos4, q4, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 6 ), flPrevFrame );
							index = 7;
						}
						else
						{
							t = prevBlendY + prevBlendY;
							StudioCalcRotations( pos3, q3, pOldSeqdesc, pOldAnim, flPrevFrame );
							StudioCalcRotations( pos2, q2, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 1 ), flPrevFrame );
							StudioCalcRotations( pos4, q4, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 3 ), flPrevFrame );
							index = 4;
						}
					}

					StudioCalcRotations( pos, q, pOldSeqdesc, LookupAnimation( pOldSeqdesc, index ), flPrevFrame );

					float flOldWeightX = s / 255.0f;
					StudioSlerpBones( q3, pos3, q2, pos2, flOldWeightX );
					StudioSlerpBones( q4, pos4, q, pos, flOldWeightX );

					float flOldWeightY = t / 255.0f;
					StudioSlerpBones( q3, pos3, q4, pos4, flOldWeightY );
				}
				else
				{
					StudioCalcRotations( pos3, q3, pOldSeqdesc, pOldAnim, flPrevFrame );

					if( pOldSeqdesc->numblends > 1 )
					{
						mstudioanim_t *pa1 = LookupAnimation( pOldSeqdesc, 1 );
						StudioCalcRotations( pos2, q2, pOldSeqdesc, pa1, flPrevFrame );

						float flOldWeightX = ( float ) m_pCurrentEntity->latched.prevseqblending[0] / 255.0f;
						StudioSlerpBones( q3, pos3, q2, pos2, flOldWeightX );

						if( pOldSeqdesc->numblends == 4 )
						{
							StudioCalcRotations( pos4, q4, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 2 ), flPrevFrame );
							StudioCalcRotations( pos, q, pOldSeqdesc, LookupAnimation( pOldSeqdesc, 3 ), flPrevFrame );

							float flOldWeightX2 = ( float ) m_pCurrentEntity->latched.prevseqblending[0] / 255.0f;
							StudioSlerpBones( q4, pos4, q, pos, flOldWeightX2 );

							float flOldWeightY = ( float ) m_pCurrentEntity->latched.prevseqblending[1] / 255.0f;
							StudioSlerpBones( q3, pos3, q4, pos4, flOldWeightY );
						}
					}
				}

				float flInterpWeight = 1.0f - ( m_clTime - m_pCurrentEntity->latched.sequencetime ) / 0.2f;
				StudioSlerpBones( q, pos, q3, pos3, flInterpWeight );
			}
		}
	}

	m_pCurrentEntity->latched.prevframe = f;
	mstudiobone_t *pbone = ( mstudiobone_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->boneindex );
	player_info_t *pPlayerInfo = m_pPlayerInfo;
	int gaitsequence = pPlayerInfo ? pPlayerInfo->gaitsequence : 0;
	int numbones = m_pStudioHeader->numbones;

	if( !( ( sequence - 19 ) <= 5 || gaitsequence <= 0 || sequence == 7 || sequence == 8 ) )
	{
		if( gaitsequence >= m_pStudioHeader->numseq )
		{
			pPlayerInfo->gaitsequence = 0;
			gaitsequence = 0;
		}

		mstudioseqdesc_t *pGaitSeqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + gaitsequence;
		mstudioanim_t *pGaitAnim = StudioGetAnim( m_pRenderModel, pGaitSeqdesc );

		StudioCalcRotations( pos2, q2, pGaitSeqdesc, pGaitAnim, pPlayerInfo->gaitframe );

		if( numbones > 0 )
		{
			copy = 1;

			for( i = 0; i < numbones; ++i )
			{
				if( i == 8 ) 
					copy = 0;
				if( i == 5 || i == 2 ) 
					copy = 1;

				if( copy )
				{
					pos[i][0] = pos2[i][0];
					pos[i][1] = pos2[i][1];
					pos[i][2] = pos2[i][2];

					q[i][0] = q2[i][0];
					q[i][1] = q2[i][1];
					q[i][2] = q2[i][2];
					q[i][3] = q2[i][3];
				}
			}
		}
	}

	if( numbones > 0 )
	{
		float bonematrix[3][4];

		for( i = 0; i < numbones; ++i )
		{
			QuaternionMatrix( q[i], bonematrix );

			bonematrix[0][3] = pos[i][0];
			bonematrix[1][3] = pos[i][1];
			bonematrix[2][3] = pos[i][2];

			if( pbone[i].parent == -1 )
			{
				if( IEngineStudio.IsHardware() )
				{
					ConcatTransforms( *m_protationmatrix, bonematrix, ( *m_pbonetransform )[i] );
					MatrixCopy( ( *m_pbonetransform )[i], ( *m_plighttransform )[i] );
				}
				else
				{
					ConcatTransforms( *m_paliastransform, bonematrix, ( *m_pbonetransform )[i] );
					ConcatTransforms( *m_protationmatrix, bonematrix, ( *m_plighttransform )[i] );
				}

				StudioFxTransform( m_pCurrentEntity, ( *m_pbonetransform )[i] );
			}
			else
			{
				ConcatTransforms( ( *m_pbonetransform )[pbone[i].parent], bonematrix, ( *m_pbonetransform )[i] );
				ConcatTransforms( ( *m_plighttransform )[pbone[i].parent], bonematrix, ( *m_plighttransform )[i] );
			}
		}
	}
}

void CGameStudioModelRenderer::StudioEstimateGait( entity_state_t *pplayer )
{
	vec3_t est_velocity;
	float dt, flYawDiff, flYaw;
	bool bIsDeathAnim, bIsProneAnim, bSandbagDeployed;

	bIsDeathAnim = false;
	bIsProneAnim = false;

	player_info_t *pPlayerInfo = m_pPlayerInfo;
	cl_entity_t *pCurrentEntity = m_pCurrentEntity;
	int sequence = pCurrentEntity->curstate.sequence;

	if( ( pPlayerInfo->gaitsequence - 15 ) > 1 && sequence != 17 )
		bIsDeathAnim = ( sequence == 18 );

	bIsProneAnim = ( ( sequence - 19 ) <= 5 );

	flYaw = m_clTime - m_clOldTime;
	if( flYaw < 0.0f )
	{
		m_flGaitMovement = 0.0f;
		return;
	}

	dt = 1.0f;
	if( flYaw <= 1.0f )
	{
		dt = flYaw;
		if( flYaw == 0.0f )
		{
			m_flGaitMovement = 0.0f;
			return;
		}
	}

	if( pPlayerInfo->renderframe == m_nFrameCount )
	{
		m_flGaitMovement = 0.0f;
		return;
	}

	if( m_fGaitEstimation )
	{
		est_velocity = pCurrentEntity->origin - pPlayerInfo->prevgaitorigin;
		pPlayerInfo->prevgaitorigin = pCurrentEntity->origin;

		float flLength = Length( est_velocity );
		m_flGaitMovement = flLength;

		if( dt <= 0.0f || ( flLength / dt ) < 5.0f )
		{
			m_flGaitMovement = 0.0f;
			est_velocity = { 0.0f, 0.0f, 0.0f };
		}
	}
	else
	{
		est_velocity = pplayer->velocity;
		m_flGaitMovement = Length( est_velocity ) * dt;
	}

	studiohdr_t *pStudioHdr = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( pCurrentEntity->model );
	bSandbagDeployed = IsSandbagAnimation( pCurrentEntity->curstate.sequence, pStudioHdr );

	if( bSandbagDeployed )
	{
		pPlayerInfo->gaityaw = pCurrentEntity->angles[1];
		return;
	}

	if( est_velocity[1] != 0.0f || est_velocity[0] != 0.0f )
	{
		if( !bIsDeathAnim )
		{
			pPlayerInfo->gaityaw = atan2( est_velocity[1], est_velocity[0] ) * 180.0f / M_PI;
			
			if( pPlayerInfo->gaityaw > 180.0f )  
				pPlayerInfo->gaityaw = 180.0f;

			if( pPlayerInfo->gaityaw < -180.0f ) 
				pPlayerInfo->gaityaw = -180.0f;
		}
		return;
	}

	if( !bIsProneAnim && !bIsDeathAnim )
	{
		flYawDiff = pCurrentEntity->angles[1] - pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - 360.0f * floor( ( flYawDiff + 180.0f ) / 360.0f );

		if( flYawDiff <= -5.0f || flYawDiff >= 5.0f )
		{
			if( flYawDiff < -90.0f || flYawDiff > 90.0f )
				pCurrentEntity->baseline.fuser1 = 3.5f;
			else
				pCurrentEntity->baseline.fuser1 = 0.0f;
		}
		else
		{
			pCurrentEntity->baseline.fuser1 = 0.05f;
		}

		float flBlendStep;

		if( dt >= 0.25f )
			flBlendStep = dt * flYawDiff;
		else
			flBlendStep = flYawDiff * ( dt * pCurrentEntity->baseline.fuser1 );

		if( fabs( flBlendStep ) >= 0.1f )
			pPlayerInfo->gaityaw += flBlendStep;

		pPlayerInfo->gaityaw = pPlayerInfo->gaityaw - 360.0f * floor( pPlayerInfo->gaityaw / 360.0f );

		m_flGaitMovement = 0.0f;
		return;
	}

	pPlayerInfo->gaityaw = pPlayerInfo->gaityaw + pCurrentEntity->angles[1] - pPlayerInfo->gaityaw;
}

void CGameStudioModelRenderer::StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch )
{
	float range = 3.0f * ( *pPitch );

	if( range <= -45.0f )
	{
		*pBlend = 255;
		*pPitch = 0.0f;
	}
	else if( range >= 45.0f )
	{
		*pBlend = 0;
		*pPitch = 0.0f;
	}
	else
	{
		range = ( 45.0f - range ) * 255.0f / 90.0f;
		*pBlend = ( int ) range;
		*pPitch = 0.0f;
	}
}

void CGameStudioModelRenderer::CalculatePitchBlend( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	int iBlend;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;
	StudioPlayerBlend( pseqdesc, &iBlend, &m_pCurrentEntity->angles.x );

	m_pCurrentEntity->latched.prevangles.x = m_pCurrentEntity->angles.x;
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = iBlend;
	m_pCurrentEntity->latched.prevseqblending[1] = iBlend;
}

void CGameStudioModelRenderer::CalculateYawBlend( entity_state_t *pplayer )
{
	float flYaw, float maxyaw, float blend_yaw;

	StudioEstimateGait( pplayer );

	if( ( m_pPlayerInfo->gaitsequence - 15 ) > 1 )
	{
		int sequence = m_pCurrentEntity->curstate.sequence;

		if( sequence != 17 && sequence != 18 )
		{
			flYaw = m_pCurrentEntity->angles.y - m_pPlayerInfo->gaityaw;
			flYaw = flYaw - 360.0f * floor( ( flYaw + 180.0f ) / 360.0f );

			if( m_flGaitMovement != 0.0f )
			{
				maxyaw = 120.0f;

				if( flYaw > maxyaw )
				{
					m_pPlayerInfo->gaityaw -= 180.0f;
					m_flGaitMovement = -m_flGaitMovement;
					flYaw -= 180.0f;
				}
				else if( flYaw < -maxyaw )
				{
					m_pPlayerInfo->gaityaw += 180.0f;
					m_flGaitMovement = -m_flGaitMovement;
					flYaw += 180.0f;
				}
			}

			blend_yaw = ( flYaw / 90.0f ) * 128.0f + 127.0f;
			int iFinalBlend = 0;

			if( blend_yaw <= 255.0f )
			{
				if( blend_yaw >= 0.0f )
					iFinalBlend = ( int ) ( 255.0f - blend_yaw );
				else
					iFinalBlend = 255;
			}

			m_pCurrentEntity->curstate.blending[0] = iFinalBlend;
			m_pCurrentEntity->latched.prevblending[0] = iFinalBlend;
			m_pCurrentEntity->latched.prevseqblending[0] = iFinalBlend;
			m_pCurrentEntity->angles.y = m_pPlayerInfo->gaityaw;

			if( m_pCurrentEntity->angles.y < 0.0f )
				m_pCurrentEntity->angles.y += 360.0f;

			m_pCurrentEntity->latched.prevangles[1] = m_pCurrentEntity->angles.y;
		}
	}
}

void CGameStudioModelRenderer::StudioProcessGait( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	float dt;

	CalculatePitchBlend( pplayer );
	CalculateYawBlend( pplayer );

	dt = m_clTime - m_clOldTime;
	if( dt < 0.0f )
		dt = 0.0f;

	else if( dt > 1.0f )
		dt = 1.0f;

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pStudioHeader + m_pStudioHeader->seqindex ) + pplayer->gaitsequence;

	if( pseqdesc->linearmovement.x <= 0.0f )
		m_pPlayerInfo->gaitframe += dt * pseqdesc->fps * m_pCurrentEntity->curstate.framerate;
	else
		m_pPlayerInfo->gaitframe += ( m_flGaitMovement / pseqdesc->linearmovement.x ) * ( float ) pseqdesc->numframes;

	if( pseqdesc->numframes > 0 )
	{
		m_pPlayerInfo->gaitframe -= floor( m_pPlayerInfo->gaitframe / ( float ) pseqdesc->numframes ) * ( float ) pseqdesc->numframes;

		if( m_pPlayerInfo->gaitframe < 0.0f )
		{
			m_pPlayerInfo->gaitframe += ( float ) pseqdesc->numframes;
		}
	}
}

void CGameStudioModelRenderer::SavePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent;

	ent = IEngineStudio.GetCurrentEntity();

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

	memcpy( st->blending, ent->curstate.blending, sizeof( st->blending ) );
	memcpy( st->controller, ent->curstate.controller, sizeof( st->controller ) );

	st->lv = ent->latched;
}

void GetSequenceInfo( studiohdr_t *pstudiohdr, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	mstudioseqdesc_t *pseqdesc;

	if( !pstudiohdr )
		return;

	int sequence = pev->sequence;

	if( sequence >= pstudiohdr->numseq || sequence < 0 )
	{
		*pflFrameRate = 0.0f;
		*pflGroundSpeed = 0.0f;
		return;
	}

	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex ) + sequence;

	if( pseqdesc->numframes > 1 )
	{
		*pflFrameRate = ( 256.0f * pseqdesc->fps ) / ( float ) ( pseqdesc->numframes - 1 );

		float flDistance = sqrt( pseqdesc->linearmovement.x * pseqdesc->linearmovement.x +
			pseqdesc->linearmovement.y * pseqdesc->linearmovement.y +
			pseqdesc->linearmovement.z * pseqdesc->linearmovement.z );
 
		*pflGroundSpeed = ( flDistance * pseqdesc->fps ) / ( float ) ( pseqdesc->numframes - 1 );
	}
	else
	{
		*pflFrameRate = 256.0f;
		*pflGroundSpeed = 0.0f;
	}
}

int GetSequenceFlags( studiohdr_t *pstudiohdr, client_anim_state_t *pev )
{
	mstudioseqdesc_t *pseqdesc;

	if( !pstudiohdr )
		return 0;

	int sequence = pev->sequence;

	if( sequence < pstudiohdr->numseq && sequence >= 0 )
	{
		pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex ) + sequence;
		return pseqdesc->flags;
	}

	return 0;
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
	static double oldtime = 0.0f;

	client_anim_state_t *st;
	cl_entity_t *ent;
	int oldseq;
	double curtime, dt;
	float rt, gs;

	ent = IEngineStudio.GetCurrentEntity();

	if( !ent )
		return;

	st = &g_clientstate;

	curtime = gEngfuncs.GetClientTime();
	dt = curtime - oldtime;

	if( dt < 0.0 )
		dt = 0.0;

	else if( dt > 1.0 )
	{
		dt = 1.0;
	}

	oldtime = curtime;

	oldseq = st->sequence;
	st->framerate = 1.0f;

	DoD_GetSequence( &st->sequence, &st->gaitsequence );
	DoD_GetOrientation( &st->origin.x, &st->angles.x );
	st->realangles = st->angles;

	if( st->sequence != oldseq )
	{
		st->frame = 0.0f;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		memcpy( st->lv.prevseqblending, st->blending, sizeof( st->blending ) );
		memcpy( st->lv.prevcontroller, st->controller, sizeof( st->controller ) );
	}

	studiohdr_t *pstudiohdr = ( studiohdr_t * ) IEngineStudio.Mod_Extradata( ent->model );
	GetSequenceInfo( pstudiohdr, st, &rt, &gs );
	st->m_fSequenceLoops = GetSequenceFlags( pstudiohdr, st ) & 1;
	StudioFrameAdvance( st, rt, ( float ) dt );

	ent->angles = st->realangles;
	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;
	ent->curstate.sequence = st->sequence;
	ent->curstate.frame = st->frame;
	ent->curstate.animtime = st->animtime;
	ent->curstate.framerate = st->framerate;

	pplayer->gaitsequence = st->gaitsequence;

	memcpy( ent->curstate.blending, st->blending, sizeof( ent->curstate.blending ) );
	memcpy( ent->curstate.controller, st->controller, sizeof( ent->curstate.controller ) );

	ent->latched = st->lv;
}

void CGameStudioModelRenderer::RestorePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent;

	ent = IEngineStudio.GetCurrentEntity();

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

	memcpy( st->blending, ent->curstate.blending, sizeof( st->blending ) );
	memcpy( st->controller, ent->curstate.controller, sizeof( st->controller ) );

	st->lv = ent->latched;

	ent->curstate.angles = g_state.angles;
	ent->curstate.origin = g_state.origin;
	ent->angles = g_state.realangles;
	ent->curstate.sequence = g_state.sequence;
	ent->curstate.animtime = g_state.animtime;
	ent->curstate.frame = g_state.frame;
	ent->curstate.framerate = g_state.framerate;

	pplayer->gaitsequence = g_state.gaitsequence;

	memcpy( ent->curstate.blending, g_state.blending, sizeof( ent->curstate.blending ) );
	memcpy( ent->curstate.controller, g_state.controller, sizeof( ent->curstate.controller ) );

	ent->latched = g_state.lv;
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

	if( ( flags & 1 ) != 0 && !IEngineStudio.StudioCheckBBox() )
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

	if( ( flags & 2 ) != 0 )
	{
		StudioCalcAttachments();
		StudioClientEvents();

		if( ent->index > 0 )
		{
			cl_entity_t *pPlayerEnt = gEngfuncs.GetEntityByIndex( ent->index );
			memcpy( pPlayerEnt->attachment, ent->attachment, sizeof( ent->attachment ) );
		}
	}

	if( ( flags & 1 ) != 0 )
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
	int renderfx, axis, i, j;
	float offset, flTimeDelta, flScale;

	renderfx = ent->curstate.renderfx;

	if( renderfx < kRenderFxDistort )
		return;

	if( renderfx > kRenderFxHologram )
	{
		if( renderfx == kRenderFxExplode )
		{
			if( iRenderStateChanged )
			{
				flTimeDelta = m_clTime;
				iRenderStateChanged = 0;
				g_flStartScaleTime = flTimeDelta;
			}
			else
			{
				flTimeDelta = g_flStartScaleTime;
			}

			flTimeDelta = m_clTime - g_flStartScaleTime;

			if( flTimeDelta > 0.0f )
			{
				if( flTimeDelta > 2.0f )
					flScale = 0.001f;
				else
					flScale = 1.0f - flTimeDelta * 0.5f;

				for( i = 0; i < 3; ++i )
				{
					for( j = 0; j < 3; ++j )
					{
						transform[i][j] *= flScale;
					}
				}
			}
		}
	}
	else if( renderfx == kRenderFxHologram )
	{
		if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			if( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
			{
				gEngfuncs.pfnRandomLong( 0, 1 );

				offset = gEngfuncs.pfnRandomFloat( -3.0f, 5.0f );
				axis = gEngfuncs.pfnRandomLong( 0, 2 );
				transform[axis][3] += offset;
			}
		}
	}
	else if( renderfx == kRenderFxDistort )
	{
		i = gEngfuncs.pfnRandomLong( 0, 1 );

		if( i != 1 )
			i = gEngfuncs.pfnRandomLong( 0, 2 );

		flScale = gEngfuncs.pfnRandomFloat( 1.0f, 1.49f );
		VectorScale( transform[i], flScale, transform[i] );
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

			if( WpnInfo[i].leftreload[0] != '\0' )
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

extern void HUD_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );

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
