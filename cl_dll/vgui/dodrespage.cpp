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
// dodrespage.cpp
//

#include <string.h>
#include <stdio.h>

#include <VGUI_SurfaceBase.h>

#include "hud.h"
#include "cl_util.h"
#include "vgui_loadtga.h"

#include "vgui/dodmenu.h"
#include "vgui/dodkeyvalues.h"
#include "vgui/dodrespage.h"

// The pixels of a loaded TGA are in the protected member of Bitmap, the image panel scales them to its size
class CBitmapAccess : public vgui::BitmapTGA
{
public:
	static const unsigned char *GetPixels( vgui::BitmapTGA *pBitmap )
	{
		return static_cast<CBitmapAccess *>( pBitmap )->_rgba;
	}
};

CDoDImagePanel::CDoDImagePanel( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_pBitmap = NULL;
	m_iTexture = 0;
	m_bFill = false;

	m_iFillColor[0] = 0;
	m_iFillColor[1] = 0;
	m_iFillColor[2] = 0;
}

void CDoDImagePanel::SetImage( const char *pszFile )
{
	char szPath[128];

	snprintf( szPath, sizeof( szPath ), "%s.tga", pszFile );

	m_pBitmap = vgui_LoadTGA( szPath );
	m_iTexture = 0;

	if( !m_pBitmap )
		gEngfuncs.Con_DPrintf( "Missing image: %s\n", szPath );
}

void CDoDImagePanel::SetFillColor( int r, int g, int b )
{
	m_bFill = true;

	m_iFillColor[0] = r;
	m_iFillColor[1] = g;
	m_iFillColor[2] = b;
}

void CDoDImagePanel::paintBackground( void )
{
	int wide, tall;

	if( !m_bFill )
		return;

	getPaintSize( wide, tall );

	// the alpha of VGUI1 is the transparency, 0 is opaque
	drawSetColor( m_iFillColor[0], m_iFillColor[1], m_iFillColor[2], 0 );
	drawFilledRect( 0, 0, wide, tall );
}

void CDoDImagePanel::paint( void )
{
	int wide, tall;

	if( !m_pBitmap || !getSurfaceBase() )
		return;

	if( !m_iTexture )
	{
		m_pBitmap->getSize( wide, tall );

		m_iTexture = getSurfaceBase()->createNewTextureID();
		drawSetTextureRGBA( m_iTexture, (const char *)CBitmapAccess::GetPixels( m_pBitmap ), wide, tall );
	}

	getPaintSize( wide, tall );

	drawSetColor( 255, 255, 255, 0 );
	drawSetTexture( m_iTexture );
	drawTexturedRect( 0, 0, wide, tall );
}

CDoDProgressBar::CDoDProgressBar( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_flProgress = 0.0f;
}

void CDoDProgressBar::SetProgress( float flProgress )
{
	if( flProgress < 0.0f )
		flProgress = 0.0f;
	else if( flProgress > 1.0f )
		flProgress = 1.0f;

	m_flProgress = flProgress;

	repaint();
}

void CDoDProgressBar::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	// DoDProgressBar::ApplySchemeSettings: the track is ClassMenuDark and the bar is ClassMenuLight
	drawSetColor( DODCOLOR_DARK, 0 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( DODCOLOR_LIGHT, 0 );
	drawFilledRect( 0, 0, (int)( wide * m_flProgress ), tall );
}

