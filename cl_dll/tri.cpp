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

#include "tri.h"

#include "r_studioint.h"

#include "../particleman/particleman/src/particleman/particlefx_header/pman_frustum.h"

#include "pm_defs.h"

cldll_func_dst_t *g_pcldstAddrs;

extern engine_studio_api_t IEngineStudio;

extern cvar_t *cl_fog;
extern cvar_t *cl_particlefx;

struct ref_params_s *pparams;

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
		char sz[256];
		sprintf( sz, "sprites/cursor.spr" );
		gHUD.m_hsprCursor = SPR_Load( sz );
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
	g_pcldstAddrs->pDrawNormalTriangles();
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

	g_pcldstAddrs->pDrawTransparentTriangles();

	vec3_t v_angles;

	VectorCopy( v_angles, pparams->viewangles );

	if( g_iOnlyClientDraw )
	{
		if( g_iOnlyClientDraw == OBS_CHASE_LOCKED && gHUD.GetMinimapState() > 0 )
		{
			gHUD.m_ObjectiveIcons.CalcIconLocations();
			gHUD.m_DoDMap.DrawOverview();
		}
	}
	else
	{
		if( cl_fog->value != 0.0 )
			RenderDoDFog();
		
		if( cl_particlefx->value >= 1.0 && IEngineStudio.IsHardware() )
		{
			if( g_iUser1 != OBS_ROAMING && g_iUser1 )
				g_pParticleMan->SetVariables( g_flGravity, v_angles );
			else
				g_pParticleMan->SetVariables( g_flGravity, gHUD.m_vecAngles);
			
			g_pParticleMan->Update();

			int g_iWeatherType;
			float g_flWeatherTime;

			if( gEngfuncs.GetClientTime() > g_flWeatherTime && cl_particlefx->value >= 2.0 )
			{
				if( g_iWeatherType == 1 )
				{
					UpdateRain();
					g_flWeatherTime = gEngfuncs.GetClientTime() + 0.3f;
				}
				else if( g_iWeatherType == 2 )
				{
					UpdateSnow();
					g_flWeatherTime = gEngfuncs.GetClientTime() + 0.7f;
				}
			}
		}

		if( gHUD.m_iFOV < 90 )
			gHUD.m_Scope.DrawTriApiScope();
		
		gHUD.m_MortarHud.DrawPredictedMortarImpactSite();
	}
}

void UpdateRain( void )
{

}

void UpdateSnow( void )
{

}