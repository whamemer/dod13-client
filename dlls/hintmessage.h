//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( HINTMESSAGEQUEUE_H )
#define HINTMESSAGEQUEUE_H

#include "cbase.h"

class CHintMessageQueue
{
private:
    void Reset( void );
    void Update( CBaseEntity *client );
    bool AddMessage( const char *message );
    bool IsEmpty( void );

    int m_iCount;
    int m_iCurrentMessage;
    int m_iLastMessage;

    float m_tmMessageEnd;

    const char *m_aszMessage[8];
};

#endif // !HINTMESSAGEQUEUE_H