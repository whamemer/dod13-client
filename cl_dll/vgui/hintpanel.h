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
// hintpanel.h
//
// The hint panel of DoD (CHintPanel of the original): the message that the server sends with HudText and a hint flag is shown
// in a box at the top left with the head of a soldier of the team of the player. The layout is Resource/UI/HintPanel.res
// (the box, the label and the images of the three teams), the panel hides itself after one second for 20 characters.
//

#ifndef HINTPANEL_H
#define HINTPANEL_H

#include <VGUI_Panel.h>

#include "utlstring.h"
#include "utlvector.h"

#include "vgui/dodrespage.h"

class CHintPanel : public vgui::Panel
{
public:
	CHintPanel( int x, int y, int wide, int tall );

	// team: 0 is the British, 1 the Americans, 2 the Germans; the text is a token or the text itself
	void ShowHintMessage( int team, const char *pszText );

	// the panel is hidden when the time of the message is over
	void OnTick( void );

	virtual void paintBackground( void );
	virtual void paint( void );

	// the panel takes no input (SetMouseInputEnabled( false ) of the original)
	virtual vgui::Panel *isWithinTraverse( int x, int y ) { return NULL; }

private:
	void Layout( void );
	void SetVisibleImagePanel( const char *pszName, bool bState );

	CDoDResPage *m_pPage;
	ResControl_t *m_pFrame;
	ResControl_t *m_pLabel;

	// the text is wrapped when it is shown and again when the size of the screen changes
	CUtlString m_Text;
	CUtlVector<CUtlString> m_Lines;

	float m_fShowTime;
	int m_iLayoutHeight;
};

#endif // HINTPANEL_H
