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
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"

IParticleMan *g_pParticleMan;

extern BEAM *pBeam;
extern BEAM *pBeam2;

extern float g_lastFOV;
extern int g_iAlive;

int CHud::MsgFunc_ResetHUD( const char *pszName, int iSize, void *pbuf )
{

}

int CHud::MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf )
{

}

void CHud::MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{

}

int CHud::MsgFunc_Damage( const char *pszName, int iSize, void *pbuf )
{

}