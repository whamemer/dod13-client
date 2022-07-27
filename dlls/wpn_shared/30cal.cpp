#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

enum CAL30_e
{
    CAL30_UPIDLE,
    CAL30_UPIDLE8,
    CAL30_UPIDLE7,
    CAL30_UPIDLE6,
    CAL30_UPIDLE5,
    CAL30_UPIDLE4,
    CAL30_UPIDLE3,
    CAL30_UPIDLE2,
    CAL30_UPIDLE1,
    CAL30_UPIDLEEMPTY,
    CAL30_DOWNIDLE,
    CAL30_DOWNIDLE8,
    CAL30_DOWNIDLE7,
    CAL30_DOWNIDLE6,
    CAL30_DOWNIDLE5,
    CAL30_DOWNIDLE4,
    CAL30_DOWNIDLE3,
    CAL30_DOWNIDLE2,
    CAL30_DOWNIDLE1,
    CAL30_DOWNIDLEEMPTY,
    CAL30_DOWNTOUP,
    CAL30_DOWNTOUP8,
    CAL30_DOWNTOUP7,
    CAL30_DOWNTOUP6,
    CAL30_DOWNTOUP5,
    CAL30_DOWNTOUP4,
    CAL30_DOWNTOUP3,
    CAL30_DOWNTOUP2,
    CAL30_DOWNTOUP1,
    CAL30_DOWNTOUPEMPTY,
    CAL30_UPTODOWN,
    CAL30_UPTODOWN8,
    CAL30_UPTODOWN7,
    CAL30_UPTODOWN6,
    CAL30_UPTODOWN5,
    CAL30_UPTODOWN4,
    CAL30_UPTODOWN3,
    CAL30_UPTODOWN2,
    CAL30_UPTODOWN1,
    CAL30_UPTODOWNEMPTY,
    CAL30_UPSHOOT,
    CAL30_UPSHOOT8,
    CAL30_UPSHOOT7,
    CAL30_UPSHOOT6,
    CAL30_UPSHOOT5,
    CAL30_UPSHOOT4,
    CAL30_UPSHOOT3,
    CAL30_UPSHOOT2,
    CAL30_UPSHOOT1,
    CAL30_DOWNSHOOT,
    CAL30_DOWNSHOOT8,
    CAL30_DOWNSHOOT7,
    CAL30_DOWNSHOOT6,
    CAL30_DOWNSHOOT5,
    CAL30_DOWNSHOOT4,
    CAL30_DOWNSHOOT3,
    CAL30_DOWNSHOOT2,
    CAL30_DOWNSHOOT1,
    CAL30_RELOAD
};

LINK_ENTITY_TO_CLASS( weapon_30cal, C30CAL );
LINK_ENTITY_TO_CLASS( ammo_30cal, C30CAL );

void C30CAL::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/w_30cal.mdl" );
    m_iDefaultAmmo = CAL30_DEFAULT_GIVE;

    CBipodWeapon::Spawn( WEAPON_CAL30 );
}

void C30CAL::Precache( void )
{
    PRECACHE_MODEL( "models/v_30cal.mdl" );
    PRECACHE_MODEL( "models/w_30cal.mdl" );
    PRECACHE_MODEL( "models/p_30cal.mdl" );

    PRECACHE_SOUND( "weapons/357_cock1.wav" );
    PRECACHE_SOUND( "weapons/chainpull.wav" );
    PRECACHE_SOUND( "weapons/bulletchain.wav" );
    PRECACHE_SOUND( "weapons/clampdown.wav" );
    PRECACHE_SOUND( "weapons/mgbolt.wav" );
    PRECACHE_SOUND( "weapons/30cal_shoot.wav" );
    PRECACHE_SOUND( "weapons/tommy_draw_slideback.wav" );

    m_usFire30CAL = PRECACHE_EVENT( 1, "events/weapons/30cal.sc" );
}

