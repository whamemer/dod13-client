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
//=========================================================
// GameRules
//=========================================================
#pragma once
#if !defined(GAMERULES_H)
#define GAMERULES_H

class CBasePlayerItem;
class CBasePlayer;
class CItem;
class CBasePlayerAmmo;

class CGameRules
{
	virtual void Init( void ) = 0;
	virtual void Think( void ) = 0;

	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;

	virtual BOOL FAllowFlashlight( void ) = 0;
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) = 0;

	virtual BOOL IsMultiplayer( void ) = 0;
	virtual BOOL IsDeathmatch( void ) = 0;
	virtual BOOL IsTeamplay( void ) { return FALSE; };
	virtual BOOL IsCoOp( void ) = 0;
	virtual const char *GetGameDescription( void ) { return "Day of Defeat"; }

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] ) = 0;
	virtual void InitHUD( CBasePlayer *pl ) = 0;
	virtual void ClientDisconnected( edict_t *pClient ) = 0;

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	virtual BOOL FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) { return TRUE; };
	virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) { return TRUE; }

	virtual void PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual void PlayerThink( CBasePlayer *pPlayer ) = 0;
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;

	virtual BOOL AllowAutoTargetCrosshair( void ) { return TRUE; };
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd ) { return FALSE; };
	virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer );

	virtual void PlayerDeathThink( CBasePlayer *pPlayer ) = 0;
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor ) = 0;
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor ) = 0;

	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon ) = 0;
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon ) = 0;
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon ) = 0;
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon ) = 0;

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;

	virtual int ItemShouldRespawn( CItem *pItem ) = 0;
	virtual float FlItemRespawnTime( CItem *pItem ) = 0;
	virtual Vector VecItemRespawnSpot( CItem *pItem ) = 0;

	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount ) = 0;
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo ) = 0;
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo ) = 0;
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo ) = 0;

	virtual float FlHealthChargerRechargeTime( void ) = 0;
	virtual float FlHEVChargerRechargeTime( void ) = 0;

	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;

	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;

	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;

	virtual int GetTeamIndex( const char *pTeamName ) { return -1; }
	virtual const char *GetIndexedTeamName( int teamIndex ) { return ""; }
	virtual BOOL IsValidTeam( const char *pTeamName ) { return TRUE; }
	virtual void ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib ) {}
	virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer ) { return ""; }

	virtual BOOL PlayTextureSounds( void ) { return TRUE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol ) { return TRUE; }

	virtual BOOL FAllowMonsters( void ) = 0;

	virtual void EndMultiplayerGame( void ) {}

	virtual int RoundState( void ) { return 1; }

	virtual void VoteKick( int m_iD, bool i_UserID ) = 0;

	virtual void SetRoundState( int i_State ) = 0;
	virtual int SetRoundState( void ) = 0;
	virtual void SetRestartTime( float f_Time ) = 0;
	virtual void SendTeamScores( int m_iTeam ) = 0;

	virtual float GetMaxWaveTime( int nTeam ) { return 0.0f; }
	virtual float getWaveTime( int nTeam ) { return 0.0f; }
	virtual void setWaveTime( int iTeam, float fTime ) = 0;

	virtual int TeamScores( int iTeam ) { return 1; }
	virtual void SetTeamScores( int iTeam, int iScore ) = 0;
	virtual void AddToTeamScores( int iTeam, int iScore ) = 0;

	virtual void setMapTime( void ) = 0;
	virtual void RestartRound( void ) = 0;
	virtual float getMapTime( void ) { return 0.0f; }

	virtual bool IsRoundFrozen( void ) { return false; }

	virtual void SetClanMatch( void ) = 0;
	virtual bool IsClanMatch( void ) { return false; }
	virtual bool IsClanMatchActive( void ) { return false; }

	virtual void RefreshSkillData( void ) = 0;

    virtual unsigned short Camera( void ) = 0;
    virtual unsigned short usVoice( void ) = 0;
    virtual unsigned short bodyDamage( void ) = 0;
    virtual unsigned short roundRestartSound( void ) = 0;
    virtual unsigned short gerVoice( void ) = 0;
    virtual unsigned short Pain( void ) = 0;
    virtual unsigned short Smoke( void ) = 0;
    virtual unsigned short BloodSprite( void ) = 0;
    virtual unsigned short BloodStream( void ) = 0;
    virtual unsigned short BulletTracers( void ) = 0;
    virtual unsigned short SparkShower( void ) = 0;
    virtual unsigned short ScreenFades( void ) = 0;
    virtual unsigned short BubbleTrails( void ) = 0;
    virtual unsigned short Bubbles( void ) = 0;
    virtual unsigned short Shards( void ) = 0;
    virtual unsigned short Explosion( void ) = 0;
    virtual unsigned short PopHelmet( void ) = 0;
    virtual unsigned short Prone( void ) = 0;
    virtual unsigned short RoundReset( void ) = 0;
    virtual unsigned short StartTrail( void ) = 0;

    virtual void SetCamera( short s_Precache ) = 0;
    virtual void SetusVoice( short s_Precache ) = 0;
    virtual void SetbodyDamage( short s_Precache ) = 0;
    virtual void SetroundRestartSound( short s_Precache ) = 0;
    virtual void SetgerVoice( short s_Precache ) = 0;
    virtual void SetPain( short s_Precache ) = 0;
    virtual void SetSmoke( short s_Precache ) = 0;
    virtual void SetProne( short s_Precache ) = 0;
    virtual void SetBloodSprite( short s_Precache ) = 0;
    virtual void SetBloodStream( short s_Precache ) = 0;
    virtual void SetBulletTracers( short s_Precache ) = 0;
    virtual void SetSparkShower( short s_Precache ) = 0;
    virtual void SetScreenFades( short s_Precache ) = 0;
    virtual void SetBubbleTrails( short s_Precache ) = 0;
    virtual void SetBubbles( short s_Precache ) = 0;
    virtual void SetShards( short s_Precache ) = 0;
    virtual void SetExplosion( short s_Precache ) = 0;
    virtual void SetPopHelmet( short s_Precache ) = 0;
    virtual void SetRoundReset( short s_Precache ) = 0;
    virtual void SetRocketTrail( short s_Precache ) = 0;

    virtual void SetMapChangeFromTime( int iTime ) = 0;
};

