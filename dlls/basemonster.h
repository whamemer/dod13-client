/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#pragma once
#if !defined(BASEMONSTER_H)
#define BASEMONSTER_H

//
// generic Monster
//
class CBaseMonster : public CBaseToggle
{
public:
	enum SCRIPTSTATE {
		SCRIPT_PLAYING = 0,
		SCRIPT_WAIT = 1,
		SCRIPT_CLEANUP = 2,
		SCRIPT_WALK_TO_MARK = 3,
		SCRIPT_RUN_TO_MARK = 4,
	};

	int                        m_afConditions;
	EHANDLE                    m_hEnemy;
	EHANDLE                    m_hTargetEnt;
	EHANDLE                    m_hOldEnemy[MAX_OLD_ENEMIES];
	Vector                     m_vecOldEnemy[MAX_OLD_ENEMIES];
	float                      m_flFieldOfView;
	float                      m_flWaitFinished;
	float                      m_flMoveWaitFinished;
	Activity                   m_Activity;
	Activity                   m_IdealActivity;
	int                        m_LastHitGroup;
	MONSTERSTATE               m_MonsterState;
	MONSTERSTATE               m_IdealMonsterState;
	int                        m_iTaskStatus;
	Schedule_t *m_pSchedule;
	int                        m_iScheduleIndex;
	WayPoint_t                 m_Route[ROUTE_SIZE];
	int                        m_movementGoal;
	int                        m_iRouteIndex;
	float                      m_moveWaitTime;
	Vector                     m_vecMoveGoal;
	Activity                   m_movementActivity;
	int                        m_iAudibleList;
	int                        m_afSoundTypes;
	Vector                     m_vecLastPosition;
	int                        m_iHintNode;
	int                        m_afMemory;
	int                        m_iMaxHealth;
	Vector                     m_vecEnemyLKP;
	int                        m_cAmmoLoaded;
	int                        m_afCapability;
	float                      m_flNextAttack;
	int                        m_bitsDamageType;
	unsigned char              m_rgbTimeBasedDamage[8];
	int                        m_lastDamageAmount;
	int                        m_bloodColor;
	int                        m_failSchedule;
	float                      m_flHungryTime;
	float                      m_flDistTooFar;
	float                      m_flDistLook;
	int                        m_iTriggerCondition;
	string_t                   m_iszTriggerTarget;
	Vector                     m_HackedGunPos;
	SCRIPTSTATE                m_scriptState;
	CCineMonster *m_pCine;
	float                      m_flLastYawTime;
	int                        m_iClass;

	static TYPEDESCRIPTION     m_SaveData[];
	static Schedule_t *m_scheduleList[];

