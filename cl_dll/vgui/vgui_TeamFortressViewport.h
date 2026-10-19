//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef TEAMFORTRESSVIEWPORT_H
#define TEAMFORTRESSVIEWPORT_H

#include <VGUI_Panel.h>
#include <VGUI_TextPanel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>
#include <VGUI_ActionSignal.h>
#include <VGUI_InputSignal.h>
#include <VGUI_Scheme.h>
#include <VGUI_Image.h>
#include <VGUI_FileInputStream.h>
#include <VGUI_BitmapTGA.h>
#include <VGUI_App.h>
#include <VGUI_LineBorder.h>
#include <VGUI_ScrollPanel.h>
#include <VGUI_ScrollBar.h>
#include <VGUI_Slider.h>

#include "hud.h"

// custom scheme handling
#include "vgui/vgui_SchemeManager.h"

#define MENU_TEAM 					2
#define MENU_CLASS 					3
#define MENU_MAPBRIEFING			4
#define MENU_INTRO 					5
#define MENU_SPECHELP				9
using namespace vgui;

class Cursor;
class ScorePanel;
class CDoDSpectatorGUI;
class CCommandMenu;
class CommandLabel;
class CommandButton;
class CMenuPanel;
class CTransparentPanel;
class CDoDClassMenu;
class CDoDTextWindow;
class CDoDTeamMenu;
class TeamFortressViewport;

char *GetVGUITGAName( const char *pszName );
BitmapTGA *LoadTGAForRes( const char* pImageName );
void ScaleColors( int &r, int &g, int &b, int a );
extern TeamFortressViewport *gViewPort;


// Command Menu positions 
#define MAX_MENUS			80
#define MAX_BUTTONS			100

#define BUTTON_SIZE_Y			YRES(30)
#define CMENU_SIZE_X			XRES(160)

#define SUBMENU_SIZE_X			(CMENU_SIZE_X / 8)
#define SUBMENU_SIZE_Y			(BUTTON_SIZE_Y / 6)

#define CMENU_TOP			(BUTTON_SIZE_Y * 4)

//#define MAX_TEAMNAME_SIZE		64
#define MAX_BUTTON_SIZE			32

// Arrows
enum
{
	ARROW_UP,
	ARROW_DOWN,
	ARROW_LEFT,
	ARROW_RIGHT,
};

//==============================================================================
// VIEWPORT PIECES
//============================================================
// Wrapper for an Image Label without a background
class CImageLabel : public Label
{
public:
	BitmapTGA	*m_pTGA;

	void LoadImage(const char * pImageName);
	CImageLabel( const char* pImageName,int x,int y );
	CImageLabel( const char* pImageName,int x,int y,int wide,int tall );

	virtual int getImageTall();
	virtual int getImageWide();

	virtual void paintBackground()
	{
		// Do nothing, so the background's left transparent.
	}
};

// Command Label
// Overridden label so we can darken it when submenus open
class CommandLabel : public Label
{
private:
	int		m_iState;

public:
	CommandLabel( const char *text, int x, int y, int wide, int tall ) : Label( text, x, y, wide, tall )
	{
		m_iState = false;
	}

	void PushUp()
	{
		m_iState = false;
		repaint();
	}

	void PushDown()
	{
		m_iState = true;
		repaint();
	}
};

//============================================================
// Command Buttons
class CommandButton : public Button
{
private:
	int		m_iPlayerClass;
	bool		m_bFlat;

	// Submenus under this button
	CCommandMenu *m_pSubMenu;
	CCommandMenu *m_pParentMenu;
	CommandLabel *m_pSubLabel;

	char m_sMainText[MAX_BUTTON_SIZE];
	char m_cBoundKey;

	SchemeHandle_t m_hTextScheme;

	void RecalculateText( void );

public:
	bool	m_bNoHighlight;

