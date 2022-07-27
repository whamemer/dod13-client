#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum MG42_e
{
    MG42_UPIDLE,
    MG42_UPIDLE81,
    MG42_UPIDLE72,
    MG42_UPIDLE63,
    MG42_UPIDLE54,
    MG42_UPIDLE45,
    MG42_UPIDLE36,
    MG42_UPIDLE27,
    MG42_UPIDLE18,
    MG42_UPIDLEEMPTY,
    MG42_DOWNIDLE,
    MG42_DOWNIDLE8,
    MG42_DOWNIDLE7,
    MG42_DOWNIDLE6,
    MG42_DOWNIDLE5,
    MG42_DOWNIDLE4,
    MG42_DOWNIDLE3,
    MG42_DOWNIDLE2,
    MG42_DOWNIDLE1,
    MG42_DOWNIDLEEMPTY,
    MG42_DOWNTOUP8,
    MG42_DOWNTOUP7,
    MG42_DOWNTOUP6,
    MG42_DOWNTOUP5,
    MG42_DOWNTOUP4,
    MG42_DOWNTOUP3,
    MG42_DOWNTOUP2,
    MG42_DOWNTOUP1,
    MG42_DOWNTOUPEMPTY,
    MG42_UPTODOWN,
    MG42_UPTODOWN8,
    MG42_UPTODOWN7,
    MG42_UPTODOWN6,
    MG42_UPTODOWN5,
    MG42_UPTODOWN4,
    MG42_UPTODOWN3,
    MG42_UPTODOWN2,
    MG42_UPTODOWN1,
    MG42_UPTODOWNEMPTY,
    MG42_UPSHOOT,
    MG42_UPSHOOT8,
    MG42_UPSHOOT7,
    MG42_UPSHOOT6,
    MG42_UPSHOOT5,
    MG42_UPSHOOT4,
    MG42_UPSHOOT3,
    MG42_UPSHOOT2,
    MG42_UPSHOOT1,
    MG42_DOWNSHOOT,
    MG42_DOWNSHOOT8,
    MG42_DOWNSHOOT7,
    MG42_DOWNSHOOT6,
    MG42_DOWNSHOOT5,
    MG42_DOWNSHOOT4,
    MG42_DOWNSHOOT3,
    MG42_DOWNSHOOT2,
    MG42_DOWNSHOOT1,
    MG42_RELOAD
};

LINK_ENTITY_TO_CLASS( weapon_mg42, CMG34 );
LINK_ENTITY_TO_CLASS( ammo_mg42, CMG34 );

void CMG42::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_mg34.mdl" );
    m_iDefaultAmmo = MG42_DEFAULT_GIVE;

    CBipodWeapon::Spawn( WEAPON_MG42 );
}

void CMG42::Precache( void )
{
    PRECACHE_MODEL( "models/v_mg42.mdl" );
    PRECACHE_MODEL( "models/w_mg42.mdl" );
    PRECACHE_MODEL( "models/p_mg42.mdl" );

    PRECACHE_SOUND( "weapons/weaponempty.wav" );
    PRECACHE_SOUND( "weapons/chainpull.wav" );
    PRECACHE_SOUND( "weapons/bulletchain.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/mg42_shoot.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );
    PRECACHE_SOUND( "weapons/mgoverheat.wav" );

    m_usFireMG42 = PRECACHE_EVENT( 1, "events/weapons/mg42.sc" );
}

int CMG42::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm_ger";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _66MM_GER_MAX_CARRY;
    p->iMaxAmmo2 = -1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_MG42;
    p->iMaxClip = MG42_MAX_CLIP;
	p->iId = m_iId = WEAPON_MG42;
    p->iBulletId = BULLET_PLAYER_MG42;
    p->flRecoil = 15.0f;
	p->iWeight = MG42_WEIGHT;

	return 1;
}

BOOL CMG42::Deploy( void )
{
    return DefaultDeploy( "models/v_mg42.mdl", "models/p_mg42.mdl", , "mg42" );
}

int CMG42::Classify( void )
{
    return CLASS_MACHINEGUN;
}

int CMG42::GetReloadAnim( void )
{
    return MG42_RELOAD;
}

int CMG42::GetDrawAnim( void )
{
    return GetDownToUpAnim();
}

int CMG42::GetIdleAnim( void )
{
    if( CBipodWeapon::IsDeployed() )
        return m_iClip <= MG34_DOWNIDLE;
    else
        return ( m_iClip <= MG34_DOWNIDLE ) + MG34_UPIDLE;
}

int CMG42::GetUpToDownAnim( void )
{
    int result = RANDOM_LONG( 0, 8 );

    if( m_iClip <= MG42_UPIDLE1 )
    {
        switch( result )
        {
        case 0:
        	SendWeaponAnim( MG42_UPTODOWN8 );
        	break;
        case 1:
        	SendWeaponAnim( MG42_UPTODOWN7 );
        	break;
        case 2:
        	SendWeaponAnim( MG42_UPTODOWN6 );
        	break;
        case 3:
            SendWeaponAnim( MG42_UPTODOWN5 );
            break;
        case 4:
            SendWeaponAnim( MG42_UPTODOWN4 );
            break;
        case 5:
            SendWeaponAnim( MG42_UPTODOWN3 );
            break;
        case 6:
            SendWeaponAnim( MG42_UPTODOWN2 );
            break;
        case 7:
            SendWeaponAnim( MG42_UPTODOWN1 );
            break;
        case 8:
            SendWeaponAnim( MG42_UPTODOWNEMPTY );
            break;
        }
    }
    else
    {
        return MG42_UPTODOWN8;
    }

    return result;
}

int CMG42::GetDownToUpAnim( void )
{
    int result = RANDOM_LONG( 0, 8 );

    if( m_iClip <= MG42_UPIDLE1 )
    {
        switch( result )
        {
        case 0:
        	SendWeaponAnim( MG42_DOWNTOUP8 );
        	break;
        case 1:
        	SendWeaponAnim( MG42_DOWNTOUP7 );
        	break;
        case 2:
        	SendWeaponAnim( MG42_DOWNTOUP6 );
        	break;
        case 3:
            SendWeaponAnim( MG42_DOWNTOUP5 );
            break;
        case 4:
            SendWeaponAnim( MG42_DOWNTOUP4 );
            break;
        case 5:
            SendWeaponAnim( MG42_DOWNTOUP3 );
            break;
        case 6:
            SendWeaponAnim( MG42_DOWNTOUP2 );
            break;
        case 7:
            SendWeaponAnim( MG42_DOWNTOUP1 );
            break;
        case 8:
            SendWeaponAnim( MG42_DOWNTOUPEMPTY );
            break;
        }
    }
    else
    {
        return MG42_DOWNTOUP8;
    }

    return result;
}

void CMG42AmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/axis_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL CMG42AmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_MG42CLIP_GIVE, "ammo_66mm_ger", _66MM_GER_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}