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
// dodmenu.h
//
// What the menus of DoD share: the fonts and the colors of Resource/ClientScheme.res, the localized texts and
// the panel with the black polygons of the background (DoDViewport::Start of the original). The layouts of the
// original are in 640x480 and are scaled with the height of the screen (YRES) on both axes.
//

#ifndef DODMENU_H
#define DODMENU_H

#include "vgui/vgui_TeamFortressViewport.h"

#define DODMENU_SIZE( x ) YRES( x )

// The size of the text relative to the layout: the fonts of the original are smaller than their tall scaled with the
// height of the screen, 0.88 is measured on the screenshots of the original
#define DODMENU_FONT_SCALE 0.88f

// The colors of ClientScheme.res; the alpha of VGUI1 is the transparency, 0 is opaque
#define DODCOLOR_DARK		64, 52, 36	// ClassMenuDark: dividers, borders, tracks
#define DODCOLOR_BUTTON		55, 43, 29	// the background of a button (not ClassMenuDark)
#define DODCOLOR_LIGHT		175, 127, 1	// ClassMenuLight: a button under the mouse, the bars
#define DODCOLOR_BRIGHT		255, 255, 255	// BrightControlText
#define DODCOLOR_TEXT		255, 176, 0	// BaseText

// The fonts of the scheme (Times New Roman)
enum
{
	DODFONT_DEFAULT = 0,	// ClassMenuDefault
	DODFONT_TITLE,	// ClassMenuTitleFont
	DODFONT_BOLD,	// ClassMenuBoldFont
	DODFONT_BODY,	// ClassMenuBodyFont
	DODFONT_SCORE,	// Default: Times New Roman 13, the rows of the scoreboard
	DODFONT_SCORESMALL,	// DefaultVerySmall: Courier New 11, the header of the scoreboard
	NUM_DODFONTS
};

// The fonts depend on the height of the screen, they are made again when it changes
vgui::Font *DoD_GetMenuFont( int iFont );

// The index of a font by the name in the scheme, ClassMenuDefault when the name is unknown
int DoD_FindMenuFont( const char *pszSchemeName );

// The text of the one-byte charset that VGUI1 draws: the UTF-8 sequences are converted to Latin-1 unless the cvar
// vgui_utf8 of the engine is set, a byte that is not UTF-8 stays as it is
void DoD_ConvertText( const char *pszText, char *pszDest, int iDestSize );

// The byte of a code point in Latin-1, '?' when there is none; the typographic marks that it has no place for are
// the plain ones
char DoD_CodePointToLatin1( int iCodePoint );

// The localized text of a token ("#Token") without the mnemonic marks ("&1 Allies" is "1 Allies"),
// the text itself when it is not a token
void DoD_GetLocalized( const char *pszToken, char *pszDest, int iDestSize );

// The color of a team of ClientScheme.res (team0..team3 and team4 for the others, GetStringTeamColor of the original): the allies
// are green, the axis are red, the others are orange
void DoD_GetTeamColor( int iTeam, int &r, int &g, int &b );

// The digit that the token marks with '&' ("&1 Allies" is 1), -1 when there is none
int DoD_GetMnemonic( const char *pszToken );

// The text of a label is centered vertically and VGUI2 does not clip it, VGUI1 does: the label is made as tall as
// the font. The size of the text of a label is made when the text is set and it is not changed with the font,
// so the text has to be set after this call.
void DoD_PlaceLabel( vgui::Label *pLabel, vgui::Font *pFont, int x, int y, int wide, int tall );

// A label that is narrower than its text clips it in VGUI1: the label is made as wide as the text, it grows to the
// side that the alignment of the text points away from. Call it after the text is set.
void DoD_FitLabel( vgui::Label *pLabel, vgui::Label::Alignment alignment );

// The panel of a menu of DoD: the black polygons of the background with the cut corners
class CDoDMenuPanel : public CMenuPanel
{
public:
	CDoDMenuPanel( int iRemoveMe, int x, int y, int wide, int tall );

	virtual void paintBackground( void );
};

#endif // DODMENU_H
