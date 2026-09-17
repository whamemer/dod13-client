//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined ( EV_HLDMH )
#define EV_HLDMH

int EV_GetWeaponBody( void );
void EV_ResetAnimationEvents( int index );
float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType );
void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, float *vecSrc, float *vecEnd, int iBulletType );
void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd );
void RemoveBody( TEMPENTITY *te, float frametime, float current_time );
void HitBody( TEMPENTITY *ent, pmtrace_t *ptr );
void CreateCorpse( vec3_t vOrigin, vec3_t vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody );
void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType );
int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount, float *tracerOrigin );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots,
	float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType,
	int iTracerFreq, int *tracerCount );
void EV_HLDM_DoDSurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType );
float EV_HLDM_WaterHeight( vec3_t position, float minz, float maxz );
int EV_HLDM_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult );
void P_Rubble( event_args_t *args );
char EV_TexNameToType( char *pTextureName );
void CreateFlyingRubble( vec3_t origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, vec3_t vNormal );
#endif // EV_HLDMH
