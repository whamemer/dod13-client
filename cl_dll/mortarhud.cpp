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
//  mortar.cpp - implementation of the CMortarHud class
//

#include "hud.h"
#include "dod_shared.h"

extern vec3_t v_angles;
extern float g_flMortarInitialVelocity, g_flMortarGravity;

float VecToYaw( float *vec )
{
	return 1.0f;
}

CMortarHud::CMortarHud( void )
{

}

CMortarHud::~CMortarHud( void )
{

}

int CMortarHud::Init( void )
{
	return 1;
}

int CMortarHud::VidInit( void )
{
	return 1;
}

void CMortarHud::CalculateFireAngle( float *pitch, float *yaw )
{

}

int CMortarHud::Draw( float flTime )
{
	return !gHUD.IsInMortarDeploy();
}

void CMortarHud::DrawPredictedMortarImpactSite( void )
{
	return;
}

CTrajectoryList::CTrajectoryList( void )
{

}

CTrajectoryList::~CTrajectoryList( void )
{

}

void CTrajectoryList::GetTrajectory( vec3_t *launchPos, vec3_t *targetPos, float *pitch1, float *pitch2, float *yaw )
{

}

trajectory_t *CTrajectoryList::AddTrajectory( vec3_t *p_targetPos )
{
	return NULL;
}

void CTrajectoryList::InvalidateAllTrajectories( void )
{

}

void CTrajectoryList::CalculateTrajectory( vec3_t *launchPos, vec3_t *targetPos, float *pitch1, float *pitch2, float *yaw )
{

}