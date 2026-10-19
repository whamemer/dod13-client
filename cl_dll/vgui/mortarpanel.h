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
// mortarpanel.h
//
// Historical reference: the mortar class of DoD was developed but never shipped, the server library does not allow to choose it and
// its command "maim" does nothing, so nothing shows this panel in the game. It is kept as it is in client.so (restored from it
// for completeness) and is not tested in the game.
//
// The mortar panel of DoD (CMortarPanel of the original): the bar of the power of the shot of the mortar. The player holds the
// attack button, the pointer runs over the bar (it bounces at its ends and slows down a bit with every pass) and the shot is
// sent to the server with the value of the pointer ("maim") when the button is released. The layout is
// Resource/UI/MortarPanel.res (the box and the label, the box is the one of the hint panel).
//

#ifndef MORTARPANEL_H
#define MORTARPANEL_H

#include <VGUI_Panel.h>

#include "vgui/dodrespage.h"

class CMortarPanel : public vgui::Panel
{
public:
	CMortarPanel( int x, int y, int wide, int tall );

	void ShowMortarPanel( void );
	void HideMortarPanel( void );

	// IDoDViewPort::PostMortarValue: a value over 0 starts the pointer, a value under 0 hides the panel
	void PostMortarValue( float flValue );

	// the pointer is moved while the panel is shown
	void OnTick( void );

	virtual void paintBackground( void );
	virtual void paint( void );

	// the panel takes no input (SetMouseInputEnabled( false ) of the original)
	virtual vgui::Panel *isWithinTraverse( int x, int y ) { return NULL; }

private:
	void Layout( void );
	void AttackDown( void );
	void AttackUp( void );
	void SetLabelText( const char *pszText );

	CDoDResPage *m_pPage;
	ResControl_t *m_pFrame;
	ResControl_t *m_pLabel;

	char m_szLabel[256];

	bool m_bAttack;
	double m_fAimValue;		// 0..1000
	double m_fAimVelocity;	// units per second
	int m_iNumPasses;

	int m_iLayoutHeight;
};

#endif // MORTARPANEL_H
