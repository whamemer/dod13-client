#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum ENFIELD_e
{
    ENFIELD_IDLE,
    ENFIELD_SHOOT,
    ENFIELD_RELOAD,
    ENFIELD_DRAW,
    ENFIELD_RELOADLONG ,
    ENFIELD_SWING
};

enum SCOPEDENFIELD_e
{
    SCOPEDENFIELD_IDLE,
    SCOPEDENFIELD_SHOOT,
    SCOPEDENFIELD_RELOAD,
    SCOPEDENFIELD_DRAW,
    SCOPEDENFIELD_RELOADLONG
};

LINK_ENTITY_TO_CLASS( weapon_enfield, CENFIELD );
LINK_ENTITY_TO_CLASS( weapon_scopedenfield, CENFIELD );
LINK_ENTITY_TO_CLASS( ammo_enfield, CENFIELD );

void CENFIELD::Spawn( void )
{
    Precache();

    if( !m_iWeaponState )
        SET_MODEL( ENT( pev ), "models/w_enfield.mdl" );
    else
        SET_MODEL( ENT( pev ), "models/w_enfield_scoped.mdl" );

    m_iDefaultAmmo = ENFIELD_DEFAULT_GIVE;
    m_iId = WEAPON_ENFIELD;
    
    FallInit();
}

void CENFIELD::Precache( void )
{
    PRECACHE_MODEL( "models/v_enfield.mdl" );
    PRECACHE_MODEL( "models/w_enfield.mdl" );
    PRECACHE_MODEL( "models/p_enfield.mdl" );

    PRECACHE_SOUND( "weapons/enfield_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/boltforward.wav" );
    PRECACHE_SOUND( "weapons/boltback.wav" );
    PRECACHE_SOUND( "weapons/tommy_reload_clipin.wav" );

    m_usFireEnfield = PRECACHE_EVENT( 1, "events/weapons/enfield.sc" );

    PRECACHE_MODEL( "models/v_enfield_scoped.mdl" );
    PRECACHE_MODEL( "models/w_enfield_scoped.mdl" );
    PRECACHE_MODEL( "models/p_enfields.mdl" );

    PRECACHE_SOUND( "weapons/enfieldsniper_shoot.wav" );

    m_usFireScopedEnfield = PRECACHE_EVENT( 1, "events/weapons/scopedenfield.sc" );
}

int CENFIELD::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_16mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _16MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_BRENBAR;
    p->iMaxClip = ENFIELD_MAX_CLIP;
    p->iId = m_iId = WEAPON_ENFIELD;
    p->iBulletId = BULLET_PLAYER_ENFIELD;
    p->iWeight = ENFIELD_WEIGHT;

    return 1;
}

int CENFIELD::AddToPlayer( CBasePlayer *pPlayer )
{
    if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CENFIELD::Deploy( void )
{
    if( !m_iWeaponState )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ;
        return DefaultDeploy( "models/v_enfield.mdl", "models/p_enfield.mdl", ENFIELD_DRAW, "enfield" );
    }
    else
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ;
        return DefaultDeploy( "models/v_enfield_scoped.mdl", "models/p_enfields.mdl", SCOPEDENFIELD_DRAW, "scopedenfield" );
    }
        
}

BOOL CENFIELD::CanHolster( void )
{
    return TRUE;
}

void CENFIELD::PrimaryAttack( void )
{
    // TODO: WHAMER
}

void CENFIELD::SecondaryAttack( void )
{

}

void CENFIELD::Reload( void )
{

}

void CENFIELD::WeaponIdle( void )
{

}

void CENFIELD::ThinkZoomOutIn( void )
{

}

void CENFIELD::Holster( int skiplocal )
{

}

int CENFIELD::Classify( void )
{
    return !m_iWeaponState ? CLASS_SNIPER : CLASS_SCOPED_SNIPER;
}

void CEnfieldAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CEnfieldAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_ENFIELDCLIP_GIVE, "ammo_16mm", _16MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}