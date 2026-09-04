//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "GL/gl.h"
#include "r_studioint.h"
#include "triangleapi.h"
#include "hud.h"

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

	if( gHUD.GetWaterLevel() <= 2 )
	{
		if( g_iOnlyClientDraw <= 0 && gHUD.GetMinimapState() )
			foglevel = 0;
		else
			foglevel = cl_fog->value;

		if( cl_fog_start->value >= 0.0f )
		{
			if( cl_fog_end->value >= 0.0f )
			{
				fogColor[0] = cl_fog_red->value;
				fogColor[1] = cl_fog_green->value;
				fogColor[2] = cl_fog_blue->value;

				if( foglevel == 1 )
				{
					if( IEngineStudio.IsHardware() == 1 )
					{
						fogColor[0] = 0.5f;
						fogColor[1] = 0.5f;
						fogColor[2] = 0.5f;
						fogColor[3] = 1.0f;
						
						glFogf( GL_FOG_MODE, ( GLfloat ) GL_LINEAR ); // glFogi() alternate
						glFogfv( GL_FOG_COLOR, fogColor );
						glFogf( GL_FOG_DENSITY, cl_fog_density->value );
						glHint( GL_FOG_HINT, GL_NICEST );
						glFogf( GL_FOG_START, cl_fog_start->value );
						glFogf( GL_FOG_END, cl_fog_end->value );
						glEnable( GL_FOG );
					}
				}
				else if( foglevel == 2 )
				{
					gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start->value, cl_fog_end->value, 1 );
				}
				else
				{
					gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start->value, cl_fog_end->value, 1 );

					if( IEngineStudio.IsHardware() == 1 )
						glDisable( GL_FOG );
				}
			}
		}
	}
}