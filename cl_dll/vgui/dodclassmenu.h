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
// dodclassmenu.h
//
// The class menus of DoD (CDoDClassMenu of the original). The layouts are Resource/UI/ClassMenu_*.res of the
// original, the page of a class that is shown under the mouse is Resource/Classes/<command>.res, the menu is
// made of them with CDoDResPage.
//

#ifndef DODCLASSMENU_H
#define DODCLASSMENU_H

#include "vgui/dodmenu.h"
#include "vgui/dodrespage.h"

class CDoDClassMenu : public CDoDMenuPanel
{
public:
	CDoDClassMenu( int iRemoveMe, int x, int y, int wide, int tall );

	virtual bool SlotInput( int iSlot );
	virtual void Open( void );
	virtual void Update( void );
	virtual void SetActiveInfo( int iInput );
	virtual void paintBackground( void );

	virtual void Initialize( void );

	virtual void Reset( void )
	{
		CMenuPanel::Reset();
		m_iActiveClass = 0;
	}

	// The menu to show, MENU_CLASS_ALLIES..MENU_CLASS_AXIS_PARA of DoDViewport; the layout is loaded when it is shown
	void SetMenu( int iMenu );

	// Bits 0..11: the button cls_buttonN is shown, bits 16..27: the class is full
	void SetClassMask( int iMask );

private:
	enum
	{
		NUM_MENUS = 5,
		NUM_CLASSES = 28,
		BUTTON_NONE = 0
	};

	bool LoadMenu( int iMenu );
	void Layout( void );

	CDoDResPage *GetClassPage( int iClass, const char *pszCommand );
	void HideClassPage( void );

	bool IsClassFull( int iClass );
	void UpdateNumClassLabel( void );

	// the menu of the layouts: 0 allies, 1 allies paratroopers, 2 british, 3 axis, 4 axis paratroopers
	int m_iMenu;
	int m_iLoadedMenu;
	CDoDResPage *m_pMenuPages[NUM_MENUS];

	// the pages of the classes by the number of the class in g_ClassInfo
	CDoDResPage *m_pClassPages[NUM_CLASSES];
	CDoDResPage *m_pCurrentClassPage;

	int m_iClassMask;

	// the class that the mouse is over, 0 when there is none; the label with the players of the class
	int m_iActiveClass;
	char m_szNumOnTeam[256];

	int m_iLayoutHeight;
};

#endif // DODCLASSMENU_H
