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
#if !defined(CBASE_H)
#define CBASE_H
/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseAnimating
			CBaseToggle
				CBaseMonster
					CBasePlayer
			CBasePlayerItem
				CBasePlayerWeapon
	CBasePlayerAmmo
*/

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION			0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE			0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER				0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions

#include "saverestore.h"
#include "schedule.h"

#if !defined(MONSTEREVENT_H)
#include "monsterevent.h"
#endif

#include "exportdef.h"

typedef enum
{
	USE_OFF = 0,
	USE_ON,
	USE_SET,
	USE_TOGGLE,
	USE_KILL,
	USE_NOT = 6,
	USE_START,
	USE_BREAK,
	USE_UPDATE
} USE_TYPE;

typedef void(CBaseEntity::*BASEPTR)( void );
typedef void(CBaseEntity::*ENTITYFUNCPTR)( CBaseEntity *pOther );
typedef void(CBaseEntity::*USEPTR)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

// For CLASSIFY
#define	CLASS_NONE			0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define	CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY		4
#define CLASS_ALIEN_MILITARY		5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR		9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON		12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON		13 // hornets and snarks.launched by the alien menace
#define CLASS_VEHICLE			14
#define	CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.

class CBaseEntity;
class CBaseToggle;
class CBaseMonster;
class CBasePlayer;
class CBasePlayerItem;
class CSquadMonster;

#define	SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.

//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t *m_pent;
	int m_serialnumber;

public:
	edict_t *Get( void );
	edict_t *Set( edict_t *pent );

	operator int ();

	operator CBaseEntity *();

	CBaseEntity *operator = ( CBaseEntity *pEntity );
	CBaseEntity *operator ->();
};

//
// Base Entity.  All entity types derive from this
//
class CBaseEntity
{
public:
	// pointers to engine data
	entvars_t *pev;

	// path corners
	CBaseEntity *m_pGoalEnt;
	CBaseEntity *m_pLink;

	// state of the entity at the start of the round, it is stored by RoundStore and brought back by RoundRespawnEnt
	vec3_t m_rrOrigin;
	vec3_t m_rrMins;
	vec3_t m_rrMaxs;
	vec3_t m_rrAngles;
	vec3_t m_rrMovedir;
	int m_rrSpawnflags;
	int m_rrSolid;
	int m_rrMovetype;
	int m_rrEffects;
	int m_rrFlags;
	int m_rrFrags;
	int m_rrRendermode;
	float m_rrRenderamt;
	vec3_t m_rrRendercolor;
	int m_rrRenderfx;
	float m_rrHealth;
	float m_rrFrame;
	float m_rrSpeed;
	string_t m_rrTarget;
	string_t m_rrTargetname;
	string_t m_rrNetname;
	float m_rrTakedamage;
	float m_rrScale;
	float m_rrGravity;

