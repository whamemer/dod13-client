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
// dodscrollpanel.cpp
//

#include "hud.h"
#include "cl_util.h"

#include <VGUI_ScrollBar.h>
#include <VGUI_TextImage.h>

#include "vgui_defaultinputsignal.h"

#include "vgui/dodscrollpanel.h"

CDoDScrollButton::CDoDScrollButton( bool bUp ) : Button( "", 0, 0, 10, 10 )
{
	m_bUp = bUp;
}

void CDoDScrollButton::paint( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	// the arrow of the original is small and light gray: 7 pixels wide and 4 high at 2.5 pixels of a unit, its top is 10 units
	// under the top of the button and its middle is 3.6 units from the right edge. The alpha of VGUI1 is the transparency, 0 is
	// opaque; a pressed button moves its arrow by 2 pixels (ScrollBarButtonDepressedBorder has the inset 2 2 0 0)
	int iPressed = isSelected() ? 2 : 0;
	int iHalf = (int)( (float)ScreenHeight / 480.0f * 1.2f + 0.5f );
	int iCenterX = wide - YRES( 3.6f ) + iPressed;
	int iTop = YRES( 10.0f ) + iPressed;

	drawSetColor( 176, 174, 174, 0 );

	for( int i = 0; i <= iHalf; i++ )
	{
		int iRowHalf = m_bUp ? i : ( iHalf - i );

		drawFilledRect( iCenterX - iRowHalf, iTop + i, iCenterX + iRowHalf + 1, iTop + i + 1 );
	}
}

void CDoDScrollButton::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	// black with the border of ClassMenuDark, the depressed border has the same colors
	drawSetColor( 0, 0, 0, 0 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( 64, 52, 36, 0 );
	drawOutlinedRect( 0, 0, wide, tall );
}

CDoDSlider::CDoDSlider( bool bVertical ) : Slider( 0, 0, 10, 10, bVertical )
{
	/* Nothing. */
}

void CDoDSlider::paintBackground( void )
{
	// the handle and the track are not visible in the original (ScrollBarSliderFgColor is ControlBG, the
	// background is ControlDarkBG over the black window), the text is scrolled with the arrow buttons
}

// The wheel of the mouse: the text goes up with the wheel up
class CWheelSignal : public vgui::CDefaultInputSignal
{
public:
	CWheelSignal( CDoDScrollPanel *pScroll ) : m_pScroll( pScroll ) {}

	virtual void mouseWheeled( int delta, vgui::Panel *panel )
	{
		m_pScroll->ScrollByWheel( delta );
	}

private:
	CDoDScrollPanel *m_pScroll;
};

CDoDScrollPanel::CDoDScrollPanel( int x, int y, int wide, int tall ) : ScrollPanel( x, y, wide, tall )
{
	m_pWheelSignal = new CWheelSignal( this );

	AddWheelTarget( this );
	AddWheelTarget( getClient() );
	AddWheelTarget( getClientClip() );

	vgui::ScrollBar *pScrollBar = getVerticalScrollBar();

	pScrollBar->setButton( new CDoDScrollButton( true ), 0 );
	pScrollBar->setButton( new CDoDScrollButton( false ), 1 );
	pScrollBar->setSlider( new CDoDSlider( true ) );
	pScrollBar->setPaintBorderEnabled( false );
	pScrollBar->setPaintBackgroundEnabled( false );
	pScrollBar->setPaintEnabled( false );

	// only the vertical scroll bar is used and it is always shown
	setScrollBarAutoVisible( false, false );
	setScrollBarVisible( false, true );
}

void CDoDScrollPanel::SetBarWidth( int wide )
{
	vgui::ScrollBar *pScrollBar = getVerticalScrollBar();

	pScrollBar->setSize( wide, pScrollBar->getTall() );

	validate();
}

int CDoDScrollPanel::GetBarWidth( void )
{
	return getVerticalScrollBar()->getWide();
}

void CDoDScrollPanel::AddWheelTarget( vgui::Panel *pPanel )
{
	pPanel->addInputSignal( m_pWheelSignal );
}

void CDoDScrollPanel::ScrollByWheel( int iDelta )
{
	vgui::ScrollBar *pScrollBar = getVerticalScrollBar();

	// there is nothing to scroll without the bar (a page that fits is not scrolled)
	if( !pScrollBar->isVisible() )
		return;

	pScrollBar->setValue( pScrollBar->getValue() - iDelta * YRES( 14 ) );
}

CDoDLinesPanel::CDoDLinesPanel( void ) : Panel( 0, 0, 10, 10 )
{
	m_pFont = NULL;
	m_iPitch = 1;
	m_r = 255;
	m_g = 176;
	m_b = 0;
}

void CDoDLinesPanel::SetLinesFont( vgui::Font *font )
{
	m_pFont = font;
}

void CDoDLinesPanel::SetLinesColor( int r, int g, int b )
{
	m_r = r;
	m_g = g;
	m_b = b;
}

// A line of the text is broken after the last space that lets it fit, a word that is longer than the width is broken
// where it does not fit
static void AddLine( CUtlVector<CUtlString> &lines, const char *pszText, int iLength )
{
	CUtlString line;

	line.SetDirect( pszText, iLength );
	lines.AddToTail( line );
}

