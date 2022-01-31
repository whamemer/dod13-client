#include "hud.h"

void RenderDoDFog( void )
{
    int i;
    float fogColor_0[4], fogColor[3];

	if( CHud::GetWaterLevel( &gHUD ) <= 2 )
    {
        if( cl_fog_start->value >= 0.0f )
        {
            if( cl_fog_end->value >= 0.0f )
            {
                fogColor[0] = cl_fog_red->value;
                fogColor[1] = cl_fog_green->value;
                fogColor[2] = cl_fog_blue->value;

                if( cl_fog->value == 1 )
                {
                    if( IEngineStudio.IsHardware() == 1 )
                    {
                        fogColor_0[0] = 0.5f;
                        fogColor_0[1] = 0.5f;
                        fogColor_0[2] = 0.5f;
                        fogColor_0[3] = 1.0f;

                        glFogi( 0xB65u, 9729 );
                        glFogfv( 0xB66u, fogColor_0 );
                        glFogf( 0xB62u, cl_fog_density->value );
                        glHint( 0xC54u, 0x1102u );
                        glFogf( 0xB63u, cl_fog_start->value );
                        glFogf( 0xB64u, cl_fog_end->value );
                        glEnable( 0xB60u );
                    }
                }
                else if( cl_fog->value == 2 )
                {
                    gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start, cl_fog_end, 1 );
                }
                else
                {
                    gEngfuncs.pTriAPI->Fog( fogColor, cl_fog_start, cl_fog_end, 0 );

                    if( IEngineStudio.IsHardware() == 1 )
                        glDisable( 0xB60u );
                }
            }
        }
    }
}