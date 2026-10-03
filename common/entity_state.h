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
#pragma once
#if !defined(ENTITY_STATE_H)
#define ENTITY_STATE_H

// For entityType below
#define ENTITY_NORMAL		(1<<0)
#define ENTITY_BEAM			(1<<1)

// Entity state is used for the baseline and for delta compression of a packet of 
//  entities that is sent to a client.
typedef struct entity_state_s entity_state_t;

struct entity_state_s
{
	int                        entityType;
	int                        number;
	float                      msg_time;
	int                        messagenum;

	vec3_t                     origin;
	vec3_t                     angles;

	int                        modelindex;
	int                        sequence;
	float                      frame;
	int                        colormap;
	short int                  skin;
	short int                  solid;
	int                        effects;
	float                      scale;
	byte                       eflags;

	int                        rendermode;
	int                        renderamt;
	color24                    rendercolor;
	int                        renderfx;

	int                        movetype;
	float                      animtime;
	float                      framerate;
	int                        body;
	unsigned char              controller[4];
	unsigned char              blending[4];
	vec3_t                     velocity;

	vec3_t                     mins;
	vec3_t                     maxs;

	int                        aiment;
	int                        owner;

	float                      friction;
	float                      gravity;

	int                        team;
	int                        playerclass;
	int                        health;
	qboolean                   spectator;
	int                        weaponmodel;
	int                        gaitsequence;
	vec3_t                     basevelocity;
	int                        usehull;
	int                        oldbuttons;
	int                        onground;
	int                        iStepLeft;
	float                      flFallVelocity;

	float                      fov;
	int                        weaponanim;

	vec3_t                     startpos;
	vec3_t                     endpos;
	float                      impacttime;
	float                      starttime;

	int                        iuser1;
	int                        iuser2;
	int                        iuser3;
	int                        iuser4;
	float                      fuser1;
	float                      fuser2;
	float                      fuser3;
	float                      fuser4;
	vec3_t                     vuser1;
	vec3_t                     vuser2;
	vec3_t                     vuser3;
	vec3_t                     vuser4;
};

#include "pm_info.h"

typedef struct clientdata_s
{
	vec3_t		origin;
	vec3_t		velocity;

	int		viewmodel;
	vec3_t		punchangle;
	int		flags;
	int		waterlevel;
	int		watertype;
	vec3_t		view_ofs;
	float		health;

	int		bInDuck;
	int		weapons; // remove?
	
	int		flTimeStepSound;
	int		flDuckTime;
	int		flSwimTime;
	int		waterjumptime;

	float		maxspeed;

	float		fov;
	int		weaponanim;

	int		m_iId;
	int		ammo_shells;
	int		ammo_nails;
	int		ammo_cells;
	int		ammo_rockets;
	float		m_flNextAttack;
	
	int		tfstate;
	int		pushmsec;
	int		deadflag;
	char		physinfo[MAX_PHYSINFO_STRING];

	// For mods
	int		iuser1;
	int		iuser2;
	int		iuser3;
	int		iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
	vec3_t		vuser1;
	vec3_t		vuser2;
	vec3_t		vuser3;
	vec3_t		vuser4;

} clientdata_t;

#include "weaponinfo.h"

typedef struct local_state_s
{
	entity_state_t	playerstate;
	clientdata_t	client;
	weapon_data_t	weapondata[64];
} local_state_t;

#endif//ENTITY_STATE_H
