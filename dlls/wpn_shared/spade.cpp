//
// spade.cpp
//
// implementation of CSpade class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"

#include "dod_shared.h"

extern p_wpninfo_s *WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_spade, CSpade )

void CSpade::Spawn( void )
{
    CMeleeWeapon::Spawn( WEAPON_SPADE );
}

void CSpade::Precache( void )
{
    PRECACHE_MODEL( WpnInfo[WEAPON_SPADE].vmodel );
    PRECACHE_MODEL( WpnInfo[WEAPON_SPADE].wmodel );

    CMeleeWeapon::Precache();
}

int CSpade::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 0;
    p->iPosition = 2;
    p->iId = m_iId = WEAPON_SPADE;
    p->iWeight = WEAPON_NOWEIGHT;
    return 1;
}

int CSpade::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
        return KNIFE_SLASH2;
    
    return KNIFE_SLASH1;
}

int CSpade::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CSpade::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}