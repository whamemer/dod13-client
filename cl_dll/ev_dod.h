//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined ( EV_DODH )
#define EV_DODH

int EV_GetWeaponBody( void );
void EV_ResetAnimationEvents( int index );
float EV_DOD_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType );
void EV_DOD_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, float *vecSrc, float *vecEnd, int iBulletType );
void EV_DOD_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd );
void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType );
void EV_DOD_SurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType );
int EV_DOD_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount, float *tracerOrigin );
void EV_DOD_FireBullets( int idx, float *forward, float *right, float *up, int cShots,
	float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType,
	int iTracerFreq, int *tracerCount );
float EV_DOD_WaterHeight( Vector position, float minz, float maxz );
int EV_DOD_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult );
void EV_DOD_BubbleTrails( Vector from, Vector to, int count );
void CreateCorpse( Vector vOrigin, Vector vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody );
void P_Rubble( struct event_args_s *args );
void P_ExplosionSmoke( struct event_args_s *args );
char EV_FindHitTexture( float *start, float *end );
char EV_TexNameToType( char *pTextureName );
void CreateFlyingRubble( Vector origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, Vector vNormal );
#endif // EV_DODH
