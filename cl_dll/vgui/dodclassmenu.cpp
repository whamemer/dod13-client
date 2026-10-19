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
// dodclassmenu.cpp
//
// The class menus of DoD, see dodclassmenu.h. The bit of a class in the mask of the menu and the number of the
// class in g_ClassInfo are the ones of CDoDClassMenu::IsClassFull and CDoDClassMenu::UpdateNumClassLabel of the
// original.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"
#include "dod_shared.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodviewport.h"
#include "vgui/dodclassmenu.h"

namespace
{

// the layouts of the menus 10..14 of the VGUIMenu message
const char *s_szMenuFiles[] =
{
	"Resource/UI/ClassMenu_Allies.res",
	"Resource/UI/ClassMenu_Allies_Para.res",
	"Resource/UI/ClassMenu_Allies_Brit.res",
	"Resource/UI/ClassMenu_Axis.res",
	"Resource/UI/ClassMenu_Axis_Para.res"
};

// the number in g_ClassInfo of the random class, the mouse over it shows no page and no players
const int CLASS_RANDOM = 27;

// the number of the class in g_ClassInfo by the command of its button, 0 when it is unknown
int FindClass( const char *pszCommand )
{
	for( int i = 1; i < 28; i++ )
	{
		if( !stricmp( g_ClassInfo[i].selectcmd, pszCommand ) )
			return i;
	}

	return 0;
}

}

CDoDClassMenu::CDoDClassMenu( int iRemoveMe, int x, int y, int wide, int tall ) : CDoDMenuPanel( iRemoveMe, x, y, wide, tall )
{
	m_iMenu = 0;
	m_iLoadedMenu = -1;
	m_pCurrentClassPage = NULL;
	m_iClassMask = 0xFFFF;
	m_iActiveClass = 0;
	m_szNumOnTeam[0] = '\0';
	m_iLayoutHeight = ScreenHeight;

	memset( m_pMenuPages, 0, sizeof( m_pMenuPages ) );
	memset( m_pClassPages, 0, sizeof( m_pClassPages ) );
}

void CDoDClassMenu::SetMenu( int iMenu )
{
	m_iMenu = iMenu - MENU_CLASS_ALLIES;

	if( m_iMenu < 0 || m_iMenu >= NUM_MENUS )
		m_iMenu = 0;
}

void CDoDClassMenu::SetClassMask( int iMask )
{
	m_iClassMask = iMask;
}

// Makes the page of the menu when it is shown for the first time and shows it
bool CDoDClassMenu::LoadMenu( int iMenu )
{
	if( !m_pMenuPages[iMenu] )
	{
		CDoDResPage *pPage = new CDoDResPage( 0, 0, ScreenWidth, ScreenHeight );

		pPage->setParent( this );

		if( !pPage->LoadControls( s_szMenuFiles[iMenu] ) )
			gEngfuncs.Con_Printf( "Missing the layout of the class menu: %s\n", s_szMenuFiles[iMenu] );

		// the buttons: a class is chosen with its command, the cancel button closes the menu
		for( int i = 0; i < pPage->GetNumControls(); i++ )
		{
			ResControl_t *pControl = pPage->GetControl( i );

			if( pControl->iType != RESCONTROL_BUTTON )
				continue;

			CDoDButton *pButton = static_cast<CDoDButton *>( pControl->pPanel );

			if( !stricmp( pControl->szName, "CancelButton" ) )
			{
				pButton->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
			}
			else if( !strnicmp( pControl->szName, "cls_button", 10 ) )
			{
				pButton->addActionSignal( new CMenuHandler_StringCommand( pControl->szCommand, true ) );

				// the mouse over the button shows the page of the class
				if( pControl->bMouseOver )
					pButton->addInputSignal( new CHandler_MenuButtonOver( this, atoi( pControl->szName + 10 ) ) );
			}
		}

		m_pMenuPages[iMenu] = pPage;
	}

	for( int i = 0; i < NUM_MENUS; i++ )
	{
		if( m_pMenuPages[i] )
			m_pMenuPages[i]->setVisible( i == iMenu );
	}

	HideClassPage();

	m_iLoadedMenu = iMenu;
	m_iActiveClass = 0;
	m_szNumOnTeam[0] = '\0';

	return true;
}