	CommandButton(const char* text,int x,int y,int wide,int tall, bool bNoHighlight, bool bFlat);
	// Constructors
	CommandButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight = false);
	CommandButton( int iPlayerClass, const char* text,int x,int y,int wide,int tall, bool bFlat );

	void Init( void );

	// Menu Handling
	void AddSubMenu( CCommandMenu *pNewMenu );
	void AddSubLabel( CommandLabel *pSubLabel )
	{
		m_pSubLabel = pSubLabel;
	}

	virtual int IsNotValid( void )
	{
		return false;
	}

	void UpdateSubMenus( int iAdjustment );
	int GetPlayerClass() { return m_iPlayerClass; };
	CCommandMenu *GetSubMenu() { return m_pSubMenu; };

	CCommandMenu *getParentMenu( void );
	void setParentMenu( CCommandMenu *pParentMenu );

	// Overloaded vgui functions
	virtual void paint();
	virtual void setText( const char *text );
	virtual void paintBackground();

	void cursorEntered( void );
	void cursorExited( void );

	void setBoundKey( char boundKey );
	char getBoundKey( void );
};

//============================================================
// Command Menus
class CCommandMenu : public Panel
{
private:
	CCommandMenu *m_pParentMenu;
	int			  m_iXOffset;
	int			  m_iYOffset;

	// Buttons in this menu
	CommandButton *m_aButtons[ MAX_BUTTONS ];
	int			  m_iButtons;

	// opens menu from top to bottom (0 = default), or from bottom to top (1)?
	int				m_iDirection; 
public:
	CCommandMenu( CCommandMenu *pParentMenu, int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset = x;
		m_iYOffset = y;
		m_iButtons = 0;
		m_iDirection = 0;
	}

	CCommandMenu( CCommandMenu *pParentMenu, int direction, int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset = x;
		m_iYOffset = y;
		m_iButtons = 0;
		m_iDirection = direction;
	}

	float		m_flButtonSizeY;
	void		AddButton( CommandButton *pButton );
	bool		RecalculateVisibles( int iNewYPos, bool bHideAll );
	void		RecalculatePositions( int iYOffset );
	void		MakeVisible( CCommandMenu *pChildMenu );

	CCommandMenu	*GetParentMenu() { return m_pParentMenu; };
	int		 GetXOffset() { return m_iXOffset; };
	int		 GetYOffset() { return m_iYOffset; };
	int		 GetDirection() { return m_iDirection; };
	int		 GetNumButtons() { return m_iButtons; };
	CommandButton	*FindButtonWithSubmenu( CCommandMenu *pSubMenu );

	void		 ClearButtonsOfArmedState( void );

	void		RemoveAllButtons(void);


	bool		 KeyInput( int keyNum );

	virtual		 void paintBackground();
};
//==============================================================================
class TeamFortressViewport : public Panel
{
private:
	vgui::Cursor* _cursorNone;
	vgui::Cursor* _cursorArrow;

	int			 m_iInitialized;

	CCommandMenu *m_pCommandMenus[MAX_MENUS];
	CCommandMenu *m_pCurrentCommandMenu;
	float		 m_flMenuOpenTime;
	float		 m_flScoreBoardLastUpdated;
	float		 m_flSpectatorPanelLastUpdated;
	int			 m_iNumMenus;
	int			 m_iCurrentTeamNumber;
	int			 m_iCurrentPlayerClass;
	int			 m_iUser1;
	int			 m_iUser2;
	int			 m_iUser3;

	// VGUI Menus
	void		 CreateTeamMenu( void );
	CMenuPanel*	 ShowTeamMenu( void );
	void		 CreateClassMenu( void );
	CMenuPanel*	 ShowClassMenu( void );
	void		 CreateSpectatorMenu( void );
	
	// Scheme handler
	CSchemeManager m_SchemeManager;

	int					m_iAllowSpectators;

	// Data for specific sections of the Command Menu
	int			m_iNumberOfTeams;
	int			m_iRandomPC;
	char		m_sTeamNames[5][MAX_TEAMNAME_SIZE];

	char		m_sMapName[64];

	// helper function to update the player menu entries

public:
	TeamFortressViewport(int x,int y,int wide,int tall);
	virtual ~TeamFortressViewport() {}
	void Initialize( void );

	int		CreateCommandMenu( const char * menuFile, int direction, int yOffset, bool flatDesign, float flButtonSizeX, float flButtonSizeY, int xOffset );
	void	CreateScoreBoard( void );
	CommandButton * CreateCustomButton( char *pButtonText, char * pButtonName, int  iYOffset );

	void UpdateCursorState( void );
	void UpdateCommandMenu(int menuIndex);
	void UpdateOnPlayerInfo( void );
	void UpdateHighlights( void );

