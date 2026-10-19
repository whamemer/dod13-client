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
#if !defined(IDODVIEWPORT_H)
#define IDODVIEWPORT_H

struct ExtraScoreBoardInfo
{
	int score;
};

// the same as team_info_t of the HUD, the layout is from public/cl_dll/IVGuiClientDll.h of the original
typedef struct
{
	const char *name;
	int frags;
	int deaths;
	int ping;
	int players;
	int packetloss;
	int teamnumber;
} VGuiLibraryTeamInfo_t;

// Interface between the client DLL and the DoD viewport (menus, scoreboard, spectator GUI, mortar panel...)
class IDoDViewPort
{
public:
	virtual int GetForceCamera( void ) = 0;
	virtual void setWidescreenVisible( bool state ) = 0;
	virtual bool isWidescreenVisible( void ) = 0;
	virtual void setWidescreenSubtitle( int subtitle ) = 0;
	virtual void updateTeamMenu( void ) = 0;
	virtual void ShowHintMessage( const char *msg ) = 0;
	virtual void ShowSpectatorGUI( void ) = 0;
	virtual void DeactivateSpectatorGUI( void ) = 0;
	virtual void PaintBackgroundPanel( void ) = 0;
	virtual int GetTeamScore( int team ) = 0;
	virtual void ShowMortarPanel( void ) = 0;
	virtual void HideMortarPanel( void ) = 0;
	virtual void PostMortarValue( float value ) = 0;
	virtual ExtraScoreBoardInfo GetExtraScoreBoardInfo( int playerIndex ) = 0;
	virtual VGuiLibraryTeamInfo_t GetPlayerTeamInfo( int playerIndex ) = 0;
};

extern IDoDViewPort *gDoDViewPortInterface;

#endif // IDODVIEWPORT_H
