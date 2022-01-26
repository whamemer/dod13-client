#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_melee, CMeleeWeapon );

void CMeleeWeapon::Spawn( void )
{
    CMeleeWeapon::Precache();

    FallInit();
}

void CMeleeWeapon::Precache( void )
{
    PRECACHE_SOUND( "weapons/knife_deploy1.wav" );
    PRECACHE_SOUND( "weapons/knife_hit1.wav" );
    PRECACHE_SOUND( "weapons/knife_hit2.wav" );
    PRECACHE_SOUND( "weapons/knife_hit3.wav" );
    PRECACHE_SOUND( "weapons/knife_hit4.wav" );
    PRECACHE_SOUND( "weapons/knife_hitwall1.wav" );
    PRECACHE_SOUND( "weapons/knife_slash1.wav" );
    PRECACHE_SOUND( "weapons/knife_slash2.wav" );
    PRECACHE_SOUND( "weapons/knifeswing.wav" );
    PRECACHE_SOUND( "weapons/knifeswing2.wav" );

    m_usKnifeFireEvent = PRECACHE_EVENT( 1, "events/weapons/melee.sc" );
}

int CMeleeWeapon::AddToplayer( CBasePlayer *pPlayer )
{
    return FALSE;
}

BOOL CMeleeWeapon::Deploy( void )
{
    return DefaultDeploy( 0, 0, KNIFE_DRAW, "knife", 1 );
}

void CMeleeWeapon::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CMeleeWeapon::PrimaryAttack( void )
{
    CMeleeWeapon::Swing( 1 );
}

void CMeleeWeapon::Smack( void )
{
    DecalGunshot( &m_trHit, BULLET_NONE );
}

void CMeleeWeapon::SwingAgain( void )
{
    CMeleeWeapon::Swing( 0 );
}

int CMeleeWeapon::Swing( int fFirst )
{

}

int CMeleeWeapon::Stab( int fFirst )
{

}

void CMeleeWeapon::WeaponIdle( void )
{
    int iAnim;
    float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0f, 1.0f );

    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( flRand > 0.0f )
        {
            iAnim = KNIFE_IDLE;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
        }
    }
}

LINK_ENTITY_TO_CLASS( weapon_amerkinfe, CAmerKnife );

void CAmerKnife::Spawn( void )
{
    Precache();
    m_iId = WEAPON_AMER_KNIFE;
    SET_MODEL( ENT( pev ), "models/w_amerk.mdl" );

    FallInit();
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

LINK_ENTITY_TO_CLASS( weapon_spade, CSpade );

void CSpade::Spawn( void )
{
    Precache();
    m_iId = WEAPON_SPADE;
    SET_MODEL( ENT( pev ), "models/w_spade.mdl" );

    FallInit();
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
	p->iMaxClip = SPADE_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_SPADE;
	p->iWeight = SPADE_WEIGHT;

	return 1;
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

LINK_ENTITY_TO_CLASS( weapon_amerkinfe, CGerKnife );

void CGerKnife::Spawn( void )
{
    Precache();
    m_iId = WEAPON_AMER_KNIFE;
    SET_MODEL( ENT( pev ), "models/w_fairbairn.mdl" );

    FallInit();
}

void CGerKnife::Precache( void )
{
    PRECACHE_MODEL( "models/v_fairbairn.mdl" );
    PRECACHE_MODEL( "models/w_fairbairn.mdl" );
    PRECACHE_MODEL( "models/p_fairbairn.mdl" );

    CGerKnife::Precache();
}

int CGerKnife::GetItemInfo( ItemInfo *p )
{
    p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GERKNIFE_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_GER_KNIFE;
	p->iWeight = GERKNIFE_WEIGHT;

	return 1;
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