//
// scope.cpp
//
// implementation of CHudScope class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "r_studioint.h"

#include <string.h>
#include <stdio.h>

#include "dod_shared.h"

#include "screenfade.h"
#include "shake.h"

#include "pm_shared.h"

#include "../com_weapons.h"

#include "demo_api.h"
#include "view.h"
#include "triangleapi.h"

DECLARE_MESSAGE( m_Scope, Scope )

extern engine_studio_api_t IEngineStudio;

/*
====================
CalcFov
====================
*/
float CalcFovDOD( float fov_x, float width, float height )
{
	float a;
	float x;

	if( fov_x < 1.0f || fov_x > 179.0f )
		fov_x = 90.0f;	// error, set to 90

	x = width / tan( fov_x / 360.0f * M_PI_F );

	a = atan ( height / x );

	a = a * 360.0f / M_PI_F;

	return a;
}

int CHudScope::Init( void )
{
    m_iWeaponId = 0;
    m_bWeaponChanged = false;
    m_nLastWpnId = 0;

    HOOK_MESSAGE( Scope );

    m_iFlags |= HUD_ACTIVE;

    gHUD.AddHudElem( this );

    return 1;
}

int CHudScope::VidInit( void )
{
    int HUD_scope_spring = gHUD.GetSpriteIndex( "scope_spring" );
    int HUD_scope_k43 = gHUD.GetSpriteIndex( "scope_k43" );
    int HUD_scope_binoc = gHUD.GetSpriteIndex( "scope_binoc" );
    int HUD_scope_enfield = gHUD.GetSpriteIndex( "scope_enfield" );

    if( HUD_scope_spring > 0 )
        spring_sprite = gHUD.GetSprite( HUD_scope_spring );

    if( IEngineStudio.IsHardware() )
    {
        spring_model = gEngfuncs.GetSpritePointer( spring_sprite );
    }
    else
    {
        spring_model = gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_spring_soft.spr" ) );
    }

    if( HUD_scope_k43 > 0 )
        k43_sprite = gHUD.GetSprite( HUD_scope_k43 );

    if( IEngineStudio.IsHardware() )
    {
        k43_model = gEngfuncs.GetSpritePointer( k43_sprite );
    }
    else
    {
        k43_model = gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_k43_soft.spr" ) );
    }

    if( HUD_scope_enfield > 0 )
        k43_sprite = gHUD.GetSprite( HUD_scope_enfield );

    if( IEngineStudio.IsHardware() )
    {
        enfield_model = gEngfuncs.GetSpritePointer( enfield_sprite );
    }
    else
    {
        enfield_model = gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_enfield_soft.spr" ) );
    }

    Reset();

    return 1;
}

int CHudScope::MsgFunc_Scope( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );
    int x = READ_BYTE();

    if( x != m_iWeaponId )
        m_bWeaponChanged = true;
    
    m_iWeaponId = x;

    return 1;
}

void CHudScope::SetScope( int weaponId )
{
    if( m_iWeaponId != weaponId )
        m_bWeaponChanged = true;

    m_iWeaponId = weaponId;
}

void CHudScope::Reset( void )
{
    if( m_iWeaponId )
        m_bWeaponChanged = true;
    
    m_iWeaponId = 0;
}

int CHudScope::Draw( float flTime )
{
    return 1;
}

void CHudScope::Think( void )
{
    if( m_bWeaponChanged )
    {
        if( m_iWeaponId == WEAPON_SCOPEDKAR || m_iWeaponId == WEAPON_SPRING || m_iWeaponId == WEAPON_FG42 
            || m_iWeaponId == WEAPON_BINOC || m_iWeaponId == WEAPON_ENFIELD && gHUD.g_iWeaponFlags )
        {

            screenfade_t sf;
            gEngfuncs.pfnGetScreenFade( &sf );

		    sf.fader = 0;
		    sf.fadeg = 0;
		    sf.fadeb = 0;
		    sf.fadealpha = -1;
		    sf.fadeFlags = FFADE_OUT;

            gEngfuncs.pfnGetScreenFade( &sf );
        }
        else
        {
            screenfade_t sf;
            gEngfuncs.pfnGetScreenFade( &sf );

		    sf.fader = 0;
		    sf.fadeg = 0;
		    sf.fadeb = 0;
		    sf.fadealpha = 0;
		    sf.fadeFlags = FFADE_LONGFADE;

            gEngfuncs.pfnGetScreenFade( &sf );
        }

        m_bWeaponChanged = false;

        // TODO: WHAMER
    }
}

