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

#include "dod_shared.h"

class CBasePlayer;

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity );

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
	typedef enum
	{
		SATCHEL_DETONATE = 0,
		SATCHEL_RELEASE
	} SATCHELCODE;

	grenType g_GrenType;
	int m_flClipTime;
	int m_iOldMoveType;
	int m_iBodyGibs;
	BOOL m_fRegisteredSound;

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual int BloodColor( void ) { return DONT_BLEED; }
	virtual void BounceSound( void );

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootPickup( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, grenType value, float f );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );

	void EXPORT Smoke( void );
	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT RocketExplodeTouch( CBaseEntity *pOther );
	void EXPORT MortarShellExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT TumbleThink( void );
	void EXPORT RemoveMe( void );

	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PickupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

// weapon id in cl_dll/dod_shared.h

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))
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

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
	int		iBulletId;
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
	CBasePlayer *m_pPlayer;
	CBasePlayerItem *m_pNext;
	int m_iId;
	BOOL m_bReadyToRespawn;
	vec3_t m_vRespawnOrigin;
	vec3_t m_vRespawnAngles;
	BOOL m_bTriggerSpawnOnDie;

	static TYPEDESCRIPTION m_SaveData[];
	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

	virtual void SetObjectCollisionBox( void );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	virtual CBaseEntity *Respawn( void );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE; }
	virtual void RoundRespawn( void );

	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }
	virtual int GetItemInfo( ItemInfo *p ) { return 0; }
	virtual BOOL CanDeploy( void ) { return TRUE; }
	virtual BOOL Deploy( void ) { return TRUE; }
	virtual BOOL SpawnDeploy( void ) { return TRUE; }
	virtual BOOL CanDrop( void ) { return CanHolster(); }
	virtual void SetDmgTime( float time ) { }
	virtual void DropGren( void ) { }
	virtual BOOL CanHolster( void ) { return TRUE; }
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { }
	virtual void ItemPreFrame( void ) { }
	virtual void ItemPostFrame( void ) { }
	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer( CBasePlayer *pPlayer );
	virtual int PrimaryAmmoIndex( void ) { return -1; }
	virtual int SecondaryAmmoIndex( void ) { return -1; }
	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }
	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; }
	virtual int iItemSlot( void ) { return 0; }

	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );
	void EXPORT PickupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FallThink( void );
	void EXPORT Materialize( void );
	void EXPORT AttemptToMaterialize( void );

	void FallInit( void );
	void CheckRespawn( void );
	void SpawnReplacement( void );
	int iItemPosition( void ) { return ItemInfoArray[m_iId].iPosition; }
	const char *pszAmmo1( void ) { return ItemInfoArray[m_iId].pszAmmo1; }
	int iMaxAmmo1( void ) { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char *pszAmmo2( void ) { return ItemInfoArray[m_iId].pszAmmo2; }
	int iMaxAmmo2( void ) { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char *pszName( void ) { return ItemInfoArray[m_iId].pszName; }
	int iMaxClip( void ) { return ItemInfoArray[m_iId].iMaxClip; }
	int iWeight( void ) { return ItemInfoArray[m_iId].iWeight; }
	int iFlags( void ) { return ItemInfoArray[m_iId].iFlags; }
};

// inventory items that
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	int m_iPlayEmptySound;
	int m_fFireOnEmpty;
	int m_fInSpecialReload;
	float m_flNextPrimaryAttack;
	float m_flNextSecondaryAttack;
	float m_flTimeWeaponIdle;
	int m_iPrimaryAmmoType;
	int m_iSecondaryAmmoType;
	int m_iClip;
	int m_iClientClip;
	int m_iClientWeaponState;
	int m_fInReload;
	int m_iDefaultAmmo;
	int m_fInAttack;
	int *current_ammo;
	int m_iWeaponState;
	float m_flWeaponHeat;

	static TYPEDESCRIPTION m_SaveData[];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pOriginal );
	virtual void UpdateItemInfo( void ) { }
	virtual BOOL CanDeploy( void );
	virtual void ItemPostFrame( void );
	virtual void Holster( int skiplocal );
	virtual int PrimaryAmmoIndex( void );
	virtual int SecondaryAmmoIndex( void );
	virtual CBasePlayerItem *GetWeaponPtr( void ) { return this; }
	virtual int UpdateClientData( CBasePlayer *pPlayer );
	virtual int Classify( void );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon );
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );
	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; }
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );
	virtual void SendWeaponAnim( int iAnim, int skiplocal );
	virtual BOOL IsUseable( void );

	virtual Vector Aim( float accuracyFactor, CBasePlayer *pOther, int shared_rand );
	virtual float flAim( float accuracyFactor, CBasePlayer *pOther );
	virtual void RemoveStamina( float removeAmount, CBasePlayer *pOther );
	virtual int ChangeFOV( int fov );
	virtual int ZoomOut( void );
	virtual int ZoomIn( void );
	virtual int GetFOV( void );
	virtual bool PlayerIsWaterSniping( void );
	virtual void UpdateZoomSpeed( void );

	virtual void PrimaryAttack( void ) { }
	virtual void SecondaryAttack( void ) { }
	virtual void Reload( void ) { }
	virtual void Special( void ) { }
	virtual void WeaponIdle( void ) { }
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) { return TRUE; }
	virtual BOOL UseDecrement( void ) { return FALSE; }

	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMax );
	void PostMortarValue( float value );
	void SendMortarFireCommand( char *c );
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, int skiplocal );
	int DefaultReload( int iClipSize, int iAnim, float fDelay );
	void EXPORT ThinkZoomOut( void );
	void EXPORT ThinkZoomIn( void );
	void PrintState( void );
	BOOL TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal );
	int GetRoundState( void );
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	virtual CBaseEntity *Respawn( void );
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; }

	void EXPORT DefaultTouch( CBaseEntity *pOther );
	void EXPORT Materialize( void );
	static bool GiveGenericAmmo( CBaseEntity *pOther );
};

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
		CGreaseGun
		CFG42
		CK43
		CENFIELD
		CSTEN
