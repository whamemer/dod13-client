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
// CClientEnvModel.cpp
//
// the static env_models of the map are drawn by the client only (the server knows just the targeted ones).
// a proxy temp entity that follows the viewer makes the studio renderer call, see CStudioModelRenderer::StudioDrawModel
//

#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "r_efx.h"
#include "event_api.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "r_studioint.h"

extern engine_studio_api_t IEngineStudio;

void EV_EnvModelCallback( tempent_s *ent, float frametime, float currenttime );

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
	int i;

	pstudiohdr = (studiohdr_t *)model;

	if( !pstudiohdr )
		return 0;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)pstudiohdr + pstudiohdr->seqindex );

	for( i = 0; i < pstudiohdr->numseq; i++ )
	{
		if( !stricmp( pseqdesc[i].label, label ) )
			return i;
	}

	return 0;
}

void CClientEnvModel::RemoveAllModels( void )
{
	memset( m_sEnvModels, 0, sizeof( m_sEnvModels ) );

	m_iNumEnvModels = 0;

	if( m_teEnvModelTE && m_teEnvModelTE->callback == EV_EnvModelCallback )
		m_teEnvModelTE->die = 0;

	m_teEnvModelTE = NULL;
}

void CClientEnvModel::AddEnvModel( env_model_t *pModel )
{
	env_model_t *pEnvModel;
	void *pmodel;

	if( m_iNumEnvModels >= MAX_ENV_MODELS )
	{
		gEngfuncs.Con_DPrintf( "CClientEnvModel::AddEnvModel: Too many static env_models! Limit is %d\n", MAX_ENV_MODELS );
		return;
	}

	pEnvModel = &m_sEnvModels[m_iNumEnvModels];
	*pEnvModel = *pModel;

	gEngfuncs.CL_LoadModel( pEnvModel->szModel, &pEnvModel->iModel );
	pEnvModel->pModel = IEngineStudio.GetModelByIndex( pEnvModel->iModel );

	pmodel = IEngineStudio.Mod_Extradata( pEnvModel->pModel );
	pEnvModel->sequence = GetSequence( pmodel, pEnvModel->szSequence );

	if( pModel->spawnflags & SF_ENVMODEL_DROPTOFLOOR )
	{
		int newZ;
		vec3_t vEndPos = pEnvModel->vecOrigin;
		int halfHeight = 0;
		pmtrace_t tr;

		vEndPos.z = -8000.0f;

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( pEnvModel->vecOrigin, vEndPos, PM_STUDIO_IGNORE | PM_GLASS_IGNORE, -1, &tr );

		if( tr.fraction == 1.0f )
			newZ = (int)pEnvModel->vecOrigin.z;
		else
			newZ = (int)( tr.endpos.z + halfHeight );

		pEnvModel->vecOrigin.z = (float)newZ;
	}

	m_iNumEnvModels++;

}

extern vec3_t v_origin;

// keeps the proxy at the viewer, so it is never culled and the models get drawn every frame
void EV_EnvModelCallback( tempent_s *ent, float frametime, float currenttime )
{
	if( gEngfuncs.IsSpectateOnly() )
	{
		ent->entity.origin = v_origin;
	}
	else
	{
		cl_entity_t *localPlayer = gEngfuncs.GetLocalPlayer();

		ent->entity.origin = localPlayer->origin;
	}

	// Xash3D frees the temp entity when "die" passes (0.75 sec after the allocation)
	ent->die = currenttime + 1.0f;
}

void CClientEnvModel::Think( void )
{
	int modelindex = 0;
	vec3_t org( 0, 0, 0 );
	model_t *p;

	if( m_teEnvModelTE )
	{
		// The engine gives a freed slot to the next effect, so it has to be our proxy that did not expire
		if( m_teEnvModelTE->callback == EV_EnvModelCallback && m_teEnvModelTE->die >= gEngfuncs.GetClientTime() )
			return;

		// WHAMER: Hello, Day Of Defeat author!
		gEngfuncs.Con_DPrintf( "Warning! CClientEnvModel::m_teEnvModelTE died somehow, tell Mugsy!\n" );
	}

	gEngfuncs.CL_LoadModel( "models/p_stick.mdl", &modelindex );
	p = IEngineStudio.GetModelByIndex( modelindex );

	m_teEnvModelTE = gEngfuncs.pEfxAPI->CL_TempEntAlloc( org, p );

	if( m_teEnvModelTE )
	{
		m_teEnvModelTE->flags |= FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_SCALE;
		m_teEnvModelTE->callback = EV_EnvModelCallback;
		m_teEnvModelTE->entity.curstate.entityType = 4;
	}
}
