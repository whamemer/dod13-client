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
// amer_knife.cpp
//
// implementation of CAmerKnife class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

extern struct p_wpninfo_s *P_WpnInfo;

LINK_ENTITY_TO_CLASS( weapon_amerknife, CAmerKnife )

enum KNIFE_e 
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW
};

void CAmerKnife::Spawn( void )
{
    CMeleeWeapon::Spawn( WEAPON_AMERKNIFE );
}

void CAmerKnife::Precache( void )
{
    PRECACHE_MODEL( P_WpnInfo[WEAPON_AMERKNIFE].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_AMERKNIFE].wmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_FAIRBAIRN].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_FAIRBAIRN].wmodel );

    CMeleeWeapon::Precache();
}

int CAmerKnife::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 0;
    p->iPosition = 0;
    p->iId = WEAPON_AMERKNIFE;
    p->iWeight = CROWBAR_WEIGHT;
    return 1;
}

int CAmerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
        return KNIFE_SLASH2;
    
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