	virtual int Save( CSave &save ) override;
	virtual int Restore( CRestore &restore ) override;
	virtual void KeyValue( KeyValueData *pkvd ) override;
	virtual int Classify( void ) { return m_iClass; }
	virtual int BloodColor( void ) { return m_bloodColor; }
	virtual CBaseMonster *MyMonsterPointer( void ) { return this; }
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	virtual void Look( int iDistance );
	virtual void RunAI( void );
	virtual BOOL IsAlive( void ) { return pev->deadflag != 2; }
	virtual BOOL ShouldFadeOnDeath( void );
	virtual float ChangeYaw( int yawSpeed );
	virtual void MonsterThink( void );
	virtual int IRelationship( CBaseEntity *pTarget );
	virtual void MonsterInit( void );
	virtual void MonsterInitDead( void );
	virtual void BecomeDead( void );
	virtual void StartMonster( void );
	virtual CBaseEntity *BestVisibleEnemy( void );
	virtual BOOL FInViewCone( CBaseEntity *pEntity );
	virtual BOOL FInViewCone( Vector *pOrigin );
	virtual int CheckLocalMove( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist );
	virtual void Move( float flInterval );
	virtual void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	virtual BOOL ShouldAdvanceRoute( float flInterval );
	virtual Activity GetStoppedActivity( void ) { return ACT_IDLE; }
	virtual void Stop( void ) { m_IdealActivity = GetStoppedActivity(); }
	virtual BOOL CheckRangeAttack1( float flDot, float flDist );
	virtual BOOL CheckRangeAttack2( float flDot, float flDist );
	virtual BOOL CheckMeleeAttack1( float flDot, float flDist );
	virtual BOOL CheckMeleeAttack2( float flDot, float flDist );
	virtual Schedule_t *ScheduleFromName( const char *pName );
	virtual void StartTask( Task_t *pTask );
	virtual void RunTask( Task_t *pTask );
	virtual Schedule_t *GetScheduleOfType( int iType );
	virtual Schedule_t *GetSchedule( void );
	virtual void ScheduleChange( void ) {}
	virtual int CanPlaySequence( BOOL fDisruptable, int iState );
	virtual int CanPlaySentence( BOOL fDisruptable ) { return GetStoppedActivity(); }
	virtual void PlaySentence( const char *pszSentence, float duration, float volume, float attenuation );
	virtual void PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener );
	virtual void SentenceStop( void );
	virtual MONSTERSTATE GetIdealState( void );
	virtual void SetActivity( Activity NewActivity );
	virtual void ReportAIState( void );
	virtual int CheckEnemy( CBaseEntity *pEnemy );
	virtual BOOL FTriangulate( const Vector &vecStart, const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pResult );
	virtual void SetYawSpeed( void ) { return; }
	virtual BOOL BuildNearestRoute( Vector vecMinEnemy, Vector vecMaxEnemy, float flMaxDist, float flMinDist );
	virtual BOOL FindCover( Vector vecMinEnemy, Vector vecMaxEnemy, float flMaxDist, float flMinDist );
	virtual BOOL FValidateCover( const Vector &vecCoverLocation ) { return TRUE; }
	virtual float CoverRadius( void ) { return 784; }
	virtual BOOL FCanCheckAttacks( void );
	virtual void CheckAmmo( void ) { return; }
	virtual int IgnoreConditions( void );
	virtual BOOL FValidateHintType( short sHintType );
	virtual BOOL FCanActiveIdle( void );
	virtual int ISoundMask( void );
	virtual CSound *PBestSound( void );
	virtual CSound *PBestScent( void );
	virtual float HearingSensitivity( void ) { return 1.0; }
	virtual BOOL FBecomeProne( void ) override;
	virtual void BarnacleVictimBitten( entvars_t *pevBarnacle );
	virtual void BarnacleVictimReleased( void );
	virtual void DoDSetScriptReset( void ) { m_scriptState = SCRIPT_CLEANUP; }
	virtual void PrescheduleThink( void ) { return; }
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) override;
	virtual Activity GetDeathActivity( void );
	virtual void Killed( entvars_t *pevAttacker, int iGib ) override;
	virtual void GibMonster( void );
	virtual BOOL HasHumanGibs( void );
	virtual BOOL HasAlienGibs( void );
	virtual void FadeMonster( void );
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center() * 0.75 + EyePosition() * 0.25; }
	virtual Vector GetGunPosition( void );
	virtual int TakeHealth( float flHealth, int bitsDamageType ) override;
	virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;
	virtual int IsMoving( void ) { return m_movementGoal != MOVEGOAL_NONE; }
	virtual void DeathSound( void ) { return; };
	virtual void AlertSound( void ) { return; };
	virtual void IdleSound( void ) { return; };
	virtual void PainSound( void ) { return; };
	virtual void StopFollowing( BOOL clearSchedule ) {}

	void MonsterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void CorpseUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) { return; }
	void Listen( void );
	float VecToYaw( Vector vecDir );
	float FlYawDiff( void );
	float DamageForce( float flDamage ) { return 0.0f; }
	void CallMonsterThink( void ) { MonsterThink(); }
	void CorpseFallThink( void );
	void MonsterInitThink( void );
	Task_t *GetTask( void ) { return NULL; }
	void SetSequenceByName( char *szSequence );
	void SetState( MONSTERSTATE State );
	void CheckAttacks( CBaseEntity *pTarget, float flDist );
	void PushEnemy( CBaseEntity *pEnemy, Vector &vecPath );
	BOOL PopEnemy( void );
	BOOL FGetNodeRoute( Vector vecDest );
	void TaskComplete( void );
	void MovementComplete( void );
	void TaskFail( void );
	void TaskBegin( void );
	int TaskIsRunning( void );
	int TaskIsComplete( void );
	int MovementIsComplete( void );
	int IScheduleFlags( void );
	BOOL FRefreshRoute( void );
	BOOL FRouteClear( void );
	void RouteSimplify( CBaseEntity *pTargetEnt );
	void AdvanceRoute( float flInterval );
	void MakeIdealYaw( Vector vecDest );
	BOOL BuildRoute( const Vector &vecDest, int iMoveFlag, CBaseEntity *pTargetEnt );
	int RouteClassify( int iLine );
	void InsertWaypoint( Vector vecLocation, int iFlags );
	BOOL FindLateralCover( const Vector &vecMinEnemy, const Vector &vecMaxEnemy );
	int FindHintNode( void );
	void SetTurnActivity( void );
	float FLSoundVolume( CSound *pSound );
	BOOL MoveToNode( Activity activity, float flTimeout, const Vector &vecTarget );
	BOOL MoveToTarget( Activity activity, float flTimeout );
	BOOL MoveToLocation( Activity activity, float flTimeout, const Vector &vecTarget );
	BOOL MoveToEnemy( Activity activity, float flTimeout );
	float OpenDoorAndWait( entvars_t *pevDoor );
	void StopAnimation( void );
	BOOL FHaveSchedule( void );
	BOOL FScheduleValid( void );
	void ClearSchedule( void );
	BOOL FScheduleDone( void );
	void ChangeSchedule( Schedule_t *pNewSchedule );
	void NextScheduledTask( void );
	Schedule_t *ScheduleInList( const char *pName, Schedule_t **pList, int iCount );
	void SetEyePosition( void );
	BOOL FShouldEat( void );
	void Eat( float flDuration );
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iType );
	BOOL FacingIdeal( void );
	BOOL FCheckAITrigger( void );
	BOOL NoFriendlyFire( void );
	BOOL BBoxFlat( void );
	BOOL GetEnemy( void );
	void MakeDamageBloodDecal( int iDamage, float flYaw, TraceResult *ptr, const Vector &vecDir );
	float UpdateTarget( entvars_t *pevTarget );
	Activity GetSmallFlinchActivity( void );
	BOOL ShouldGibMonster( int iGib );
	void CallGibMonster( void );
	Vector ShootAtEnemy( const Vector &vecSrc );
	int DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void RadiusDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iClassIgnore );
	void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iClassIgnore );
	void RouteClear( void );
	void RouteNew( void );
	void Remember( int iMemory );
	void Forget( int iMemory );
	BOOL HasMemory( int iMemory );
	BOOL HasAllMemories( int iMemory );
	BOOL ExitScriptedSequence( void );
	BOOL CineCleanup( void );
	void StartPatrol( CBaseEntity *pPathStart );
	CBaseEntity *DropItem( char *pszItemName, const Vector &vecOrigin, const Vector &vecAngles );
};
#endif // BASEMONSTER_H
