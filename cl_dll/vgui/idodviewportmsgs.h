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
#if !defined(IDODVIEWPORTMSGS_H)
#define IDODVIEWPORTMSGS_H

#include "hud.h"

// The user messages that the client DLL forwards to the DoD viewport
class IDoDViewPortMsgs
{
public:
	virtual CHudMsgFunc( ForceCam ) = 0;
	virtual CHudMsgFunc( ScoreInfo ) = 0;
	virtual CHudMsgFunc( ScoreInfoLong ) = 0;
	virtual CHudMsgFunc( TeamScore ) = 0;
	virtual CHudMsgFunc( MapMarker ) = 0;
	virtual CHudMsgFunc( WaveTime ) = 0;
	virtual CHudMsgFunc( WaveStatus ) = 0;
	virtual CHudMsgFunc( WideScreen ) = 0;
	virtual CHudMsgFunc( ScoreShort ) = 0;
	virtual CHudMsgFunc( Frags ) = 0;
	virtual CHudMsgFunc( ObjScore ) = 0;
	virtual CHudMsgFunc( PStatus ) = 0;
	virtual CHudMsgFunc( PTeam ) = 0;
	virtual CHudMsgFunc( PClass ) = 0;
	virtual CHudMsgFunc( CurMarker ) = 0;
	virtual CHudMsgFunc( VGUIMenu ) = 0;
};

extern IDoDViewPortMsgs *gDoDViewPortMsgs;

#endif // IDODVIEWPORTMSGS_H
