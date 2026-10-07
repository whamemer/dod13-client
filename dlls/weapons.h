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
#include "dod_shared.h"

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
	BOOL		m_fRegisteredSound;

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	virtual void Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual int  ObjectCaps( void ) override;
	virtual void Killed( entvars_t *pevAttacker, int iGib ) override;
	virtual int  BloodColor( void ) override;
	virtual void BounceSound( void );

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootPickup( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, grenType value, float f );

	void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );
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

#define WEAPON_IS_ONTARGET 0x40

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
	CBasePlayer		*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int              m_iId;
	BOOL             m_bReadyToRespawn;
	Vector           m_vRespawnOrigin;
	Vector           m_vRespawnAngles;
	BOOL             m_bTriggerSpawnOnDie;

	static TYPEDESCRIPTION m_SaveData[];
	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

	virtual void SetObjectCollisionBox( void ) override;
	virtual int  Save( CSave &save ) override;
	virtual int  Restore( CRestore &restore ) override;
	virtual CBaseEntity *Respawn( void ) override;
	virtual int  ObjectCaps( void ) override { return 10; }
	virtual void RoundRespawn( void ) override { return; }

	virtual int  AddToPlayer( CBasePlayer *pPlayer );
	virtual int  AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }
	virtual int  GetItemInfo( ItemInfo *p ) { return 0; }
	virtual BOOL CanDeploy( void ) { return TRUE; }
	virtual BOOL Deploy( void ) { return TRUE; }
	virtual BOOL SpawnDeploy( void ) { return TRUE; }
	virtual BOOL CanDrop( void ) { return CBasePlayerItem::CanHolster(); }
	virtual void SetDmgTime( float time ) { return; }
	virtual void DropGren( void ) { return; }
	virtual BOOL CanHolster( void ) { return TRUE; }
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; }
	virtual void ItemPreFrame( void ) { return; }
	virtual void ItemPostFrame( void ) { return; }
	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer( CBasePlayer *pPlayer );
	virtual int  PrimaryAmmoIndex( void ) { return -1; }
	virtual int  SecondaryAmmoIndex( void ) { return -1; }
	virtual int  UpdateClientData( CBasePlayer *pPlayer ) { return 0; }
	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; }
	virtual int  iItemSlot( void ) { return 0; }

	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );
	void EXPORT PickupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FallThink( void );
	void EXPORT Materialize( void );
	void EXPORT AttemptToMaterialize( void );

	void FallInit( void );
	void CheckRespawn( void );
	void SpawnReplacement( void ) { return; }
	int  iItemPosition( void ) { return ItemInfoArray[m_iId].iPosition; }
	const char *pszAmmo1( void ) { return ItemInfoArray[m_iId].pszAmmo1; }
	int  iMaxAmmo1( void ) { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char *pszAmmo2( void ) { return ItemInfoArray[m_iId].pszAmmo2; }
	int  iMaxAmmo2( void ) { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char *pszName( void ) { return ItemInfoArray[m_iId].pszName; }
	int  iMaxClip( void ) { return ItemInfoArray[m_iId].iMaxClip; }
	int  iWeight( void ) { return ItemInfoArray[m_iId].iWeight; }
	int  iFlags( void ) { return ItemInfoArray[m_iId].iFlags; }
};

// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	int   m_iPlayEmptySound;
	int   m_fFireOnEmpty;
	int   m_fInSpecialReload;
	float m_flNextPrimaryAttack;
	float m_flNextSecondaryAttack;
	float m_flTimeWeaponIdle;
	int   m_iPrimaryAmmoType;
	int   m_iSecondaryAmmoType;
	int   m_iClip;
	int   m_iClientClip;
	int   m_iClientWeaponState;
	int   m_fInReload;
	int   m_iDefaultAmmo;
	int   m_fInAttack;
	int   *current_ammo;
	int   m_iWeaponState;
	float m_flWeaponHeat;

	static TYPEDESCRIPTION m_SaveData[];

	virtual int  Save( CSave &save ) override;
	virtual int  Restore( CRestore &restore ) override;
	virtual int  AddToPlayer( CBasePlayer *pPlayer ) override;
	virtual int  AddDuplicate( CBasePlayerItem *pItem ) override;
	virtual void UpdateItemInfo( void ) override {};
	virtual BOOL CanDeploy( void ) override;
	virtual void ItemPostFrame( void ) override;
	virtual void Holster( int skiplocal = 0 ) override;
	virtual int  PrimaryAmmoIndex( void ) override;
	virtual int  SecondaryAmmoIndex( void ) override;
	virtual CBasePlayerItem *GetWeaponPtr( void ) override { return ( CBasePlayerItem * ) this; };

	virtual int  ExtractAmmo( CBasePlayerWeapon *pWeapon );
	virtual int  ExtractClipAmmo( CBasePlayerWeapon *pWeapon );
	virtual int  AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );
	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );
	virtual BOOL IsUseable( void );

	virtual Vector Aim( float accuracyFactor, CBasePlayer *pOther, unsigned int shared_rand );
	virtual float  flAim( float accuracyFactor, CBasePlayer *pOther );
	virtual void   RemoveStamina( float removeAmount, CBasePlayer *pother ) { return; }
	virtual int    ChangeFOV( int fov );
	virtual int    ZoomOut( void );
	virtual int    ZoomIn( void );
	virtual int    GetFOV( void );
	virtual bool   PlayerIsWaterSniping( void );
	virtual void   UpdateZoomSpeed( void ) { return; }

	virtual void PrimaryAttack( void ) { return; }
	virtual void SecondaryAttack( void ) { return; }
	virtual void Reload( void ) { return; }
	virtual void Special( void ) { return; }
	virtual void WeaponIdle( void ) { return; }
	virtual int  UpdateClientData( CBasePlayer *pPlayer );
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) { return FALSE; };
	virtual BOOL UseDecrement( void ) { return FALSE; };
	virtual int  Classify( void ) { return 0; }

	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );
	void PostMortarValue( float value );
	void SendMortarFireCommand( char *c );
	BOOL DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, const char *szAnimReloadExt, int skiplocal = 0, int body = 0 );
	int  DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );
	void ThinkZoomOut( void );
	void ThinkZoomIn( void );
	void PrintState( void );
	BOOL TimedDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, const char *szAnimReloadExtm, float idleTime, float attackTime, int skiplocal = 0 );
	float GetNextAttackDelay( float delay );
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void ) override;
	virtual CBaseEntity *Respawn( void ) override;
	virtual BOOL AddAmmo( CBaseEntity *pOther );

	void EXPORT DefaultTouch( CBaseEntity *pOther );
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

