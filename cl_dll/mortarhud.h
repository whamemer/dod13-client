//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
//			
//  mortarhud.h
//
// class CMortarHud declaration
//

#pragma once
#if !defined(MORTARTHUD_H)
#define MORTARTHUD_H

struct trajectory_t
{
    float vTargetPos[3];
    float fPitch1, fPitch2;
    float fYaw;
    float fLastUsedTime;
};

class CTrajectoryList
{
public:
    enum
    {
        MAX_TRAJECTORIES = 32,
        TRAJECTORY_LIFETIME = 30
    };

    void GetTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw );
    void CalculateTrajectory( Vector launchPos, Vector targetPos, float *pitch1, float *pitch2, float *yaw );
    trajectory_t *AddTrajectory( Vector targetPos );
    void InvalidateAllTrajectories( void );

private:
    vec3_t m_vecLaunchPos;
    trajectory_t m_Trajectories[MAX_TRAJECTORIES];
};

class CMortarHud : public CHudBase
{
public:
    int Init( void );
    int VidInit( void );
    int Draw( float flTime );

    void CalculateFireAngle( float *pitch, float *yaw );
    void DrawPredictedMortarImpactSite( void );

    trajectory_t *m_TrajectoryList;
};
#endif // MORTARTHUD_H