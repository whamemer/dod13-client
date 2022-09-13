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
// spade.cpp
//
// implementation of CSpade class
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "dod_gamerules.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_spade, CSpade )

extern struct p_wpninfo_s P_WpnInfo[];

enum KNIFE_e 
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW
};

void CSpade::Spawn( void )
{
    CMeleeWeapon::Spawn( WEAPON_SPADE );
}

void CSpade::Precache( void )
{
    PRECACHE_MODEL( P_WpnInfo[WEAPON_SPADE].vmodel );
    PRECACHE_MODEL( P_WpnInfo[WEAPON_SPADE].wmodel );

    CMeleeWeapon::Precache();
}

int CSpade::GetItemInfo( ItemInfo *p )
{
    p->pszName = STRING( pev->classname );
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
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