CDoDResPage::CDoDResPage( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
{
	m_iNumControls = 0;

	memset( m_Controls, 0, sizeof( m_Controls ) );

	setPaintBackgroundEnabled( false );
}

static int GetAlignment( const char *pszAlignment )
{
	if( !stricmp( pszAlignment, "east" ) )
		return vgui::Label::a_east;

	if( !stricmp( pszAlignment, "center" ) )
		return vgui::Label::a_center;

	return vgui::Label::a_west;
}

// The color of a fill: a name of the scheme or r g b a, the hex values of the files are not a color there
static void GetFillColor( const char *pszColor, int &r, int &g, int &b, bool &bValid )
{
	int iAlpha;

	bValid = true;

	if( !stricmp( pszColor, "ClassMenuDark" ) )
	{
		r = 64;
		g = 52;
		b = 36;
	}
	else if( !stricmp( pszColor, "ClassMenuLight" ) )
	{
		r = 175;
		g = 127;
		b = 1;
	}
	else if( strncmp( pszColor, "0x", 2 ) && sscanf( pszColor, "%d %d %d %d", &r, &g, &b, &iAlpha ) == 4 )
	{
		bValid = iAlpha != 0;
	}
	else
	{
		bValid = false;
	}
}

void CDoDResPage::AddControl( CDoDKeyValues *pKeys )
{
	const char *pszType = pKeys->GetString( "ControlName", "" );

	if( m_iNumControls >= MAX_CONTROLS )
		return;

	ResControl_t *pControl = &m_Controls[m_iNumControls];

	memset( pControl, 0, sizeof( *pControl ) );

	strncpy( pControl->szName, pKeys->GetString( "fieldName", pKeys->GetName() ), sizeof( pControl->szName ) - 1 );
	strncpy( pControl->szText, pKeys->GetString( "labelText", pKeys->GetString( "text", "" ) ), sizeof( pControl->szText ) - 1 );
	strncpy( pControl->szCommand, pKeys->GetString( "Command", "" ), sizeof( pControl->szCommand ) - 1 );

	pControl->x = pKeys->GetInt( "xpos" );
	pControl->y = pKeys->GetInt( "ypos" );
	pControl->wide = pKeys->GetInt( "wide" );
	pControl->tall = pKeys->GetInt( "tall" );

	pControl->iFont = DoD_FindMenuFont( pKeys->GetString( "font", "" ) );
	pControl->iAlignment = GetAlignment( pKeys->GetString( "textAlignment", "west" ) );
	pControl->bBright = pKeys->GetInt( "brighttext" ) != 0;
	pControl->bDull = pKeys->GetInt( "dulltext" ) != 0;
	pControl->bVisible = pKeys->GetInt( "visible", 1 ) != 0;
	pControl->flProgress = pKeys->GetFloat( "progress" );

	if( !stricmp( pszType, "Label" ) )
	{
		vgui::Label *pLabel = new vgui::Label( "", 0, 0, 10, 10 );

		pLabel->setBgColor( 0, 0, 0, 255 );
		pLabel->setContentAlignment( (vgui::Label::Alignment)pControl->iAlignment );

		pControl->iType = RESCONTROL_LABEL;
		pControl->pPanel = pLabel;
	}
	else if( !stricmp( pszType, "ImagePanel" ) )
	{
		CDoDImagePanel *pImage = new CDoDImagePanel( 0, 0, 10, 10 );
		int r, g, b;
		bool bValid;

		if( pKeys->GetString( "image", "" )[0] )
			pImage->SetImage( pKeys->GetString( "image" ) );

		GetFillColor( pKeys->GetString( "fillcolor", "" ), r, g, b, bValid );

		if( bValid )
			pImage->SetFillColor( r, g, b );

		pControl->iType = RESCONTROL_IMAGE;
		pControl->pPanel = pImage;
	}
	else if( !stricmp( pszType, "ProgressBar" ) )
	{
		CDoDProgressBar *pBar = new CDoDProgressBar( 0, 0, 10, 10 );

		pBar->SetProgress( pControl->flProgress );

		pControl->iType = RESCONTROL_BAR;
		pControl->pPanel = pBar;
	}
	else if( !stricmp( pszType, "RichText" ) || !stricmp( pszType, "TextEntry" ) )
	{
		pControl->iType = RESCONTROL_TEXT;
		pControl->pPanel = new CDoDRichText( 0, 0, 10, 10 );
	}
	else if( !stricmp( pszType, "HTML" ) )
	{
		pControl->iType = RESCONTROL_HTML;
		pControl->pPanel = new CDoDHtmlView( 0, 0, 10, 10 );
	}
	else if( !stricmp( pszType, "MouseOverPanelButton" ) || !stricmp( pszType, "DoDButton" ) || !stricmp( pszType, "Button" ) )
	{
		CDoDButton *pButton = new CDoDButton( "", 0, 0, 10, 10, DoD_GetMenuFont( DODFONT_DEFAULT ) );
		char szText[64];

		pButton->SetCommand( pControl->szCommand );

		DoD_GetLocalized( pControl->szText, szText, sizeof( szText ) );
		pButton->SetButtonText( szText );

		pControl->iType = RESCONTROL_BUTTON;
		pControl->bMouseOver = !stricmp( pszType, "MouseOverPanelButton" );
		pButton->SetPlain( !stricmp( pszType, "Button" ) );
		pControl->pPanel = pButton;
	}
	else
	{
		pControl->iType = RESCONTROL_OTHER;
	}

	if( pControl->pPanel )
	{
		pControl->pPanel->setParent( this );
		pControl->pPanel->setVisible( pControl->bVisible );
	}

	m_iNumControls++;
}

bool CDoDResPage::LoadControls( const char *pszFile )
{
	CDoDKeyValues *pRoot = CDoDKeyValues::LoadFromFile( pszFile );

	if( !pRoot )
		return false;

	// the file has one section, the sections inside are the controls
	CDoDKeyValues *pSection = pRoot->GetFirstSubKey();

	if( pSection )
	{
		CDoDKeyValues *pControls[MAX_CONTROLS];
		int iCount = 0;

		for( CDoDKeyValues *pKeys = pSection->GetFirstSubKey(); pKeys && iCount < MAX_CONTROLS; pKeys = pKeys->GetNextKey() )
		{
			// a control with a bigger zpos is above the others: the controls are made in the order of zpos, the
			// controls with the same zpos stay in the order of the file
			int j = iCount++;

			while( j > 0 && pControls[j - 1]->GetInt( "zpos" ) > pKeys->GetInt( "zpos" ) )
			{
				pControls[j] = pControls[j - 1];
				j--;
			}

			pControls[j] = pKeys;
		}

		for( int i = 0; i < iCount; i++ )
			AddControl( pControls[i] );
	}

	delete pRoot;

	Layout();

	return true;
}

ResControl_t *CDoDResPage::FindControl( const char *pszName )
{
	for( int i = m_iNumControls - 1; i >= 0; i-- )
	{
		if( !stricmp( m_Controls[i].szName, pszName ) )
			return &m_Controls[i];
	}

	return NULL;
}

// The text of a control: the localized one, or the one that the menu has set
static void GetControlText( ResControl_t *pControl, char *pszText, int iSize )
{
	if( pControl->bLiteral )
	{
		strncpy( pszText, pControl->szText, iSize - 1 );
		pszText[iSize - 1] = '\0';
	}
	else
	{
		DoD_GetLocalized( pControl->szText, pszText, iSize );
	}
}

void CDoDResPage::ApplyControl( ResControl_t *pControl )
{
	char szText[512];

	if( !pControl->pPanel )
		return;

	switch( pControl->iType )
	{
	case RESCONTROL_LABEL:
		{
			vgui::Label *pLabel = static_cast<vgui::Label *>( pControl->pPanel );

			DoD_PlaceLabel( pLabel, DoD_GetMenuFont( pControl->iFont ), pControl->x, pControl->y, pControl->wide, pControl->tall );

			// the alpha of VGUI1 is the transparency: BrightControlText is white, LabelDimText has the alpha 164
			if( pControl->bBright )
				pLabel->setFgColor( DODCOLOR_BRIGHT, 0 );
			else if( pControl->bDull )
				pLabel->setFgColor( DODCOLOR_TEXT, 91 );
			else
				pLabel->setFgColor( DODCOLOR_TEXT, 0 );

			// the text is set after the font
			GetControlText( pControl, szText, sizeof( szText ) );
			pLabel->setText( "%s", szText );

			// VGUI2 does not clip the text of a label
			DoD_FitLabel( pLabel, (vgui::Label::Alignment)pControl->iAlignment );
		}
		break;

	case RESCONTROL_TEXT:
		{
			CDoDRichText *pText = static_cast<CDoDRichText *>( pControl->pPanel );

			pText->SetRichBounds( DODMENU_SIZE( pControl->x ), DODMENU_SIZE( pControl->y ), DODMENU_SIZE( pControl->wide ), DODMENU_SIZE( pControl->tall ), DODMENU_SIZE( 21 ) );
			pText->SetRichFont( DoD_GetMenuFont( pControl->iFont ) );

			if( pControl->bBright )
				pText->SetRichColor( DODCOLOR_BRIGHT );
			else
				pText->SetRichColor( DODCOLOR_TEXT );

			GetControlText( pControl, szText, sizeof( szText ) );
			pText->SetRichText( szText );
		}
		break;

	case RESCONTROL_HTML:
		static_cast<CDoDHtmlView *>( pControl->pPanel )->SetHtmlBounds( DODMENU_SIZE( pControl->x ), DODMENU_SIZE( pControl->y ), DODMENU_SIZE( pControl->wide ), DODMENU_SIZE( pControl->tall ), DODMENU_SIZE( 21 ) );
		break;

	case RESCONTROL_BUTTON:
		static_cast<CDoDButton *>( pControl->pPanel )->SetButtonFont( DoD_GetMenuFont( DODFONT_DEFAULT ) );
		pControl->pPanel->setBounds( DODMENU_SIZE( pControl->x ), DODMENU_SIZE( pControl->y ), DODMENU_SIZE( pControl->wide ), DODMENU_SIZE( pControl->tall ) );
		break;

	default:
		pControl->pPanel->setBounds( DODMENU_SIZE( pControl->x ), DODMENU_SIZE( pControl->y ), DODMENU_SIZE( pControl->wide ), DODMENU_SIZE( pControl->tall ) );
		break;
	}
}

void CDoDResPage::Layout( void )
{
	for( int i = 0; i < m_iNumControls; i++ )
		ApplyControl( &m_Controls[i] );
}

void CDoDResPage::SetControlText( ResControl_t *pControl, const char *pszText )
{
	if( !pControl || ( pControl->iType != RESCONTROL_LABEL && pControl->iType != RESCONTROL_TEXT ) )
		return;

	strncpy( pControl->szText, pszText, sizeof( pControl->szText ) - 1 );
	pControl->szText[sizeof( pControl->szText ) - 1] = '\0';

	pControl->bLiteral = true;

	ApplyControl( pControl );
}
