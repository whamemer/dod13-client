//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#ifdef USE_PMAN
#include "tri.h"

extern IParticleMan *g_pParticleMan;
#endif

//extern engine_studio_api_t IEngineStudio;

float g_flWeatherTime;
int g_iWeatherType;

extern cvar_t *cl_particlefx;
extern cvar_t *cl_fog;
extern float g_flGravity;
extern vec3_t v_origin;

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
}

//#define TEST_IT	1
#if TEST_IT

/*
=================
Draw_Triangles

Example routine.  Draws a sprite offset from the player origin.
=================
*/
void Draw_Triangles( void )
{
	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if( !player )
		return;

	org = player->origin;

	org.x += 50;
	org.y += 50;

	if( gHUD.m_hsprCursor == 0 )
	{
		gHUD.m_hsprCursor = SPR_Load( "sprites/cursor.spr" );
	}

	if( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ) )
	{
		return;
	}

	// Create a triangle, sigh
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	// Overload p->color with index into tracer palette, p->packedColor with brightness
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );
	// UNDONE: This gouraud shading causes tracers to disappear on some cards (permedia2)
	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y, org.z );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}
#endif

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	gHUD.m_Spectator.DrawOverview();
#if TEST_IT
//	Draw_Triangles();
#endif
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
#if TEST_IT
//	Draw_Triangles();
#endif
}


/*
==================================

DAY OF DEFEAT PARTICLES CODE HERE

==================================
*/
#ifdef USE_PMAN

CDoDParticle *CDoDParticle::Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
	const char *classname, bool bDistCull )
{
	return NULL;
}

void CDoDParticle::Touch( vec3_t *pos, vec3_t *normal, int index )
{

}

void CDoDParticle::Die( void )
{

}

vec3_t GetViewAngles( void )
{
	return Vector( 0, 0, 0 );
}

void CDoDParticle::Think( float time )
{

}

void CDoDParticle::Force( void )
{

}

void CDoDParticle::SetGlobalWind( float *vecWind )
{

}

void CDoDParticle::AddGlobalWind( void )
{

}

void CreateExplosionSmoke( vec3_t origin, vec3_t vVelocity, bool bInsideSmoke, bool bSpawnInside, bool bBlowable )
{

}

void CreateExplosionSmokeInside( vec3_t origin )
{

}

void TriangleWallPuff::Think( float time )
{

}

void CreateDebrisWallPuff( vec3_t origin, vec3_t vVelocity, vec3_t vColor, int iPuff )
{

}

void CDoDRocketTrail::Think( float time )
{

}

CDoDRocketTrail *CDoDRocketTrail::Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
	const char *classname, bool bDistCull )
{
	return NULL;
}

void CDoDDirtExploDust::Think( float time )
{

}

CDoDDirtExploDust *CDoDDirtExploDust::Create( vec3_t *pos, vec3_t *normal, model_s *sprite, float size, float brightness,
	const char *classname, bool bDistCull )
{
	return NULL;
}

void UpdateSnow( void )
{

}

void UpdateRain( void )
{

}

#endif // USE_PMAN