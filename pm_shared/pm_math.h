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
// pm_math.h -- math primitives of the shared movement code
//
// The movement code is built into the client and into the server, vec3_t of it is the Vector class. The math functions
// take the pointers to floats, a Vector converts to them.
#pragma once
#if !defined(PM_MATH_H)
#define PM_MATH_H

#if HAVE_CMATH
#include <cmath>
#else
#include <math.h>
#endif

typedef float vec_t;

#include "vector.h"

#if !defined(vec3_t)
#define vec3_t Vector
#endif

#if !defined(M_PI)
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#if !defined(M_PI_F)
#define M_PI_F		(float)M_PI
#endif

struct mplane_s;

extern float vec3_origin[3];
extern int nanmask;

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorClear(a) {(a)[0]=0.0;(a)[1]=0.0;(a)[2]=0.0;}

void VectorMA( const float *veca, float scale, const float *vecb, float *vecc );

vec_t _DotProduct( const float *v1, const float *v2 );
void _VectorSubtract( const float *veca, const float *vecb, float *out );
void _VectorAdd( const float *veca, const float *vecb, float *out );
void _VectorCopy( const float *in, float *out );

int VectorCompare( const float *v1, const float *v2 );
float Length( const float *v );
float Distance( const float *v1, const float *v2 );
void CrossProduct( const float *v1, const float *v2, float *cross );
float VectorNormalize( float *v );		// returns vector length
void VectorInverse( float *v );
void VectorScale( const float *in, vec_t scale, float *out );

void R_ConcatRotations( float in1[3][3], float in2[3][3], float out[3][3] );
void R_ConcatTransforms( float in1[3][4], float in2[3][4], float out[3][4] );

void AngleVectors( const float *angles, float *forward, float *right, float *up );
void AngleVectorsTranspose( const float *angles, float *forward, float *right, float *up );
#define AngleIVectors	AngleVectorsTranspose

void AngleMatrix( const float *angles, float ( *matrix )[4] );
void AngleIMatrix( const float *angles, float ( *matrix )[4] );
void VectorTransform( const float *in1, float in2[3][4], float *out );

void NormalizeAngles( float *angles );
void InterpolateAngles( float *start, float *end, float *output, float frac );
float AngleBetweenVectors( const float *v1, const float *v2 );

void VectorMatrix( float *forward, float *right, float *up );
void VectorAngles( const float *forward, float *angles );

int InvertMatrix( const float *m, float *out );

int BoxOnPlaneSide( float *emins, float *emaxs, struct mplane_s *plane );
float anglemod( float a );

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))
#endif // PM_MATH_H
