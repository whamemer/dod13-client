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
#include "hintmessage.h"
#include "unisignals.h"

#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

#define PFLAG_ONLADDER		( 1<<0 )
#define PFLAG_ONSWING		( 1<<0 )
#define PFLAG_ONTRAIN		( 1<<1 )
#define PFLAG_ONBARNACLE	( 1<<2 )
#define PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define PFLAG_USING			( 1<<4 )		// Using a continuous entity
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

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETRELATION,
	SBAR_END
};

class CObject;

class CBasePlayer : public CBaseMonster
{
private:
	int m_iPlayerSpeed;

public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual void Jump( void );
	virtual void Duck( void );
	void WaterMove( void );
	virtual void UpdateClientData( void );

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
	char m_szTextureName[13];
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

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames( void );

	int m_nCustomSprayFrames;
	float m_flNextDecalTime;

	virtual BOOL AddPlayerItem( CBasePlayerItem *pItem );
	virtual BOOL RemovePlayerItem( CBasePlayerItem *pItem );
	void DropPlayerItem( char *pszItemName, bool bForceDrop );
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
	virtual int GiveAmmo( int iCount, const char *szName, int iMax );
	void SendAmmoUpdate( void );

	CBasePlayerItem *m_rgpPlayerItems[6];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;
	CBasePlayerItem *m_pLastItem;
	int m_rgAmmo[32];
	int m_rgAmmoLast[32];
	int m_rgItems[5];

	virtual Vector BodyTarget( const Vector &posSrc ) { return Center() + pev->view_ofs * RANDOM_FLOAT( 0.5, 1.3 ); }
	virtual BOOL IsAlive( void ) { return ( pev->deadflag == DEAD_NO ) && pev->health > 0; }
	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual BOOL IsPlayer( void ) { return TRUE; }
	virtual BOOL IsNetClient( void ) { return TRUE; }

	BOOL IsOnLadder( void );
	virtual int Classify( void ) { return CLASS_PLAYER; }
	virtual int ObjectCaps( void ) { return 0; }
	virtual const char *TeamID( void ) { if( pev == NULL ) return ""; return m_szTeamName; }

	char m_szTeamName[16];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_playerSaveData[];

