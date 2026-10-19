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
#if !defined(HINTMESSAGE_H)
#define HINTMESSAGE_H

#define MAX_HINT_MESSAGES 8

class CBaseEntity;

class CHintMessageQueue
{
public:
	void Reset( void );
	void Update( CBaseEntity &player );
	bool AddMessage( const char *message );
	bool IsEmpty( void ) { return m_iCount == 0; }

private:
	int m_iCount;
	int m_iCurrentMessage;
	int m_iLastMessage;
	float m_tmMessageEnd;
	const char *m_aszMessage[MAX_HINT_MESSAGES];
};

#endif // HINTMESSAGE_H