// The page of a class is the layout Resource/Classes/<command>.res placed over ClassInfo of the menu
CDoDResPage *CDoDClassMenu::GetClassPage( int iClass, const char *pszCommand )
{
	if( iClass <= 0 || iClass >= NUM_CLASSES || iClass == CLASS_RANDOM )
		return NULL;

	if( !m_pClassPages[iClass] )
	{
		char szFile[96];
		int x = 220, y = 85, wide = 366, tall = 344;

		if( m_iLoadedMenu >= 0 && m_pMenuPages[m_iLoadedMenu] )
		{
			ResControl_t *pInfo = m_pMenuPages[m_iLoadedMenu]->FindControl( "ClassInfo" );

			if( pInfo )
			{
				x = pInfo->x;
				y = pInfo->y;
				wide = pInfo->wide;
				tall = pInfo->tall;
			}
		}

		CDoDResPage *pPage = new CDoDResPage( DODMENU_SIZE( x ), DODMENU_SIZE( y ), DODMENU_SIZE( wide ), DODMENU_SIZE( tall ) );

		pPage->setParent( this );
		pPage->setVisible( false );

		// a class without the file has an empty page
		snprintf( szFile, sizeof( szFile ), "resource/classes/%s.res", pszCommand );
		pPage->LoadControls( szFile );

		m_pClassPages[iClass] = pPage;
	}

	return m_pClassPages[iClass];
}

void CDoDClassMenu::HideClassPage( void )
{
	if( m_pCurrentClassPage )
		m_pCurrentClassPage->setVisible( false );

	m_pCurrentClassPage = NULL;
}

// The pages and the fonts depend on the height of the screen
void CDoDClassMenu::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	setBounds( 0, 0, ScreenWidth, ScreenHeight );

	for( int i = 0; i < NUM_MENUS; i++ )
	{
		if( m_pMenuPages[i] )
		{
			m_pMenuPages[i]->setBounds( 0, 0, ScreenWidth, ScreenHeight );
			m_pMenuPages[i]->Layout();
		}
	}

	for( int i = 0; i < NUM_CLASSES; i++ )
	{
		if( m_pClassPages[i] )
		{
			m_pClassPages[i]->setBounds( DODMENU_SIZE( 220 ), DODMENU_SIZE( 85 ), DODMENU_SIZE( 366 ), DODMENU_SIZE( 344 ) );
			m_pClassPages[i]->Layout();
		}
	}

	m_szNumOnTeam[0] = '\0';
}

// Called each time a new level is started
void CDoDClassMenu::Initialize( void )
{
	m_iActiveClass = 0;
	m_szNumOnTeam[0] = '\0';

	HideClassPage();
}

// Called every time the menu is displayed and when the info of the players is updated
void CDoDClassMenu::Update( void )
{
	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	if( m_iLoadedMenu != m_iMenu )
		LoadMenu( m_iMenu );

	CDoDResPage *pPage = m_pMenuPages[m_iLoadedMenu];

	if( !pPage )
		return;

	for( int i = 0; i < pPage->GetNumControls(); i++ )
	{
		ResControl_t *pControl = pPage->GetControl( i );

		if( pControl->iType != RESCONTROL_BUTTON )
			continue;

		CDoDButton *pButton = static_cast<CDoDButton *>( pControl->pPanel );

		if( !stricmp( pControl->szName, "CancelButton" ) )
		{
			// a player that has a class can close the menu
			pButton->setVisible( g_iPlayerClass != 0 );
		}
		else if( !strnicmp( pControl->szName, "cls_button", 10 ) )
		{
			int iBit = atoi( pControl->szName + 10 ) - 1;

			if( iBit < 0 || iBit > 11 )
				continue;

			pButton->setVisible( ( m_iClassMask >> iBit ) & 1 );
			pButton->SetGreyedOut( ( ( m_iClassMask >> ( iBit + 16 ) ) & 1 ) != 0 );
		}
	}
}

// Update the menu before opening it
void CDoDClassMenu::Open( void )
{
	Update();

	HideClassPage();
	m_iActiveClass = 0;
	m_szNumOnTeam[0] = '\0';

	CMenuPanel::Open();
}

// Number keys: the number in the text of the button ("&1 Rifleman"), 0 is the cancel button
bool CDoDClassMenu::SlotInput( int iSlot )
{
	CDoDResPage *pPage = ( m_iLoadedMenu >= 0 ) ? m_pMenuPages[m_iLoadedMenu] : NULL;

	if( !pPage )
		return false;

	for( int i = 0; i < pPage->GetNumControls(); i++ )
	{
		ResControl_t *pControl = pPage->GetControl( i );

		if( pControl->iType != RESCONTROL_BUTTON || !pControl->pPanel->isVisible() )
			continue;

		if( DoD_GetMnemonic( pControl->szText ) == iSlot )
		{
			static_cast<CDoDButton *>( pControl->pPanel )->fireActionSignal();

			return true;
		}
	}

	return false;
}

