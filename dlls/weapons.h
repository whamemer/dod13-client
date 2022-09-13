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

class CBasePlayer;
extern int gmsgWeapPickup;

void DeactivateSatchels( CBasePlayer *pOwner );

typedef enum
{
    G_MILLSGRENADE = 0,
    G_HANDGRENADE,
    G_STICKGRENADE
} grenType;

class CGrenade : public CBaseMonster
{
public:
	void Spawn( void );

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

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
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );
    void EXPORT RemoveMe( void );

    virtual void Precache( void );
	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );
    virtual int ObjectCaps( void );
    
    grenType g_GrenType;
    float m_flClipTime;
    int m_iOldMoveType;
    int m_iBodyGibs;
	BOOL m_fRegisteredSound;
};

// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

// weapon id
#define WEAPON_NONE				0
#define WEAPON_AMERKNIFE		1
#define WEAPON_GERKNIFE			2
#define WEAPON_COLT             3
#define WEAPON_LUGER            4
#define WEAPON_GARAND           5
#define WEAPON_SCOPEDKAR        6
#define WEAPON_THOMPSON         7
#define WEAPON_MP44             8
#define WEAPON_SPRING           9
#define WEAPON_KAR              10
#define WEAPON_BAR              11
#define WEAPON_MP40             12
#define WEAPON_HANDGRENADE      13
#define WEAPON_STICKGRENADE     14
#define WEAPON_STICKGRENADEX    15
#define WEAPON_HANDGRENADEX     16
#define WEAPON_MG42             17
#define WEAPON_CAL30            18
#define	WEAPON_SPADE			19
#define WEAPON_M1CARBINE        20
#define WEAPON_MG34             21
#define WEAPON_GREASEGUN        22
#define WEAPON_FG42             23
#define WEAPON_K43              24
#define WEAPON_ENFIELD          25
#define WEAPON_STEN             26
#define WEAPON_BREN             27
#define WEAPON_WEBLEY           28
#define WEAPON_BAZOOKA          29
#define WEAPON_PSCHRECK         30
#define WEAPON_PIAT             31
#define WEAPON_BINOC            33

/*
#define WEAPON_AMERKNIFE		1
#define WEAPON_GERKNIFE			2
#define WEAPON_GERPARAKNIFE		2
#define WEAPON_COLT				3
#define WEAPON_LUGER			4
#define WEAPON_GARAND			5
#define WEAPON_SCOPEDKAR		6
#define WEAPON_THOMPSON			7
#define WEAPON_MP44				8
#define WEAPON_SPRING			9
#define WEAPON_KAR				10
#define WEAPON_BAR				11
#define WEAPON_MP40				12
#define WEAPON_MG42				17
#define WEAPON_30CAL			18
#define WEAPON_SPADE			19
#define WEAPON_M1CARBINE		20
#define WEAPON_MG34				21
#define WEAPON_GREASEGUN		22
#define WEAPON_FG42				23
#define WEAPON_K43				24
#define WEAPON_ENFIELD			25
#define WEAPON_SCOPEDENFIELD	(0 ? 25 : -1)
#define WEAPON_STEN				26
#define WEAPON_BREN				27
#define WEAPON_WEBLEY			28
#define WEAPON_FOLDINGCARBINE	20
#define WEAPON_SCOPED_FG42		65
*/

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				60	// Default from hl is 31

#define MAX_NORMAL_BATTERY	100

// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define BREN_WEIGHT
#define MG34_WEIGHT
#define MG42_WEIGHT
#define CAL30_WEIGHT
#define BAR_WEIGHT
#define AMER_KNIFE_WEIGHT       0
#define GERKNIFE_WEIGHT		    0
#define SPADE_WEIGHT		    0
#define COLT_WEIGHT
#define LUGER_WEIGHT
#define WEBLEY_WEIGHT
#define ENFIELD_WEIGHT
#define FG42_WEIGHT

// weapon clip/carry ammo capacities
#define _55MM_MAX_CARRY                 // BREN, BAR, FG42, SCOPED FG42
#define _66MM_MAX_CARRY                 // MG34
#define _66MM_GER_MAX_CARRY             // MG42
#define _66MM_US_MAX_CARRY              // 30CAL
#define _12MM_MAX_CARRY                 // COLT, LUGER, WEBLEY
#define _16MM_MAX_CARRY                 // ENFIELD, SCOPED ENFIELD

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