static void WrapLine( vgui::Font *pFont, const char *line, int size, int iWide, CUtlVector<CUtlString> &lines )
{
	int pos = 0;

	if( size == 0 )
	{
		AddLine( lines, line, 0 );
		return;
	}

	while( pos < size )
	{
		int end = pos;
		int lastSpace = -1;
		int w, t;

		while( end < size )
		{
			CUtlString piece;

			piece.SetDirect( line + pos, end - pos + 1 );

			pFont->getTextSize( piece.Get(), w, t );

			if( w > iWide && end > pos )
				break;

			if( line[end] == ' ' && end > pos )
				lastSpace = end;

			end++;
		}

		if( end >= size )
		{
			AddLine( lines, line + pos, size - pos );
			break;
		}

		if( lastSpace != -1 )
		{
			AddLine( lines, line + pos, lastSpace - pos );
			pos = lastSpace + 1;
		}
		else
		{
			AddLine( lines, line + pos, end - pos );
			pos = end;
		}
	}
}

// The lines of a text: it is broken at the new lines and every line is wrapped to the width, the new line at the end of the
// text does not start a line
void DoD_WrapText( vgui::Font *pFont, const char *pszText, int iWide, CUtlVector<CUtlString> &lines )
{
	const char *pszParagraph = pszText;

	lines.RemoveAll();

	for( const char *p = pszText; ; p++ )
	{
		if( *p == '\n' || *p == '\0' )
		{
			if( *p == '\0' && p == pszParagraph )
				break;

			WrapLine( pFont, pszParagraph, (int)( p - pszParagraph ), iWide, lines );
			pszParagraph = p + 1;

			if( *p == '\0' )
				break;
		}
	}
}

int CDoDLinesPanel::SetLines( const char *pszText, int iWide, int iPitch )
{
	m_iPitch = iPitch > 0 ? iPitch : 1;

	if( !m_pFont )
	{
		m_Lines.RemoveAll();
		return 0;
	}

	DoD_WrapText( m_pFont, pszText, iWide, m_Lines );

	setSize( iWide, m_Lines.Count() * m_iPitch + 1 );
	repaint();

	return m_Lines.Count() * m_iPitch;
}

void CDoDLinesPanel::paintBackground( void )
{
	/* Nothing. */
}

void CDoDLinesPanel::paint( void )
{
	int textWide, textTall;

	if( !m_pFont )
		return;

	drawSetTextFont( m_pFont );
	drawSetTextColor( m_r, m_g, m_b, 0 );

	// the first line starts at the top: the original puts the cap of it 2.4 units under the border of the box, that is the
	// top of a line and the empty space over the letters of the font
	m_pFont->getTextSize( "A", textWide, textTall );

	for( int i = 0; i < m_Lines.Count(); i++ )
	{
		if( m_Lines[i].IsEmpty() )
			continue;

		drawSetTextPos( 0, (int)i * m_iPitch + ( m_iPitch - textTall ) / 4 );
		drawPrintText( m_Lines[i].Get(), (int)m_Lines[i].Length() );
	}
}

CDoDRichText::CDoDRichText( int x, int y, int wide, int tall ) : CDoDScrollPanel( x, y, wide, tall )
{
	m_iTextWide = wide;
	m_iInset = 0;
	m_iLinePitch = 0;
	m_pFont = NULL;

	m_pText = new vgui::TextPanel( "", 0, 0, wide, tall );
	m_pText->setParent( getClient() );

	AddWheelTarget( m_pText );
	m_pText->setFgColor( 255, 255, 255, 0 );
	m_pText->setBgColor( 0, 0, 0, 255 );

	m_pLines = new CDoDLinesPanel;
	m_pLines->setParent( getClient() );
	m_pLines->setVisible( false );

	AddWheelTarget( m_pLines );
}

void CDoDRichText::SetRichBounds( int x, int y, int wide, int tall, int barWide )
{
	setBounds( x, y, wide, tall );
	SetBarWidth( barWide );

	m_iTextWide = wide - GetBarWidth() - m_iInset;
	m_pText->setPos( m_iInset, 0 );
	m_pText->setSize( m_iTextWide, tall );
	m_pLines->setPos( m_iInset, 0 );
}

void CDoDRichText::SetRichInset( int iInset )
{
	m_iInset = iInset;
}

void CDoDRichText::SetRichLineTall( int iPitch )
{
	m_iLinePitch = iPitch;
}

void CDoDRichText::SetRichFont( vgui::Font *font )
{
	m_pFont = font;
	m_pText->setFont( font );
	m_pLines->SetLinesFont( font );
}

void CDoDRichText::SetRichColor( int r, int g, int b )
{
	// the alpha of VGUI1 is the transparency, 0 is opaque
	m_pText->setFgColor( r, g, b, 0 );
	m_pLines->SetLinesColor( r, g, b );
}

void CDoDRichText::SetRichText( const char *text )
{
	int iWide, iTall;

	if( m_iLinePitch > 0 )
	{
		// the lines with their pitch
		m_pText->setVisible( false );
		m_pLines->setVisible( true );
		m_pLines->SetLines( text, m_iTextWide, m_iLinePitch );
	}
	else
	{
		m_pLines->setVisible( false );
		m_pText->setVisible( true );

		// the size of the text image is made when the text is set, the text is wrapped to the width of the panel
		// and the panel is as tall as the wrapped text
		m_pText->setText( text );
		m_pText->setSize( m_iTextWide, 1 );
		m_pText->getTextImage()->getTextSizeWrapped( iWide, iTall );
		m_pText->setSize( m_iTextWide, iTall > 0 ? iTall : 1 );
	}

	validate();
	setScrollValue( 0, 0 );
}