// The mouse is over the button cls_button<iInput>, the page of its class is shown
void CDoDClassMenu::SetActiveInfo( int iInput )
{
	CDoDResPage *pMenuPage = ( m_iLoadedMenu >= 0 ) ? m_pMenuPages[m_iLoadedMenu] : NULL;
	char szName[32];

	if( !pMenuPage )
		return;

	snprintf( szName, sizeof( szName ), "cls_button%d", iInput );

	ResControl_t *pButton = pMenuPage->FindControl( szName );

	if( !pButton || pButton->iType != RESCONTROL_BUTTON )
		return;

	HideClassPage();

	m_iActiveClass = FindClass( pButton->szCommand );
	m_szNumOnTeam[0] = '\0';

	CDoDResPage *pPage = GetClassPage( m_iActiveClass, pButton->szCommand );

	if( pPage )
	{
		pPage->setVisible( true );
		m_pCurrentClassPage = pPage;
	}

	UpdateNumClassLabel();
}

bool CDoDClassMenu::IsClassFull( int iClass )
{
	int iBit = iClass - 10;

	if( (unsigned int)( iClass - 10 ) > 10 )
	{
		iBit = iClass - 1;

		if( (unsigned int)( iClass - 21 ) <= 5 )
			iBit = iClass - 21;
	}

	if( iBit < 0 || iBit + 16 > 31 )
		return false;

	return ( ( m_iClassMask >> ( iBit + 16 ) ) & 1 ) != 0;
}

// Under the page of a class: how many players of the class are in the team, or that the class is full
void CDoDClassMenu::UpdateNumClassLabel( void )
{
	CDoDResPage *pPage = ( m_iLoadedMenu >= 0 ) ? m_pMenuPages[m_iLoadedMenu] : NULL;

	if( !pPage )
		return;

	ResControl_t *pNumber = pPage->FindControl( "numberOnTeam" );
	ResControl_t *pFull = pPage->FindControl( "classFull" );

	if( !pNumber || !pFull || !pNumber->pPanel || !pFull->pPanel )
		return;

	// no class under the mouse or the random one
	if( m_iActiveClass <= 0 || m_iActiveClass == CLASS_RANDOM )
	{
		pNumber->pPanel->setVisible( false );
		pFull->pPanel->setVisible( false );
		return;
	}

	char szNumPlayers[8];
	char szFormat[256];
	char szText[256];
	char szKey[260];
	int iNumPlayers = 0;

	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		gEngfuncs.pfnGetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( !g_PlayerInfoList[i].name || g_PlayerExtraInfo[i].teamnumber != g_iTeamNumber )
			continue;

		if( g_PlayerExtraInfo[i].playerclass == m_iActiveClass )
			iNumPlayers++;
	}

	snprintf( szNumPlayers, sizeof( szNumPlayers ), "%i", iNumPlayers );

	if( IsClassFull( m_iActiveClass ) )
	{
		DoD_GetLocalized( "#Dod_class_full", szFormat, sizeof( szFormat ) );
		DoD_ConstructLocalized( szText, sizeof( szText ), szFormat, szNumPlayers, "", "", "" );

		pNumber->pPanel->setVisible( false );
		pFull->pPanel->setVisible( true );

		// the text is set when it is changed, the label is updated every frame
		snprintf( szKey, sizeof( szKey ), "F%s", szText );

		if( strcmp( szKey, m_szNumOnTeam ) )
		{
			strncpy( m_szNumOnTeam, szKey, sizeof( m_szNumOnTeam ) - 1 );
			m_szNumOnTeam[sizeof( m_szNumOnTeam ) - 1] = '\0';

			pPage->SetControlText( pFull, szText );

			// the text of a full class is red
			static_cast<vgui::Label *>( pFull->pPanel )->setFgColor( 255, 0, 0, 0 );
		}
	}
	else
	{
		if( iNumPlayers == 1 )
		{
			DoD_GetLocalized( "#Dod_Currently_on_your_team_one", szText, sizeof( szText ) );
		}
		else
		{
			DoD_GetLocalized( "#Dod_Currently_on_your_team", szFormat, sizeof( szFormat ) );
			DoD_ConstructLocalized( szText, sizeof( szText ), szFormat, szNumPlayers, "", "", "" );
		}

		pFull->pPanel->setVisible( false );
		pNumber->pPanel->setVisible( true );

		snprintf( szKey, sizeof( szKey ), "N%s", szText );

		if( strcmp( szKey, m_szNumOnTeam ) )
		{
			strncpy( m_szNumOnTeam, szKey, sizeof( m_szNumOnTeam ) - 1 );
			m_szNumOnTeam[sizeof( m_szNumOnTeam ) - 1] = '\0';

			pPage->SetControlText( pNumber, szText );
		}
	}
}

void CDoDClassMenu::paintBackground( void )
{
	// the number of the players is updated every frame
	UpdateNumClassLabel();

	CDoDMenuPanel::paintBackground();
}
