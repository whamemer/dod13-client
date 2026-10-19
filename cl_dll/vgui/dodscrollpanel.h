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
// dodscrollpanel.h
//
// The scroll panel of the DoD menus: the scroll bar has the black square arrow buttons with the brown border
// (ClassMenuDark of Resource/ClientScheme.res) like the VGUI2 one of the original, it is always shown.
//

#ifndef DODSCROLLPANEL_H
#define DODSCROLLPANEL_H

#include <VGUI_Button.h>
#include <VGUI_Slider.h>
#include <VGUI_ScrollPanel.h>
#include <VGUI_TextPanel.h>
#include <VGUI_Font.h>

#include <string>
#include <vector>

class CDoDScrollButton : public vgui::Button
{
public:
	CDoDScrollButton( bool bUp );

	virtual void paint( void );
	virtual void paintBackground( void );

private:
	bool m_bUp;
};

class CDoDSlider : public vgui::Slider
{
public:
	CDoDSlider( bool bVertical );

	virtual void paintBackground( void );
};

class CDoDScrollPanel : public vgui::ScrollPanel
{
public:
	CDoDScrollPanel( int x, int y, int wide, int tall );

	// the width of the vertical scroll bar, the arrow buttons are squares of this size
	void SetBarWidth( int wide );
	int GetBarWidth( void );

	// The mouse wheel scrolls the text (as in the RichText and HTML controls of VGUI2): the wheel goes to the panel
	// under the cursor only, so the panels that hold the text are added
	void AddWheelTarget( vgui::Panel *pPanel );
	void ScrollByWheel( int iDelta );

private:
	vgui::InputSignal *m_pWheelSignal;
};

// A text that is drawn line by line with the pitch of the lines that is set: the lines of the original are farther apart than
// the height of the font of VGUI1 (12 units for the font of 13)
// The lines of a text: it is broken at the new lines and every line is wrapped to the width (in pixels) with the font
void DoD_WrapText( vgui::Font *pFont, const char *pszText, int iWide, std::vector<std::string> &lines );

class CDoDLinesPanel : public vgui::Panel
{
public:
	CDoDLinesPanel( void );

	void SetLinesFont( vgui::Font *font );
	void SetLinesColor( int r, int g, int b );

	// the lines are broken at the new lines of the text and wrapped to the width, the height of them is returned
	int SetLines( const char *pszText, int iWide, int iPitch );

	virtual void paintBackground( void );
	virtual void paint( void );

private:
	std::vector<std::string> m_Lines;
	vgui::Font *m_pFont;
	int m_iPitch;
	int m_r, m_g, m_b;
};

// RichText of the original: a text that is wrapped to the width of the panel without the scroll bar
class CDoDRichText : public CDoDScrollPanel
{
public:
	CDoDRichText( int x, int y, int wide, int tall );

	// the size includes the scroll bar
	void SetRichBounds( int x, int y, int wide, int tall, int barWide );
	void SetRichFont( vgui::Font *font );
	void SetRichColor( int r, int g, int b );

	// the text starts a bit to the right of the left edge, it is wrapped to what is left of the width
	void SetRichInset( int iInset );

	// the lines are drawn with this pitch in pixels instead of the height of the font, 0 is the text of VGUI1
	void SetRichLineTall( int iPitch );

	// the text is wrapped and the panel is scrolled to the top
	void SetRichText( const char *text );

private:
	vgui::TextPanel *m_pText;
	CDoDLinesPanel *m_pLines;
	int m_iLinePitch;
	vgui::Font *m_pFont;
	int m_iTextWide;
	int m_iInset;
};

#endif // DODSCROLLPANEL_H
