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
//  CClientEnvModel.cpp - implementation of the CClientEnvModel class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "studio.h"
#include "com_model.h"
#include "pmtrace.h"
#include "event_api.h"
#include "entity_types.h"
#include "dod_shared.h"

extern engine_studio_api_t IEngineStudio;

int CClientEnvModel::Init( void )
{
	m_iFlags |= HUD_ACTIVE;
	memset( m_sEnvModels, 0, sizeof( m_sEnvModels ) );
	gHUD.AddHudElem( this );
	return 1;
}

int CClientEnvModel::VidInit( void )
{
	return 1;
}

int GetSequence( void *model, const char *label )
{
	studiohdr_t *pstudiohdr;
	mstudioseqdesc_t *pseqdesc;

	pstudiohdr = ( studiohdr_t *)model;

	if( pstudiohdr )
	{
		pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) pstudiohdr + pstudiohdr->seqindex );

		for( int i = 0; i < pstudiohdr->numseq; i++ )
		{
			if( _stricmp( pseqdesc[i].label, label ) == 0 )
			{
				return i;
			}
		}
	}

	return 0;
}

void CClientEnvModel::RemoveAllModels( void )
{
	memset( m_sEnvModels, 0, sizeof( m_sEnvModels ) );
	m_iNumEnvModels = 0;

	if( m_teEnvModelTE )
		m_teEnvModelTE->die = 0.0;

	m_teEnvModelTE = NULL;
}

void CClientEnvModel::AddEnvModel( env_model_t *pModel )
{
	if( !pModel )
		return;

	if( m_iNumEnvModels > 191 )
	{
		gEngfuncs.Con_DPrintf( "CClientEnvModel::AddEnvModel: Too many static env_models! Limit is %d\n", 192 );
		return;
	}

	env_model_t *pNewModel = &m_sEnvModels[m_iNumEnvModels];
	*pNewModel = *pModel;

	gEngfuncs.CL_LoadModel( (const char*)pNewModel, &pNewModel->iModel );

	pNewModel->pModel = IEngineStudio.GetModelByIndex( pNewModel->iModel );

	void *pStudioHdr = IEngineStudio.Mod_Extradata( pNewModel->pModel );

	if( pStudioHdr )
		pNewModel->sequence = GetSequence( pStudioHdr, pNewModel->szSequence );
	else
		pNewModel->sequence = 0;

	if( ( pModel->spawnflags & 2 ) != 0 )
	{
		int newZ, halfHeight;
		vec3_t vEndPos;
		pmtrace_t tr;

		vEndPos.x = pNewModel->vecOrigin.x;
		vEndPos.y = pNewModel->vecOrigin.y;
		vEndPos.z = -8000.0f;

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( pNewModel->vecOrigin, vEndPos, 5, -1, &tr );

		if( tr.fraction == 1.0f )
		{
			newZ = ( int ) pNewModel->vecOrigin.z;
		}
		else
		{
			halfHeight = 0;
			newZ = ( int ) tr.endpos.z + halfHeight;
		}

		pNewModel->vecOrigin.z = ( float ) newZ;
	}

	m_iNumEnvModels++;
}


extern vec3_t v_origin;

void EV_EnvModelCallback( tempent_s *ent, float frametime, float currenttime )
{
	if( gEngfuncs.IsSpectateOnly() )
		ent->entity.origin = v_origin;
	else
		ent->entity.origin = gEngfuncs.GetLocalPlayer()->origin;
}

void CClientEnvModel::Think( void )
{
	int modelindex;
	vec3_t org;
	model_t *p;

	if( m_teEnvModelTE )
	{
		if( m_teEnvModelTE->die >= 0.0f )
			return;

		org = { 0.0f, 0.0f, 0.0f };
		modelindex = 0;
		
		// WHAMER: Hello, Day Of Defeat author!
		gEngfuncs.Con_DPrintf( "Warning! CClientEnvModel::m_teEnvModelTE died somehow, tell Mugsy!\n" );
	}
	else
	{
		org = { 0.0f, 0.0f, 0.0f };
		modelindex = 0;
	}

	gEngfuncs.CL_LoadModel( "models/p_stick.mdl", &modelindex );

	p = IEngineStudio.GetModelByIndex( modelindex );

	m_teEnvModelTE = gEngfuncs.pEfxAPI->CL_TempEntAlloc( org, p );

	if( m_teEnvModelTE )
	{
		m_teEnvModelTE->flags |= ( FTENT_COLLIDEALL | FTENT_CLIENTCUSTOM ); 
		m_teEnvModelTE->callback = EV_EnvModelCallback;
		m_teEnvModelTE->entity.curstate.entityType = ET_FRAGMENTED;
	}
}
