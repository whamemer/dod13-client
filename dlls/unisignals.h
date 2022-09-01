//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( UNISIGNALSH )
#define UNISIGNALSH

class CUnifiedSignals
{
private:
    int Update( void );
    void Signal( int m_iSignal );
    int GetState( void );

    float m_flSignal;
    float m_flState;
};
#endif // !UNISIGNALSH