	// The spectator GUI (IViewPort of the original)
	virtual void UpdateSpectatorPanel( void );
	void ShowSpectatorGUI( void );
	void ShowSpectatorGUIBar( void );
	void HideSpectatorGUI( void );
	void DeactivateSpectatorGUI( void );
	void SpectatorGUIEnableInsetView( int iValue );
	void SetSpectatorBanner( const char *pszImage );
	bool IsSpectatorGUIVisible( void );
	bool IsSpectatorBarVisible( void );
	bool IsAnyVGUIMenuActive( void ) { return m_pCurrentMenu != NULL; }

	// VGUI1 has no key input for the GUI: the key DUCK closes the bottom bar from here (OnKeyCodePressed of the GUI)
	void SpectatorGUIDuckKey( void );

	int	 KeyInput( int down, int keynum, const char *pszCurrentBinding );
	void InputPlayerSpecial( void );

	void ShowCommandMenu(int menuIndex);
	void InputSignalHideCommandMenu( void );
	void HideCommandMenu( void );
	void SetCurrentCommandMenu( CCommandMenu *pNewMenu );
	void SetCurrentMenu( CMenuPanel *pMenu );

	void ShowScoreBoard( void );
	void HideScoreBoard( void );
	bool IsScoreBoardVisible( void );

	bool AllowedToPrintText( void );

	virtual void ShowVGUIMenu( int iMenu );
	void HideVGUIMenu( void );
	void HideTopMenu( void );

	// The description of the map: the text window of the help of the spectator
	CMenuPanel *ShowMapBriefing( void );

	// The panel of the message of the day, DoDViewport has its own
	virtual CMenuPanel *ShowMOTDMenu( void );

	// The text window of the help of the spectator (CSpecHelpWindow of the original)
	CMenuPanel *ShowSpectatorHelp( void );

	CCommandMenu *CreateSubMenu( CommandButton *pButton, CCommandMenu *pParentMenu, int iYOffset, int iXOffset = 0 );

	// Data Handlers
	int GetNumberOfTeams() { return m_iNumberOfTeams; };
	int IsRandomPC() { return m_iRandomPC; };
	char *GetTeamName( int iTeam ) { return m_sTeamNames[iTeam]; };
	int GetAllowSpectators() { return m_iAllowSpectators; };

	// Message Handlers
	CHudMsgFunc( TeamNames );
	virtual CHudMsgFunc( VGUIMenu );
	void ShowMOTD();
	CHudMsgFunc( RandomPC );
	CHudMsgFunc( ServerName );
	CHudMsgFunc( Spectator );
	CHudMsgFunc( AllowSpec );
	CHudMsgFunc( SpecFade );	
	CHudMsgFunc( ResetFade );	

	// Input
	bool SlotInput( int iSlot );

	virtual void paintBackground();

	CSchemeManager *GetSchemeManager( void ) { return &m_SchemeManager; }
	ScorePanel *GetScoreBoard( void ) { return m_pScoreBoard; }

	void *operator new( size_t stAllocateBlock );

public:
	// VGUI Menus
	CMenuPanel		*m_pCurrentMenu;
	CDoDTeamMenu	*m_pTeamMenu;
	int						m_StandardMenu;	// indexs in m_pCommandMenus
	CDoDClassMenu	*m_pClassMenu;
	ScorePanel		*m_pScoreBoard;
	CDoDSpectatorGUI *m_pSpectatorGUI;
	CDoDTextWindow *m_pSpecHelp;
	CDoDTextWindow *m_pMapBriefing;
	char			m_szServerName[ MAX_SERVERNAME_LENGTH ];
};

//============================================================
// Command Menu Button Handlers
#define MAX_COMMAND_SIZE	256

class CMenuHandler_StringCommand : public ActionSignal
{
protected:
	char	m_pszCommand[MAX_COMMAND_SIZE];
	int	m_iCloseVGUIMenu;
public:
	CMenuHandler_StringCommand( const char *pszCommand )
	{
		strlcpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE );
		m_iCloseVGUIMenu = false;
	}

	CMenuHandler_StringCommand( const char *pszCommand, int iClose )
	{
		strlcpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE );
		m_iCloseVGUIMenu = true;
	}

	virtual void actionPerformed(Panel *panel)
	{
		gEngfuncs.pfnFilteredClientCmd( m_pszCommand );

		if( m_iCloseVGUIMenu )
			gViewPort->HideTopMenu();
		else
			gViewPort->HideCommandMenu();
	}
};

