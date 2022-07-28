#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum COLT_e
{
    COLT_IDLE,
    COLT_SHOOT,
    COLT_SHOOT2,
    COLT_RELOAD_EMPTY,
    COLT_RELOAD,
    COLT_DRAW,
    COLT_SHOOT_EMPTY,
    COLT_IDLE_EMPTY
};

LINK_ENTITY_TO_CLASS( weapon_colt, CCOLT );
LINK_ENTITY_TO_CLASS( ammo_colt, CCOLT );

void CCOLT::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_colt.mdl" );
    m_iDefaultAmmo = COLT_DEFAULT_GIVE;

    CPistol::Spawn( WEAPON_COLT );
}

void CCOLT::Precache( void )
{
    PRECACHE_MODEL( "models/v_colt.mdl" );
    PRECACHE_MODEL( "models/w_colt.mdl" );
    PRECACHE_MODEL( "models/p_colt.mdl" );

    PRECACHE_SOUND( "weapons/colt_shoot.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipin.wav" );
    PRECACHE_SOUND( "weapons/colt_reload_clipout.wav" );
    PRECACHE_SOUND( "weapons/colt_draw_pullslide.wav" );

    m_usFireColt = PRECACHE_EVENT( 1, "events/weapons/colt.sc" );
}

int CCOLT::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_12mm";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _12MM_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_PISTOL;
    p->iMaxClip = COLT_MAX_CLIP;
	p->iId = m_iId = WEAPON_COLT;
    p->iBulletId = BULLET_PLAYER_COLT;
    p->flSpread = 0.035f;
	p->iWeight = COLT_WEIGHT;

	return 1;
}

BOOL CCOLT::Deploy( void )
{
    return DefaultDeploy( "models/v_colt.mdl", "models/p_colt.mdl", COLT_DRAW, "colt" );
}

int CCOLT::GetReloadAnim( void )
{
    return COLT_RELOAD - ( m_iClip == COLT_IDLE );
}

int CCOLT::GetDrawAnim( void )
{
    return COLT_DRAW;
}

int CCOLT::GetIdleAnim( void )
{
    return m_iClip == COLT_IDLE ? COLT_IDLE_EMPTY : COLT_IDLE;
}

void CColtAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CColtAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_COLTCLIP_GIVE, "ammo_12mm", _12MM_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}