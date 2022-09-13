/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
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

LINK_ENTITY_TO_CLASS( weapon_amerknife, CGerKnife )

extern struct p_wpninfo_s P_WpnInfo[];

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
    PRECACHE_MODEL( P_WpnInfo[WEAPON_GERKNIFE].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_GERKNIFE].wmodel );
    PRECACHE_MODEL( P_WpnInfo[35].vmodel );

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
    p->iId = WEAPON_GERKNIFE;
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