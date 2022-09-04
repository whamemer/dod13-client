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
#if !defined(PLAYER_H)
#define PLAYER_H

#include "pm_materials.h"

#include "unisignals.h"
#include "hintmessage.h"

#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define PFLAG_ONLADDER		( 1<<0 )
#define PFLAG_ONSWING		( 1<<0 )
#define PFLAG_ONTRAIN		( 1<<1 )
#define PFLAG_ONBARNACLE	( 1<<2 )
#define PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define PFLAG_USING		( 1<<4 )		// Using a continuous entity
#define PFLAG_OBSERVER		( 1<<5 )		// player is locked in stationary cam mode. Spectators can move, observers can't.

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
    PLAYER_PRONE1,
    PLAYER_PRONE2,
    PLAYER_PRONE3,
    PLAYER_RELOAD,
    PLAYER_SIGNAL,
    PLAYER_ATTACK2,
    PLAYER_ROLLGRENADE,
    PLAYER_RPGSHOULDER,
    PLAYER_EXPLODE_BACK,
    PLAYER_EXPLODE_FORWARD,
    PLAYER_BUT_SWING
} PLAYER_ANIM;

enum JoinState
{
    JUSTJOINED = 0,
    READYTOGO,
    JOINED
};

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END
};

#define CHAT_INTERVAL 1.0f

class CBasePlayer : public CBaseMonster
{
public:
    int m_iPlayerSpeed;
    int random_seed;
    int m_iPlayerSound;
    int m_iTargetVolume;
    int m_iWeaponVolume;
    int m_iExtraSoundTypes;

    float m_flStopExtraSoundTime;

    int m_afButtonLast;
    int m_afButtonPressed;
    int m_afButtonReleased;

    edict_t *m_pentSndLast;

    float m_flSndRoomtype;
    float m_flSndRange;
    float m_flFallVelocity;

    int m_fKnownItem;
    int m_fNewAmmo;

    unsigned int m_afPhysicsFlags;

    float m_fNextSuicideTime;
    float m_flTimeWeaponIdle;

    char m_szTextureName[CBTEXTURENAMEMAX];
    char m_chTextureType;

    int m_idrowndmg;
    int m_idrownrestored;
    int m_bitsHUDDamage;

    BOOL m_fInitHUD;
    BOOL m_fGameHUDInitialized;

    int m_iTrain;

    BOOL m_fWeapon;

    JoinState m_iJoiningState;

    EHANDLE m_pTank;
    float m_fDeadTime;

    int m_iUpdateTime;
    int m_iHideHUD;

    float m_iRespawnFrames;

    int m_iClientHealth;
    int m_iClientHideHUD;
    int m_iClientFOV;
    int m_nCustomSprayFrames;

    float m_flNextDecalTime;

    CBasePlayerItem *m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;
	CBasePlayerItem *m_pLastItem;

    int	m_rgAmmo[MAX_AMMO_SLOTS];
	int	m_rgAmmoLast[MAX_AMMO_SLOTS];
    int m_rgItems[MAX_WEAPON_SLOTS];

    char m_szTeamName[TEAM_NAME_LENGTH];

    static TYPEDESCRIPTION m_playerSaveData[];

    int m_irdytospawn;
    int m_imissedwave;
    int m_ilastteam;
    int m_ijustjoined;
    int m_chatMsg;
    int m_deathMsg;
    int m_handSignal;

    float m_nextRunThink;
    float m_flIdleTimer;
    float m_inextprone;

    int m_voteid;
    int m_iFOV;

    float m_fGunDropTime;

    int m_iNextClass;
    bool m_bIsRandomClass;

    EHANDLE m_hObserverTarget;
    float m_flNextObserverInput;
    int m_iObserverWeapon;
    int m_iObserverLastMode;
    int m_bIsObserver;

    BOOL HasObject;
    //CObject *m_pObject; // WHAMER: TODO

    char *m_sPlayerModel;
    
    float m_fNextStamina;

    int m_nMGAmmoCount;
    int m_nGenericAmmoCount;

    int m_iClassSpeed;
    float m_fSpeedFactor;

    bool m_bSlowedByHit;
    float m_flUnslowTime;
    float m_flNextVoice;
    float m_flNextHand;