	virtual void RoundRespawn( void ) { }
	virtual void RoundRespawnEnt( void )
	{
		m_pfnThink = NULL;
		m_pfnTouch = NULL;
		m_pfnUse = NULL;
		m_pfnBlocked = NULL;

		pev->nextthink = 0;

		if( m_rrOrigin.x == 0 && m_rrOrigin.y == 0 && m_rrOrigin.z == 0 )
		{
			pev->mins = m_rrMins;
			pev->maxs = m_rrMaxs;
		}
		else
		{
			pev->origin = m_rrOrigin;
		}

		pev->angles = m_rrAngles;
		pev->movedir = m_rrMovedir;
		pev->spawnflags = m_rrSpawnflags;
		pev->solid = m_rrSolid;
		pev->effects = m_rrEffects;
		pev->flags = m_rrFlags;
		pev->frags = m_rrFrags;
		pev->movetype = m_rrMovetype;
		pev->rendermode = m_rrRendermode;
		pev->renderamt = m_rrRenderamt;
		pev->rendercolor = m_rrRendercolor;
		pev->renderfx = m_rrRenderfx;
		pev->health = m_rrHealth;
		pev->frame = m_rrFrame;
		pev->speed = m_rrSpeed;
		pev->target = m_rrTarget;
		pev->targetname = m_rrTargetname;
		pev->netname = m_rrNetname;
		pev->takedamage = m_rrTakedamage;
		pev->scale = m_rrScale;
		pev->gravity = m_rrGravity;

		RoundRespawn();
	}
	virtual void RoundStore( void )
	{
		m_rrOrigin = pev->origin;

		if( m_rrOrigin.x == 0 && m_rrOrigin.y == 0 && m_rrOrigin.z == 0 )
		{
			m_rrMins = pev->mins;
			m_rrMaxs = pev->maxs;
		}

		m_rrAngles = pev->angles;
		m_rrMovedir = pev->movedir;
		m_rrSpawnflags = pev->spawnflags;
		m_rrSolid = pev->solid;
		m_rrMovetype = pev->movetype;
		m_rrEffects = pev->effects;
		m_rrFlags = pev->flags;
		m_rrFrags = pev->frags;
		m_rrRendermode = pev->rendermode;
		m_rrRenderamt = pev->renderamt;
		m_rrRendercolor = pev->rendercolor;
		m_rrRenderfx = pev->renderfx;
		m_rrHealth = pev->health;
		m_rrFrame = pev->frame;
		m_rrSpeed = pev->speed;
		m_rrTarget = pev->target;
		m_rrTargetname = pev->targetname;
		m_rrNetname = pev->netname;
		m_rrTakedamage = pev->takedamage;
		m_rrScale = pev->scale;
		m_rrGravity = pev->gravity;
	}

	// initialization functions
	virtual void Spawn( void ) { }
	virtual void Precache( void ) { }
	virtual void KeyValue( KeyValueData *pkvd ) { pkvd->fHandled = FALSE; }
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	virtual int ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void Activate( void ) { }
	virtual void area_SetIndex( int index ) { }
	virtual void area_SendStatus( CBasePlayer *p ) { }

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void SetObjectCollisionBox( void );

	// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
	// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify( void ) { return CLASS_NONE; }
	virtual void DeathNotice( entvars_t *pevChild ) { } // monster maker children use this to tell the monster maker that they have died.

	virtual STATE GetState( void ) { return STATE_OFF; }
	virtual STATE GetState( CBaseEntity *pEnt ) { return GetState(); }

	static TYPEDESCRIPTION m_SaveData[];

	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual int BloodColor( void ) { return DONT_BLEED; }
	virtual void TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual CBaseMonster *MyMonsterPointer( void ) { return NULL; }
	virtual CSquadMonster *MySquadMonsterPointer( void ) { return NULL; }
	virtual int GetToggleState( void ) { return TS_AT_TOP; }
	virtual void AddPoints( int score, BOOL bAllowNegativeScore ) { }
	virtual void AddPointsToTeam( int score, BOOL bAllowNegativeScore ) { }
	virtual BOOL AddPlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
	virtual BOOL RemovePlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
	virtual int GiveAmmo( int iAmount, const char *szName, int iMax ) { return -1; }
	virtual float GetDelay( void ) { return 0; }
	virtual int IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void OverrideReset( void ) { }
	virtual int DamageDecal( int bitsDamageType );
	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState( TOGGLE_STATE state ) { }
	virtual void StartSneaking( void ) { }
	virtual void StopSneaking( void ) { }
	virtual BOOL OnControls( entvars_t *pev ) { return FALSE; }
	virtual BOOL IsSneaking( void ) { return FALSE; }
	virtual BOOL IsAlive( void ) { return ( pev->deadflag == DEAD_NO ) && pev->health > 0; }
	virtual BOOL IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	virtual BOOL HasTarget( string_t targetname ) { return FStrEq( STRING( targetname ), STRING( pev->target ) ); }
	virtual BOOL IsInWorld( void );
	virtual BOOL IsPlayer( void ) { return FALSE; }
	virtual BOOL IsNetClient( void ) { return FALSE; }
	virtual const char *TeamID( void ) { return ""; }

