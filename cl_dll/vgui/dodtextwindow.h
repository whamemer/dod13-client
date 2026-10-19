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
// dodtextwindow.h
//
// The text window of the original (CVGUITextWindow with TextWindow.res): a transparent frame without a title, the text
// in a box with a scroll bar and the OK button. DoD uses it for the help of the spectator (CSpecHelpWindow): the window
// gives the spectator GUI back (Activate) when it is closed.
//

#ifndef DODTEXTWINDOW_H
#define DODTEXTWINDOW_H

#include <string>

#include "vgui/dodmenu.h"
#include "vgui/dodrespage.h"

class CDoDTextWindow : public CMenuPanel
{
public:
	CDoDTextWindow( int iRemoveMe, int x, int y, int wide, int tall );

	virtual void Open( void );
	virtual void Close( void );
	virtual void paintBackground( void );

	// the help of the spectator gives the spectator GUI back when it is closed, the other texts do not
	void SetSpectatorHelp( bool bSpectatorHelp ) { m_bSpectatorHelp = bSpectatorHelp; }

	// CVGUITextWindow::Activate: the message of the box, the line breaks of the files are CR LF and both of them break a line
	void Activate( const char *pszMessage );

private:
	void Layout( void );
	void ApplyMessage( void );

	CDoDResPage *m_pPage;
	ResControl_t *m_pMessage;
	ResControl_t *m_pOK;

	// the border of the box is drawn over the text, a text that is scrolled out at the edge does not touch it
	vgui::Panel *m_pOutline;

	std::string m_Message;
	int m_iLayoutHeight;
	bool m_bSpectatorHelp;
};

#endif // DODTEXTWINDOW_H
