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
// dodbutton.h
//
// The button of the DoD menus (DoDButton of the original): the dark brown background, the light brown one under
// the mouse, white text. A button of a class that is full is dim and its text is red under the mouse
// (CDoDClassMenu::SetButtonGreyout of the original).
//

#ifndef DODBUTTON_H
#define DODBUTTON_H

#include <VGUI_Button.h>
#include <VGUI_Font.h>

class CDoDButton : public vgui::Button
{
public:
	CDoDButton( const char *text, int x, int y, int wide, int tall, vgui::Font *font );

	void SetButtonText( const char *text );
	void SetButtonFont( vgui::Font *font );

	// the command of the button in the layout of the original (Command)
	void SetCommand( const char *command );
	const char *GetCommand( void ) { return m_szCommand; }

	// a plain Button of a layout (ControlName "Button"): ClassMenuDark background and the text in the middle
	void SetPlain( bool bPlain );

	// the button that has the focus (the default one) has a dotted rectangle inside of it
	void SetFocusRect( bool bFocusRect );

	void SetGreyedOut( bool bGreyedOut );
	bool IsGreyedOut( void ) { return m_bGreyedOut; }

	virtual void paint( void );
	virtual void paintBackground( void );

private:
	char m_szText[64];
	char m_szCommand[64];
	bool m_bGreyedOut;
	bool m_bPlain;
	bool m_bFocusRect;
	vgui::Font *m_pFont;
};

#endif // DODBUTTON_H
