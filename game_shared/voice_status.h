//========= Copyright (c) 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_STATUS_H
#define VOICE_STATUS_H
#pragma once

#include "hud.h"

#include "voice_banmgr.h"
#include "bitvec.h"
#include "voice_common.h"

class IVoiceStatusHelper
{
public:
    virtual ~IVoiceStatusHelper() {}

    virtual void GetPlayerTextColor( int entindex, int color[3] ) = 0;
    virtual void UpdateCursorState( void ) = 0;
    virtual int GetAckIconHeight( void ) = 0;
    virtual bool CanShowSpeakerLabels( void ) = 0;
    virtual const char *GetPlayerLocation( int entindex ) = 0;
};

class IVoiceStatus
{
    virtual int GetSpeakerStatus( int iPlayer );
    virtual bool IsTalking( void );
    virtual bool ServerAcked( void );
};

class IVoiceHud
{
public:
    virtual int Init( IVoiceStatusHelper *pHelper, IVoiceStatus *pStatus );
    virtual int VidInit( void );
    virtual void CreateEntities( void );
    virtual void UpdateLocation( int entindex, const char *location );
    virtual void UpdateSpeakerStatus( int entindex, bool bTalking );
    virtual void RepositionLabels( void );
};

class CVoiceStatus : public CHudBase, public IVoiceStatus
{
public:
    CVoiceStatus();
    virtual ~CVoiceStatus();

    virtual int Init( IVoiceStatusHelper *m_pHelper );
    virtual int	VidInit( void );

    void UpdateSpeakerStatus( int entindex, bool bTalking );
    virtual int GetSpeakerStatus( int iPlayer );
    void HandleVoiceMaskMsg( int iSize, void *pbuf );
    void HandleReqStateMsg( int iSize, void *pbuf );
    void Frame( double frametime );

    void StartSquelchMode( void );
    void StopSquelchMode( void );
    bool IsInSquelchMode( void );

    bool IsPlayerBlocked( int iPlayerIndex );
    bool IsPlayerAudible( int iPlayerIndex );

    void SetPlayerBlockedState( int iPlayerIndex, bool blocked );

    void UpdateServerState( bool bForce );

    virtual bool CanShowSpeakerLabels( void );
    virtual void GetPlayerTextColor( int entindex, int *color );
    virtual int GetAckIconHeight( void );
    virtual bool IsTalking( void );
    virtual bool ServerAcked( void );

    CVoiceBanMgr *GetBanMgr( void );

private:
    enum
    {
        VOICE_TALKING = 1,
        VOICE_BANNED,
        VOICE_NEVERSPOKEN,
        VOICE_NOTTALKING
    };

    float m_LastUpdateServerState;
    int m_bServerModEnable;

    CPlayerBitVec m_VoicePlayers;
    CPlayerBitVec m_AudiblePlayers;
    CPlayerBitVec m_VoiceEnabledPlayers;
    CPlayerBitVec m_ServerBannedPlayers;

    IVoiceStatusHelper *m_pHelper;

    bool m_bInSquelchMode;
    bool m_bTalking;
    bool m_bServerAcked;

    CVoiceBanMgr m_BanMgr;

    bool m_bBanMgrInitialized;

    char *m_pchGameDir;
};
#endif // VOICE_STATUS_H
