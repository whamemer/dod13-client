//=========== (C) Copyright 1996-2002 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Client DLL VGUI Viewport
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include <VGUI_Label.h>
#include <VGUI_Panel.h>
#include <VGUI_Button.h>
#include <VGUI_InputSignal.h>
#include <VGUI_Scheme.h>
#include <VGUI_App.h>

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "keydefs.h"
#include "demo.h"
#include "demo_api.h"

#include "vgui/vgui_int.h"
#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodclientscoreboard.h"
#include "vgui/dodspectatorgui.h"
#include "vgui/dodteammenu.h"
#include "vgui/dodclassmenu.h"
#include "vgui/dodtextwindow.h"
#include "vgui/dodmenu.h"
#include "dod_localize.h"

#include "shake.h"
#include "screenfade.h"

#include "input_mouse.h"

void IgnoreNextMouseDelta();

class CCommandMenu;

// Scoreboard positions
#define SBOARD_INDENT_X			XRES( 104 )
#define SBOARD_INDENT_Y			YRES( 40 )

// low-res scoreboard indents
#define SBOARD_INDENT_X_512		30
#define SBOARD_INDENT_Y_512		30

#define SBOARD_INDENT_X_400		0
#define SBOARD_INDENT_Y_400		20

void IN_ResetMouse( void );
extern float *GetClientColor( int clientIndex );

using namespace vgui;

// Get the name of TGA file, based on GameDir
char *GetVGUITGAName( const char *pszName )
{
	int i, len;
	char sz[256];
	static char gd[256];
	const char *gamedir;

	if( ScreenWidth < 640 )
		i = 320;
	else
		i = 640;

	safe_snprintf( sz, sizeof( sz ), pszName, i );

	gamedir = gEngfuncs.pfnGetGameDirectory();
	len = safe_snprintf( gd, sizeof( gd ), "%s/gfx/vgui/%s.tga", gamedir, sz );
	if( len < 0 ) return 0;
	return gd;
}

//================================================================
// COMMAND MENU
//================================================================
void CCommandMenu::AddButton( CommandButton *pButton )
{
	if( m_iButtons >= MAX_BUTTONS )
		return;

	m_aButtons[m_iButtons] = pButton;
	m_iButtons++;
	pButton->setParent( this );
	pButton->setFont( Scheme::sf_primary3 );

	// give the button a default key binding
	if( m_iButtons < 10 )
	{
		pButton->setBoundKey( m_iButtons + '0' );
	}
	else if( m_iButtons == 10 )
	{
		pButton->setBoundKey( '0' );
	}
}

void CCommandMenu::RemoveAllButtons(void)
{
	/*
	for(int i=0;i<m_iButtons;i++)
	{
		CommandButton *pTemp = m_aButtons[i]; 
		m_aButtons[i] = NULL;
		
		pTemp
		if(pTemp)
		{
			delete(pTemp);
		}
		
	}
	*/
	removeAllChildren();
	m_iButtons=0;
	
}

