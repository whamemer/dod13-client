//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined(HINTMESSAGE_H)
#define HINTMESSAGE.H

#include "cbase.h"

#define HINTMSG_LENGHT 8

class CHintMessageQueue
{
public:
	int m_iCount;
	int m_iCurrentMessage;
	int m_iLastMessage;
	float m_tmMessageEnd;
	const char* m_aszMessage[HINTMSG_LENGHT];

	void Reset( void );
	void Update( CBaseEntity& ent );
	bool AddMessage( const char* msg );
	bool IsEmpty( void );
};

#endif // HINTMESSAGE.H