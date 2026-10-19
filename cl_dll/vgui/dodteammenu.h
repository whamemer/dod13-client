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
// dodteammenu.h
//
// The team menu of DoD (CDoDTeamMenu of the original). The controls are the ones of Resource/UI/TeamMenu.res of the original
// (CDoDResPage loads them: the labels, the buttons with their commands and the divider lines); the menu makes the description of
// the map and the list of the players of the team under the mouse itself, they have the places of the controls MapInfo and
// numberOnTeam. The coordinates are in 640x480 and are scaled with the height of the screen (YRES) on both axes.
//

#ifndef DODTEAMMENU_H
#define DODTEAMMENU_H

#include "vgui/dodmenu.h"
#include "vgui/dodbutton.h"
#include "vgui/dodrespage.h"
#include "vgui/dodscrollpanel.h"

class CDoDTeamMenu : public CDoDMenuPanel
{
public:
	CDoDTeamMenu( int iRemoveMe, int x, int y, int wide, int tall );

	virtual bool SlotInput( int iSlot );
	virtual void Open( void );
	virtual void Update( void );
	virtual void SetActiveInfo( int iInput );
	virtual void paintBackground( void );

	virtual void Initialize( void );

	virtual void Reset( void )
	{
		CMenuPanel::Reset();
		m_iActiveTeam = 0;
	}

private:
	enum
	{
		BUTTON_ALLIES = 0,
		BUTTON_BRITISH,
		BUTTON_AXIS,
		BUTTON_SPECTATE,
		BUTTON_AUTOASSIGN,
		NUM_BUTTONS
	};

	void Layout( void );
	void UpdateNumOnTeamLabel( void );
	void SetVisibleButton( int iButton, bool bVisible );
	CDoDButton *GetButton( int iButton );

	CDoDResPage *m_pPage;

	// the controls of the layout that the menu uses
	ResControl_t *m_pMapName;
	ResControl_t *m_pButtons[NUM_BUTTONS];
	ResControl_t *m_pCancelButton;
	ResControl_t *m_pMapInfo;
	ResControl_t *m_pNumOnTeamControl;

	CDoDRichText *m_pBriefing;
	TextPanel *m_pNumOnTeam;

	char m_szNumOnTeam[512];

	bool m_bUpdatedMapName;

	// the team of the button under the mouse: 1 allies, 2 axis, 3 spectators, the others have no list of players
	int m_iActiveTeam;

	// the height of the screen the layout was made for
	int m_iLayoutHeight;
};

#endif // DODTEAMMENU_H