//-----------------------------------------------------------------------------
// Purpose: Tries to find a button that has a key bound to the input, and
//			presses the button if found
// Input  : keyNum - the character number of the input key
// Output : Returns true if the command menu should close, false otherwise
//-----------------------------------------------------------------------------
bool CCommandMenu::KeyInput( int keyNum )
{
	// loop through all our buttons looking for one bound to keyNum
	for( int i = 0; i < m_iButtons; i++ )
	{
		if( !m_aButtons[i]->IsNotValid() )
		{
			if( m_aButtons[i]->getBoundKey() == keyNum )
			{
				// hit the button
				if( m_aButtons[i]->GetSubMenu() )
				{
					// open the sub menu
					gViewPort->SetCurrentCommandMenu( m_aButtons[i]->GetSubMenu() );
					return false;
				}
				else
				{
					// run the bound command
					m_aButtons[i]->fireActionSignal();
					return true;
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: clears the current menus buttons of any armed (highlighted) 
//			state, and all their sub buttons
//-----------------------------------------------------------------------------
void CCommandMenu::ClearButtonsOfArmedState( void )
{
	for( int i = 0; i < GetNumButtons(); i++ )
	{
		m_aButtons[i]->setArmed( false );

		if( m_aButtons[i]->GetSubMenu() )
		{
			m_aButtons[i]->GetSubMenu()->ClearButtonsOfArmedState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSubMenu - 
// Output : CommandButton
//-----------------------------------------------------------------------------
CommandButton *CCommandMenu::FindButtonWithSubmenu( CCommandMenu *pSubMenu )
{
	for( int i = 0; i < GetNumButtons(); i++ )
	{
		if( m_aButtons[i]->GetSubMenu() == pSubMenu )
			return m_aButtons[i];
	}

	return NULL;
}

// Recalculate the visible buttons
bool CCommandMenu::RecalculateVisibles( int iYOffset, bool bHideAll )
{
	int		i, iCurrentY = 0;
	int		iVisibleButtons = 0;

	// Cycle through all the buttons in this menu, and see which will be visible
	for( i = 0; i < m_iButtons; i++ )
	{
		int iClass = m_aButtons[i]->GetPlayerClass();

		if( ( iClass && iClass != g_iPlayerClass ) || ( m_aButtons[i]->IsNotValid() ) || bHideAll )
		{
			m_aButtons[i]->setVisible( false );
			if( m_aButtons[i]->GetSubMenu() != NULL )
			{
				(m_aButtons[i]->GetSubMenu())->RecalculateVisibles( 0, true );
			}
		}
		else
		{
 			// If it's got a submenu, force it to check visibilities
			if( m_aButtons[i]->GetSubMenu() != NULL )
			{
				if( !( m_aButtons[i]->GetSubMenu() )->RecalculateVisibles( 0, false ) )
				{
					// The submenu had no visible buttons, so don't display this button
					m_aButtons[i]->setVisible( false );
					continue;
				}
			}

			m_aButtons[i]->setVisible( true );
			iVisibleButtons++;
		}
	}

	// Set Size
	setSize( _size[0], ( iVisibleButtons * ( m_flButtonSizeY - 1 ) ) + 1 );

	if( iYOffset )
	{
		m_iYOffset = iYOffset;
	}

	for( i = 0; i < m_iButtons; i++ )
	{
		if( m_aButtons[i]->isVisible() )
		{
			if( m_aButtons[i]->GetSubMenu() != NULL )
				( m_aButtons[i]->GetSubMenu() )->RecalculateVisibles( iCurrentY + m_iYOffset, false );

			// Make sure it's at the right Y position
			// m_aButtons[i]->getPos( iXPos, iYPos );
			if( m_iDirection )
			{
				m_aButtons[i]->setPos( 0, ( iVisibleButtons - 1 ) * ( m_flButtonSizeY - 1 ) - iCurrentY );
			}
			else
			{
				m_aButtons[i]->setPos( 0, iCurrentY );
			}

			iCurrentY += ( m_flButtonSizeY - 1 );
		}
	}

	return iVisibleButtons ? true : false;
}

// Make sure all submenus can fit on the screen
void CCommandMenu::RecalculatePositions( int iYOffset )
{
	int iTop;
	int iAdjust = 0;

	m_iYOffset+= iYOffset;

	if( m_iDirection )
		iTop = ScreenHeight - ( m_iYOffset + _size[1] );
	else
		iTop = m_iYOffset;

	if( iTop < 0 )
		iTop = 0;

	// Calculate if this is going to fit onscreen, and shuffle it up if it won't
	int iBottom = iTop + _size[1];

	if( iBottom > ScreenHeight )
	{
		// Move in increments of button sizes
		while( iAdjust < ( iBottom - ScreenHeight ) )
		{
			iAdjust += m_flButtonSizeY - 1;
		}

		iTop -= iAdjust;

		// Make sure it doesn't move off the top of the screen (the menu's too big to fit it all)
		if( iTop < 0 )
		{
			iAdjust -= ( 0 - iTop );
			iTop = 0;
		}
	}

	setPos( _pos[0], iTop );

	// We need to force all menus below this one to update their positions now, because they
	// might have submenus riding off buttons in this menu that have just shifted.
	for( int i = 0; i < m_iButtons; i++ )
		m_aButtons[i]->UpdateSubMenus( iAdjust );
}

// Make this menu and all menus above it in the chain visible
void CCommandMenu::MakeVisible( CCommandMenu *pChildMenu )
{
/*
	// Push down the button leading to the child menu
	for( int i = 0; i < m_iButtons; i++ )
	{
		if( ( pChildMenu != NULL ) && ( m_aButtons[i]->GetSubMenu() == pChildMenu ) )
		{
			m_aButtons[i]->setArmed( true );
		}
		else
		{
			m_aButtons[i]->setArmed( false );
		}
	}
*/

	setVisible( true );
	if( m_pParentMenu )
		m_pParentMenu->MakeVisible( this );
}

//================================================================
// CreateSubMenu
CCommandMenu *TeamFortressViewport::CreateSubMenu( CommandButton *pButton, CCommandMenu *pParentMenu, int iYOffset, int iXOffset )
{
	int iXPos = 0;
	int iYPos = 0;
	int iWide = CMENU_SIZE_X;
	int iTall = 0;
	int iDirection = 0;

	if( pParentMenu )
	{
		iXPos = m_pCurrentCommandMenu->GetXOffset() + ( CMENU_SIZE_X - 1 ) + iXOffset;
		iYPos = m_pCurrentCommandMenu->GetYOffset() + iYOffset;
		iDirection = pParentMenu->GetDirection();
	}

	CCommandMenu *pMenu = new CCommandMenu( pParentMenu, iDirection, iXPos, iYPos, iWide, iTall );
	pMenu->setParent( this );
	pButton->AddSubMenu( pMenu );
	pButton->setFont( Scheme::sf_primary3 );
	pMenu->m_flButtonSizeY = m_pCurrentCommandMenu->m_flButtonSizeY;

	// Create the Submenu-open signal
	InputSignal *pISignal = new CMenuHandler_PopupSubMenuInput( pButton, pMenu );
	pButton->addInputSignal( pISignal );

	// Put a > to show it's a submenu
	CImageLabel *pLabel = new CImageLabel( "arrow", CMENU_SIZE_X - SUBMENU_SIZE_X, SUBMENU_SIZE_Y );
	pLabel->setParent( pButton );
	pLabel->addInputSignal( pISignal );

	// Reposition
	pLabel->getPos( iXPos, iYPos );
	pLabel->setPos( CMENU_SIZE_X - pLabel->getImageWide(), ( BUTTON_SIZE_Y - pLabel->getImageTall() ) / 2 );

	// Create the mouse off signal for the Label too
	if( !pButton->m_bNoHighlight )
		pLabel->addInputSignal( new CHandler_CommandButtonHighlight( pButton ) );

	return pMenu;
}

//-----------------------------------------------------------------------------
// Purpose: Makes sure the memory allocated for TeamFortressViewport is nulled out
// Input  : stAllocateBlock - 
// Output : void *
//-----------------------------------------------------------------------------
void *TeamFortressViewport::operator new( size_t stAllocateBlock )
{
//	void *mem = Panel::operator new( stAllocateBlock );
	void *mem = ::operator new( stAllocateBlock );
	memset( mem, 0, stAllocateBlock );
	return mem;
}

//-----------------------------------------------------------------------------
// Purpose: InputSignal handler for the main viewport
//-----------------------------------------------------------------------------
class CViewPortInputHandler : public InputSignal
{
public:
	bool bPressed;

	CViewPortInputHandler()
	{
	}

	virtual void cursorMoved( int x, int y, Panel *panel ) {}
	virtual void cursorEntered( Panel *panel ) {}
	virtual void cursorExited( Panel *panel ) {}
	virtual void mousePressed( MouseCode code, Panel *panel ) 
	{
		if( code != MOUSE_LEFT )
		{
			// send a message to close the command menu
			// this needs to be a message, since a direct call screws the timing
			gEngfuncs.pfnClientCmd( "ForceCloseCommandMenu\n" );
		}
	}

	virtual void mouseReleased( MouseCode code, Panel *panel )
	{
	}

	virtual void mouseDoublePressed( MouseCode code, Panel *panel ) {}
	virtual void mouseWheeled( int delta, Panel *panel ) {}
	virtual void keyPressed( KeyCode code, Panel *panel ) {}
	virtual void keyTyped( KeyCode code, Panel *panel ) {}
	virtual void keyReleased( KeyCode code, Panel *panel ) {}
	virtual void keyFocusTicked( Panel *panel ) {}
};

//================================================================
TeamFortressViewport::TeamFortressViewport( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall ), m_SchemeManager( wide, tall )
{
	gViewPort = this;
	m_iInitialized = false;
	m_pTeamMenu = NULL;
	m_pClassMenu = NULL;
	m_pScoreBoard = NULL;
	m_pSpectatorGUI = NULL;
	m_pSpecHelp = NULL;
	m_pMapBriefing = NULL;
	m_pCurrentMenu = NULL;
	m_pCurrentCommandMenu = NULL;

	Initialize();
	addInputSignal( new CViewPortInputHandler );

	int r, g, b, a;

	Scheme* pScheme = App::getInstance()->getScheme();

	// primary text color
	// Get the colors
	//!! two different types of scheme here, need to integrate
	SchemeHandle_t hPrimaryScheme = m_SchemeManager.getSchemeHandle( "Primary Button Text" );
	{
		// font
		pScheme->setFont( Scheme::sf_primary1, m_SchemeManager.getFont( hPrimaryScheme ) );

		// text color
		m_SchemeManager.getFgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( Scheme::sc_primary1, r, g, b, a );		// sc_primary1 is non-transparent orange

		// background color (transparent black)
		m_SchemeManager.getBgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( Scheme::sc_primary3, r, g, b, a );

		// armed foreground color
		m_SchemeManager.getFgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( Scheme::sc_secondary2, r, g, b, a );

		// armed background color
		m_SchemeManager.getBgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( Scheme::sc_primary2, r, g, b, a );

		//!! need to get this color from scheme file
		// used for orange borders around buttons
		m_SchemeManager.getBorderColor( hPrimaryScheme, r, g, b, a );
		// pScheme->setColor( Scheme::sc_secondary1, r, g, b, a );
		pScheme->setColor( Scheme::sc_secondary1, 255 * 0.7, 170 * 0.7, 0, 0);
	}

	// Change the second primary font (used in the scoreboard)
	SchemeHandle_t hScoreboardScheme = m_SchemeManager.getSchemeHandle( "Scoreboard Text" );
	{
		pScheme->setFont( Scheme::sf_primary2, m_SchemeManager.getFont( hScoreboardScheme ) );
	}
	
	// Change the third primary font (used in command menu)
	SchemeHandle_t hCommandMenuScheme = m_SchemeManager.getSchemeHandle( "CommandMenu Text" );
	{
		pScheme->setFont( Scheme::sf_primary3, m_SchemeManager.getFont( hCommandMenuScheme ) );
	}

	App::getInstance()->setScheme( pScheme );

	// VGUI MENUS
	CreateTeamMenu();
	CreateClassMenu();
	CreateSpectatorMenu();
	CreateScoreBoard();
	// Init command menus
	m_iNumMenus = 0;
	m_iCurrentTeamNumber = m_iUser1 = m_iUser2 = m_iUser3 = 0;

	m_StandardMenu = CreateCommandMenu( "commandmenu.txt", 0, CMENU_TOP, false, CMENU_SIZE_X, BUTTON_SIZE_Y, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Called everytime a new level is started. Viewport clears out it's data.
//-----------------------------------------------------------------------------
void TeamFortressViewport::Initialize( void )
{
	// Force each menu to Initialize
	if( m_pTeamMenu )
	{
		m_pTeamMenu->Initialize();
	}
	if( m_pClassMenu )
	{
		m_pClassMenu->Initialize();
	}
	if( m_pScoreBoard )
	{
		m_pScoreBoard->Initialize();
		HideScoreBoard();
	}
	if( m_pSpectatorGUI )
	{
		// Spectator menu doesn't need initializing
		m_pSpectatorGUI->HideGUI();
	}

	// Make sure all menus are hidden
	HideVGUIMenu();
	HideCommandMenu();

	// Clear out some data
	m_iRandomPC = false;
	m_flScoreBoardLastUpdated = 0;
	m_flSpectatorPanelLastUpdated = 0;

	// reset player info
	g_iPlayerClass = 0;
	g_iTeamNumber = 0;

	m_sMapName[0] = '\0';
	m_szServerName[0] = '\0';
	for( int i = 0; i < 5; i++ )
	{
		m_sTeamNames[i][0] = '\0';
	}

	App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_none) );
}

class CException;
//-----------------------------------------------------------------------------
// Purpose: Read the Command Menu structure from the txt file and create the menu.
//			Returns Index of menu in m_pCommandMenus
//-----------------------------------------------------------------------------
int TeamFortressViewport::CreateCommandMenu( const char *menuFile, int direction, int yOffset, bool flatDesign, float flButtonSizeX, float flButtonSizeY, int xOffset )
{
	// COMMAND MENU
	// Create the root of this new Command Menu
	int newIndex = m_iNumMenus;
	
	m_pCommandMenus[newIndex] = new CCommandMenu( NULL, direction, xOffset, yOffset, flButtonSizeX, 300 );	// This will be resized once we know how many items are in it
	m_pCommandMenus[newIndex]->setParent( this );
	m_pCommandMenus[newIndex]->setVisible( false );
	m_pCommandMenus[newIndex]->m_flButtonSizeY = flButtonSizeY;

	m_iNumMenus++;

	// Read Command Menu from the txt file
	char token[1024];
	char *pFileStart = (char*)gEngfuncs.COM_LoadFile( menuFile, 5, NULL );
	if( !pFileStart )
	{
		gEngfuncs.Con_DPrintf( "Unable to open %s\n", menuFile);
		SetCurrentCommandMenu( NULL );
		return newIndex;
	}

#if XASH_WIN32
	try
	{
#endif
		// Now start parsing the menu structure
		m_pCurrentCommandMenu = m_pCommandMenus[newIndex];
		char szLastButtonText[32] = "file start";
		char* pfile = gEngfuncs.COM_ParseFile( pFileStart, token );
		while( ( strlen ( token ) > 0 ) && ( m_iNumMenus < MAX_MENUS ) )
		{
			// Keep looping until we hit the end of this menu
			while( token[0] != '}' && ( strlen( token ) > 0 ) )
			{
				char cText[32] = "";
				char cBoundKey[32] = "";
				char cCustom[32] = "";
				static const int cCommandLength = 128;
				char cCommand[cCommandLength] = "";
				char szMap[MAX_MAPNAME] = "";
				int  iPlayerClass = 0;
				int  iCustom = false;
				int  iTeamOnly = -1;
				int  iToggle = 0;
				int  iButtonY;
				bool bGetExtraToken = true;
				CommandButton *pButton = NULL;
			
				// We should never be here without a Command Menu
				if( !m_pCurrentCommandMenu )
				{
					gEngfuncs.Con_Printf( "Error in %s file after '%s'.\n", menuFile, szLastButtonText );
					gEngfuncs.COM_FreeFile( pFileStart );	// Vit_amiN: prevent the memory leak
					m_iInitialized = false;

					return newIndex;
				}

				// token should already be the bound key, or the custom name
				strlcpy( cCustom, token, sizeof( cCustom ));

				// See if it's a custom button
				if( !strcmp( cCustom, "CUSTOM" ) )
				{
					iCustom = true;

					// Get the next token
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
				}
				// See if it's a map
				else if( !strcmp( cCustom, "MAP" ) )
				{
					// Get the mapname
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					strlcpy( szMap, token, MAX_MAPNAME );

					// Get the next token
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
				}
				else if( !strncmp( cCustom, "TEAM", 4 ) ) // TEAM1, TEAM2, TEAM3, TEAM4
				{
					// make it a team only button
					iTeamOnly = atoi( cCustom + 4 );
				
					// Get the next token
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
				}
				else if( !strncmp( cCustom, "TOGGLE", 6 ) ) 
				{
					iToggle = true;

					// Get the next token
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
				}
				else
				{
					// See if it's a Class
				}

				// Get the button bound key
				strlcpy( cBoundKey, token, 32 );

				// Get the button text
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strlcpy( cText, CHudTextMessage::BufferedLocaliseTextString( token ), 32 );	// Vit_amiN: localize button text

				// save off the last button text we've come across (for error reporting)
				strcpy( szLastButtonText, cText );

				// Get the button command
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strlcpy( cCommand, token, cCommandLength );

				iButtonY = ( BUTTON_SIZE_Y - 1 ) * m_pCurrentCommandMenu->GetNumButtons();
			
				// Custom button handling
				if( iCustom )
				{
					pButton = CreateCustomButton( cText, cCommand, iButtonY );

					// Get the next token to see if we're a menu
					pfile = gEngfuncs.COM_ParseFile( pfile, token );

					if( token[0] == '{' )
					{
						strcpy( cCommand, token );
					}
					else
					{
						bGetExtraToken = false;
					}
				}
				else if( szMap[0] != '\0' )
				{
					// create a map button
					pButton = new MapButton( szMap, cText, xOffset, iButtonY, flButtonSizeX, flButtonSizeY );
				}
				else if( iTeamOnly != -1 )
				{
					// button that only shows up if the player is on team iTeamOnly
					pButton = new TeamOnlyCommandButton( iTeamOnly, cText, xOffset, iButtonY, flButtonSizeX, flButtonSizeY, flatDesign );
				}
				else if( iToggle && direction == 0 )
				{
					pButton = new ToggleCommandButton( cCommand, cText, xOffset, iButtonY, flButtonSizeX, flButtonSizeY, flatDesign );
				}
				else
				{
					// normal button
					pButton = new CommandButton( iPlayerClass, cText, xOffset, iButtonY, flButtonSizeX, flButtonSizeY, flatDesign );
				}

				// add the button into the command menu
				if( pButton )
				{
					m_pCurrentCommandMenu->AddButton( pButton );
					pButton->setBoundKey( cBoundKey[0] );
					pButton->setParentMenu( m_pCurrentCommandMenu );

					// Override font in CommandMenu
					pButton->setFont( Scheme::sf_primary3 );
				}

				// Find out if it's a submenu or a button we're dealing with
				if( cCommand[0] == '{' )
				{
					if( m_iNumMenus >= MAX_MENUS )
					{
						gEngfuncs.Con_Printf( "Too many menus in %s past '%s'\n", menuFile, szLastButtonText );
					}
					else if( pButton )
					{
						// Create the menu
						m_pCommandMenus[m_iNumMenus] = CreateSubMenu( pButton, m_pCurrentCommandMenu, iButtonY );
						m_pCurrentCommandMenu = m_pCommandMenus[m_iNumMenus];
						m_iNumMenus++;
					}
				}
				else if( !iCustom )
				{
					// Create the button and attach it to the current menu
					if( iToggle )
						pButton->addActionSignal( new CMenuHandler_ToggleCvar( cCommand ) );
					else
						pButton->addActionSignal( new CMenuHandler_StringCommand( cCommand ) );

					// Create an input signal that'll popup the current menu
					pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput( pButton, m_pCurrentCommandMenu ) );
				}

				// Get the next token
				if( bGetExtraToken )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
				}
			}

			// Move back up a menu
			m_pCurrentCommandMenu = m_pCurrentCommandMenu->GetParentMenu();

			pfile = gEngfuncs.COM_ParseFile( pfile, token );
		}
#if XASH_WIN32
	}
	catch( CException *e )
	{
		e;
		//e->Delete();
		e = NULL;
		gEngfuncs.COM_FreeFile( pFileStart );	// Vit_amiN: prevent the memory leak
		m_iInitialized = false;
		return newIndex;
	}
#endif

	SetCurrentMenu( NULL );
	SetCurrentCommandMenu( NULL );
	gEngfuncs.COM_FreeFile( pFileStart );

	m_iInitialized = true;

	return newIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pButtonText - 
//			*pButtonName - 
// Output : CommandButton
//-----------------------------------------------------------------------------
CommandButton *TeamFortressViewport::CreateCustomButton( char *pButtonText, char *pButtonName, int iYOffset )
{
	CommandButton *pButton = NULL;

	// The description of the map and the message of the server (the help menu of commandmenu.txt of DoD)
	if( !strcmp( pButtonName, "!MAPBRIEFING" ) )
	{
		pButton = new CommandButton( pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y );
		pButton->addActionSignal( new CMenuHandler_TextWindow( MENU_MAPBRIEFING ) );
	}
	else if( !strcmp( pButtonName, "!SERVERINFO" ) )
	{
		pButton = new CommandButton( pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y );
		pButton->addActionSignal( new CMenuHandler_TextWindow( MENU_INTRO ) );
	}

	return pButton;
}

//=======================================================================
void TeamFortressViewport::ShowCommandMenu( int menuIndex )
{
	if( !m_iInitialized )
		return;

	// Already have a menu open.
	if( m_pCurrentMenu )  
		return;

	// is the command menu open?
	if( m_pCurrentCommandMenu == m_pCommandMenus[menuIndex] )
	{
		HideCommandMenu();
		return;
	}

	// Not visible while in intermission
	if( gHUD.m_iIntermission )
		return;

	// Recalculate visible menus
	UpdateCommandMenu( menuIndex );
	HideVGUIMenu();

	SetCurrentCommandMenu( m_pCommandMenus[menuIndex] );
	m_flMenuOpenTime = gHUD.m_flTime;
	UpdateCursorState();

	// get command menu parameters
	for( int i = 2; i < gEngfuncs.Cmd_Argc(); i++ )
	{
		const char *param = gEngfuncs.Cmd_Argv( i - 1 );
		if( param )
		{
			if( m_pCurrentCommandMenu->KeyInput(param[0]) )
			{
				// kill the menu open time, since the key input is final
				HideCommandMenu();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles the key input of "-commandmenu"
// Input  : 
//-----------------------------------------------------------------------------
void TeamFortressViewport::InputSignalHideCommandMenu()
{
	if( !m_iInitialized )
		return;

	// if they've just tapped the command menu key, leave it open
	if( (m_flMenuOpenTime + 0.3 ) > gHUD.m_flTime )
		return;

	HideCommandMenu();
}

//-----------------------------------------------------------------------------
// Purpose: Hides the command menu
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideCommandMenu()
{
	if( !m_iInitialized )
		return;

	if( m_pCommandMenus[m_StandardMenu] )
	{
		m_pCommandMenus[m_StandardMenu]->ClearButtonsOfArmedState();
	}

	m_flMenuOpenTime = 0.0f;
	SetCurrentCommandMenu( NULL );
	UpdateCursorState();
}

//-----------------------------------------------------------------------------
// Purpose: Bring up the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::ShowScoreBoard( void )
{
	if( m_pScoreBoard )
	{
		// No Scoreboard in single-player
		if( gEngfuncs.GetMaxClients() > 1 )
		{
			m_pScoreBoard->Open();
			UpdateCursorState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the scoreboard is up
//-----------------------------------------------------------------------------
bool TeamFortressViewport::IsScoreBoardVisible( void )
{
	if( m_pScoreBoard )
		return m_pScoreBoard->isVisible();

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Hide the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideScoreBoard( void )
{
	// Prevent removal of scoreboard during intermission
	if( gHUD.m_iIntermission )
		return;

	if( m_pScoreBoard && m_pScoreBoard->isVisible() )
	{
		m_pScoreBoard->setVisible( false );

		GetClientVoiceMgr()->StopSquelchMode();

		UpdateCursorState();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activate's the player special ability
//			called when the player hits their "special" key
//-----------------------------------------------------------------------------
void TeamFortressViewport::InputPlayerSpecial( void )
{
	if( !m_iInitialized )
		return;

	// if it's any other class, just send the command down to the server
	ClientCmd( "_special" );
}

// Set the submenu of the Command Menu
void TeamFortressViewport::SetCurrentCommandMenu( CCommandMenu *pNewMenu )
{
	for( int i = 0; i < m_iNumMenus; i++ )
		m_pCommandMenus[i]->setVisible( false );

	m_pCurrentCommandMenu = pNewMenu;

	if( m_pCurrentCommandMenu )
		m_pCurrentCommandMenu->MakeVisible( NULL );
}

void TeamFortressViewport::UpdateCommandMenu( int menuIndex )
{
	m_pCommandMenus[menuIndex]->RecalculateVisibles( 0, false );
	m_pCommandMenus[menuIndex]->RecalculatePositions( 0 );
}

void COM_FileBase( const char *in, char *out );

// The text of the bottom bar and the target of the GUI (TeamFortressViewport::UpdateSpectatorPanel of the original)
void TeamFortressViewport::UpdateSpectatorPanel()
{
	char szBottomText[128];
	char szMapName[128];
	int iTarget;

	if( !m_pSpectatorGUI )
		return;

	snprintf( szBottomText, sizeof( szBottomText ), "#Spec_Mode%d", g_iUser1 );

	// check if spectator combinations are still valid
	gHUD.m_Spectator.CheckSettings();

	// the target is the player that is followed, the roaming has none
	if( g_iUser2 <= 0 || g_iUser2 > gEngfuncs.GetMaxClients() || g_iUser1 == OBS_ROAMING )
		iTarget = -1;
	else
		iTarget = g_iUser2;

	COM_FileBase( gEngfuncs.pfnGetLevelName(), szMapName );

	// nothing is updated without a target; the name of the player is not shown in the free overview with the inset window
	if( iTarget > 0 )
	{
		bool bPipInsetOff = ( gHUD.m_Spectator.m_pip->value == INSET_OFF );

		if( !( g_iUser1 == OBS_MAP_FREE && !bPipInsetOff ) )
		{
			gEngfuncs.pfnGetPlayerInfo( iTarget, &g_PlayerInfoList[iTarget] );

			if( g_PlayerInfoList[iTarget].name )
				strlcpy( szBottomText, g_PlayerInfoList[iTarget].name, sizeof( szBottomText ) );
		}

		m_pSpectatorGUI->Update( szBottomText, iTarget, g_iUser1, gEngfuncs.IsSpectateOnly() != 0, gHUD.m_Spectator.m_iSpectatorNumber, szMapName );
		m_pSpectatorGUI->UpdateSpectatorPlayerList();
	}
}

// The GUI shows the label of the player in the bottom bar
void TeamFortressViewport::ShowSpectatorGUI( void )
{
	if( m_iInitialized && m_pSpectatorGUI )
		m_pSpectatorGUI->Activate();
}

// The bottom bar with the lists
void TeamFortressViewport::ShowSpectatorGUIBar( void )
{
	if( m_iInitialized && m_pSpectatorGUI )
		m_pSpectatorGUI->ShowBottomBar();
}

void TeamFortressViewport::HideSpectatorGUI( void )
{
	if( m_iInitialized && m_pSpectatorGUI )
	{
		if( m_pSpectatorGUI->IsActiveGUIVisible() )
		{
			if( gEngfuncs.IsSpectateOnly() )
				DuckMessage( "#Spec_Duck" );
		}

		m_pSpectatorGUI->HideGUI();
	}
}

void TeamFortressViewport::DeactivateSpectatorGUI( void )
{
	if( m_iInitialized && m_pSpectatorGUI )
	{
		m_pSpectatorGUI->DeactivateGUI();

		DuckMessage( "#Spec_Duck" );
	}
}

void TeamFortressViewport::SpectatorGUIEnableInsetView( int iValue )
{
	if( m_pSpectatorGUI )
		m_pSpectatorGUI->EnableInsetView( iValue );
}

void TeamFortressViewport::SetSpectatorBanner( const char *pszImage )
{
	if( m_pSpectatorGUI )
		m_pSpectatorGUI->SetImage( pszImage );
}

bool TeamFortressViewport::IsSpectatorGUIVisible( void )
{
	return m_iInitialized && m_pSpectatorGUI && m_pSpectatorGUI->IsGUIVisible();
}

bool TeamFortressViewport::IsSpectatorBarVisible( void )
{
	return m_iInitialized && m_pSpectatorGUI && m_pSpectatorGUI->IsActiveGUIVisible();
}

void TeamFortressViewport::SpectatorGUIDuckKey( void )
{
	if( m_pSpectatorGUI )
		m_pSpectatorGUI->OnDuckKey();
}

//======================================================================
void TeamFortressViewport::CreateScoreBoard( void )
{
	int xdent = SBOARD_INDENT_X, ydent = SBOARD_INDENT_Y;
	if( ScreenWidth == 512 )
	{
		xdent = SBOARD_INDENT_X_512; 
		ydent = SBOARD_INDENT_Y_512;
	}
	else if( ScreenWidth == 400 )
	{
		xdent = SBOARD_INDENT_X_400; 
		ydent = SBOARD_INDENT_Y_400;
	}

	m_pScoreBoard = new ScorePanel( xdent, ydent, ScreenWidth - ( xdent * 2 ), ScreenHeight - ( ydent * 2 ) );
	m_pScoreBoard->setParent( this );
	m_pScoreBoard->setVisible( false );
}

//======================================================================
// Set the VGUI Menu
void TeamFortressViewport::SetCurrentMenu( CMenuPanel *pMenu )
{
	m_pCurrentMenu = pMenu;
	if( m_pCurrentMenu )
	{
		// Don't open menus in demo playback
		if( gEngfuncs.pDemoAPI->IsPlayingback() )
			return;

		m_pCurrentMenu->Open();
	}
	else
	{
		gEngfuncs.pfnClientCmd( "closemenus;" );
	}
}

//================================================================
// The description of the map (the file maps/<map>.txt): the text window is the one of the help of the spectator (CVGUITextWindow of
// the original)
CMenuPanel *TeamFortressViewport::ShowMapBriefing( void )
{
	char sz[256];

	// Get the current mapname, and open it's map briefing text
	if( m_sMapName[0] )
	{
		strcpy( sz, "maps/" );
		strcat( sz, m_sMapName );
		strcat( sz, ".txt" );
	}
	else
	{
		const char *level = gEngfuncs.pfnGetLevelName();

		if( !level )
			return NULL;

		strcpy( sz, level );
		char *ch = strchr( sz, '.' );

		if( ch )
			*ch = '\0';

		strcat( sz, ".txt" );

		// pull out the map name
		strcpy( m_sMapName, level );
		ch = strchr( m_sMapName, '.' );

		if( ch )
			*ch = 0;

		ch = strchr( m_sMapName, '/' );

		if( ch )
		{
			// move the string back over the '/'
			memmove( m_sMapName, ch + 1, strlen( ch ) + 1 );
		}
	}

	char *pfile = (char *)gEngfuncs.COM_LoadFile( sz, 5, NULL );

	if( !pfile )
		return NULL;

	if( !m_pMapBriefing )
	{
		m_pMapBriefing = new CDoDTextWindow( false, 0, 0, ScreenWidth, ScreenHeight );
		m_pMapBriefing->setParent( this );
		m_pMapBriefing->setVisible( false );
	}

	m_pMapBriefing->Reset();
	m_pMapBriefing->Activate( pfile );

	gEngfuncs.COM_FreeFile( pfile );

	return m_pMapBriefing;
}

//================================================================
// VGUI Menus
void TeamFortressViewport::ShowVGUIMenu( int iMenu )
{
	CMenuPanel *pNewMenu = NULL;

	// Don't open menus in demo playback
	if( gEngfuncs.pDemoAPI->IsPlayingback() )
		return;

	// Don't open any menus except the MOTD during intermission
	// MOTD needs to be accepted because it's sent down to the client 
	// after map change, before intermission's turned off
	if( gHUD.m_iIntermission && iMenu != MENU_INTRO )
		return;

	// Don't create one if it's already in the list
	if( m_pCurrentMenu )
	{
		CMenuPanel *pMenu = m_pCurrentMenu;
		while( pMenu != NULL )
		{
			if( pMenu->GetMenuID() == iMenu )
				return;

			pMenu = pMenu->GetNextMenu();
		}
	}

	switch( iMenu )
	{
	case MENU_TEAM:		
		pNewMenu = ShowTeamMenu(); 
		break;

	// Map Briefing removed now that it appears in the team menu
	case MENU_MAPBRIEFING:
		pNewMenu = ShowMapBriefing();
		break;

	case MENU_INTRO:
		pNewMenu = ShowMOTDMenu();
		break;

	case MENU_SPECHELP:
		pNewMenu = ShowSpectatorHelp();
		break;
	case MENU_CLASS:
		pNewMenu = ShowClassMenu();
		break;

	default:
		break;
	}

	if( !pNewMenu )
		return;

	// Close the Command Menu if it's open
	HideCommandMenu();

	// A menu takes the screen: the spectator GUI is hidden (the pending dialogs of OnTick of the original)
	if( IsSpectatorGUIVisible() )
		HideSpectatorGUI();

	pNewMenu->SetMenuID( iMenu );
	pNewMenu->SetActive( true );
	pNewMenu->setParent( this );

	// See if another menu is visible, and if so, cache this one for display once the other one's finished
	if( m_pCurrentMenu )
	{
		if( m_pCurrentMenu->GetMenuID() == MENU_CLASS && iMenu == MENU_TEAM )
		{
			CMenuPanel *temp = m_pCurrentMenu;
			m_pCurrentMenu->Close();
			m_pCurrentMenu = pNewMenu;
			m_pCurrentMenu->SetNextMenu( temp );
			m_pCurrentMenu->Open();
			UpdateCursorState();
		}
		else
		{
			m_pCurrentMenu->SetNextMenu( pNewMenu );
		}
	}
	else
	{
		m_pCurrentMenu = pNewMenu;
		m_pCurrentMenu->Open();
		UpdateCursorState();
	}
}

// Removes all VGUI Menu's onscreen
void TeamFortressViewport::HideVGUIMenu()
{
	while( m_pCurrentMenu )
	{
		HideTopMenu();
	}
}

// Remove the top VGUI menu, and bring up the next one
void TeamFortressViewport::HideTopMenu()
{
	if( m_pCurrentMenu )
	{
		// Close the top one
		m_pCurrentMenu->Close();

		// Bring up the next one
		gViewPort->SetCurrentMenu( m_pCurrentMenu->GetNextMenu() );
	}

	UpdateCursorState();
}

// Return TRUE if the HUD's allowed to print text messages
bool TeamFortressViewport::AllowedToPrintText( void )
{
	// Prevent text messages when fullscreen menus are up
	if( m_pCurrentMenu && g_iPlayerClass == 0 )
	{
		int iId = m_pCurrentMenu->GetMenuID();
		if( iId == MENU_TEAM || iId == MENU_CLASS || iId == MENU_INTRO )
			return FALSE;
	}

	return TRUE;
}

//======================================================================================
// TEAM MENU
//======================================================================================
// Bring up the Team selection Menu
CMenuPanel* TeamFortressViewport::ShowTeamMenu()
{
	// Don't open menus in demo playback
	if( gEngfuncs.pDemoAPI->IsPlayingback() )
		return NULL;

	m_pTeamMenu->Reset();
	return m_pTeamMenu;
}

void TeamFortressViewport::CreateTeamMenu()
{
	// Create the panel
	m_pTeamMenu = new CDoDTeamMenu( false, 0, 0, ScreenWidth, ScreenHeight );
	m_pTeamMenu->setParent( this );
	m_pTeamMenu->setVisible( false );
}

//======================================================================================
// CLASS MENU
//======================================================================================
// Bring up the Class selection Menu
CMenuPanel* TeamFortressViewport::ShowClassMenu()
{
	// Don't open menus in demo playback
	if( gEngfuncs.pDemoAPI->IsPlayingback() )
		return NULL;

	m_pClassMenu->Reset();
	return m_pClassMenu;
}

void TeamFortressViewport::CreateClassMenu()
{
	// Create the panel
	m_pClassMenu = new CDoDClassMenu( false, 0, 0, ScreenWidth, ScreenHeight );
	m_pClassMenu->setParent( this );
	m_pClassMenu->setVisible( false );
}

//======================================================================================
//======================================================================================
// SPECTATOR MENU
//======================================================================================
// Spectator "Menu" explaining the Spectator buttons
void TeamFortressViewport::CreateSpectatorMenu()
{
	// Create the Panel
	m_pSpectatorGUI = new CDoDSpectatorGUI( 0, 0, ScreenWidth, ScreenHeight );
	m_pSpectatorGUI->setParent( this );
	m_pSpectatorGUI->setVisible( false );
}

//======================================================================================
// UPDATE HUD SECTIONS
//======================================================================================
// We've got an update on player info
// Recalculate any menus that use it.
void TeamFortressViewport::UpdateOnPlayerInfo()
{
	if( m_pTeamMenu )
		m_pTeamMenu->Update();

	if( m_pClassMenu )
		m_pClassMenu->Update();

	if( m_pScoreBoard )
		m_pScoreBoard->Update();
}

void TeamFortressViewport::UpdateCursorState()
{
	// Need cursor if any VGUI window is up
	if( IsSpectatorBarVisible() || m_pCurrentMenu || m_pTeamMenu->isVisible() || GetClientVoiceMgr()->IsInSquelchMode() )
	{
		CurrentMouseInput()->IN_SetVisibleMouse(true);
		IgnoreNextMouseDelta();
		App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow) );
		return;
	}
	else if( m_pCurrentCommandMenu )
	{
		// commandmenu doesn't have cursor if hud_capturemouse is turned off
		if( gHUD.m_pCvarStealMouse->value != 0.0f )
		{
			CurrentMouseInput()->IN_SetVisibleMouse(true);
			IgnoreNextMouseDelta();
			App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow) );
			return;
		}
	}

	App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_none) );
	CurrentMouseInput()->IN_SetVisibleMouse(false);

	// Don't reset mouse in demo playback
	if( !gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		IN_ResetMouse();
	}
}

void TeamFortressViewport::UpdateHighlights()
{
	if( m_pCurrentCommandMenu )
		m_pCurrentCommandMenu->MakeVisible( NULL );
}

void TeamFortressViewport::paintBackground()
{
	int wide, tall;
	getParent()->getSize( wide, tall );
	setSize( wide, tall );

	// See if the command menu is visible and needs recalculating due to some external change
	if( g_iTeamNumber != m_iCurrentTeamNumber )
	{
		UpdateCommandMenu( m_StandardMenu );

		if( m_pClassMenu )
		{
			m_pClassMenu->Update();
		}

		m_iCurrentTeamNumber = g_iTeamNumber;
	}

	if( g_iPlayerClass != m_iCurrentPlayerClass )
	{
		UpdateCommandMenu( m_StandardMenu );

		m_iCurrentPlayerClass = g_iPlayerClass;
	}

	// OnTick of the original: the spectator GUI follows the mode of the observer
	if( g_iUser1 != m_iUser1 || g_iUser2 != m_iUser2 )
	{
		m_iUser1 = g_iUser1;
		m_iUser2 = g_iUser2;

		UpdateSpectatorPanel();
	}

	if( !g_iUser1 || IsSpectatorGUIVisible() || IsAnyVGUIMenuActive() )
	{
		if( !g_iUser1 && IsSpectatorGUIVisible() )
			HideSpectatorGUI();
	}
	else
	{
		ShowSpectatorGUI();
	}

	// Update the Scoreboard, if it's visible
	if( m_pScoreBoard->isVisible() && ( m_flScoreBoardLastUpdated < gHUD.m_flTime ) )
	{
		m_pScoreBoard->Update();
		m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;
	}

	int extents[4];

	getAbsExtents( extents[0], extents[1], extents[2], extents[3] );

	VGui_ViewportPaintBackground( extents );
}

//================================================================
// Number Key Input
bool TeamFortressViewport::SlotInput( int iSlot )
{
	// If there's a menu up, give it the input
	if( m_pCurrentMenu )
		return m_pCurrentMenu->SlotInput( iSlot );

	return FALSE;
}

// Direct Key Input
int TeamFortressViewport::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	// Enter gets out of Spectator Mode by bringing up the Team Menu
	if( m_iUser1 && gEngfuncs.Con_IsVisible() == false )
	{
		if( down && ( keynum == K_ENTER || keynum == K_KP_ENTER ) )
			ShowVGUIMenu( MENU_TEAM );
	}

	// Open Text Window?
	if( m_pCurrentMenu && gEngfuncs.Con_IsVisible() == false )
	{
		int iMenuID = m_pCurrentMenu->GetMenuID();

		// Get number keys as Input for Team/Class menus
		if( iMenuID == MENU_TEAM || iMenuID == MENU_CLASS )
		{
			// Escape gets you out of Team/Class menus if the Cancel button is visible
			if( keynum == K_ESCAPE )
			{
				if( ( iMenuID == MENU_TEAM && g_iTeamNumber ) || ( iMenuID == MENU_CLASS && g_iPlayerClass ) )
				{
					HideTopMenu();
					return 0;
				}
			}

			for( int i = '0'; i <= '9'; i++ )
			{
				if( down && ( keynum == i ) )
				{
					SlotInput( i - '0' );
					return 0;
				}
			}
		}

		// Grab enter keys to close TextWindows
		if( down && ( keynum == K_ENTER || keynum == K_KP_ENTER || keynum == K_SPACE || keynum == K_ESCAPE ) )
		{
			if( iMenuID == MENU_MAPBRIEFING || iMenuID == MENU_INTRO || iMenuID == MENU_SPECHELP )
			{
				HideTopMenu();
				return 0;
			}
		}

		// Grab jump key on Team Menu as autoassign
		if( pszCurrentBinding && down && !strcmp( pszCurrentBinding, "+jump" ) )
		{
			if( iMenuID == MENU_TEAM )
			{
				m_pTeamMenu->SlotInput( 5 );
				return 0;
			}
		}

	}

	// if we're in a command menu, try hit one of it's buttons
	if( down && m_pCurrentCommandMenu )
	{
		// Escape hides the command menu
		if( keynum == K_ESCAPE )
		{
			HideCommandMenu();
			return 0;
		}

		// only trap the number keys
		if( keynum >= '0' && keynum <= '9' )
		{
			if( m_pCurrentCommandMenu->KeyInput( keynum ) )
			{
				// a final command has been issued, so close the command menu
				HideCommandMenu();
			}

			return 0;
		}
	}

	return 1;
}

//================================================================
// Message Handlers
int TeamFortressViewport::MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iNumberOfTeams = READ_BYTE();

	for( int i = 0; i < m_iNumberOfTeams; i++ )
	{
		int teamNum = i + 1;

		gHUD.m_TextMessage.LocaliseTextString( READ_STRING(), m_sTeamNames[teamNum], MAX_TEAMNAME_SIZE );
	}

	// Update the Team Menu
	if( m_pTeamMenu )
		m_pTeamMenu->Update();

	return 1;
}

int TeamFortressViewport::MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iMenu = READ_BYTE();

	// Map briefing includes the name of the map (because it's sent down before the client knows what map it is)
	if( iMenu == MENU_MAPBRIEFING )
		strlcpy( m_sMapName, READ_STRING(), sizeof( m_sMapName ));

	// Bring up the menu6
	ShowVGUIMenu( iMenu );

	return 1;
}

CMenuPanel *TeamFortressViewport::ShowMOTDMenu( void )
{
	// the message of the day is the panel of DoDViewport
	return NULL;
}

// The help of the spectator: the window is made once, its text is the token of the localization
CMenuPanel *TeamFortressViewport::ShowSpectatorHelp( void )
{
	if( !m_pSpecHelp )
	{
		m_pSpecHelp = new CDoDTextWindow( false, 0, 0, ScreenWidth, ScreenHeight );
		m_pSpecHelp->setParent( this );
		m_pSpecHelp->setVisible( false );
		m_pSpecHelp->SetSpectatorHelp( true );
	}

	const char *pszText = DoD_FindLocalized( "#Spec_Help_Text" );

	m_pSpecHelp->Reset();
	m_pSpecHelp->Activate( pszText ? pszText : "" );

	return m_pSpecHelp;
}

void TeamFortressViewport::ShowMOTD()
{
	// don't show MOTD for HLTV spectators
	if( !gEngfuncs.IsSpectateOnly() )
	{
		ShowVGUIMenu( MENU_INTRO );
	}
}

int TeamFortressViewport::MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iRandomPC = READ_BYTE();

	return 1;
}

int TeamFortressViewport::MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	strlcpy( m_szServerName, READ_STRING(), sizeof( m_szServerName ));

	return 1;
}

int TeamFortressViewport::MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	short cl = READ_BYTE();
	if( cl > 0 && cl <= MAX_PLAYERS )
	{
		g_IsSpectator[cl] = READ_BYTE();
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iAllowSpectators = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu( m_StandardMenu );

	// If the team menu is up, update it too
	if( m_pTeamMenu )
		m_pTeamMenu->Update();

	return 1;
}

// used to reset the player's screen immediately
int TeamFortressViewport::MsgFunc_ResetFade( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

// used to fade a player's screen out/in when they're spectating someone who is teleported
int TeamFortressViewport::MsgFunc_SpecFade( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}