class CSPDoDRules : public CGameRules
{
protected:
	unsigned short m_Camera;
    unsigned short m_usVoice;
    unsigned short m_bodyDamage;
    unsigned short m_roundRestartSound;
    unsigned short m_gerVoice;
    unsigned short m_Pain;
    unsigned short m_Smoke;
    unsigned short m_Prone;
    unsigned short m_BloodSprite;
    unsigned short m_BloodStream;
    unsigned short m_BulletTracers;
    unsigned short m_SparkShower;
    unsigned short m_ScreenFades;
    unsigned short m_BubbleTrails;
    unsigned short m_Bubbles;
    unsigned short m_Shards;
    unsigned short m_Explosion;
    unsigned short m_PopHelmet;
    unsigned short m_RoundReset;
    unsigned short m_RocketTrail;

	CSPDoDRules( void );

	virtual void Init( void );
	virtual void Think( void );

	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );

	virtual BOOL FAllowFlashlight( void );
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] );
	virtual void InitHUD( CBasePlayer *pl );
	virtual void ClientDisconnected( edict_t *pClient );

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );

	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );

	virtual void PlayerDeathThink( CBasePlayer *pPlayer );
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

	virtual float FlHealthChargerRechargeTime( void );

	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

	virtual const char *GetTeamID( CBaseEntity *pEntity );

	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );

	virtual BOOL FAllowMonsters( void );

	virtual int RoundState( void );

	virtual void VoteKick( int m_iD, bool i_UserID );

	virtual void SetRoundState( int i_State );
	virtual void SetRestartTime( float f_Time );
	virtual void SendTeamScores( int m_iTeam );

	virtual float GetMaxWaveTime( int nTeam );
	virtual float getWaveTime( int nTeam );
	virtual void setWaveTime( int iTeam, float fTime );

	virtual int TeamScores( int iTeam );
	virtual void SetTeamScores( int iTeam, int iScore );
	virtual void AddToTeamScores( int iTeam, int iScore );

	virtual void setMapTime( void );
	virtual float getMapTime( void );

	virtual bool IsRoundFrozen( void );

	virtual void SetClanMatch( void );
	virtual bool IsClanMatch( void );
	virtual bool IsClanMatchActive( void );

    virtual unsigned short Camera( void );
    virtual unsigned short usVoice( void );
    virtual unsigned short bodyDamage( void );
    virtual unsigned short roundRestartSound( void );
    virtual unsigned short gerVoice( void );
    virtual unsigned short Pain( void );
    virtual unsigned short Smoke( void );
    virtual unsigned short BloodSprite( void );
    virtual unsigned short BloodStream( void );
    virtual unsigned short BulletTracers( void );
    virtual unsigned short SparkShower( void );
    virtual unsigned short ScreenFades( void );
    virtual unsigned short BubbleTrails( void );
    virtual unsigned short Bubbles( void );
    virtual unsigned short Shards( void );
    virtual unsigned short Explosion( void );
    virtual unsigned short PopHelmet( void );
    virtual unsigned short Prone( void );
    virtual unsigned short RoundReset( void );
    virtual unsigned short StartTrail( void );

    virtual void SetCamera( short s_Precache );
    virtual void SetusVoice( short s_Precache );
    virtual void SetbodyDamage( short s_Precache );
    virtual void SetroundRestartSound( short s_Precache );
    virtual void SetgerVoice( short s_Precache );
    virtual void SetPain( short s_Precache );
    virtual void SetSmoke( short s_Precache );
    virtual void SetProne( short s_Precache );
    virtual void SetBloodSprite( short s_Precache );
    virtual void SetBloodStream( short s_Precache );
    virtual void SetBulletTracers( short s_Precache );
    virtual void SetSparkShower( short s_Precache );
    virtual void SetScreenFades( short s_Precache );
    virtual void SetBubbleTrails( short s_Precache );
    virtual void SetBubbles( short s_Precache );
    virtual void SetShards( short s_Precache );
    virtual void SetExplosion( short s_Precache );
    virtual void SetPopHelmet( short s_Precache );
    virtual void SetRoundReset( short s_Precache );
    virtual void SetRocketTrail( short s_Precache );

	virtual int ReturnGameRules( void );

    virtual void SetMapChangeFromTime( int iTime );
};

extern DLL_GLOBAL CGameRules *g_pGameRules;
#endif // GAMERULES_H
