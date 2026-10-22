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
// dodhtml.h
//
// HTML control of the original (the page of the MOTD is HTML in VGUI2, there is a browser behind it), VGUI1 has
// none: the part of HTML and CSS that the pages of the servers use is laid out and drawn here. The text is wrapped
// to the width of the view, the view is scrolled with the arrow buttons of CDoDScrollPanel.
//
// The tags: h1..h6, p, div, br, hr, b, strong, i, em, u, a, font, center, span, small, big, sub, sup, code, pre, ul,
// ol, li, blockquote, table (the rows are lines, the cells are separated), body (bgcolor, text, link), the entities.
// <style>: the rules by tag, class and id, the colors, the font and its parts, the margins, the alignment, the line
// height, display:none. The images, the links and the scripts are not supported.
//

#ifndef DODHTML_H
#define DODHTML_H

#include "utlstring.h"

#include "vgui/dodscrollpanel.h"

class CDoDHtmlContent;

class CDoDHtmlView : public CDoDScrollPanel
{
public:
	CDoDHtmlView( int x, int y, int wide, int tall );

	// the size includes the scroll bar
	void SetHtmlBounds( int x, int y, int wide, int tall, int barWide );

	// The page: the UTF-8 text is converted to the charset of VGUI1, the view is scrolled to the top
	void SetHtml( const char *pszHtml );

	virtual void paintBackground( void );

private:
	void Relayout( void );

	CUtlString m_strHtml;
	CDoDHtmlContent *m_pContent;

	int m_iContentWide;
	int m_iBackground[3];
};

#endif // DODHTML_H
