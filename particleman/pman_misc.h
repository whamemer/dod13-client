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

#ifndef PMAN_MISC_H
#define PMAN_MISC_H

void UpdateParticles( void );
bool IsGamePaused( void );
long MaxParticleClassSize( unsigned long lSize );

void VectorScale( const float* in, float scale, float* out );
void VectorAngles( const float* forward, float* angles );
float VectorNormalize( float* v );
void VectorMA( const float* veca, float scale, const float* vecb, float* vecc );

extern float g_flOldTime;
extern Vector g_vViewAngles;
extern const Vector g_vecZero;
extern float g_flGravity;

extern int g_lMaxParticleClassSize;

extern struct cl_enginefuncs_s gEngfuncs;

#endif //PMAN_MISC_H