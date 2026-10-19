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
// dod_fog.cpp
//
// the fog of the client, it is drawn in HUD_DrawTransparentTriangles
//

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "triangleapi.h"
#include "r_studioint.h"

#include "tri.h"

extern engine_studio_api_t IEngineStudio;

extern cvar_t *cl_fog_density;
extern cvar_t *cl_fog_start;
extern cvar_t *cl_fog_end;
extern cvar_t *cl_fog;
extern cvar_t *cl_fog_red;
extern cvar_t *cl_fog_green;
extern cvar_t *cl_fog_blue;

extern int g_iOnlyClientDraw;

void RenderDoDFog( void )
{
	int foglevel;
	float fogColor[3];

	// no fog under the water
	if( gHUD.GetWaterLevel() > 2 )
		return;

#if FOG_MINIMAP_FIX
	// the fog is for every mode of the map, SuspendDoDFog turns it off while the map itself is drawn
	foglevel = (int)cl_fog->value;
#else
	// no fog when the map is shown
	if( g_iOnlyClientDraw <= 0 && gHUD.GetMinimapState() )
		foglevel = 0;
	else
		foglevel = (int)cl_fog->value;
#endif

	if( cl_fog_start->value < 0.0f || cl_fog_end->value < 0.0f )
		return;

	fogColor[0] = cl_fog_red->value;
	fogColor[1] = cl_fog_green->value;
	fogColor[2] = cl_fog_blue->value;

	if( foglevel == 1 )
	{
		// the original sets the fog of OpenGL here (linear, gray 0.5, hint nicest), but the client has no
		// access to the OpenGL of the renderer in Xash3D (ref_gl, ref_soft, ... are separate modules),
		// TriAPI Fog does the same and the engine tracks its state
		if( IEngineStudio.IsHardware() == 1 )
		{
			float fogGray[3] = { 127.5f, 127.5f, 127.5f };

			gEngfuncs.pTriAPI->Fog( fogGray, cl_fog_start->value, cl_fog_end->value, 1 );
		}
	}
	else if( foglevel == 2 )
	{
		gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start->value, cl_fog_end->value, 1 );
	}
	else
	{
		gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start->value, cl_fog_end->value, 0 );
	}
}

#if FOG_MINIMAP_FIX
// The map is the second view of the frame and the fog that the engine keeps would gray it, so the fog is turned off while
// the map is drawn, RenderDoDFog turns it on again for the next frame. The range is empty on purpose: TriAPI Fog with a
// valid range only disables GL_FOG, and the next TriAPI RenderMode of the map turns it on again from the flag of the
// engine (R_AllowFog); an empty range is the one that also clears that flag
void SuspendDoDFog( void )
{
	float fogColor[3] = { 0.0f, 0.0f, 0.0f };

	gEngfuncs.pTriAPI->Fog( fogColor, 0.0f, 0.0f, 0 );
}
#endif
