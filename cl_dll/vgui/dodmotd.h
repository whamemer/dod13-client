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
// dodmotd.h
//
// The message of the day of DoD (CDoDMOTD of the original). The layout is Resource/UI/MOTD.res of the original: the
// name of the server, the page and the OK button. The page is HTML, the original shows it with a browser control
// (a URL is opened, a text is written to a temporary file and opened), here CDoDHtmlView lays it out.
//

#ifndef DODMOTD_H
#define DODMOTD_H

#include "vgui/dodmenu.h"
#include "vgui/dodrespage.h"

class CDoDMOTD : public CDoDMenuPanel
{
public:
	CDoDMOTD( int iRemoveMe, int x, int y, int wide, int tall );

	virtual void Open( void );
	virtual void paintBackground( void );

	// CClientMOTD::Activate: the title is the name of the server, the message is the page, a URL or a text
	void Activate( const char *pszTitle, const char *pszMessage );

private:
	void Layout( void );

	CDoDResPage *m_pPage;

	ResControl_t *m_pServerName;
	ResControl_t *m_pMessage;

	int m_iLayoutHeight;
};

#endif // DODMOTD_H