*/

class CMeleeWeapon : public CBasePlayerWeapon
{
protected:
	int m_iSwing;
	TraceResult m_trHit;
	int m_iFireEvent;

public:
	void Spawn( int weapon_id );
	virtual void Precache( void );
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int iItemSlot( void ) { return 0; }

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void ) { }
	virtual BOOL Deploy( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );

	virtual int Classify( void ) { return CLASS_MELEE; }
	virtual BOOL CanDrop( void ) { return FALSE; }
	virtual BOOL UseDecrement( void ) { return TRUE; }

	virtual int GetIdleAnim( void ) { return 0; }
	virtual int GetDrawAnim( void ) { return 0; }
	virtual int GetSlashAnim( int iSwingType ) { return 0; }

	void EXPORT Smack( void );
	void EXPORT SwingAgain( void );
	int Swing( int fFirst );
	int Stab( int fFirst );
};

class CPistol : public CBasePlayerWeapon
{
protected:
	int m_iFireEvent;

public:
	void Spawn( int weapon_id );
	virtual int iItemSlot( void ) { return 1; }

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void ) { }
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void ) { return CLASS_PISTOL; }
	virtual BOOL CanDrop( void ) { return FALSE; }
	virtual BOOL UseDecrement( void ) { return TRUE; }

	virtual int GetIdleAnim( void ) { return 0; }
	virtual int GetDrawAnim( void ) { return 0; }
	virtual int GetReloadAnim( void ) { return 0; }
};

class CBipodWeapon : public CBasePlayerWeapon
{
protected:
	int m_iFireEvent;
	int m_iOverheatEvent;

public:
	void Spawn( int weapon_id );
	virtual void Precache( void );
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int iItemSlot( void ) { return 2; }

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual BOOL CanDrop( void ) { return CanHolster(); }
	virtual BOOL UseDecrement( void ) { return TRUE; }

	virtual int GetIdleAnim( void ) { return 0; }
	virtual int GetDrawAnim( void ) { return 0; }
	virtual int GetReloadAnim( void ) { return 0; }
	virtual int GetUpToDownAnim( void ) { return 0; }
	virtual int GetDownToUpAnim( void ) { return 0; }

	void ForceUndeploy( void );
	float GetBipodSpread( void );
	bool IsDeployed( void );
	void EXPORT CoolThink( void );
};

class CRocketWeapon : public CBasePlayerWeapon
{
public:
	virtual int iItemSlot( void ) { return 2; }
	virtual int Classify( void ) { return CLASS_ROCKET; }
	virtual BOOL UseDecrement( void ) { return TRUE; }

	virtual void ReSlow( void ) { }
};

class CDoDGrenade : public CBasePlayerWeapon
{
protected:
	float m_flStartThrow;
	float m_flReleaseThrow;

public:
	virtual int iItemSlot( void ) { return 4; }
	virtual int Classify( void ) { return CLASS_GRENADE; }
	virtual BOOL CanDrop( void ) { return FALSE; }
	virtual BOOL ShouldWeaponIdle( void ) { return FALSE; }
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CDoDGrenadeEx : public CDoDGrenade
{
public:
	virtual int Classify( void ) { return CLASS_GRENADE_EX; }
};

class CHandGrenade : public CDoDGrenade
{
private:
	vec3_t angThrow;
	vec3_t vecSrc;
	vec3_t vecThrow;
	float flVel;
	BOOL m_bUnderhand;

public:
	virtual void Spawn( void );
	virtual int GetItemInfo( ItemInfo *p );
	virtual void Precache( void );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );
	virtual void DropGren( void );
	virtual BOOL CanDeploy( void );
};

class CHandGrenadeEx : public CDoDGrenadeEx
{
private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );
	virtual BOOL CanDeploy( void );
	virtual void DropGren( void );
	virtual void SetDmgTime( float time );
};

