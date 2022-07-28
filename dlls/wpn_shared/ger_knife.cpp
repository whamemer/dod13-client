#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_gerkinfe, CGerKnife );

void CGerKnife::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_paraknife.mdl" );

	CMeleeWeapon::Spawn( WEAPON_GER_KNIFE );
}

void CGerKnife::Precache( void )
{
    PRECACHE_MODEL( "models/v_paraknife.mdl" );
    PRECACHE_MODEL( "models/w_paraknife.mdl" );
    PRECACHE_MODEL( "models/p_paraknife.mdl" );

    CMeleeWeapon::Precache();
}

int CGerKnife::GetItemInfo( ItemInfo *p )
{
    p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GERKNIFE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_GER_KNIFE;
	p->iWeight = GERKNIFE_WEIGHT;

	return 1;
}

BOOL CGerKnife::Deploy( void )
{
    return DefaultDeploy( "models/v_paraknife.mdl", "models/p_paraknife.mdl", KNIFE_DRAW, "gerknife" );
}

int CGerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
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