void CHudScope::DrawTriApiScope( void )
{
    extern float in_fov;

    vec3_t viewangles;

    gEngfuncs.GetViewAngles( (float *)viewangles );

    if( g_iUser1 == OBS_IN_EYE )
    {
        m_iWeaponId = WEAPON_SPRING;
        viewangles = v_angles;
    }

    int iPlaying = 0;

    if( gHUD.m_iFOV - 1 < 90 )
        iPlaying = gEngfuncs.pDemoAPI->IsPlayingback() == 0;

    if( in_fov == gHUD.m_iFOV && IEngineStudio.IsHardware() != 0 && gEngfuncs.pDemoAPI->IsPlayingback() )
    {
        if( in_fov )
            in_fov = 90;

        if( !m_iWeaponId )
            m_iWeaponId = gHUD.GetCurrentWeaponId();
        
        switch( m_iWeaponId )
        {
            case WEAPON_SCOPEDKAR:
                spring_model = k43_model;
                break;
            case WEAPON_SPRING:
                spring_model = spring_model;
                break;
            case WEAPON_FG42:
                if( !gHUD.g_iWeaponFlags )
                    return;
                spring_model = k43_model;
                break;
            case WEAPON_ENFIELD:
                if( !gHUD.g_iWeaponFlags )
                    return;
                spring_model = enfield_model;
                break;
            case WEAPON_BINOC:
                spring_model = binoc_model;
                break;
            default:
                return;
        }

        if( spring_model )
        {
            float fov;
            float width, height;

            fov = CalcFovDOD( in_fov, (float)ScreenWidth, (float)ScreenHeight );

            width = tan( in_fov / 360.0f * M_PI_F ) * 5.0f * 1.1f;
            height = tan( fov / 360.0f * M_PI_F ) * 5.0f * 1.1f;

            vec3_t view, forward, right, up, temporg, v_origin;

            if( gHUD.m_fPlayerDead )
                view.z = 80.f;
            
            gEngfuncs.pfnAngleVectors( view, forward, right, up );
            gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
            gEngfuncs.pTriAPI->CullFace( TRI_FRONT );

            int i;

            while( TRUE )
            {
                while( TRUE )
                {
                    if( !gEngfuncs.pTriAPI->SpriteTexture( (model_s *)spring_model, i - 1 ) )
                        return;
                    
                    gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
                    gEngfuncs.pTriAPI->Begin( TRI_QUADS );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    if( i != 3 )
                        break;

                    temporg = -height * up + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = forward * 5.0f + v_origin;
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = width * right + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = -height * up + width * right + ( forward * 5.0f + v_origin );

pasti:
                    ++i;
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );

                    gEngfuncs.pTriAPI->End();
                }

                if( i == 4 )
                {
                    temporg = -height * up + right * -width + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = right * -width + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = forward * 5.0f + v_origin;
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = -height * up + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );

                    gEngfuncs.pTriAPI->End();
                    return;
                }

                if( i == 2 )
                {
                    temporg = forward * 5.0f + v_origin;
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = height * up + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = height * up + width * right + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    gEngfuncs.pTriAPI->Brightness( 1 );
                    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                    temporg = width * right + ( forward * 5.0f + v_origin );
                    gEngfuncs.pTriAPI->Vertex3fv( temporg );
                    
                    goto pasti;
                }

                temporg = -width * right + ( forward * 5.0f + v_origin );
                gEngfuncs.pTriAPI->Vertex3fv( temporg );
                gEngfuncs.pTriAPI->Brightness( 1 );
                gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                temporg = height * up + -width * right + ( forward * 5.0f + v_origin );
                gEngfuncs.pTriAPI->Vertex3fv( temporg );
                gEngfuncs.pTriAPI->Brightness( 1 );
                gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                temporg = height * up + ( forward * 5.0f + v_origin );
                gEngfuncs.pTriAPI->Vertex3fv( temporg );
                gEngfuncs.pTriAPI->Brightness( 1 );
                gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

                temporg = forward * 5.0f + v_origin;
                gEngfuncs.pTriAPI->Vertex3fv( temporg );

                gEngfuncs.pTriAPI->End();
                ++i;
            }
        }
    }
}