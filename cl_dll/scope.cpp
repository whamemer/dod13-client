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

DECLARE_COMMAND( m_Scope, Scope )

extern engine_studio_api_t IEngineStudio;

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
            || m_iWeaponId == WEAPON_BINOC || m_iWeaponId == WEAPON_ENFIELD && g_iWeaponFlags )
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

}