#define BREN_MAX_CLIP
#define MG34_MAX_CLIP
#define MG42_MAX_CLIP
#define CAL30_MAX_CLIP
#define BAR_MAX_CLIP
#define COLT_MAX_CLIP
#define LUGER_MAX_CLIP
#define WEBLEY_MAX_CLIP
#define ENFIELD_MAX_CLIP
#define FG42_MAX_CLIP

// the default amount of ammo that comes with each gun when it spawns
#define BREN_DEFAULT_GIVE      30
#define MG34_DEFAULT_GIVE      75
#define MG42_DEFAULT_GIVE      250
#define CAL30_DEFAULT_GIVE
#define BAR_DEFAULT_GIVE
#define MELEE_DEFAULT_GIVE     0
#define COLT_DEFAULT_GIVE
#define LUGER_DEFAULT_GIVE
#define WEBLEY_DEFAULT_GIVE
#define ENFIELD_DEFAULT_GIVE
#define FG42_DEFAULT_GIVE

// The amount of ammo given to a player by an ammo item.
#define AMMO_BRENCLIP_GIVE		BREN_MAX_CLIP
#define AMMO_MG34CLIP_GIVE      MG34_MAX_CLIP
#define AMMO_MG42CLIP_GIVE      MG42_MAX_CLIP
#define AMMO_CAL30CLIP_GIVE     CAL30_MAX_CLIP
#define AMMO_BARCLIP_GIVE       BAR_MAX_CLIP
#define AMMO_COLTCLIP_GIVE      COLT_MAX_CLIP
#define AMMO_LUGERCLIP_GIVE     LUGER_MAX_CLIP
#define AMMO_WEBLEYCLIP_GIVE    WEBLEY_MAX_CLIP
#define AMMO_ENFIELDCLIP_GIVE   ENFIELD_MAX_CLIP
#define AMOO_FG42CLIP_GIVE      FG42_MAX_CLIP

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_357,
	BULLET_PLAYER_BUCKSHOT,
	BULLET_PLAYER_CROWBAR,

	BULLET_PLAYER_COLT,
	BULLET_PLAYER_LUGER,
	BULLET_PLAYER_GARAND,
	BULLET_PLAYER_SCOPEDKAR,
	BULLET_PLAYER_THOMPSON,
	BULLET_PLAYER_MP44,
    BULLET_PLAYER_SPRING,
	BULLET_PLAYER_KAR,
	BULLET_PLAYER_BAR,
	BULLET_PLAYER_MP40,
	BULLET_PLAYER_MG42,
	BULLET_PLAYER_MG34,
	BULLET_PLAYER_30CAL,
	BULLET_PLAYER_M1CARBINE,
	BULLET_PLAYER_GREASEGUN,
	BULLET_PLAYER_FG42,
	BULLET_PLAYER_K43,
	BULLET_PLAYER_ENFIELD,
    BULLET_PLAYER_STEN,
	BULLET_PLAYER_BREN,
	BULLET_PLAYER_WEBLEY,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM
} Bullet;

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;
	int		iMaxAmmo1;
	const char	*pszAmmo2;
	int		iMaxAmmo2;
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;
    int     iBulletId;
    float   flSpread;
    float   flRecoil;
} ItemInfo;

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16

// Day of Defeat item info flags
#define ITEM_FLAG_PISTOL	64 // CCOLT, CWEBLEY, CLUGER

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }
	void EXPORT DestroyItem( void );
    void EXPORT PickupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT DefaultTouch( CBaseEntity *pOther );
	void EXPORT FallThink ( void ); 
	void EXPORT Materialize( void );
	void EXPORT AttemptToMaterialize( void );
	CBaseEntity* Respawn ( void );
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( void ) { return TRUE; };

    virtual BOOL SpawnDeploy( void ) { return TRUE; };
    virtual int ObjectCaps( void ) { return MAX_PATH_SIZE; };
    virtual BOOL CanDrop( void ) { return this->CanHolster(); };
    virtual void SetDmgTime( float time ) { ; };
    virtual void DropGren( void ) { ; };

	virtual BOOL CanHolster( void ) { return TRUE; };
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void ) { return; }
	virtual void ItemPostFrame( void ) { return; }

	virtual void Drop( void );
	virtual void Kill( void );
    virtual void SpawnReplacement( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; };

    virtual void RoundRespawn( void );

    static TYPEDESCRIPTION m_SaveData[];

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int m_iId;

    BOOL m_bReadyToRespawn;
    Vector m_vRespawnOrigin;
    Vector m_vRespawnAngles;
    BOOL m_bTriggerSpawnOnDie;

	virtual int iItemSlot( void ) { return 0; }

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }
};

