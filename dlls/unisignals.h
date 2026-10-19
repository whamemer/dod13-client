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
#if !defined(UNISIGNALS_H)
#define UNISIGNALS_H

class CUnifiedSignals
{
public:
	CUnifiedSignals();

	int Update( void );
	void Signal( int flag );
	int GetState( void );

private:
	int m_flSignal;
	int m_flState;
};

inline CUnifiedSignals::CUnifiedSignals()
{
	m_flSignal = 0;
	m_flState = 0;
}

#endif // UNISIGNALS_H