int C30CAL::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = "ammo_66mm_us";
    p->pszAmmo2 = NULL;
    p->iMaxAmmo1 = _66MM_US_MAX_CARRY;
    p->iMaxAmmo2 = -1;
    p->iSlot = 2;
    p->iPosition = 0;
    p->iFlags = ITEM_FLAG_MG34CAL30;
    p->iMaxClip = CAL30_MAX_CLIP;
	p->iId = m_iId = WEAPON_CAL30;
    p->iBulletId = BULLET_PLAYER_30CAL;
    p->flRecoil = 15.0f;
	p->iWeight = CAL30_WEIGHT;

	return 1;
}

BOOL C30CAL::Deploy( void )
{
    return DefaultDeploy( "models/v_30cal.mdl", "models/p_30cal.mdl", , "30cal" );
}

int C30CAL::Classify( void )
{
    return CLASS_MACHINEGUN;
}

int C30CAL::GetReloadAnim( void )
{
    return CAL30_RELOAD;
}

int C30CAL::GetDrawAnim( void )
{
    return GetDownToUpAnim();
}

int C30CAL::GetIdleAnim( void )
{
    return FALSE;
}

int C30CAL::GetUpToDownAnim( void )
{
    int result = RANDOM_LONG( 0, 8 );

    if( m_iClip <= CAL30_UPIDLE1 )
    {
        switch( result )
        {
        case 0:
        	SendWeaponAnim( CAL30_UPTODOWN8 );
        	break;
        case 1:
        	SendWeaponAnim( CAL30_UPTODOWN7 );
        	break;
        case 2:
        	SendWeaponAnim( CAL30_UPTODOWN6 );
        	break;
        case 3:
            SendWeaponAnim( CAL30_UPTODOWN5 );
            break;
        case 4:
            SendWeaponAnim( CAL30_UPTODOWN4 );
            break;
        case 5:
            SendWeaponAnim( CAL30_UPTODOWN3 );
            break;
        case 6:
            SendWeaponAnim( CAL30_UPTODOWN2 );
            break;
        case 7:
            SendWeaponAnim( CAL30_UPTODOWN1 );
            break;
        case 8:
            SendWeaponAnim( CAL30_UPTODOWNEMPTY );
            break;
        }
    }
    else
    {
        return CAL30_UPTODOWN;
    }

    return result;
}

int C30CAL::GetDownToUpAnim( void )
{
    int result = RANDOM_LONG( 0, 8 );

    if( m_iClip <= CAL30_UPIDLE1 )
    {
        switch( result )
        {
        case 0:
        	SendWeaponAnim( CAL30_DOWNTOUP8 );
        	break;
        case 1:
        	SendWeaponAnim( CAL30_DOWNTOUP7 );
        	break;
        case 2:
        	SendWeaponAnim( CAL30_DOWNTOUP6 );
        	break;
        case 3:
            SendWeaponAnim( CAL30_DOWNTOUP5 );
            break;
        case 4:
            SendWeaponAnim( CAL30_DOWNTOUP4 );
            break;
        case 5:
            SendWeaponAnim( CAL30_DOWNTOUP3 );
            break;
        case 6:
            SendWeaponAnim( CAL30_DOWNTOUP2 );
            break;
        case 7:
            SendWeaponAnim( CAL30_DOWNTOUP1 );
            break;
        case 8:
            SendWeaponAnim( CAL30_DOWNTOUPEMPTY );
            break;
        }
    }
    else
    {
        return CAL30_DOWNTOUP8;
    }

    return result;
}

void C30CALAmmoClip::Spawn( void )
{
    SET_MODEL( ENT( pev ), "models/allied_ammo.mdl" );
    CBasePlayerAmmo::Spawn();
}

BOOL C30CALAmmoClip::AddAmmo( CBaseEntity *pOther )
{
	if( pOther->GiveAmmo( AMMO_CAL30CLIP_GIVE, "ammo_66mm_us", _66MM_US_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/ammopickup.wav", 1, ATTN_NORM );
        return TRUE;
	}
	return FALSE;
}