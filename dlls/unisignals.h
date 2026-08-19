//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined(UNISIGNALS_H)
#define UNISIGNALS.H

class CUnifiedSignals 
{
public:
	int m_flSignal;
	int m_flState;

	int Update( void );
	void Signal( int signal );
	int GetState( void );
};

#endif // UNISIGNALS_H