// This works the same as CMenuHandler_StringCommand, except it watches the string command 
// for specific commands, and modifies client vars based upon them.
class CMenuHandler_StringCommandWatch : public CMenuHandler_StringCommand
{
private:
public:
	CMenuHandler_StringCommandWatch( const char *pszCommand ) : CMenuHandler_StringCommand( pszCommand )
	{
	}

	CMenuHandler_StringCommandWatch( const char *pszCommand, int iClose ) : CMenuHandler_StringCommand( pszCommand, iClose )
	{
	}

	virtual void actionPerformed( Panel *panel )
	{
		CMenuHandler_StringCommand::actionPerformed( panel );

		// Try to guess the player's new team (it'll be corrected if it's wrong)
		if( !strcmp( m_pszCommand, "jointeam 1" ) )
			g_iTeamNumber = 1;
		else if( !strcmp( m_pszCommand, "jointeam 2" ) )
			g_iTeamNumber = 2;
		else if( !strcmp( m_pszCommand, "jointeam 3" ) )
			g_iTeamNumber = 3;
		else if( !strcmp( m_pszCommand, "jointeam 4" ) )
			g_iTeamNumber = 4;
	}
};

class CMenuHandler_PopupSubMenuInput : public InputSignal
{
private:
	CCommandMenu	*m_pSubMenu;
	Button		*m_pButton;
public:
	CMenuHandler_PopupSubMenuInput( Button *pButton, CCommandMenu *pSubMenu )
	{
		m_pSubMenu = pSubMenu;
		m_pButton = pButton;
	}

	virtual void cursorMoved( int x, int y, Panel *panel )
	{
		// gViewPort->SetCurrentCommandMenu( m_pSubMenu );
	}

	virtual void cursorEntered( Panel *panel )
	{
		gViewPort->SetCurrentCommandMenu( m_pSubMenu );

		if( m_pButton )
			m_pButton->setArmed( true );
	};
	virtual void cursorExited(Panel* Panel) {};
	virtual void mousePressed(MouseCode code,Panel* panel)  {};
	virtual void mouseDoublePressed(MouseCode code,Panel* panel)  {};
	virtual void mouseReleased(MouseCode code,Panel* panel) {};
	virtual void mouseWheeled(int delta,Panel* panel) {};
	virtual void keyPressed(KeyCode code,Panel* panel) {};
	virtual void keyTyped(KeyCode code,Panel* panel) {};
	virtual void keyReleased(KeyCode code,Panel* panel) {};
	virtual void keyFocusTicked(Panel* panel) {};
};

#define HIDE_TEXTWINDOW		0

class CMenuHandler_TextWindow : public ActionSignal
{
private:
	int	m_iState;

public:
	CMenuHandler_TextWindow( int iState )
	{
		m_iState = iState;
	}

	virtual void actionPerformed( Panel *panel )
	{
		if( m_iState == HIDE_TEXTWINDOW )
		{
			gViewPort->HideTopMenu();
		}
		else 
		{
			gViewPort->HideCommandMenu();
			gViewPort->ShowVGUIMenu( m_iState );
		}
	}
};

class CMenuHandler_ToggleCvar : public ActionSignal
{
private:
	struct cvar_s	*m_cvar;

public:
	CMenuHandler_ToggleCvar( char *cvarname )
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer( cvarname );
	}

	virtual void actionPerformed( Panel *panel )
	{
		if( m_cvar->value )
			m_cvar->value = 0.0f;
		else
			m_cvar->value = 1.0f;

		// hide the menu 
		gViewPort->HideCommandMenu();

		gViewPort->UpdateSpectatorPanel();
	}
};


class CHandler_MenuButtonOver : public InputSignal
{
private:
	int		 m_iButton;
	CMenuPanel	*m_pMenuPanel;
public:
	CHandler_MenuButtonOver( CMenuPanel *pPanel, int iButton )
	{
		m_iButton = iButton;
		m_pMenuPanel = pPanel;
	}
		
	void cursorEntered( Panel *panel );

