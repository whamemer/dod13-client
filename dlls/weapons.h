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
#if !defined(WEAPONS_H)
#define WEAPONS_H

#include "effects.h"
#include "hud.h"

class CBasePlayer;
extern int gmsgWeapPickup;	

void DeactivateSatchels( CBasePlayer *pOwner );

typedef enum
{
	G_MILLSGRENADE = 0,
	G_HANDGRENADE,
	G_STICKGRENADE
} grenType;

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	grenType	g_GrenType;
	int			m_flClipTime;
	int			m_iOldMoveType;
	int			m_iBodyGibs;
	BOOL		m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.

	void Spawn( void );

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade* ShootPickup( entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time, grenType value, float f );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	virtual void Explode( TraceResult *pTrace, int bitsDamageType );
	void EXPORT Smoke( void );

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT RocketExplodeTouch( CBaseEntity *pOther );
	void EXPORT MortarShellExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PickupUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );
	void EXPORT RemoveMe( void );

	virtual void Precache( void );
	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );
};

#define WEAPON_NONE				0
#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))
#define WEAPON_SUIT				63
#define WEAPON_NOWEIGHT			-1
#define WEAPON_NOCARRY			0
#define WEAPON_NOCLIP			-1
#define WEAPON_DEFAULT_GIVE		0
#define AMMO_DEFAULT_GIVE		0

// bullet types in cl_dll/dod_shared.h

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon
#define ITEM_FLAG_NOAUTOSWITCHTO	32

#define ITEM_FLAG_GRENADE			24
#define ITEM_FLAG_PISTOL			64
#define ITEM_FLAG_RIFLE				128
#define ITEM_FLAG_66MM_BIPOD		130
#define ITEM_FLAG_ROCKET			642
#define ITEM_FLAG_66MM_GER_BIPOD	2178

#define WPNSTATE_SCOPED				(1<<0)
#define WPNSTATE_ROCKET_SLOW		(2<<0)

#define WEAPON_IS_ONTARGET 0x40

// for guns Classify
#define	CLASS_NONE 				0
#define CLASS_MELEE				1
#define CLASS_GRENADE			2
#define CLASS_GRENADE_EX		4
#define CLASS_PISTOL			8
#define CLASS_RIFLE				16	// CM1Carbine, CGarand, CKAR, CK43, CENFIELD
#define CLASS_SCOPE_RIFLE		32	// CScopedKar, CSPRING, CFG42, CENFIELD
#define CLASS_AUTO_RIFLE		64	// CThompson, CMP44, CMP40, CGreaseGun
#define CLASS_MACHINEGUNS		256 // CBAR, CMG42, CMG34, C30CAL, CBREN, CFG42
#define CLASS_ROCKET			512

typedef struct
{
	int			iSlot;
	int			iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int			iMaxAmmo1;	// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int			iMaxAmmo2;	// max ammo 2
	const char	*pszName;
	int			iMaxClip;
	int			iId;
	int			iFlags;
	int			iWeight;	// this value used to determine this weapon's importance in autoselection.
	int			iBulletId;
	float		flSpread;
	float		flRecoil;
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void EXPORT PickupUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void EXPORT FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize( void );// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn ( void );// copy a weapon
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo( ItemInfo* p ) { return 0; }	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; }
	virtual BOOL Deploy( void ) { return TRUE; }

	virtual BOOL SpawnDeploy( void ) { return TRUE; }
	virtual int ObjectCaps( void ) { return 10; }
	virtual BOOL CanDrop( void ) { return CBasePlayerItem::CanHolster(); }
	virtual void SetDmgTime( float time ) { return; }
	virtual void DropGren( void ) { return; }

	virtual BOOL CanHolster( void ) { return TRUE; }	// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; }

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void SpawnReplacement( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; }
	virtual void RoundRespawn( void ) { return; }

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;										// WEAPON_???
	BOOL m_bReadyToRespawn;
	Vector m_vRespawnOrigin;
	Vector m_vRespawnAngles;
	BOOL m_bTriggerSpawnOnDie;

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};

// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {};	// updates HUD state

	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
							// attack key(s)
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations

	void PostMortarValue( float value ) { return; }
	void SendMortarFireCommand( char *c ) { return; }

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	virtual Vector Aim( float accuracyFactor, CBasePlayer *pOther, unsigned int shared_rand );
	virtual float flAim( float accuracyFactor, CBasePlayer *pOther ) { return accuracyFactor; }
	virtual void RemoveStamina( float removeAmount, CBasePlayer *pother ) { return; }
	virtual int ChangeFOV( int fov );
	virtual int ZoomOut( void );
	virtual int ZoomIn( void );
	virtual int GetFOV( void ) { return (int)g_lastFOV; }
	virtual bool PlayerIsWaterSniping( void );
	void ThinkZoomOut( void );
	void ThinkZoomIn( void );
	virtual void UpdateZoomSpeed( void ) { return; }

	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void Special( void ) { return; }
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };

	int	PrimaryAmmoIndex(); 
	int	SecondaryAmmoIndex(); 

	void PrintState( void );

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; };
	BOOL TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExtm, float idleTime,
					float attackTime, int skiplocal = 0 );
	virtual int Classify( void ) { return 0; }
	int GetRoundState( void ) { return gHUD.m_iRoundState; }
	float GetNextAttackDelay( float delay );

	float	m_flPumpTime;
	int		m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float	m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										// the last version of m_iClip sent to hud dll
	int		m_iClientWeaponState;								// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int		m_fInReload;										// Are we in the middle of a reload;

	int		m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.
	int		m_fInReload;
	int		m_iDefaultAmmo;
	int		m_fInAttack;
	int		*current_ammo;
	int		m_iWeaponState;
	float	m_flWeaponHeat;

	// hle time creep vars
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
	bool GiveGenericAmmo( CBaseEntity *pOther );
};

extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	int  GiveAmmo( int iCount, const char *szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );

public:
	void EXPORT Kill ( void );
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );

	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	string_t m_rgiszAmmo[MAX_AMMO_SLOTS];// ammo names
	int	m_rgAmmo[MAX_AMMO_SLOTS];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#if CLIENT_DLL
bool bIsMultiplayer ( void );
void LoadVModel ( const char *szViewModel, CBasePlayer *m_pPlayer );
#endif

/*
Guns Class Hierachy

CBasePlayerItem
	CBasePlayerWeapon
		CMeleeWeapon
			CAmerKnife
			CGerKnife
			CSpade
		CPistol
			CCOLT
			CLUGER
			CWEBLEY
		CBipodWeapon
			CBAR
			CMG42
			CMG34
			C30CAL
			CBREN
		CRocketWeapon
			CBazooka
			CPschreck
			CPIAT
		CDoDGrenade
			CHandGrenade
			CStickGrenade
		CDoDGrenadeEx
			CHandGrenadeEx
			CStickGrenadeEx
		CM1Carbine
		CGarand
		CScopedKar
		CThompson
		CMP44
		CSPRING
		CKAR
		CMP40
		CFG42
		CK43
		CENFIELD
		CSTEN
*/

class CMeleeWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( int weapon_id );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( void );
	void Holster( int skiplocal );
	void PrimaryAttack( void );
	void Smack( void );
	void SwingAgain( void );
	int Swing( int fFirst );
	int Stab( int fFirst );
	void WeaponIdle( void );

	BOOL CanDrop( void ) { return FALSE; }
	int Classify( void ) { return CLASS_MELEE; }
	int GetDrawAnim( void ) { return 0; }
	int GetIdleAnim( void ) { return 0; }
	int GetSlashAnim( void ) { return 0; }
	void SecondaryAttack( void ) { return; }
	int iItemSlot( void ) { return 0; }

	int m_iSwing;
	TraceResult m_trHit;

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_iFireEvent;
};

class CPistol : public CBasePlayerWeapon
{
public:
	void Spawn( int weapon_id );
	void PrimaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void WeaponIdle( void );
	void Reload( void );

	void SecondaryAttack( void ) { return; }
	int iItemSlot( void ) { return 1; }
	int Classify( void ) { return CLASS_PISTOL; }
	BOOL CanDrop( void ) { return FALSE; }
	int GetIdleAnim( void ) { return 0; }
	int GetDrawAnim( void ) { return 0; }
	int GetReloadAnim( void ) { return 0; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_iFireEvent;
};

class CBipodWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( int weapon_id );
	void Precache( void );
	void PrimaryAttack( void );
	void CoolThink( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal );
	bool IsDeployed( void );
	float GetBipodSpread( void );
	void ForceUndeploy( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanDrop( void ) { return CanHolster(); }
	int GetIdleAnim( void ) { return 0; }
	int GetDrawAnim( void ) { return 0; }
	int GetReloadAnim( void ) { return 0; }
	int GetUpToDownAnim( void ) { return 0; }
	int GetDownToUpAnim( void ) { return 0; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_iFireEvent;
	unsigned short m_iOverheatEvent;
};

class CRocketWeapon : public CBasePlayerWeapon
{
public:
	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_ROCKET; }
	void ReSlow( void );

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CDoDGrenade : public CBasePlayerWeapon
{
public:
	int iItemSlot( void ) { return 4; }
	int Classify( void ) { return CLASS_GRENADE; }
	BOOL CanDrop( void ) { return FALSE; }
	BOOL ShouldWeaponIdle( void ) { return FALSE; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	float m_flStartThrow;
	float m_flReleaseThrow;
};

class CDoDGrenadeEx : public CBasePlayerWeapon
{
public:
	int Classify( void ) { return CLASS_GRENADE_EX; }
};

class CHandGrenade : public CDoDGrenade
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal );
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );

private:
	vec3_t angThrow, vecSrc, vecThrow;
	float flVel;
	BOOL m_bUnderhand;
};

