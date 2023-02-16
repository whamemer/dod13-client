//
// ger_knife.cpp
//
// implementation of CGerKnife class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

extern struct p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_amerknife, CGerKnife )

enum KNIFE_e 
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW
};

void CGerKnife::Spawn( void )
{
    CMeleeWeapon::Spawn( WEAPON_GERKNIFE );
}

void CGerKnife::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_GERKNIFE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_GERKNIFE].wmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_GERPARAKNIFE].vmodel );

    CMeleeWeapon::Precache();
}

int CGerKnife::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 0;
    p->iPosition = 0;
    p->iId = m_iId = WEAPON_GERKNIFE;
    p->iWeight = CROWBAR_WEIGHT;
    return 1;
}

int CGerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
        return KNIFE_SLASH2;
    
    return KNIFE_SLASH1;
}

int CGerKnife::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CGerKnife::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}