class CStickGrenade : public CDoDGrenade
{
private:
	vec3_t angThrow;
	vec3_t vecSrc;
	vec3_t vecThrow;
	float flVel;
	float m_flLeaveHand;
	BOOL m_bUnderhand;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );
	virtual void DropGren( void );
	virtual BOOL CanDeploy( void );
};

class CStickGrenadeEx : public CDoDGrenadeEx
{
private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );
	virtual void DropGren( void );
	virtual void SetDmgTime( float time );
	virtual BOOL CanDeploy( void );
};

class CAmerKnife : public CMeleeWeapon
{
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual int GetSlashAnim( int iSwingType );
	virtual int GetDrawAnim( void );
	virtual int GetIdleAnim( void );
};

class CGerKnife : public CMeleeWeapon
{
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual int GetSlashAnim( int iSwingType );
	virtual int GetDrawAnim( void );
	virtual int GetIdleAnim( void );
};

class CSpade : public CMeleeWeapon
{
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual int GetSlashAnim( int iSwingType );
	virtual int GetDrawAnim( void );
	virtual int GetIdleAnim( void );
};

class CCOLT : public CPistol
{
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual int GetReloadAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetIdleAnim( void );
};

class CLUGER : public CPistol
{
private:
	unsigned short m_usFireLuger;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );
};

class CWEBLEY : public CPistol
{
private:
	unsigned short m_usFireWebley;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );
};

class CM1Carbine : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireM1Carbine;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL SpawnDeploy( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CGarand : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireGarand;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CScopedKar : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireScopedKar;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	void EXPORT ThinkZoomOutIn( void );
	virtual void Reload( void );
	virtual void Holster( int skiplocal );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CThompson : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireThompson;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CMP44 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireMP44;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CSPRING : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireSpring;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	void EXPORT ThinkZoomOutIn( void );
	void EXPORT ThinkZoomOut( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CKAR : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireKar;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CBAR : public CBipodWeapon
{
private:
	unsigned short m_usFireBAR;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );

	virtual int Classify( void );
	virtual int GetIdleAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetReloadAnim( void );
	virtual int GetUpToDownAnim( void );
	virtual int GetDownToUpAnim( void );
};

class CMP40 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireMP40;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CMG42 : public CBipodWeapon
{
private:
	unsigned short m_usFireMG42;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );

	virtual int Classify( void );
	virtual int GetIdleAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetReloadAnim( void );
	virtual int GetUpToDownAnim( void );
	virtual int GetDownToUpAnim( void );
};

class CMG34 : public CBipodWeapon
{
private:
	unsigned short m_usFireMG34;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );

	virtual int Classify( void );
	virtual int GetIdleAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetReloadAnim( void );
	virtual int GetUpToDownAnim( void );
	virtual int GetDownToUpAnim( void );
};

class C30CAL : public CBipodWeapon
{
private:
	unsigned short m_usFire30CAL;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );

	virtual int Classify( void );
	virtual int GetIdleAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetReloadAnim( void );
	virtual int GetUpToDownAnim( void );
	virtual int GetDownToUpAnim( void );
};

class CGreaseGun : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireGreaseGun;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL SpawnDeploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CFG42 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireFG42;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void Holster( int skiplocal );
	virtual BOOL CanHolster( void );
	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
	BOOL IsDeployed( void );
};

class CK43 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireK43;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CENFIELD : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireEnfield;
	unsigned short m_usFireScopedEnfield;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	void EXPORT ThinkZoomOutIn( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CSTEN : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireSten;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );
	virtual int AddToPlayer( CBasePlayer *pPlayer );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Reload( void );
	virtual void WeaponIdle( void );

	virtual int Classify( void );
	virtual BOOL UseDecrement( void ) { return TRUE; }
};

class CBREN : public CBipodWeapon
{
private:
	unsigned short m_usFireBren;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int iItemSlot( void ) { return 2; }
	virtual int GetItemInfo( ItemInfo *p );

	virtual int Classify( void );
	virtual int GetIdleAnim( void );
	virtual int GetDrawAnim( void );
	virtual int GetReloadAnim( void );
	virtual int GetUpToDownAnim( void );
	virtual int GetDownToUpAnim( void );
};

class CBazooka : public CRocketWeapon
{
private:
	unsigned short m_usFireBazooka;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	virtual void ReSlow( void );
};

class CPschreck : public CRocketWeapon
{
private:
	unsigned short m_usFirePschreck;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	virtual void ReSlow( void );
};

class CPIAT : public CRocketWeapon
{
private:
	unsigned short m_usFirePIAT;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int GetItemInfo( ItemInfo *p );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual BOOL Deploy( void );
	virtual BOOL CanHolster( void );
	virtual void Holster( int skiplocal );
	virtual void Reload( void );
	virtual void WeaponIdle( void );
	void Raise( void );
	void Lower( void );
	void UnSlow( void );
	virtual void ReSlow( void );
};
#endif // WEAPONS_H