extern void ClearMultiDamage( void );
extern void ApplyMultiDamage( entvars_t *pevInflictor, entvars_t *pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType );

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void SpawnBlood( Vector vecSpot, int bloodColor, float flDamage );
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

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

bool bIsMultiplayer ( void );
void LoadVModel ( const char *szViewModel, CBasePlayer *m_pPlayer );

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
protected:
	int m_iSwing;
	TraceResult m_trHit;
	int m_iFireEvent;

public:
	void Spawn( int weapon_id );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int iItemSlot( void ) { return 0; }
	int Classify( void ) { return CLASS_MELEE; }
	BOOL CanDrop( void ) { return FALSE; }
	void PrimaryAttack( void );
	void SecondaryAttack( void ) { return; }
	void WeaponIdle( void );
	BOOL UseDecrement( void ) { return TRUE; }

	virtual int GetIdleAnim( void ) { return 0; }
	virtual int GetDrawAnim( void ) { return 0; }
	virtual int GetSlashAnim( int iSwingType ) { return 0; }

	void Smack( void );
	void SwingAgain( void );
	int Swing( int fFirst );
	int Stab( int fFirst );
};

class CPistol : public CBasePlayerWeapon
{
protected:
	int m_iFireEvent;

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
	BOOL UseDecrement( void ) { return TRUE; }

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
	void Precache( void );
	void PrimaryAttack( void );
	void CoolThink( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal = 0 );
	bool IsDeployed( void );
	float GetBipodSpread( void );
	void ForceUndeploy( void );
	int iItemSlot( void ) { return 2; }
	BOOL CanDrop( void ) { return CanHolster(); }
	BOOL UseDecrement( void ) { return TRUE; }