	void cursorMoved( int x, int y, Panel *panel ) {};
	void mousePressed( MouseCode code, Panel *panel ) {};
	void mouseReleased( MouseCode code, Panel *panel ) {};
	void mouseDoublePressed( MouseCode code, Panel *panel ) {};
	void cursorExited( Panel *panel) {};
	void mouseWheeled( int delta, Panel *panel ) {};
	void keyPressed( KeyCode code, Panel *panel ) {};
	void keyTyped( KeyCode code, Panel *panel ) {};
	void keyReleased( KeyCode code, Panel *panel ) {};
	void keyFocusTicked( Panel *panel ) {};
};

class CHandler_ButtonHighlight : public InputSignal
{
private:
	Button *m_pButton;
public:
	CHandler_ButtonHighlight( Button *pButton )
	{
		m_pButton = pButton;
	}

	virtual void cursorEntered( Panel *panel )
	{ 
		m_pButton->setArmed( true );
	};

	virtual void cursorExited( Panel *Panel ) 
	{
		m_pButton->setArmed( false );
	};

	virtual void mousePressed( MouseCode code, Panel *panel ) {};
	virtual void mouseReleased( MouseCode code, Panel *panel ) {};
	virtual void cursorMoved( int x, int y, Panel *panel ) {};
	virtual void mouseDoublePressed( MouseCode code, Panel *panel ) {};
	virtual void mouseWheeled( int delta, Panel *panel ) {};
	virtual void keyPressed( KeyCode code, Panel *panel ) {};
	virtual void keyTyped( KeyCode code, Panel *panel ) {};
	virtual void keyReleased( KeyCode code, Panel *panel ) {};
	virtual void keyFocusTicked( Panel *panel ) {};
};

//-----------------------------------------------------------------------------
// Purpose: Special handler for highlighting of command menu buttons
//-----------------------------------------------------------------------------
class CHandler_CommandButtonHighlight : public CHandler_ButtonHighlight
{
private:
	CommandButton *m_pCommandButton;
public:
	CHandler_CommandButtonHighlight( CommandButton *pButton ) : CHandler_ButtonHighlight( pButton )
	{
		m_pCommandButton = pButton;
	}

	virtual void cursorEntered( Panel *panel )
	{
		m_pCommandButton->cursorEntered();
	}

	virtual void cursorExited( Panel *panel )
	{
		m_pCommandButton->cursorExited();
	}
};

#define MAX_MAPNAME 256

class MapButton : public CommandButton
{
private:
	char m_szMapName[MAX_MAPNAME];

public:
	MapButton( const char *pMapName, const char *text, int x, int y, int wide, int tall ) : CommandButton( text, x, y, wide, tall )
	{
		sprintf( m_szMapName, "maps/%s.bsp", pMapName );
	}

	virtual int IsNotValid()
	{
		const char *level = gEngfuncs.pfnGetLevelName();
		if( !level )
			return true;

		// Does it match the current map name?
		if( strcmp( m_szMapName, level ) )
			return true;

		return false;
	}
};

//-----------------------------------------------------------------------------
// Purpose: CommandButton which is only displayed if the player is on team X
//-----------------------------------------------------------------------------
class TeamOnlyCommandButton : public CommandButton
{
private:
	int m_iTeamNum;

public:
	TeamOnlyCommandButton( int iTeamNum, const char *text, int x, int y, int wide, int tall, bool flat ) :
	CommandButton( text, x, y, wide, tall, false, flat ), m_iTeamNum( iTeamNum ) {}

	virtual int IsNotValid()
	{
		if( g_iTeamNumber != m_iTeamNum )
			return true;

		return CommandButton::IsNotValid();
	}
};

//-----------------------------------------------------------------------------
// Purpose: CommandButton which is only displayed if the player is on team X
//-----------------------------------------------------------------------------
class ToggleCommandButton : public CommandButton, public InputSignal
{
private:
	struct cvar_s	*m_cvar;
	CImageLabel	*pLabelOn;
	CImageLabel	*pLabelOff;

public:
	ToggleCommandButton( const char *cvarname, const char *text, int x, int y, int wide, int tall, bool flat ) : 
	CommandButton( text, x, y, wide, tall, false, flat )
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer( cvarname );

		// Put a > to show it's a submenu
		pLabelOn = new CImageLabel( "checked", 0, 0 );
		pLabelOn->setParent(this);
		pLabelOn->addInputSignal(this);
				