	CBasePlayerWeapon *GetPrimaryWeapon( void );
	void PackDeadPlayerItems( void );
	void RemoveAllItems( BOOL removeSuit );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon, BOOL bJustSpawned );

	void DelayedSpawn( void );
	void EXPORT SpawnThink( void );

	int m_irdytospawn;
	int m_imissedwave;
	int m_ilastteam;
	int m_ijustjoined;

	void EXPORT ServerSpawnCheck( void );

	int PlayerStatus( void );
	void ClientDisconnect( void );
	void UTIL_EdictScreenFade( edict_s *pEdict, const Vector &color, float fadeTime, float holdTime, int flags, int alpha );
	void PushPlayer( void );

	int m_chatMsg;
	int m_deathMsg;
	int m_handSignal;
	float m_nextRunThink;
	float m_flIdleTimer;

	void UpdatePlayerSound( void );
	virtual void DeathSound( void );
	virtual void ImpulseCommands( void );
	void EnableControl( BOOL fControl );
	void PlayerUse( void );
	void EXPORT PlayerDeathThink( void );
	virtual BOOL FBecomeProne( void );

	float m_inextprone;

	void ForceClientDllUpdate( void );
	void DeathMessage( entvars_t *pevKiller );

	void VoteMenu( int kickplayer, int menu );

	int m_voteid;

	void ForceZoomOut( void );
	void UpdateScope( int weapon_id );

	int m_iFOV;

	bool ShouldGunLower( void );

	float m_fGunDropTime;

	void AddObjectIcon( char *szIconName );
	void RemoveObjectIcon( void );

	bool PlayerHasExplodingGrenades( void );

	BOOL FlashlightIsOn( void );
	void FlashlightTurnOn( void );
	void FlashlightTurnOff( void );

	int m_iNextClass;
	bool m_bIsRandomClass;

	void StopObserver( void );
	int IsObserver( void );
	void Observer_SpectatePlayer( int iCurrent );
	void Observer_FindNextPlayer( bool bReverse );
	void Observer_HandleButtons( void );
	void Observer_SetMode( int iMode );
	void Observer_CheckTarget( void );
	void Observer_CheckProperties( void );
	bool Observer_PlayerValidTarget( CBaseEntity *pEnt, bool bSameTeam );
	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	EHANDLE m_hObserverTarget;
	float m_flNextObserverInput;
	int m_iObserverWeapon;
	int m_iObserverLastMode;
	int m_bIsObserver;

	BOOL HasObject;
	CObject *m_pObject;

	void GetObject( CObject *pObject );
	void DropObject( void );

	char *m_sPlayerModel;

	void Stamina( void );

	float m_fNextStamina;

	float GetStamina( void );
	void SetStamina( float stamina );

	void DropGenericAmmo( void );

	int m_nMGAmmoCount;
	int m_nGenericAmmoCount;

	BOOL DropItemByClass( int weaponclass );

	bool CheckPlayerSpeed( void );
	bool SetSpeed( int speed );

	int m_iClassSpeed;
	float m_fSpeedFactor;
	bool m_bSlowedByHit;
	float m_flUnslowTime;

	void ShowMenu( int slots, int time, char *text, int bitsvalid, int menu );
	void VoiceCom( int command );
	void HandSignal( int signal );
	void ClearVoiceMenu( void );
	void VoiceMenu( int menuNum );

	float m_flNextVoice;
	float m_flNextHand;
	int lastMenuSelected;

	void InitStatusBar( void );
	void UpdateStatusBar( void );

	int m_izSBarState[4];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[128];
	char m_SbarString1[128];
	EHANDLE m_hLastIDTarget;
	float m_flLastTalkTime;

	void PopHelmet( Vector *vecdir );

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

	void SetClientAreaIcon( int areaIndex, bool state );

	float m_fHandleSignalsTime;

	void HandleSignals( void );

	void AddFrags( int num, BOOL bSendMsgToAll );
	void AddObjScore( int num, BOOL bSendMsgToAll );
	virtual void AddPoints( int score, BOOL bAllowNegativeScore );
	virtual void AddPointsToTeam( int score, BOOL bAllowNegativeScore );

	int m_iObjScore;
	int m_iDeaths;
	int m_iNumTKs;
	BOOL m_bBazookaDeployed;
	int m_iMinimap;

	bool HintMessage( const char *pMessage, BOOL bOverride );
	void SetPrefsFromUserinfo( char *infobuffer );

	bool m_bShowHints;
	bool m_bAutoReload;
	CHintMessageQueue m_hintMessageQueue;
	long m_flDisplayHistory;

	void StudioEstimateGait( void );
	void StudioPlayerBlend( int *pBlend, float *pPitch );
	void CalculatePitchBlend( void );
	void CalculateYawBlend( void );
	void StudioProcessGait( void );
	Activity SetIdealActivity( Activity i_RequestedActivity );
	void SetAnimation( PLAYER_ANIM playerAnim );
	void SetWeaponAnimType( const char *szExtention );

	char m_szAnimExtention[32];
	char m_szAnimReloadExt[32];
	int m_iGaitsequence;
	float m_flGaitframe;
	float m_flGaityaw;
	vec3_t m_prevgaitorigin;
	float m_flPitch;
	float m_flYaw;
	float m_flGaitMovement;
	float m_flYawModifier;
	int m_iCurrentAnimationState;
	int m_iCurrentAnimationSequence;
	int m_pszSavedWeaponModel;
	int m_ianimupdate;

	void FireMapMarker( void );
	void NextMapMarker( void );
	void PrevMapMarker( void );
	void SetMapMarker( int marker );

	float m_flNextMapMarkerTime;
	int m_iMapMarker;

	void SpawnClientSideCorpse( void );

	bool HasWeapons( int weaponId );
	void SetHasWeapon( int weaponId );
	void ClearHasWeapon( int weapon_id );
	void RemoveAllWeaponFlags( void );

	int GetWeapons2( void );

	int m_iWeapons2;

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

#endif // PLAYER_H
