#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_spade, CSpade );

void CSpade::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_spade.mdl" );

	CMeleeWeapon::Spawn( WEAPON_SPADE );
}

void CSpade::Precache( void )
{
    PRECACHE_MODEL( "models/v_spade.mdl" );
    PRECACHE_MODEL( "models/w_spade.mdl" );
    PRECACHE_MODEL( "models/p_spade.mdl" );

    CMeleeWeapon::Precache();
}

int CSpade::GetItemInfo( ItemInfo *p )
{
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SPADE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_SPADE;
	p->iWeight = SPADE_WEIGHT;

	return 1;
}

BOOL CSpade::Deploy( void )
{
    return DefaultDeploy( "models/v_spade.mdl", "models/p_spade.mdl", KNIFE_DRAW, "spade" );
}

int CSpade::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
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
