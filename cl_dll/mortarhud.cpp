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
// mortarhud.cpp
//
// implementation of the CMortarHud class: the fire angle of the mortar and the cache of its trajectories
//

#include <math.h>

#include "hud.h"
#include "cl_util.h"

#if !defined( M_PI )
#define M_PI 3.14159265358979323846
#endif

extern "C" int VectorCompare( const float *v1, const float *v2 );

extern vec3_t v_angles;

typedef struct
{
	float vTargetPos[3];
	float fPitch1;
	float fPitch2;
	float fYaw;
	float fLastUsedTime;
} trajectory_t;

class CTrajectoryList
{
public:
	enum
	{
		MAX_TRAJECTORIES = 32,
		TRAJECTORY_LIFETIME = 30
	};

	CTrajectoryList( void );
	~CTrajectoryList( void );

	void GetTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw );
	void CalculateTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw );
	trajectory_t *AddTrajectory( Vector targetPos );
	void InvalidateAllTrajectories( void );

private:
	vec3_t m_vecLaunchPos;
	trajectory_t m_Trajectories[MAX_TRAJECTORIES];
};

const float g_flMortarInitialVelocity = 1000.0f;
const float g_flMortarGravity = 800.0f * 0.3f;

float VecToYaw( float *vec )
{
	float yaw;

	if( vec[1] == 0.0f && vec[0] == 0.0f )
	{
		yaw = 0.0f;
	}
	else
	{
		yaw = (float)(int)( atan2( vec[1], vec[0] ) * 180.0 / M_PI );

		if( yaw < 0.0f )
			yaw += 360.0f;
	}

	return yaw;
}

CMortarHud::CMortarHud( void )
{
	m_TrajectoryList = new CTrajectoryList;
}

CMortarHud::~CMortarHud( void )
{
	if( m_TrajectoryList )
		delete m_TrajectoryList;
}

int CMortarHud::Init( void )
{
	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem( this );

	return 1;
}

int CMortarHud::VidInit( void )
{
	return 1;
}

void CMortarHud::CalculateFireAngle( float *pitch, float *yaw )
{
	float playerPitch;
	float playerRatio;

	playerPitch = v_angles.x;

	if( playerPitch > 10.0f )
	{
		*pitch = 30.0f;
	}
	else if( -playerPitch > 50.0f )
	{
		*pitch = 90.0f;
	}
	else
	{
		playerRatio = ( 10.0f - playerPitch ) / 60.0f;
		*pitch = playerRatio * 60.0f + 30.0f;
	}

	*yaw = v_angles.y;
}

extern float g_flWeaponHeat;

int CMortarHud::Draw( float flTime )
{
	// the original had the drawing of the fire angles here, nothing of it is compiled
	return !gHUD.IsInMortarDeploy();
}

void CMortarHud::DrawPredictedMortarImpactSite( void )
{
	/* Nothing. */
}

CTrajectoryList::CTrajectoryList( void )
{
	memset( m_Trajectories, 0, sizeof( m_Trajectories ) );

	InvalidateAllTrajectories();
}

CTrajectoryList::~CTrajectoryList( void )
{
	/* Nothing. */
}

void CTrajectoryList::GetTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw )
{
	trajectory_t *t;
	int i;

	t = NULL;

	if( VectorCompare( launchPos, m_vecLaunchPos ) )
	{
		// the same launch position: look for a trajectory to the same target
		for( i = 0; i < MAX_TRAJECTORIES; i++ )
		{
			if( m_Trajectories[i].fLastUsedTime > 0.0f && VectorCompare( targetPos, m_Trajectories[i].vTargetPos ) )
				t = &m_Trajectories[i];
		}
	}
	else
	{
		InvalidateAllTrajectories();

		m_vecLaunchPos = launchPos;
	}

	if( !t )
		t = AddTrajectory( targetPos );

	if( t )
	{
		*pitch1 = t->fPitch1;
		*pitch2 = t->fPitch2;
		*yaw = t->fYaw;
	}
	else
	{
		// the list is full
		CalculateTrajectory( launchPos, targetPos, pitch1, pitch2, yaw );
	}
}

trajectory_t *CTrajectoryList::AddTrajectory( Vector targetPos )
{
	int i;
	trajectory_t *t;
	float time;

	time = gEngfuncs.GetClientTime();

	t = NULL;

	// the first free or expired slot
	for( i = 0; i < MAX_TRAJECTORIES; i++ )
	{
		if( m_Trajectories[i].fLastUsedTime < 0.0f || time - m_Trajectories[i].fLastUsedTime > TRAJECTORY_LIFETIME )
		{
			t = &m_Trajectories[i];
			break;
		}
	}

	if( t )
	{
		t->vTargetPos[0] = targetPos.x;
		t->vTargetPos[1] = targetPos.y;
		t->vTargetPos[2] = targetPos.z;
		t->fLastUsedTime = gEngfuncs.GetClientTime();

		CalculateTrajectory( m_vecLaunchPos, targetPos, &t->fPitch1, &t->fPitch2, &t->fYaw );
	}

	return t;
}

void CTrajectoryList::InvalidateAllTrajectories( void )
{
	for( int i = 0; i < MAX_TRAJECTORIES; i++ )
		m_Trajectories[i].fLastUsedTime = -1.0f;
}

// the two pitches that bring the shell from the launch position to the target, if there are any
void CTrajectoryList::CalculateTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw )
{
	vec3_t delta;
	double v, g, r, h, gh, vSquared, u1, u2, A, sqrtA, rr, hh;
	vec3_t to_target;
	vec3_t forward;

	delta = targetPos - launchPos;

	to_target = delta;
	forward = to_target;
	forward.z = 0.0f;

	*yaw = VecToYaw( forward );

	v = g_flMortarInitialVelocity;
	g = g_flMortarGravity;

	r = to_target.Length2D();
	h = launchPos.z - targetPos.z;

	gh = g * h;
	vSquared = v * v;
	rr = r * r;
	hh = h * h;

	A = ( 2.0 * gh + vSquared ) * ( 2.0 * gh + vSquared ) - 4.0 * g * g * ( rr + hh );

	if( A >= 0.0 )
	{
		sqrtA = sqrt( A );

		u1 = ( 2.0 * gh + vSquared + sqrtA ) * ( rr * vSquared / ( ( rr + hh ) * 2.0 * vSquared * vSquared ) );
		u2 = ( 2.0 * gh + vSquared - sqrtA ) * ( rr * vSquared / ( ( rr + hh ) * 2.0 * vSquared * vSquared ) );

		if( u1 >= 0.0 )
			*pitch1 = (float)( acos( sqrt( u1 ) ) * 57.29578 );

		if( u2 >= 0.0 )
			*pitch2 = (float)( acos( sqrt( u2 ) ) * 57.29578 );
	}
	else
	{
		*pitch1 = -1.0f;
		*pitch2 = -1.0f;
	}
}