class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon );
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };

	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {};

	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1 );

    void PostMortarValue( float value );
    void SendMortarFireCommand( char *c );

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, const char *szAnimReloadExt, int skiplocal );
	int DefaultReload( int iClipSize, int iAnim, float fDelay );

	virtual void ItemPostFrame( void );

    virtual Vector Aim( float accuracyFactor, CBasePlayer *pOther, int shared_rand );
    virtual float flAim( float accuracyFactor, CBasePlayer *pOther );
    virtual void RemoveStamina( float removeAmount, CBasePlayer *pOther );
    virtual int ChangeFOV( int fov );
    virtual int ZoomOut( void );
    virtual int ZoomIn( void );
    virtual int GetFOV( void );
    virtual bool PlayerIsWaterSniping( void );
    void ThinkZoomOut( void );
    void ThinkZoomIn( void );
    virtual void UpdateZoomSpeed( void );

	virtual void PrimaryAttack( void ) { return; }
	virtual void SecondaryAttack( void ) { return; }
	virtual void Reload( void ) { return; }
    virtual void Special( void );
	virtual void WeaponIdle( void ) { return; }
	virtual int UpdateClientData( CBasePlayer *pPlayer );
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) { return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };

	virtual int	PrimaryAmmoIndex(); 
	virtual int	SecondaryAmmoIndex(); 

	void PrintState( void );

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; };
	BOOL TimedDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, char *szAnimReloadExt, float idleTime, float attackTime, int skiplocal );
    virtual int Classify( void );
    int GetRoundState( void );

    static TYPEDESCRIPTION m_SaveData[];

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
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
    bool CBasePlayerAmmo::GiveGenericAmmo( CBaseEntity *pOther );
};

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void ClearMultiDamage();
extern void ApplyMultiDamage( entvars_t *pevInflictor, entvars_t *pevAttacker );

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
	void SecondaryAttack( void ) { ; }
	int iItemSlot( void ) { return 0; }

	virtual BOOL UseDecrement( void )
	{ 
#if CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	int m_iSwing;
	TraceResult m_trHit;

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

	void SecondaryAttack( void ) { ; }
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

protected:
	unsigned short m_iFireEvent;
};

class CBipodWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( int weapon_id );
	void Precache( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Reload( void );
	void WeaponIdle( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal );
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
	BOOL SpawnDeploy( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	void SecondaryAttack( void ) { ; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_RIFLE; }

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
	void Reload( void );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_RIFLE; }

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
	void Reload( void );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_RIFLE; }

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
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	BOOL IsDeployed( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return !m_pPlayer->IsInMGDeploy(); }
	int Classify( void ) { return m_iWeaponState & 1 == CLASS_NONE ? CLASS_MACHINEGUNS : CLASS_SCOPE_RIFLE; }

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
	void Reload( void );
	void WeaponIdle( void );

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return CLASS_RIFLE; }

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

	int iItemSlot( void ) { return 2; }
	BOOL CanHolster( void ) { return TRUE; }
	int Classify( void ) { return m_iWeaponState & 1 == CLASS_NONE ? CLASS_RIFLE : CLASS_SCOPE_RIFLE; }

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

	BOOL CanHolster( void ) { return !( m_iWeaponState & 1 ); }

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

	BOOL CanHolster( void ) { return !( m_iWeaponState & 1 ); }

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

	BOOL CanHolster( void ) { return !( m_iWeaponState & 1 ); }

private:
	unsigned short m_usFirePIAT;
};
#endif // WEAPONS_H