class CHandGrenadeEx : public CDoDGrenadeEx
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	BOOL Deploy( void );
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );

	BOOL CanHolster( void ) { return FALSE; }
	void Holster( int skiplocal ) { ; }
	void SetDmgTime( float time ) { time = m_flTimeToExplode; }

private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;
};

class CStickGrenade : public CDoDGrenade
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL underhand );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal );
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );

private:
	vec3_t angThrow, vecSrc, vecThrow;
	float flVel, m_flLeaveHand;
	BOOL m_bUnderhand;
};

class CStickGrenadeEx : public CDoDGrenadeEx
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	BOOL Deploy( void );
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );

	BOOL CanHolster( void ) { return FALSE; }
	void Holster( int skiplocal ) { ; }
	void SetDmgTime( float time ) { time = m_flTimeToExplode; }

private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;
};

class CAmerKnife : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
};

class CGerKnife : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
};

class CSpade : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
};

class CCOLT : public CPistol
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetReloadAnim( void );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
};

class CLUGER : public CPistol
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	void SecondaryAttack( void ) { ; }
	BOOL CanHolster( void ) { return TRUE; }

private:
	unsigned short m_usFireLuger;
};

class CWEBLEY : public CPistol
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	void SecondaryAttack( void ) { ; }
	BOOL CanHolster( void ) { return TRUE; }

private:
	unsigned short m_usFireWebley;
};

class CM1Carbine : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL SpawnDeploy( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireM1Carbine;
};

class CGarand : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireGarand;
};

class CScopedKar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void ThinkZoomOutIn( void );
	void Reload( void );
	void Holster( int skiplocal );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_SCOPE_RIFLE; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireScopedKar;
};

class CThompson : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireThompson;
};

class CMP44 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireMP44;
};

class CSPRING : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void ThinkZoomOutIn( void );
	void ThinkZoomOut( void );
	void Holster( int skiplocal );
	void Reload( void );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_SCOPE_RIFLE; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireSpring;
};

class CKAR : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireKar;
};

class CMP40 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireMP40;
};

class CBAR : public CBipodWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetIdleAnim( void );
	int GetDrawAnim( void );
	int GetReloadAnim( void );
	int GetUpToDownAnim( void );
	int GetDownToUpAnim( void );

	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_MACHINEGUNS; }

private:
	unsigned short m_usFireBAR;
};

class CMG42 : public CBipodWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetIdleAnim( void );
	int GetDrawAnim( void );
	int GetReloadAnim( void );
	int GetUpToDownAnim( void );
	int GetDownToUpAnim( void );

	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_MACHINEGUNS; }

private:
	unsigned short m_usFireMG42;
};

class CMG34 : public CBipodWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetIdleAnim( void );
	int GetDrawAnim( void );
	int GetReloadAnim( void );
	int GetUpToDownAnim( void );
	int GetDownToUpAnim( void );

	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_MACHINEGUNS; }

private:
	unsigned short m_usFireMG34;
};

class C30CAL : public CBipodWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetIdleAnim( void );
	int GetDrawAnim( void );
	int GetReloadAnim( void );
	int GetUpToDownAnim( void );
	int GetDownToUpAnim( void );

	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_MACHINEGUNS; }

private:
	unsigned short m_usFire30CAL;
};

class CBREN : public CBipodWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetIdleAnim( void );
	int GetDrawAnim( void );
	int GetReloadAnim( void );
	int GetUpToDownAnim( void );
	int GetDownToUpAnim( void );

	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_MACHINEGUNS; }

private:
	unsigned short m_usFireBren;
};

class CFG42 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal );
	BOOL CanHolster( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	BOOL IsDeployed( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireFG42;
};

class CK43 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireK43;
};

class CENFIELD : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void ThinkZoomOutIn( void );
	void Holster( int skiplocal );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usFireEnfield, m_usFireScopedEnfield;
};

class CSTEN : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	void SecondaryAttack( void ) { ; }
	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_AUTO_RIFLE; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireSten;
};

class CGreaseGun : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	BOOL Deploy( void );
	BOOL SpawnDeploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );

	int iItemSlot( void ) { return 2; }

	virtual BOOL UseDecrement( void )
	{
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireGreaseGun;
};

class CBazooka : public CRocketWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal );
	void Reload( void );
	void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	void ReSlow( void );

private:
	unsigned short m_usFireBazooka;
};

class CPschreck : public CRocketWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal );
	void Reload( void );
	void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	void ReSlow( void );

private:
	unsigned short m_usFirePschreck;
};

class CPIAT : public CRocketWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal );
	void Reload( void );
	void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	void ReSlow( void );

private:
	unsigned short m_usFirePIAT;
};

#endif // WEAPONS_H