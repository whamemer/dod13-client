//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( DOD_GAMERULES_H )
#define DOD_GAMERULES_H

#include "gamerules.h"

/* class CDodParaRoundTimer
{

}; */ // WHAMER: TODO

typedef struct
{
    bool m_bAlliesInfiniteLives;
    bool m_bAxisInfiniteLives;
    bool m_bAlliesArePara;
    bool m_bAxisArePara;
    bool m_bAlliesAreBrit;

    int m_iPointsAlliesElim;
    int m_iPointsAxisElim;

    string_t m_sAlliesElimTarget;
    string_t m_sAxisElimTarget;

    bool m_bUseTimer;

    string_t m_sTimerExpireTarget;

    int m_iTimerTeam;
    int m_iPointsTimerExpires;

    float m_fAlliesRespawnFactor;
    float m_fAxisRespawnFactor;
} gameplay_rules_t;

class CDoDTeamPlay : public CGameRules
{
public:
    int m_RoundDoesCount;
    int m_DoDCameraState;
    int m_RoundParaCanJoin;

    float m_flGoodToGoTime;
    float m_flRoundTime;
    float m_flCanJoinTime;
    float m_flRestartTime;
    float m_flFinalCheckTime;
    float m_flDoDMapTime;
    float m_flAlliesWaveTime;
    float m_flAxisWaveTime;
    float m_flAlliesRespawn;
    float m_flAxisRespawn;

    int m_iTeamScores[32];
    char *m_szTeamNames[32];

    float m_flSpamResetTime;

    bool m_bClanMatch;
    bool m_bClanMatchActive;

    char m_szAuthID[32];

    CBasePlayer *m_vKickENT;
    int m_vKick;
    int m_vKickVotes;
    int m_vKickSessionState;
    float m_vKickTime;

    float m_rRoundStartCount;
    int m_rReadyToUnFreeze;
    int gr_gameRules;
    gameplay_rules_t m_GamePlayRules;

    int m_iNumAlliesAlive;
    int m_iNumAxisAlive;
    int m_iNumAlliesOnTeam;
    int m_iNumAxisOnTeam;

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

    char sNextMap[32];

    int MapChangeFromTime;

private:
    bool m_bIsRoundFrozen;

    BOOL m_DisableDeathMessages;
    BOOL m_DisableDeathPenalty;

    //CDodParaRoundTimer *m_pParaTimer; // WHAMER: TODO

    int m_RoundState;

    BOOL m_bRoundRestarting;
    BOOL m_bAwaitingReadyRestart;
    BOOL m_bHeardAlliesReady;
    BOOL m_bHeardAxisReady;

    float m_flRestartRoundTime;
    float m_fLogScoresTime;

protected:
    float m_flIntermissionEndTime;
    BOOL m_iEndIntermissionButtonHit;

    CDoDTeamPlay( void );

    virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
    virtual BOOL FAllowFlashlight( void );
    virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );
    virtual BOOL IsMultiplayer( void );
    virtual BOOL IsDeathmatch( void );
    virtual BOOL IsCoOp( void );

    virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
    virtual void PlayerThink( CBasePlayer *pPlayer );
    virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
    virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );
    virtual edict_t * GetPlayerSpawnSpot( CBasePlayer *pPlayer );

    virtual BOOL AllowAutoTargetCrosshair( void );

    virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
    virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pItem );

	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

    virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

    virtual float FlHealthChargerRechargeTime( void );
    virtual float FlHEVChargerRechargeTime( void );

    virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

    virtual BOOL PlayTextureSounds( void );
    virtual BOOL FAllowMonsters( void );

    virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] );
    virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd );
    virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer );

    virtual BOOL IsTeamplay( void );
    virtual BOOL FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );
    virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
    virtual const char *GetTeamID( CBaseEntity *pEntity );

    virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target );

    virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
    virtual void InitHUD( CBasePlayer *pl );
    virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
    virtual const char *GetGameDescription( void );
    virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
    virtual void Think( void );
    virtual int GetTeamIndex( const char *pTeamName );
    virtual const char *GetIndexedTeamName( int teamIndex );
    virtual BOOL IsValidTeam( const char *pTeamName );
    virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer );
    virtual void ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib );
    virtual void EndMultiplayerGame( void );
    virtual void Init( void );
    void ChooseRandomClass( CBasePlayer *pPlayer );
    virtual void PlayerSpawn( CBasePlayer *pPlayer );
    virtual void ClientDisconnected( edict_t *pClient );

    void GoodToGo( void );
    virtual void RestartRound( void );
    void TeamWon( int iTeamWon );

    virtual float GetMaxWaveTime( int nTeam );
    virtual float getWaveTime( int nTeam );
    virtual void setWaveTime( int iTeam, float fTime );
    virtual void setMapTime( void );
    virtual float getMapTime( void );

    int GetClassLimit( int cls );
    BOOL ReachedClassLimit( int targetClass );
    int CountClasses( int targetClass );

    virtual int TeamScores( int iTeam );
    virtual void SetTeamScores( int iTeam, int iScore );
    virtual void AddToTeamScores( int iTeam, int iScore );
    virtual void SetRestartTime( float f_Time );

    virtual int RoundState( void );
    virtual void SetRoundState( int i_State );
    virtual void SendTeamScores( int m_iTeam );

    void UpdateData( CBasePlayer *pPlayer );

    virtual void SetClanMatch( void );
    virtual bool IsClanMatch( void );
    virtual bool IsClanMatchActive( void );

    void ReturnToWarmup( void );
    void CheckClanRestart( void );
    void StartClanRestart( void );
    void StartClanMatch( void );

    virtual void VoteKick( int m_iD, bool i_UserID );
    void VoteKickTimeUp( void );

    void RoundUnfreeze( void );

    virtual void PlayerDeathThink( CBasePlayer *pPlayer );

    void DetectGameRules( void );
    virtual int ReturnGameRules( void );

    bool IsAxisInfinite( void );
    bool IsAxisPara( void );
    bool IsAlliesInfinite( void );
    bool IsAlliesBrit( void );
    bool IsAlliesPara( void );
    bool IsTeamPara( int team );
    bool IsTeamInfinite( int team );

    void EnumerateTeams( int *aliveAllies, int *numAllies, int *aliveAxis, int *numAxis );
    void ClassSelect( CBasePlayer *pPlayer, int iClassSelect );
    void TeamSelect( CBasePlayer *pPlayer, int iTeamSelect );

    void VoiceCommSelect( CBasePlayer *pPlayer, int iVoiceSelect );

    void CheckOneLifeRoundConditions( void );

    virtual void RefreshSkillData( void );

    void CopyGamePlayLogic( const gameplay_rules_t *otherGamePlay );

    void TimerExpired( void );

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
    virtual void SetMapChangeFromTime( int iTime );

    void RoundFreeze( void );
    void LevelChangeResets( CBasePlayer *pPlayer );
    void SetNextMap( void );

    char *COM_Parse( char *data );
    int COM_TokenWaiting( char *buffer );

    void SendClientGamerules( CBasePlayer *pPlayer );
    void SendObjectiveStatus( CBasePlayer *pPlayer );
    void SendWaveStatus( CBasePlayer *pPlayer, int status );

    void ResetForDoDCamera( void );

    void ReadInClassInfo( void );

    int GetWeaponId( const char *weaponname );
    bool AllowSpawnableBazookas( void );

    int GetClassMask( int team );

    bool CanPlayerJoinClass( int playerclass );
    void ShowClassSelectMenu( CBasePlayer *pPlayer );
    char *GetPlayerClassName( int playerclass );

private:
    virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol );
    virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );

    void PreparePlayerForSpawn( CBasePlayer *pPlayer );

    virtual bool IsRoundFrozen( void );

protected:
    virtual void GoToIntermission( void );
    virtual void ChangeLevel( void );

    void SendMOTDToClient( edict_t *client );
};

const char g_szTeamNames[4][128];

extern char* sHandSignals[];

class CDoDDetect : public CBaseEntity
{
public:
    gameplay_rules_t m_GamePlayRules;

    int m_iAlliesInfiniteLives;
    int m_iAxisInfiniteLives;
    int m_iAlliesPara;
    int m_iAxisPara;
    int m_iAlliesElimPoints;
    int m_iAxisElimPoints;

private:
    string_t m_sMaster;

    CDoDDetect( void );

    virtual void Spawn( void );
    virtual void KeyValue( KeyValueData *pkvd );

    bool IsMasteredOn( void );
};
#endif // !DOD_GAMERULES_H