	virtual CBaseEntity *GetNextTarget( void );

	// fundamental callbacks
	void ( CBaseEntity::*m_pfnThink )( void );
	void ( CBaseEntity::*m_pfnTouch )( CBaseEntity *pOther );
	void ( CBaseEntity::*m_pfnUse )( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void ( CBaseEntity::*m_pfnBlocked )( CBaseEntity *pOther );

	virtual void Think( void ) { if( m_pfnThink ) ( this->*m_pfnThink )(); }
	virtual void Touch( CBaseEntity *pOther ) { if( m_pfnTouch ) ( this->*m_pfnTouch )( pOther ); }
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) { if( m_pfnUse ) ( this->*m_pfnUse )( pActivator, pCaller, useType, value ); }
	virtual void Blocked( CBaseEntity *pOther ) { if( m_pfnBlocked ) ( this->*m_pfnBlocked )( pOther ); }

	// allow engine to allocate instance data
	void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return ( void * )ALLOC_PRIVATE( ENT( pev ), stAllocateBlock );
	}

	// don't use this.
#if _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete( void *pMem, entvars_t *pev )
	{
		pev->flags |= FL_KILLME;
	}
#endif

	void UpdateOnRemove( void );

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut( void );
	void EXPORT SUB_FadeOut( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int ShouldToggle( USE_TYPE useType, BOOL currentState );
	int ShouldToggle( USE_TYPE useType );
	void FireBullets( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker );
	void RifleMeleeAttack( CBasePlayer *pPlayer, BOOL bSendMsgToAll );
	void FireTWalls( float flDamage, int iBulletType, entvars_t *pevAttacker, Vector vecDir, Vector vecSrc, Vector vecRight, Vector vecUp, Vector vecDest );
	Vector FireBulletsNC( Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand );

	BOOL has_disconnected;

	virtual CBaseEntity *Respawn( void ) { return NULL; }

	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int Intersects( CBaseEntity *pOther );
	void MakeDormant( void );
	int IsDormant( void );
	BOOL IsLockedByMaster( void ) { return FALSE; }

	static CBaseEntity *Instance( edict_t *pent ) { if( !pent ) pent = ENT( 0 ); return ( CBaseEntity * )GET_PRIVATE( pent ); }
	static CBaseEntity *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntity *Instance( int eoffset ) { return Instance( ENT( eoffset ) ); }

	CBaseMonster *GetMonsterPointer( entvars_t *pevMonster ) { CBaseEntity *pEntity = Instance( pevMonster ); return pEntity ? pEntity->MyMonsterPointer() : NULL; }
	CBaseMonster *GetMonsterPointer( edict_t *pentMonster ) { CBaseEntity *pEntity = Instance( pentMonster ); return pEntity ? pEntity->MyMonsterPointer() : NULL; }

	// virtual functions used by a few classes

	// used by monsters that are created by the MonsterMaker
	virtual void UpdateOwner( void ) { }

	static CBaseEntity *Create( char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner );

	virtual BOOL FBecomeProne( void ) { return FALSE; }
	edict_t *edict( void ) { return ENT( pev ); }
	EOFFSET eoffset( void ) { return OFFSET( pev ); }
	int entindex( void ) { return ENTINDEX( edict() ); }

	virtual Vector Center( void ) { return ( pev->absmax + pev->absmin ) * 0.5; } // center point of entity
	virtual Vector EyePosition( void ) { return pev->origin + pev->view_ofs; } // position of eyes
	virtual Vector EarPosition( void ) { return pev->origin + pev->view_ofs; } // position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center(); } // position to shoot at

	virtual int Illumination( void ) { return GETENTITYILLUM( ENT( pev ) ); }

	virtual BOOL FVisible( CBaseEntity *pEntity );
	virtual BOOL FVisible( const Vector &vecOrigin );

	// we use these variables to store each ammo count
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	int ammo_12mm;
	int ammo_16mm;
	int ammo_22mm;
	int ammo_44mm;
	int ammo_55mm;
	int ammo_66mm;
	int ammo_agrens;
	int ammo_agrensex;
	int ammo_ggrens;
	int ammo_ggrensex;

	// special stuff for grenades and satchels
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	int m_fireState;

	void EXPORT DecalThink( void );
	TraceResult m_trDecalTrace;
};

// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a
// member function of a base class.  static_cast is a sleezy way around that problem.

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define ResetThink( ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (NULL)
#define ResetTouch( ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (NULL)
#define ResetUse( ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (NULL)
#define ResetBlocked( ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (NULL)

class CPointEntity : public CBaseEntity
{
public:
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
	float m_flDelay;
	int m_iszKillTarget;

	virtual void KeyValue( KeyValueData_s *pkvd );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	// common member functions
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	void EXPORT DelayThink( void );
};

class CBaseAnimating : public CBaseDelay
{
public:
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	float StudioFrameAdvance( float flInterval ); // accumulate animation frame time from last time called until now
	BOOL GetSequenceFlags( void );
	int LookupActivity( int activity );
	int LookupActivityHeaviest( int activity );
	int LookupSequence( const char *label );
	void ResetSequenceInfo( void );
	void DispatchAnimEvents( float flInterval ); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent ) { }
	float SetBoneController( int iController, float flValue );
	void InitBoneControllers( void );
	float SetBlending( int iBlender, float flValue );
	void GetBonePosition( int iBone, Vector &origin, Vector &angles );
	void GetAutomovement( Vector &origin, Vector &angles, float flInterval );
	int FindTransition( int iEndingSequence, int iGoalSequence, int *piDir );
	void GetAttachment( int iAttachment, Vector &origin, Vector &angles );
	void SetBodygroup( int iGroup, int iValue );
	int GetBodygroup( int iGroup );
	int ExtractBbox( int sequence, float *mins, float *maxs );
	void SetSequenceBox( void );

	// animation needs
	float m_flFrameRate; // computed FPS for current sequence
	float m_flGroundSpeed; // computed linear movement rate for current sequence
	float m_flLastEventCheck; // last time the event list was checked
	BOOL m_fSequenceFinished; // flag set when StudioAdvanceFrame moves across a frame boundary
	BOOL m_fSequenceLoops; // true if the sequence loops
};

//
// generic Toggle entity.
//
class CBaseToggle : public CBaseAnimating
{
public:
	virtual void KeyValue( KeyValueData_s *pkvd );

	TOGGLE_STATE m_toggle_state;
	float m_flActivateFinished; // like attack_finished, but for doors
	float m_flMoveDistance; // how far a door should slide or rotate
	float m_flWait;
	float m_flLip;
	float m_flTWidth; // for plats
	float m_flTLength; // for plats

	vec3_t m_vecPosition1;
	vec3_t m_vecPosition2;
	vec3_t m_vecAngle1;
	vec3_t m_vecAngle2;

	int m_cTriggersLeft; // trigger_counter only, # of activations remaining
	float m_flHeight;
	EHANDLE m_hActivator;
	void ( CBaseToggle::*m_pfnCallWhenMoveDone )( void );
	vec3_t m_vecFinalDest;
	float m_flLinearMoveSpeed;
	vec3_t m_vecFinalAngle;

	int m_bitsDamageInflict; // DMG_ damage type that the door or tigger does

	int m_team;

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	virtual int GetToggleState( void ) { return m_toggle_state; }
	virtual STATE GetState( void );
	virtual float GetDelay( void ) { return m_flWait; }

	// common member functions
	void LinearMove( Vector vecDest, float flSpeed );
	void LinearMoveNow( void );
	void EXPORT LinearMoveDone( void );
	void AngularMove( Vector vecDestAngle, float flSpeed );
	void EXPORT AngularMoveDone( void );
	BOOL IsLockedByMaster( void );

