#include "hud.h"
#include "cl_util.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#include "r_studioint.h"

#include <gl/GL.h>

extern engine_studio_api_t IEngineStudio;

extern cvar_t *cl_fog;
extern cvar_t *cl_fog_blue;
extern cvar_t *cl_fog_density;
extern cvar_t *cl_fog_end;
extern cvar_t *cl_fog_green;
extern cvar_t *cl_fog_red;
extern cvar_t *cl_fog_start;

void RenderDoDFog( void )
{
    int foglevel;
    float fl_fogFv[4], fogColor[3];

	if( gHUD.GetWaterLevel() < 3 )
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
                        fl_fogFv[0] = 0.5f;
                        fl_fogFv[1] = 0.5f;
                        fl_fogFv[2] = 0.5f;
                        fl_fogFv[3] = 1.0f;

                        glFogi( GL_FOG_MODE, GL_LINEAR );
                        glFogfv( GL_FOG_COLOR, fl_fogFv );
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
                    gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start->value, cl_fog_end->value, 0 );

                    if( IEngineStudio.IsHardware() == 1 )
                        glDisable( GL_FOG );
                }
            }
        }
    }
}