	virtual int GetIdleAnim( void ) { return 0; }
	virtual int GetDrawAnim( void ) { return 0; }
	virtual int GetReloadAnim( void ) { return 0; }
	virtual int GetUpToDownAnim( void ) { return 0; }
	virtual int GetDownToUpAnim( void ) { return 0; }
};

class CRocketWeapon : public CBasePlayerWeapon
{
public:
	int iItemSlot( void ) { return 2; }
	int Classify( void ) { return CLASS_ROCKET; }
	BOOL UseDecrement( void ) { return TRUE; }

	virtual void ReSlow( void ) { ; }
};

class CDoDGrenade : public CBasePlayerWeapon
{
protected:
	float m_flStartThrow;
	float m_flReleaseThrow;

public:
	int iItemSlot( void ) { return 4; }
	int Classify( void ) { return CLASS_GRENADE; }
	BOOL CanDrop( void ) { return FALSE; }
	BOOL ShouldWeaponIdle( void ) { return TRUE; }
	BOOL UseDecrement( void ) { return TRUE; }
};

class CDoDGrenadeEx : public CDoDGrenade
{
public:
	int Classify( void ) { return CLASS_GRENADE_EX; }
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
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );
};

class CHandGrenadeEx : public CDoDGrenadeEx
{
private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;

public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void StartThrow( BOOL bUnderhand );
	BOOL Deploy( void );
	BOOL CanHolster( void ) { return FALSE; }
	void Holster( int skiplocal = 0 ) { /*Nothing*/ }
	void WeaponIdle( void );
	void DropGren( void );
	BOOL CanDeploy( void );
	void SetDmgTime( float time ) { m_flTimeToExplode = time; }
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
};

class CStickGrenadeEx : public CDoDGrenadeEx
{
private:
	float m_flTimeToExplode;
	BOOL m_bUnderhand;

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
	void SetDmgTime( float time ) { m_flTimeToExplode = time; }
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
private:
	unsigned short m_usFireM1Carbine;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CGarand : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireGarand;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CScopedKar : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireScopedKar;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CThompson : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireThompson;

public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void ) { return; }
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int Classify( void );
	int iItemSlot( void ) { return 2; }
	BOOL UseDecrement( void ) { return TRUE; }
};

class CMP44 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireMP44;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CSPRING : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireSpring;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CKAR : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireKar;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CMP40 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireMP40;

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
	BOOL UseDecrement( void ) { return TRUE; }
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
private:
	unsigned short m_usFireMG42;

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
};

class CMG34 : public CBipodWeapon
{
private:
	unsigned short m_usFireMG34;

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
};

class C30CAL : public CBipodWeapon
{
private:
	unsigned short m_usFire30CAL;

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
};

class CBREN : public CBipodWeapon
{
private:
	unsigned short m_usFireBren;

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
};

class CFG42 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireFG42;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CK43 : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireK43;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CENFIELD : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireEnfield;
	unsigned short m_usFireScopedEnfield;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CSTEN : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireSten;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CGreaseGun : public CBasePlayerWeapon
{
private:
	unsigned short m_usFireGreaseGun;

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
	BOOL UseDecrement( void ) { return TRUE; }
};

class CBazooka : public CRocketWeapon
{
private:
	unsigned short m_usFireBazooka;

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
};

class CPschreck : public CRocketWeapon
{
private:
	unsigned short m_usFirePschreck;

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
};

class CPIAT : public CRocketWeapon
{
private:
	unsigned short m_usFirePIAT;

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
};

#endif // WEAPONS_H