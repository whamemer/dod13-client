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
//
// dodviewport.h
//
// The DoD viewport on top of the VGUI1 viewport. The viewport of the original is VGUI2
// (DoDViewport : TeamFortressViewport of game_controls), the panels are restored with freevgui one by one,
// until a panel exists the viewport only keeps its state.
//

#ifndef DODVIEWPORT_H
#define DODVIEWPORT_H

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/idodviewport.h"
#include "vgui/idodviewportmsgs.h"

// the menus of the VGUIMenu message that are added by DoD, the others are the menus of TeamFortressViewport
enum
{
	MENU_CLASS_ALLIES = 10,	// Resource/UI/ClassMenu_Allies.res
	MENU_CLASS_ALLIES_PARA,	// Resource/UI/ClassMenu_Allies_Para.res
	MENU_CLASS_ALLIES_BRIT,	// Resource/UI/ClassMenu_Allies_Brit.res
	MENU_CLASS_AXIS,	// Resource/UI/ClassMenu_Axis.res
	MENU_CLASS_AXIS_PARA,	// Resource/UI/ClassMenu_Axis_Para.res
	MENU_MORTAR	// the mortar panel, the value of the message is 0 to hide it
};

class DoDViewport : public TeamFortressViewport, public IDoDViewPort, public IDoDViewPortMsgs
{
public:
	DoDViewport( int x, int y, int wide, int tall );
	virtual ~DoDViewport();

	// DisplayVGUIMenu of the original
	virtual void ShowVGUIMenu( int iMenu );

	// CDoDMOTD with the title and the message that the server has sent
	virtual CMenuPanel *ShowMOTDMenu( void );

	// the target of the spectator GUI and its timer
	virtual void UpdateSpectatorPanel( void );

	// the hint panel hides itself when the time of the message is over
	virtual void paintBackground( void );

	// IDoDViewPort
	virtual int GetForceCamera( void );
	virtual void setWidescreenVisible( bool state );
	virtual bool isWidescreenVisible( void );
	virtual void setWidescreenSubtitle( int subtitle );
	virtual void updateTeamMenu( void );
	virtual void ShowHintMessage( const char *msg );
	virtual void ShowSpectatorGUI( void );
	virtual void DeactivateSpectatorGUI( void );
	virtual void PaintBackgroundPanel( void );
	virtual int GetTeamScore( int team );
	virtual void ShowMortarPanel( void );
	virtual void HideMortarPanel( void );
	virtual void PostMortarValue( float value );
	virtual ExtraScoreBoardInfo GetExtraScoreBoardInfo( int teamIndex );
	virtual VGuiLibraryTeamInfo_t GetPlayerTeamInfo( int teamIndex );

	// IDoDViewPortMsgs
	virtual CHudMsgFunc( ForceCam );
	virtual CHudMsgFunc( ScoreInfo );
	virtual CHudMsgFunc( ScoreInfoLong );
	virtual CHudMsgFunc( TeamScore );
	virtual CHudMsgFunc( MapMarker );
	virtual CHudMsgFunc( WaveTime );
	virtual CHudMsgFunc( WaveStatus );
	virtual CHudMsgFunc( WideScreen );
	virtual CHudMsgFunc( ScoreShort );
	virtual CHudMsgFunc( Frags );
	virtual CHudMsgFunc( ObjScore );
	virtual CHudMsgFunc( PStatus );
	virtual CHudMsgFunc( PTeam );
	virtual CHudMsgFunc( PClass );
	virtual CHudMsgFunc( CurMarker );
	virtual CHudMsgFunc( VGUIMenu );

	void SetMapMarker( int marker );
	HSPRITE GetMapMarkerSpr( void );

	void SetClassMask( int iMask ) { m_iClassMask = iMask; }
	int GetClassMask( void ) { return m_iClassMask; }

private:
	class CMortarPanel *GetMortarPanel( void );

	int m_iForceCamera;
	int m_iForceChaseCam;
	int m_iFadeToBlack;

	// the classes of the class menu: bits 0..11 - the class is shown, bits 16..27 - the class is full
	int m_iClassMask;

	class CDoDMOTD *m_pMOTD;
	class CHintPanel *m_pHintPanel;
	class CMortarPanel *m_pMortarPanel;

	// the widescreen panel is not restored yet, only its state is kept
	bool m_bWidescreenVisible;
	int m_iWidescreenSubtitle;
};

#endif // DODVIEWPORT_H
