//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#ifndef VOICE_BANMGR_H
#define VOICE_BANMGR_H

class CVoiceBanMgr
{
public:
	CVoiceBanMgr();
	~CVoiceBanMgr();

	bool Init( char const *pGameDir );
	void Term( void );

	void SaveState( char const *pGameDir );

	bool GetPlayerBan( char const playerID[16] );
	void SetPlayerBan( char const playerID[16], bool bSquelch );

	void ForEachBannedPlayer( void (*callback)( char id[16] ) );

protected:
	class BannedPlayer
	{
	public:
		char m_PlayerID[16];
		BannedPlayer *m_pPrev, *m_pNext;
	};

	void Clear( void );
	BannedPlayer *InternalFindPlayerSquelch( char const playerID[16] );
	BannedPlayer *AddBannedPlayer( char const playerID[16] );

	BannedPlayer m_PlayerHash[256];
};
#endif // VOICE_BANMGR_H
