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
// dodrespage.h
//
// A panel with the controls of a layout of the original (Resource/UI/*.res, Resource/Classes/*.res): the
// stand-in for LoadControlSettings of VGUI2. The controls that the menus use are made: Label, ImagePanel,
// ProgressBar, RichText and the buttons (MouseOverPanelButton, DoDButton).
//

#ifndef DODRESPAGE_H
#define DODRESPAGE_H

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_BitmapTGA.h>

#include "vgui/dodbutton.h"
#include "vgui/dodscrollpanel.h"
#include "vgui/dodhtml.h"

// ImagePanel: a TGA file that is scaled to the size of the panel and/or a fill color
class CDoDImagePanel : public vgui::Panel
{
public:
	CDoDImagePanel( int x, int y, int wide, int tall );

	void SetImage( const char *pszFile );
	void SetFillColor( int r, int g, int b );

	virtual void paintBackground( void );
	virtual void paint( void );

private:
	vgui::BitmapTGA *m_pBitmap;
	int m_iTexture;

	bool m_bFill;
	int m_iFillColor[3];
};

// ProgressBar of the original with the colors of DoDProgressBar: the dark track and the light bar
class CDoDProgressBar : public vgui::Panel
{
public:
	CDoDProgressBar( int x, int y, int wide, int tall );

	void SetProgress( float flProgress );

	virtual void paintBackground( void );

private:
	float m_flProgress;
};

enum
{
	RESCONTROL_OTHER = 0,	// Frame, Panel, Menu: only the rectangle is kept
	RESCONTROL_LABEL,
	RESCONTROL_IMAGE,
	RESCONTROL_BAR,
	RESCONTROL_TEXT,	// RichText
	RESCONTROL_BUTTON,
	RESCONTROL_HTML	// CDoDHtmlView, the page is set by the menu
};

// A control of the layout, the sizes are in 640x480
struct ResControl_t
{
	char szName[48];
	int iType;

	int x;
	int y;
	int wide;
	int tall;

	int iFont;
	int iAlignment;
	bool bBright;
	bool bDull;
	bool bVisible;

	// labelText or text of the layout: a token ("#Token") or the text itself
	char szText[256];
	char szCommand[64];
	float flProgress;

	// MouseOverPanelButton: the mouse over the button shows the page of its class
	bool bMouseOver;

	// the text was set by the menu: it is shown as it is, a token is not looked up and '&' is not a mnemonic
	bool bLiteral;

	vgui::Panel *pPanel;
};

class CDoDResPage : public vgui::Panel
{
public:
	enum
	{
		MAX_CONTROLS = 64
	};

	CDoDResPage( int x, int y, int wide, int tall );

	// Makes the controls of the file, false when the file does not exist
	bool LoadControls( const char *pszFile );

	// Places the controls again: the sizes, the fonts and the texts depend on the height of the screen
	void Layout( void );

	int GetNumControls( void ) { return m_iNumControls; }
	ResControl_t *GetControl( int iIndex ) { return &m_Controls[iIndex]; }

	// The last control with the name, the layouts have two of ClassInfo
	ResControl_t *FindControl( const char *pszName );

	// Changes the text of a label or a text and shows it
	void SetControlText( ResControl_t *pControl, const char *pszText );

private:
	void AddControl( class CDoDKeyValues *pKeys );
	void ApplyControl( ResControl_t *pControl );

	ResControl_t m_Controls[MAX_CONTROLS];
	int m_iNumControls;
};

#endif // DODRESPAGE_H