    int lastMenuSelected;

    int m_izSBarState[SBAR_END];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	char m_SbarString1[SBAR_STRING_SIZE];

    EHANDLE m_hLastIDTarget;

    float m_flLastTalkTime;

    int i_cutScenes;
    int i_seenAllieSpawnScene;
    int i_seenAxisSpawnScene;
    int i_seenAllieWinScene;
    int i_seenAxisWinScene;
    int i_seenAllieLooseScene;
    int i_seenAxisLooseScene;
    int i_seenAllieDrawScene;
    int i_seenAxisDrawScene;

    CUnifiedSignals m_signals;

    int m_iCapAreaIndex;
    int m_iCapAreaIconIndex;
    int m_iObjectAreaIndex;

    float m_fHandleSignalsTime;

    int m_iObjScore;
    int m_iDeaths;
    int m_iNumTKs;

    BOOL m_bBazookaDeployed;

    int m_iMinimap;

    bool m_bShowHints;
    bool m_bAutoReload;

    CHintMessageQueue m_hintMessageQueue;

    float m_flDisplayHistory;

    char m_szAnimExtention[32];
    char m_szAnimReloadExt[32];

    int m_iGaitsequence;

    float m_flGaitframe;
    float m_flGaityaw;

    Vector m_prevgaitorigin;

    float m_flPitch;
    float m_flYaw;
    float m_flGaitMovement;
    float m_flYawModifier;

    int m_iCurrentAnimationState;
    int m_iCurrentAnimationSequence;
    int m_pszSavedWeaponModel;
    int m_ianimupdate;

    float m_flNextMapMarkerTime;

    int m_iMapMarker;
    int m_iWeapons2;

    virtual void Spawn( void );
    virtual void Precache( void );

    virtual void PreThink( void );
	virtual void PostThink( void );

    virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
    virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
    virtual int TakeHealth( float flHealth, int bitsDamageType );
    virtual void Killed( entvars_t *pevAttacker, int iGib );

    virtual void Jump( void );
	virtual void Duck( void );

    void WaterMove( void );

    virtual void UpdateClientData( void );

    void SetCustomDecalFrames( int nFrames );
    int GetCustomDecalFrames( void );

    BOOL AddPlayerItem( CBasePlayerItem *pItem );
    BOOL RemovePlayerItem( CBasePlayerItem *pItem, bool bCallHoster );
    void DropPlayerItem ( char *pszItemName, bool bForceDrop );
    BOOL HasPlayerItem( CBasePlayerItem *pCheckItem );
	BOOL HasNamedPlayerItem( const char *pszItemName );
	BOOL HasWeapons( void );
	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem( void );
	void SelectItem( const char *pstr );
	void ItemPreFrame( void );
	void ItemPostFrame( void );

    virtual Vector GetGunPosition( void );

    edict_t *GiveNamedItem( const char *szName );

    static int GetAmmoIndex( const char *psz );
	int AmmoInventory( int iAmmoIndex );
    void TabulateAmmo( void );

    virtual int GiveAmmo( int iAmount, const char *szName, int iMax );
    void SendAmmoUpdate( void );

    virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ) + pev->view_ofs * RANDOM_FLOAT( 0.5f, 1.1f ); };

    virtual BOOL IsAlive( void ) { return ( pev->deadflag == DEAD_NO ) && pev->health > 0; }
	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual	BOOL IsPlayer( void ) { return TRUE; }
	virtual BOOL IsNetClient( void ) { return TRUE; }

    BOOL IsOnLadder( void );
    virtual int Classify ( void );
    virtual int ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

    virtual const char *TeamID( void );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

    CBasePlayerWeapon *GetPrimaryWeapon( void );

    void PackDeadPlayerItems( void );
	void RemoveAllItems( BOOL removeSuit );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon, BOOL bJustSpawned );

    void DelayedSpawn( void );
    void SpawnThink( void );
    void ServerSpawnCheck( void );

    int PlayerStatus( void );
    void ClientDisconnect( void );

    void UTIL_EdictScreenFade( edict_s *edict, const Vector color, float fadeTime, float fadeHold, int alpha, int flags );

    void PushPlayer( void );
    void UpdatePlayerSound( void );

    virtual void DeathSound( void );
    virtual void ImpulseCommands( void );

    void EnableControl( BOOL fControl );

	void PlayerUse( void );
    void EXPORT PlayerDeathThink( void );

    virtual BOOL FBecomeProne( void );

    void ForceClientDllUpdate( void );

    void DeathMessage( entvars_t *pevKiller );

    void VoteMenu( int kickplayer, int menu );

    void ForceZoomOut( void );
    void UpdateScope( int weapon_id );
    bool ShouldGunLower( void );

    void AddObjectIcon( char *szIconName );
    void RemoveObjectIcon( void );
    bool PlayerHasExplodingGrenades( void );

    BOOL FlashlightIsOn( void );
    void FlashlightTurnOn( void );
    void FlashlightTurnOff( void );

    void StopObserver( void );
    int IsObserver( void ) { return pev->iuser1; };

    void Observer_SpectatePlayer( int iCurrent );
    void Observer_FindNextPlayer( bool bReverse );
	void Observer_HandleButtons();
	void Observer_SetMode( int iMode );
	void Observer_CheckTarget();
	void Observer_CheckProperties();
    bool Observer_PlayerValidTarget( CBaseEntity *pEnt, bool bSameTeam );

    void StartDeathCam( void );
    void StartObserver( Vector vecPosition, Vector vecViewAngle );

    //void GetObject( CObject *pObject ); // WHAMER: TODO
    void DropObject( void );

    void Stamina( void );
    float GetStamina( void );
    void SetStamina( float stamina );

    void DropGenericAmmo( void );
    BOOL DropItemByClass( int weaponclass );

    bool CheckPlayerSpeed( void );
    bool SetSpeed( int speed );

    void ShowMenu( int slots, int time, char *szText, int menu, int kickplayer );

    void VoiceCom( int command );
    void HandSignal( int signal );
    void ClearVoiceMenu( void );
    void VoiceMenu( int menuNum );

    void InitStatusBar( void );
    void UpdateStatusBar( void );

    void PopHelmet( Vector vecdir );

    void SetClientAreaIcon( int areaIndex, bool state );

    void HandleSignals( void );

    void AddFrags( int num, BOOL bSendMsgToAll );
    void AddObjScore( int num, BOOL bSendMsgToAll );
    virtual void AddPoints( int score, BOOL bAllowNegativeScore );
    virtual void AddPointsToTeam( int score, BOOL bAllowNegativeScore );

    bool HintMessage( const char *pMessage, BOOL bOverride );
    void SetPrefsFromUserinfo( char *infobuffer );

    void StudioEstimateGait( void );
    void StudioPlayerBlend( int *pBlend, float *pPitch );
    void CalculatePitchBlend( void );
    void CalculateYawBlend( void );
    void StudioProcessGait( void );

    Activity SetIdealActivity( Activity i_RequestedActivity );

    void SetAnimation( PLAYER_ANIM playerAnim );
    void SetWeaponAnimType( const char *pAnim );

    void FireMapMarker( void );
    void NextMapMarker( void );
    void PrevMapMarker( void );
    void SetMapMarker( int marker );

    void SpawnClientSideCorpse( void );

    bool HasWeapons( int weaponId );
    void SetHasWeapon( int weaponId );
    void ClearHasWeapon( int weaponId );
    void RemoveAllWeaponFlags( void );
    int GetWeapons2( void );

    bool IsInMGDeploy( void );
    bool IsProneDeployed( void );
    bool IsInSandbagZone( void );
    bool IsSandbagDeployed( void );
    bool IsProne( void );

    void SetProneState( int state );
    void SetSandbagState( int state );
    int GetProneState( void );

    void MortarDeploy( void );
    void MortarUnDeploy( void );
    bool IsInMortarDeploy( void );

    void GoProne( void );
    void UnProne( void );

    bool IsDucking( void );
    bool IsParatrooper( void );
    bool IsBritish( void );

    void MortarFireCommand( byte bAimValue );
    
    void SendObserverTargetWeaponAnim( void );
};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669

extern int gmsgHudText;
extern BOOL gInitHUD;

#endif // PLAYER_H