	static float AxisValue( int flags, const Vector &angles );
	static void AxisDir( entvars_t *pev );
	static float AxisDelta( int flags, const Vector &angle1, const Vector &angle2 );

	string_t m_sMaster; // If this button has a master switch, this is the targetname.
	// A master switch must be of the multisource type. If all
	// of the switches in the multisource have been triggered, then
	// the button will be allowed to operate. Otherwise, it will be deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)

// people gib if their health is <= this at the time of death
#define	GIB_HEALTH_VALUE	-30

#define	ROUTE_SIZE			8 // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define	bits_CAP_DUCK			( 1 << 0 )// crouch
#define	bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define	bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_SLASH			(1 << 2)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 3)	// heat burned
#define DMG_FREEZE			(1 << 4)	// frozen
#define DMG_FALL			(1 << 5)	// fell too far
#define DMG_BLAST			(1 << 6)	// explosive blast damage
#define DMG_CLUB			(1 << 7)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 8)	// electric shock
#define DMG_SONIC			(1 << 9)	// sound pulse shockwave
#define DMG_ENERGYBEAM		(1 << 10)	// laser or other high energy beam
#define DMG_NEVERGIB		(1 << 12)	// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1 << 13)	// with this bit OR'd in, any damage type can be made to gib victims upon death.
#define DMG_DROWN			(1 << 14)	// Drowning
// time-based damage
#define DMG_TIMEBASED		(~(0x3fff))	// mask for time-based damage

#define DMG_PARALYZE		(1 << 15)	// slows affected creature down
#define DMG_NERVEGAS		(1 << 16)	// nerve toxins, very bad
#define DMG_POISON			(1 << 17)	// blood poisioning
#define DMG_RADIATION		(1 << 18)	// radiation exposure
#define DMG_DROWNRECOVER	(1 << 19)	// drowning recovery
#define DMG_ACID			(1 << 20)	// toxic chemicals or acid burns
#define DMG_SLOWBURN		(1 << 21)	// in an oven
#define DMG_SLOWFREEZE		(1 << 22)	// in a subzero freezer
#define DMG_MORTAR			(1 << 23)	// Hit by air raid (done to distinguish grenade from mortar)

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// these are the damage types that have client hud art
#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION	2		// number of 2 second intervals to take damage
#define PARALYZE_DAMAGE		1.0		// damage to take each 2 second interval

#define NERVEGAS_DURATION	2
#define NERVEGAS_DAMAGE		5.0

#define POISON_DURATION		5
#define POISON_DAMAGE		2.0

#define RADIATION_DURATION	2
#define RADIATION_DAMAGE	1.0

#define ACID_DURATION		2
#define ACID_DAMAGE			5.0

#define SLOWBURN_DURATION	2
#define SLOWBURN_DAMAGE		1.0

#define SLOWFREEZE_DURATION	2
#define SLOWFREEZE_DAMAGE	1.0


#define	itbd_Paralyze		0
#define	itbd_NerveGas		1
#define	itbd_Poison			2
#define	itbd_Radiation		3
#define	itbd_DrownRecover	4
#define	itbd_Acid			5
#define	itbd_SlowBurn		6
#define	itbd_SlowFreeze		7
#define CDMG_TIMEBASED		8

// when calling KILLED(), a value that governs gib behavior is expected to be
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )

class CBaseMonster;
class CCineMonster;
class CSound;

#include "basemonster.h"

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	if( pev == NULL )
		pev = VARS( CREATE_ENTITY() );

	// get the private data
	a = (T *)GET_PRIVATE( ENT( pev ) );

	if( a == NULL )
	{
		// allocate private data
		a = new( pev ) T;
		a->pev = pev;
	}
	return a;
}

#define TRACER_FREQ		4			// Tracers fire every 4 bullets
#endif
