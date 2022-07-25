#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_amerkinfe, CAmerKnife );

void CAmerKnife::Spawn( void )
{	
    SET_MODEL( ENT( pev ), "models/w_amerk.mdl" );

	CMeleeWeapon::Spawn( WEAPON_AMER_KNIFE );
}

void CAmerKnife::Precache( void )
{
    PRECACHE_MODEL( "models/v_amerk.mdl" );
    PRECACHE_MODEL( "models/w_amerk.mdl" );
    PRECACHE_MODEL( "models/p_amerk.mdl" );

    CMeleeWeapon::Precache();
}

int CAmerKnife::GetItemInfo( ItemInfo *p )
{
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AMERKNIFE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_AMER_KNIFE;
	p->iWeight = AMERKNIFE_WEIGHT;

	return 1;
}

BOOL CAmerKnife::Deploy( void )
{
    return DefaultDeploy( "models/v_amerk.mdl", "models/p_amerk.mdl", KNIFE_DRAW, "amerknife" );
}

int CAmerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
    return KNIFE_SLASH1;
}

int CAmerKnife::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CAmerKnife::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}