		pLabelOff = new CImageLabel( "unchecked", 0, 0 );
		pLabelOff->setParent(this);
		pLabelOff->setEnabled(true);
		pLabelOff->addInputSignal(this);

		int textwide, texttall;
		getTextSize( textwide, texttall );
	
		// Reposition
		pLabelOn->setPos( textwide, ( tall - pLabelOn->getTall() ) / 2 );

		pLabelOff->setPos( textwide, ( tall - pLabelOff->getTall() ) / 2 );
		
		// Set text color to orange
		setFgColor( Scheme::sc_primary1 );
	}

	virtual void cursorEntered( Panel *panel )
	{
		CommandButton::cursorEntered();
	}

	virtual void cursorExited( Panel *panel )
	{
		CommandButton::cursorExited();
	}

	virtual void mousePressed( MouseCode code, Panel *panel )
	{
		doClick();
	};

	virtual void cursorMoved( int x, int y, Panel *panel ) {};

	virtual void mouseDoublePressed( MouseCode code, Panel *panel)  {};
	virtual void mouseReleased( MouseCode code, Panel *panel ) {};
	virtual void mouseWheeled( int delta, Panel *panel ) {};
	virtual void keyPressed( KeyCode code, Panel *panel ) {};
	virtual void keyTyped( KeyCode code, Panel *panel ) {};
	virtual void keyReleased( KeyCode code, Panel *panel ) {};
	virtual void keyFocusTicked( Panel *panel) {};

	virtual void paint( void )
	{
		if( !m_cvar )
		{
			pLabelOff->setVisible( false );
			pLabelOn->setVisible( false );
		} 
		else if( m_cvar->value )
		{
			pLabelOff->setVisible( false );
			pLabelOn->setVisible( true );
		}
		else
		{
			pLabelOff->setVisible( true );
			pLabelOn->setVisible( false );
		}

		CommandButton::paint();
	}
};

//================================================================
// Panel that draws itself with a transparent black background
class CTransparentPanel : public Panel
{
private:
	int	m_iTransparency;
public:
	CTransparentPanel( int iTrans, int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
	{
		m_iTransparency = iTrans;
	}

	virtual void paintBackground()
	{
		if( m_iTransparency )
		{
			// Transparent black background
			drawSetColor( 0, 0, 0, m_iTransparency );
			drawFilledRect( 0, 0, _size[0], _size[1] );
		}
	}
};

//================================================================
// Menu Panel that supports buffering of menus
class CMenuPanel : public CTransparentPanel
{
private:
	CMenuPanel	*m_pNextMenu;
	int		 m_iMenuID;
	int		 m_iRemoveMe;
	int		 m_iIsActive;
	float		 m_flOpenTime;
public:
	CMenuPanel( int iRemoveMe, int x, int y, int wide, int tall ) : CTransparentPanel( 100, x, y, wide, tall )
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	CMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CTransparentPanel( iTrans, x, y, wide, tall )
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	virtual void Reset( void )
	{
		m_pNextMenu = NULL;
		m_iIsActive = false;
		m_flOpenTime = 0;
	}

	void SetNextMenu( CMenuPanel *pNextPanel )
	{
		if( m_pNextMenu )
			m_pNextMenu->SetNextMenu( pNextPanel );
		else
			m_pNextMenu = pNextPanel;
	}

	void SetMenuID( int iID )
	{
		m_iMenuID = iID;
	}

	void SetActive( int iState )
	{
		m_iIsActive = iState;
	}

	virtual void Open( void )
	{
		setVisible( true );

		// Note the open time, so we can delay input for a bit
		m_flOpenTime = gHUD.m_flTime;
	}

	virtual void Close( void )
	{
		setVisible( false );
		m_iIsActive = false;

		if ( m_iRemoveMe )
			gViewPort->removeChild( this );

		// This MenuPanel has now been deleted. Don't append code here.
	}

	int			ShouldBeRemoved() { return m_iRemoveMe; };
	CMenuPanel* GetNextMenu() { return m_pNextMenu; };
	int			GetMenuID() { return m_iMenuID; };
	int			IsActive() { return m_iIsActive; };
	float		GetOpenTime() { return m_flOpenTime; };

	// Numeric input
	virtual bool SlotInput( int iSlot ) { return false; };
	virtual void SetActiveInfo( int iInput ) {};
};
#endif
