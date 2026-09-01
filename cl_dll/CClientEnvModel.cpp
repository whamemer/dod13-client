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
#include "dod_shared.h"

//extern engine_studio_api_t IEngineStudio;

int CClientEnvModel::Init( void )
{
	return 1;
}

int CClientEnvModel::VidInit( void )
{
	return 1;
}

int GetSequence( void *model, const char *label )
{
	return 1;
}

void CClientEnvModel::RemoveAllModels( void )
{

}

void CClientEnvModel::AddEnvModel( env_model_t *pModel )
{

}

extern vec3_t v_origin;

void EV_EnvModelCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void CClientEnvModel::Think( void )
{

}