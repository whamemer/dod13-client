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
// dodmotd.cpp
//
// The message of the day of DoD, see dodmotd.h.
//

#include <string.h>
#include <string>

#include "hud.h"
#include "cl_util.h"

#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodmotd.h"

CDoDMOTD::CDoDMOTD( int iRemoveMe, int x, int y, int wide, int tall ) : CDoDMenuPanel( iRemoveMe, x, y, wide, tall )
{
	m_iLayoutHeight = ScreenHeight;

	m_pPage = new CDoDResPage( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->setParent( this );

	if( !m_pPage->LoadControls( "Resource/UI/MOTD.res" ) )
		gEngfuncs.Con_Printf( "Missing the layout of the MOTD: Resource/UI/MOTD.res\n" );

	m_pServerName = m_pPage->FindControl( "serverName" );
	m_pMessage = m_pPage->FindControl( "Message" );

	// the OK button closes the menu
	ResControl_t *pOK = m_pPage->FindControl( "ok2" );

	if( pOK && pOK->pPanel )
		static_cast<CDoDButton *>( pOK->pPanel )->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
}

// The fonts and the sizes depend on the height of the screen
void CDoDMOTD::Layout( void )
{
	m_iLayoutHeight = ScreenHeight;

	setBounds( 0, 0, ScreenWidth, ScreenHeight );

	m_pPage->setBounds( 0, 0, ScreenWidth, ScreenHeight );
	m_pPage->Layout();
}

void CDoDMOTD::Open( void )
{
	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	CMenuPanel::Open();
}

void CDoDMOTD::paintBackground( void )
{
	CDoDMenuPanel::paintBackground();
}

// A page that is not HTML: a URL cannot be opened without a browser, the address is shown; a text keeps its lines
static void MakePage( const char *pszMessage, std::string &page )
{
	const char *pszText = pszMessage;

	// the page of the server
	if( strchr( pszMessage, '<' ) )
	{
		page = pszMessage;
		return;
	}

	// a plain text: the colors of the DoD menus, the lines and the special characters are kept
	page = "<body bgcolor=\"#000000\" text=\"#FFB000\"><p>";

	for( ; *pszText; pszText++ )
	{
		if( *pszText == '\n' )
			page += "<br>";
		else if( *pszText == '&' )
			page += "&amp;";
		else if( *pszText != '\r' )
			page += *pszText;
	}

	page += "</p></body>";
}

void CDoDMOTD::Activate( const char *pszTitle, const char *pszMessage )
{
	std::string page;

	if( m_iLayoutHeight != ScreenHeight )
		Layout();

	if( m_pServerName )
		m_pPage->SetControlText( m_pServerName, pszTitle );

	MakePage( pszMessage, page );

	if( m_pMessage && m_pMessage->pPanel )
		static_cast<CDoDHtmlView *>( m_pMessage->pPanel )->SetHtml( page.c_str() );
}
