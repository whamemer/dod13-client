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
#include "cl_util.h"
#include "dod_shared.h"
#include "pi_constant.h"

extern vec3_t v_angles;
const float g_flMortarInitialVelocity, g_flMortarGravity;

float VecToYaw( float *vec )
{
	if( vec[0] == 0.0f && vec[1] == 0.0f )
		return 0.0f;

	float yaw = ( float ) ( atan2( vec[1], vec[0] ) * 180.0 / M_PI );

	if( yaw < 0.0f )
		yaw += 360.0f;

	return yaw;
}

CMortarHud::CMortarHud( void )
{
	m_TrajectoryList = new CTrajectoryList();
}

CMortarHud::~CMortarHud( void )
{
	if( m_TrajectoryList )
	{
		delete m_TrajectoryList;
		m_TrajectoryList = NULL;
	}
}

int CMortarHud::Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_ACTIVE;
	return 1;
}

int CMortarHud::VidInit( void )
{
	return 1;
}

void CMortarHud::CalculateFireAngle( float *pitch, float *yaw )
{
	*yaw = v_angles.y;

	float playerPitch = v_angles.x;
	float flMortarPitch = 30.0f;

	if( playerPitch <= 10.0f )
	{
		if( playerPitch < -50.0f )
		{
			flMortarPitch = 90.0f;
		}
		else
		{
			float playerRatio = ( 10.0f - playerPitch ) / 60.0f;
			flMortarPitch = ( playerRatio * 60.0f ) + 30.0f;
		}
	}
	else
	{
		flMortarPitch = 30.0f;
	}

	*pitch = flMortarPitch;
}


int CMortarHud::Draw( float flTime )
{
	return !gHUD.IsInMortarDeploy();
}

void CMortarHud::DrawPredictedMortarImpactSite( void )
{
	// Nothing.
}

CTrajectoryList::CTrajectoryList( void )
{
	m_vecLaunchPos.x = 0.0f;
	m_vecLaunchPos.y = 0.0f;
	m_vecLaunchPos.z = 0.0f;

	memset( m_Trajectories, 0, sizeof( m_Trajectories ) );
	InvalidateAllTrajectories();
}

CTrajectoryList::~CTrajectoryList( void )
{
	// Nothing.
}

void CTrajectoryList::GetTrajectory( vec3_t launchPos, vec3_t targetPos, float *pitch1, float *pitch2, float *yaw )
{
	if( !VectorCompare2( launchPos, m_vecLaunchPos ) )
	{
		InvalidateAllTrajectories();
		m_vecLaunchPos = launchPos;
	}
	else
	{
		for( int i = 0; i < MAX_TRAJECTORIES; i++ )
		{
			trajectory_t *t = &m_Trajectories[i];

			if( t->fLastUsedTime >= 0.0f && VectorCompare2( targetPos, t->vTargetPos ) )
			{
				*pitch1 = t->fPitch1;
				*pitch2 = t->fPitch2;
				*yaw = t->fYaw;
				return;
			}
		}
	}

	trajectory_t *t = AddTrajectory( targetPos );

	if( t )
	{
		*pitch1 = t->fPitch1;
		*pitch2 = t->fPitch2;
		*yaw = t->fYaw;
	}
	else
	{
		*pitch1 = -1.0f;
		*pitch2 = -1.0f;
		*yaw = 0.0f;
	}
}

trajectory_t *CTrajectoryList::AddTrajectory( vec3_t targetPos )
{
	float time = gEngfuncs.GetClientTime();
	trajectory_t *t = NULL;

	for( int i = 0; i < MAX_TRAJECTORIES; i++ )
	{
		if( m_Trajectories[i].fLastUsedTime < 0.0f || ( time - m_Trajectories[i].fLastUsedTime ) > TRAJECTORY_LIFETIME )
		{
			t = &m_Trajectories[i];
			break;
		}
	}

	if( !t )
		t = &m_Trajectories[0];

	targetPos = t->vTargetPos;
	t->fLastUsedTime = time;

	CalculateTrajectory( m_vecLaunchPos, targetPos, &t->fPitch1, &t->fPitch2, &t->fYaw );

	return t;
}

void CTrajectoryList::InvalidateAllTrajectories( void )
{
	for( int i = 0; i < MAX_TRAJECTORIES; i++ )
	{
		m_Trajectories[i].fLastUsedTime = -1.0f;
	}
}

void CTrajectoryList::CalculateTrajectory( vec3_t launchPos, vec3_t targetPos, float *pitch1, float *pitch2, float *yaw )
{
	vec3_t to_target = targetPos - launchPos;

	*yaw = VecToYaw( &to_target.x );

	double h = launchPos.z - targetPos.z;
	double r = to_target.Length2D();

	double rr = r * r;
	double hh = h * h;

	double g = 480.00003;
	double vSquared = 1000000.0;
	const float radToDeg = 57.29578f;

	double gh = g * h;
	double totalDistSq = hh + rr;

	double A = ( ( vSquared + gh ) * ( vSquared + gh ) ) - g * ( totalDistSq * g );

	if( A >= 0.0 )
	{
		double sqrtA = sqrt( A );

		double denominator = ( rr * vSquared ) / ( totalDistSq * 2.0e12 );

		double u1 = ( vSquared + gh + sqrtA ) * denominator;
		double u2 = ( vSquared + gh - sqrtA ) * denominator;

		if( u1 >= 0.0 )
			*pitch1 = acos( sqrt( u1 ) ) * radToDeg;

		if( u2 >= 0.0 )
			*pitch2 = acos( sqrt( u2 ) ) * radToDeg;
	}
	else
	{
		*pitch1 = -1.0f;
		*pitch2 = -1.0f;
	}
}