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
// dodhtml.cpp
//
// See dodhtml.h.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <VGUI_Font.h>

#include "hud.h"
#include "cl_util.h"

#include "utlvector.h"

#include "vgui/dodmenu.h"
#include "vgui/dodhtml.h"

namespace
{

// The size of the text relative to the layout: the HTML control of the original draws the page in the pixels of the
// screen, a page that is made for 96 dpi is small at the high resolutions. 1.0 is the size that the page asks for.
const float HTML_ZOOM = 1.0f;

struct HtmlRun
{
	int x;
	int y;
	int iWide;
	CUtlString text;
	vgui::Font *pFont;
	int r;
	int g;
	int b;
};

struct HtmlRect
{
	int x0;
	int y0;
	int x1;
	int y1;
	int r;
	int g;
	int b;
};

struct HtmlResult
{
	CUtlVector<HtmlRun> runs;
	CUtlVector<HtmlRect> rects;

	int iHeight;
	int iBackground[3];
};

// What an element looks like: the properties of CSS that are supported
struct Style
{
	int r;
	int g;
	int b;
	float flFontSize;	// in pixels
	int iWeight;
	bool bItalic;
	bool bUnderline;
	bool bPre;
	int iAlign;	// 0 left, 1 center, 2 right
	float flLineHeight;	// the multiplier of the font size, 0 is normal
	CUtlString family;
	bool bHidden;

	float flMargin[4];	// top, right, bottom, left

	bool bBackground;
	int iBackground[3];
};

struct CssRule
{
	CUtlString tag;
	CUtlString cls;
	CUtlString id;
	CUtlString props;
	int iSpecificity;
	int iOrder;
};

struct Attr
{
	CUtlString name;
	CUtlString value;
};

float s_flZoom = 1.0f;

// CUtlString of MiniUTL has no search and no cut as std::string has: the positions are ints, -1 is nothing found
int Len( const char *psz )
{
	return (int)strlen( psz );
}

// The count that is out of the string or is negative takes the rest of it
CUtlString Substr( const char *psz, int pos, int count = -1 )
{
	int size = Len( psz );
	CUtlString result;

	if( pos < 0 )
		pos = 0;

	if( pos > size )
		pos = size;

	if( count < 0 || count > size - pos )
		count = size - pos;

	result.SetDirect( psz + pos, count );

	return result;
}

int Find( const char *psz, char ch, int pos = 0 )
{
	if( pos > Len( psz ) )
		return -1;

	const char *pszFound = strchr( psz + pos, ch );

	return pszFound ? (int)( pszFound - psz ) : -1;
}

int Find( const char *psz, const char *pszText, int pos = 0 )
{
	if( pos > Len( psz ) )
		return -1;

	const char *pszFound = strstr( psz + pos, pszText );

	return pszFound ? (int)( pszFound - psz ) : -1;
}

int FindLastOf( const char *psz, const char *pszChars )
{
	for( int i = Len( psz ) - 1; i >= 0; i-- )
	{
		if( strchr( pszChars, psz[i] ) )
			return i;
	}

	return -1;
}

CUtlString Trim( const char *psz )
{
	int start = 0;
	int end = Len( psz );

	while( start < end && isspace( (unsigned char)psz[start] ) )
		start++;

	while( end > start && isspace( (unsigned char)psz[end - 1] ) )
		end--;

	return Substr( psz, start, end - start );
}

CUtlString ToLower( const char *psz )
{
	CUtlString result( psz );

	result.ToLower();

	return result;
}

bool StartsWith( const char *psz, const char *pszPrefix )
{
	return strncmp( psz, pszPrefix, strlen( pszPrefix ) ) == 0;
}

struct NamedColor
{
	const char *pszName;
	int r;
	int g;
	int b;
};

const NamedColor s_NamedColors[] =
{
	{ "black", 0, 0, 0 },
	{ "white", 255, 255, 255 },
	{ "red", 255, 0, 0 },
	{ "green", 0, 128, 0 },
	{ "blue", 0, 0, 255 },
	{ "yellow", 255, 255, 0 },
	{ "orange", 255, 165, 0 },
	{ "gray", 128, 128, 128 },
	{ "grey", 128, 128, 128 },
	{ "silver", 192, 192, 192 },
	{ "maroon", 128, 0, 0 },
	{ "olive", 128, 128, 0 },
	{ "lime", 0, 255, 0 },
	{ "aqua", 0, 255, 255 },
	{ "cyan", 0, 255, 255 },
	{ "teal", 0, 128, 128 },
	{ "navy", 0, 0, 128 },
	{ "fuchsia", 255, 0, 255 },
	{ "magenta", 255, 0, 255 },
	{ "purple", 128, 0, 128 },
	{ "brown", 165, 42, 42 },
	{ "gold", 255, 215, 0 }
};

int HexDigit( char ch )
{
	if( ch >= '0' && ch <= '9' )
		return ch - '0';

	if( ch >= 'a' && ch <= 'f' )
		return ch - 'a' + 10;

	return -1;
}

// #rgb, #rrggbb, rgb( r, g, b ) and the names of the colors
bool ParseColor( const CUtlString &text, int &r, int &g, int &b )
{
	CUtlString value = ToLower( Trim( text ) );

	if( value.IsEmpty() )
		return false;

	if( value[0] == '#' )
	{
		int d[6];
		int digits = Len( value ) - 1;

		if( digits != 3 && digits != 6 )
			return false;

		for( int i = 0; i < digits; i++ )
		{
			d[i] = HexDigit( value[i + 1] );

			if( d[i] < 0 )
				return false;
		}

		if( digits == 3 )
		{
			r = d[0] * 17;
			g = d[1] * 17;
			b = d[2] * 17;
		}
		else
		{
			r = d[0] * 16 + d[1];
			g = d[2] * 16 + d[3];
			b = d[4] * 16 + d[5];
		}

		return true;
	}

	if( StartsWith( value, "rgb(" ) )
	{
		int ir, ig, ib;

		if( sscanf( value.Get() + 4, " %d , %d , %d", &ir, &ig, &ib ) == 3 )
		{
			r = ir;
			g = ig;
			b = ib;
			return true;
		}

		return false;
	}

	for( size_t i = 0; i < sizeof( s_NamedColors ) / sizeof( s_NamedColors[0] ); i++ )
	{
		if( value == s_NamedColors[i].pszName )
		{
			r = s_NamedColors[i].r;
			g = s_NamedColors[i].g;
			b = s_NamedColors[i].b;
			return true;
		}
	}

	return false;
}

// A length in pixels; em is relative to the font size, % to the base
bool ParseLength( const CUtlString &text, float flFontSize, float flBase, float &flResult )
{
	CUtlString value = ToLower( Trim( text ) );
	char *pszEnd = NULL;
	float flNumber = (float)strtod( value.Get(), &pszEnd );

	if( pszEnd == value.Get() )
		return false;

	CUtlString unit = Trim( pszEnd );

	if( unit.IsEmpty() || unit == "px" )
		flResult = flNumber * s_flZoom;
	else if( unit == "pt" )
		flResult = flNumber * 4.0f / 3.0f * s_flZoom;
	else if( unit == "pc" )
		flResult = flNumber * 16.0f * s_flZoom;
	else if( unit == "em" )
		flResult = flNumber * flFontSize;
	else if( unit == "rem" )
		flResult = flNumber * 16.0f * s_flZoom;
	else if( unit == "ex" )
		flResult = flNumber * flFontSize * 0.5f;
	else if( unit == "%" )
		flResult = flNumber * flBase / 100.0f;
	else if( unit == "in" )
		flResult = flNumber * 96.0f * s_flZoom;
	else if( unit == "cm" )
		flResult = flNumber * 37.8f * s_flZoom;
	else if( unit == "mm" )
		flResult = flNumber * 3.78f * s_flZoom;
	else
		return false;

	return true;
}

bool ParseFontSize( const CUtlString &text, float flParentSize, float &flResult )
{
	CUtlString value = ToLower( Trim( text ) );

	if( value == "xx-small" )
		flResult = 9.0f * s_flZoom;
	else if( value == "x-small" )
		flResult = 10.0f * s_flZoom;
	else if( value == "small" )
		flResult = 13.0f * s_flZoom;
	else if( value == "medium" )
		flResult = 16.0f * s_flZoom;
	else if( value == "large" )
		flResult = 18.0f * s_flZoom;
	else if( value == "x-large" )
		flResult = 24.0f * s_flZoom;
	else if( value == "xx-large" )
		flResult = 32.0f * s_flZoom;
	else if( value == "smaller" )
		flResult = flParentSize / 1.2f;
	else if( value == "larger" )
		flResult = flParentSize * 1.2f;
	else
		return ParseLength( value, flParentSize, flParentSize, flResult );

	return true;
}

// The first family of a list, the generic ones are the fonts of the menus
CUtlString ParseFamily( const CUtlString &text )
{
	CUtlString family = text;
	int comma = Find( family, ',' );

	if( comma != -1 )
		family = Substr( family, 0, comma );

	family = Trim( family );

	if( Len( family ) >= 2 && ( family[0] == '"' || family[0] == '\'' ) )
		family = Substr( family, 1, Len( family ) - 2 );

	CUtlString lower = ToLower( family );

	if( lower == "sans-serif" )
		return "Arial";

	if( lower == "serif" )
		return "Times New Roman";

	if( lower == "monospace" )
		return "Courier New";

	return family;
}

// The margins of the shorthand: 1 value is all of them, 2 are top and bottom / right and left, 3 are top / right and
// left / bottom, 4 are top, right, bottom and left
void ParseMargin( const CUtlString &text, Style &style )
{
	CUtlVector<CUtlString> values;
	CUtlString value = Trim( text );
	int pos = 0;

	while( pos < Len( value ) )
	{
		int end = pos;

		while( end < Len( value ) && !isspace( (unsigned char)value[end] ) )
			end++;

		values.AddToTail( Substr( value, pos, end - pos ) );

		pos = end;

		while( pos < Len( value ) && isspace( (unsigned char)value[pos] ) )
			pos++;
	}

	if( values.IsEmpty() || values.Count() > 4 )
		return;

	float flLength[4];

	for( int i = 0; i < values.Count(); i++ )
	{
		if( values[i] == "auto" )
			flLength[i] = 0.0f;
		else if( !ParseLength( values[i], style.flFontSize, 0.0f, flLength[i] ) )
			return;
	}

	switch( values.Count() )
	{
	case 1:
		style.flMargin[0] = style.flMargin[1] = style.flMargin[2] = style.flMargin[3] = flLength[0];
		break;
	case 2:
		style.flMargin[0] = style.flMargin[2] = flLength[0];
		style.flMargin[1] = style.flMargin[3] = flLength[1];
		break;
	case 3:
		style.flMargin[0] = flLength[0];
		style.flMargin[1] = style.flMargin[3] = flLength[1];
		style.flMargin[2] = flLength[2];
		break;
	default:
		for( int i = 0; i < 4; i++ )
			style.flMargin[i] = flLength[i];
		break;
	}
}

// font: [style] [weight] size[/line-height] family
void ParseFontShorthand( const CUtlString &text, Style &style, float flParentSize )
{
	CUtlString value = Trim( text );
	int pos = 0;
	bool bSize = false;

	while( pos < Len( value ) && !bSize )
	{
		int end = pos;

		while( end < Len( value ) && !isspace( (unsigned char)value[end] ) )
			end++;

		CUtlString token = ToLower( Substr( value, pos, end - pos ) );
		float flSize;

		if( token == "italic" || token == "oblique" )
		{
			style.bItalic = true;
		}
		else if( token == "bold" || token == "bolder" )
		{
			style.iWeight = 700;
		}
		else if( token == "normal" || token == "lighter" )
		{
			// the style, the weight or the variant
		}
		else if( Len( token ) == 3 && isdigit( (unsigned char)token[0] ) && token[1] == '0' && token[2] == '0' )
		{
			style.iWeight = ( token[0] - '0' ) * 100;
		}
		else
		{
			// the size, maybe with the line height
			int slash = Find( token, '/' );

			if( ParseFontSize( slash == -1 ? token : Substr( token, 0, slash ), flParentSize, flSize ) )
			{
				style.flFontSize = flSize;
				bSize = true;

				if( slash != -1 )
				{
					float flLine;

					if( ParseLength( Substr( token, slash + 1 ), style.flFontSize, style.flFontSize, flLine ) )
						style.flLineHeight = flLine / style.flFontSize;
				}
			}
		}

		pos = end;

		while( pos < Len( value ) && isspace( (unsigned char)value[pos] ) )
			pos++;
	}

	if( bSize && pos < Len( value ) )
		style.family = ParseFamily( Substr( value, pos ) );
}

// The declarations of a rule or of an attribute style: name: value; the font size is made first, the lengths in em
// are relative to it
void ApplyDeclarations( Style &style, const CUtlString &props, float flParentSize )
{
	CUtlVector<Attr> declarations;
	int pos = 0;

	while( pos < Len( props ) )
	{
		int end = Find( props, ';', pos );

		if( end == -1 )
			end = Len( props );

		CUtlString declaration = Substr( props, pos, end - pos );
		int colon = Find( declaration, ':' );

		if( colon != -1 )
		{
			Attr attr;

			attr.name = ToLower( Trim( Substr( declaration, 0, colon ) ) );
			attr.value = Trim( Substr( declaration, colon + 1 ) );

			// !important is not told from the other declarations
			int important = Find( attr.value, '!' );

			if( important != -1 )
				attr.value = Trim( Substr( attr.value, 0, important ) );

			declarations.AddToTail( attr );
		}

		pos = end + 1;
	}

	for( int i = 0; i < declarations.Count(); i++ )
	{
		const CUtlString &name = declarations[i].name;
		const CUtlString &value = declarations[i].value;
		float flSize;

		if( name == "font" )
			ParseFontShorthand( value, style, flParentSize );
		else if( name == "font-size" && ParseFontSize( value, flParentSize, flSize ) )
			style.flFontSize = flSize;
	}

	for( int i = 0; i < declarations.Count(); i++ )
	{
		const CUtlString &name = declarations[i].name;
		CUtlString value = ToLower( declarations[i].value );
		int r, g, b;
		float flLength;

		if( name == "color" )
		{
			if( ParseColor( value, r, g, b ) )
			{
				style.r = r;
				style.g = g;
				style.b = b;
			}
		}
		else if( name == "background" || name == "background-color" )
		{
			// the color is a part of the shorthand
			int pos = 0;

			while( pos < Len( value ) )
			{
				int end = pos;

				while( end < Len( value ) && !isspace( (unsigned char)value[end] ) )
					end++;

				if( ParseColor( Substr( value, pos, end - pos ), r, g, b ) )
				{
					style.bBackground = true;
					style.iBackground[0] = r;
					style.iBackground[1] = g;
					style.iBackground[2] = b;
				}

				pos = end;

				while( pos < Len( value ) && isspace( (unsigned char)value[pos] ) )
					pos++;
			}
		}
		else if( name == "font-weight" )
		{
			if( value == "bold" || value == "bolder" )
				style.iWeight = 700;
			else if( value == "normal" || value == "lighter" )
				style.iWeight = 400;
			else
				style.iWeight = atoi( value.Get() );
		}
		else if( name == "font-style" )
		{
			style.bItalic = ( value == "italic" || value == "oblique" );
		}
		else if( name == "font-family" )
		{
			style.family = ParseFamily( declarations[i].value );
		}
		else if( name == "text-decoration" )
		{
			style.bUnderline = ( Find( value, "underline" ) != -1 );
		}
		else if( name == "text-align" )
		{
			if( value == "center" )
				style.iAlign = 1;
			else if( value == "right" )
				style.iAlign = 2;
			else if( value == "left" )
				style.iAlign = 0;
		}
		else if( name == "margin" )
		{
			ParseMargin( value, style );
		}
		else if( name == "margin-top" && ParseLength( value, style.flFontSize, 0.0f, flLength ) )
		{
			style.flMargin[0] = flLength;
		}
		else if( name == "margin-right" && ParseLength( value, style.flFontSize, 0.0f, flLength ) )
		{
			style.flMargin[1] = flLength;
		}
		else if( name == "margin-bottom" && ParseLength( value, style.flFontSize, 0.0f, flLength ) )
		{
			style.flMargin[2] = flLength;
		}
		else if( name == "margin-left" && ParseLength( value, style.flFontSize, 0.0f, flLength ) )
		{
			style.flMargin[3] = flLength;
		}
		else if( name == "line-height" )
		{
			if( value == "normal" )
			{
				style.flLineHeight = 0.0f;
			}
			else
			{
				char *pszEnd = NULL;
				float flNumber = (float)strtod( value.Get(), &pszEnd );

				// a number is the multiplier, a length is divided by the font size
				if( pszEnd != value.Get() && *pszEnd == '\0' )
					style.flLineHeight = flNumber;
				else if( ParseLength( value, style.flFontSize, style.flFontSize, flLength ) && style.flFontSize > 0.0f )
					style.flLineHeight = flLength / style.flFontSize;
			}
		}
		else if( name == "display" )
		{
			style.bHidden = ( value == "none" );
		}
	}
}

bool IsBlockTag( const CUtlString &tag )
{
	static const char *s_pszBlocks[] =
	{
		"p", "div", "h1", "h2", "h3", "h4", "h5", "h6", "ul", "ol", "li", "blockquote", "pre", "hr", "table", "tr",
		"center", "address", "dl", "dt", "dd", "form", "fieldset", "body", "html"
	};

	for( size_t i = 0; i < sizeof( s_pszBlocks ) / sizeof( s_pszBlocks[0] ); i++ )
	{
		if( tag == s_pszBlocks[i] )
			return true;
	}

	return false;
}

bool IsVoidTag( const CUtlString &tag )
{
	static const char *s_pszVoids[] =
	{
		"br", "hr", "img", "meta", "link", "input", "area", "base", "col", "param", "source", "wbr"
	};

	for( size_t i = 0; i < sizeof( s_pszVoids ) / sizeof( s_pszVoids[0] ); i++ )
	{
		if( tag == s_pszVoids[i] )
			return true;
	}

	return false;
}

struct CachedFont
{
	CUtlString family;
	int iTall;
	int iWeight;
	bool bItalic;
	vgui::Font *pFont;
};

// The fonts of the page: one for a family, a size, a weight and the italic
vgui::Font *GetFont( const CUtlString &family, int iTall, int iWeight, bool bItalic )
{
	static CUtlVector<CachedFont> s_Fonts;

	for( int i = 0; i < s_Fonts.Count(); i++ )
	{
		if( s_Fonts[i].iTall == iTall && s_Fonts[i].iWeight == iWeight && s_Fonts[i].bItalic == bItalic && s_Fonts[i].family == family )
			return s_Fonts[i].pFont;
	}

	CachedFont font;

	font.family = family;
	font.iTall = iTall;
	font.iWeight = iWeight;
	font.bItalic = bItalic;

	// font-size of CSS is the size of the em; a positive height of GDI is the height of the cell (the em with the
	// internal leading, 1.2 em for Verdana), the negative one is the em: the text of the page has the size that it asks for
#if defined( _WIN32 ) || defined( DOD_FONTPLAT )
	font.pFont = new vgui::Font( family.Get(), NULL, 0, -iTall, 0, 0, iWeight, bItalic, false, false, false );
#else
	// the debugging font of freevgui takes the height as it is
	font.pFont = new vgui::Font( family.Get(), NULL, 0, iTall, 0, 0, iWeight, bItalic, false, false, false );
#endif

	s_Fonts.AddToTail( font );

	return font.pFont;
}

struct Word
{
	CUtlString text;
	vgui::Font *pFont;
	int r;
	int g;
	int b;
	bool bUnderline;
	float flWide;
	float flTall;
	float flLineHeight;
	float flGap;	// before the word
};

struct Element
{
	CUtlString tag;
	Style style;
	bool bBlock;
	bool bHiddenCounted;
	bool bList;
	int iOrdered;	// the number of the next item of an ol, 0 for a ul
};

// The layout of a page: the tags are read in order, the text goes to the lines
class CHtmlLayout
{
public:
	void Run( const CUtlString &html, int iWidth, HtmlResult &result );

private:
	void ParseCss( const CUtlString &css );
	void ParseTag( const CUtlString &tag, CUtlString &name, CUtlVector<Attr> &attrs, bool &bClosing );
	void OpenElement( const CUtlString &name, const CUtlVector<Attr> &attrs );
	void CloseElement( const CUtlString &name );
	void CloseTop( void );
	void ImplicitClose( const CUtlString &name );

	void ApplyElementStyle( Element &element, const CUtlVector<Attr> &attrs );

	void AddText( const CUtlString &raw );
	void AddWord( const CUtlString &text, bool bNoWrap );
	void FlushLine( void );
	void LineBreak( void );
	void OpenBlock( const Style &style );
	void CloseBlock( const Style &style );
	void StartLine( void );

	float AvailableWidth( void );
	float SpaceWidth( vgui::Font *pFont );

	Style &TopStyle( void ) { return m_Elements.Tail().style; }

	CUtlVector<Element> m_Elements;
	CUtlVector<CssRule> m_Rules;
	HtmlResult *m_pResult;

	int m_iWidth;
	float m_y;
	float m_flLeft;
	float m_flRight;
	float m_flPendingMargin;
	float m_flPendingGap;

	CUtlVector<Word> m_Words;
	float m_flLineX;
	bool m_bPendingSpace;

	int m_iHidden;
	int m_iCells;
	bool m_bBodySeen;
};

CUtlString DecodeEntities( const CUtlString &text )
{
	CUtlVector<char> result;
	int size = Len( text );

	for( int i = 0; i < size; i++ )
	{
		if( text[i] != '&' )
		{
			result.AddToTail( text[i] );
			continue;
		}

		int end = Find( text, ';', i );

		if( end == -1 || end - i > 10 )
		{
			result.AddToTail( '&' );
			continue;
		}

		CUtlString entity = Substr( text, i + 1, end - i - 1 );
		int iCodePoint = -1;

		if( Len( entity ) > 1 && entity[0] == '#' )
		{
			if( entity[1] == 'x' || entity[1] == 'X' )
				iCodePoint = (int)strtol( entity.Get() + 2, NULL, 16 );
			else
				iCodePoint = atoi( entity.Get() + 1 );
		}
		else if( entity == "amp" )
			iCodePoint = '&';
		else if( entity == "lt" )
			iCodePoint = '<';
		else if( entity == "gt" )
			iCodePoint = '>';
		else if( entity == "quot" )
			iCodePoint = '"';
		else if( entity == "apos" )
			iCodePoint = '\'';
		else if( entity == "nbsp" )
			iCodePoint = 0xA0;
		else if( entity == "copy" )
			iCodePoint = 0xA9;
		else if( entity == "reg" )
			iCodePoint = 0xAE;
		else if( entity == "deg" )
			iCodePoint = 0xB0;
		else if( entity == "middot" )
			iCodePoint = 0xB7;
		else if( entity == "bull" )
			iCodePoint = 0x2022;
		else if( entity == "ndash" )
			iCodePoint = 0x2013;
		else if( entity == "mdash" )
			iCodePoint = 0x2014;
		else if( entity == "hellip" )
			iCodePoint = 0x2026;
		else if( entity == "lsquo" || entity == "rsquo" )
			iCodePoint = 0x2019;
		else if( entity == "ldquo" || entity == "rdquo" )
			iCodePoint = 0x201C;

		if( iCodePoint < 0 )
		{
			result.AddToTail( '&' );
			continue;
		}

		// &#0; does not end the text
		if( iCodePoint > 0 )
			result.AddToTail( DoD_CodePointToLatin1( iCodePoint ) );

		i = end;
	}

	result.AddToTail( '\0' );

	return CUtlString( result.Base() );
}

const Attr *FindAttr( const CUtlVector<Attr> &attrs, const char *pszName )
{
	for( int i = 0; i < attrs.Count(); i++ )
	{
		if( attrs[i].name == pszName )
			return &attrs[i];
	}

	return NULL;
}

float CHtmlLayout::AvailableWidth( void )
{
	float flWidth = m_iWidth - m_flLeft - m_flRight;

	return flWidth > 1.0f ? flWidth : 1.0f;
}

float CHtmlLayout::SpaceWidth( vgui::Font *pFont )
{
	int a, b, c;

	pFont->getCharABCwide( ' ', a, b, c );

	return (float)( a + b + c );
}

// A rule: the selectors before the braces, the declarations inside; only the last part of a selector counts and the
// pseudo classes are not supported
void CHtmlLayout::ParseCss( const CUtlString &css )
{
	CUtlVector<char> stripped;
	int size = Len( css );

	// the comments
	for( int i = 0; i < size; i++ )
	{
		if( css[i] == '/' && i + 1 < size && css[i + 1] == '*' )
		{
			int end = Find( css, "*/", i + 2 );

			if( end == -1 )
				break;

			i = end + 1;
			continue;
		}

		stripped.AddToTail( css[i] );
	}

	stripped.AddToTail( '\0' );

	CUtlString text( stripped.Base() );
	int pos = 0;

	while( pos < Len( text ) )
	{
		int open = Find( text, '{', pos );

		if( open == -1 )
			break;

		CUtlString selectors = Trim( Substr( text, pos, open - pos ) );

		// the end of the block, the blocks inside (@media) are skipped
		int iDepth = 1;
		int close = open + 1;

		while( close < Len( text ) && iDepth > 0 )
		{
			if( text[close] == '{' )
				iDepth++;
			else if( text[close] == '}' )
				iDepth--;

			close++;
		}

		CUtlString props = Substr( text, open + 1, close - open - 2 );

		pos = close;

		if( selectors.IsEmpty() || selectors[0] == '@' )
			continue;

		int start = 0;

		while( start <= Len( selectors ) )
		{
			int comma = Find( selectors, ',', start );

			if( comma == -1 )
				comma = Len( selectors );

			CUtlString selector = Trim( Substr( selectors, start, comma - start ) );

			start = comma + 1;

			if( selector.IsEmpty() || Find( selector, ':' ) != -1 )
				continue;

			// the last part: the one after the last space or '>'
			int last = FindLastOf( selector, " >+~" );

			if( last != -1 )
				selector = Substr( selector, last + 1 );

			CssRule rule;

			rule.props = props;
			rule.iOrder = (int)m_Rules.Count();
			rule.iSpecificity = 0;

			// tag.class#id
			int idPos = Find( selector, '#' );
			int classPos = Find( selector, '.' );

			int tagEnd = Len( selector );

			if( idPos != -1 && idPos < tagEnd )
				tagEnd = idPos;

			if( classPos != -1 && classPos < tagEnd )
				tagEnd = classPos;

			rule.tag = ToLower( Substr( selector, 0, tagEnd ) );

			if( rule.tag == "*" )
				rule.tag.Clear();

			if( !rule.tag.IsEmpty() )
				rule.iSpecificity += 1;

			if( classPos != -1 )
			{
				int classEnd = Len( selector );

				if( idPos != -1 && idPos > classPos )
					classEnd = idPos;

				rule.cls = Substr( selector, classPos + 1, classEnd - classPos - 1 );
				rule.iSpecificity += 10;
			}

			if( idPos != -1 )
			{
				int idEnd = Len( selector );

				if( classPos != -1 && classPos > idPos )
					idEnd = classPos;

				rule.id = Substr( selector, idPos + 1, idEnd - idPos - 1 );
				rule.iSpecificity += 100;
			}

			m_Rules.AddToTail( rule );
		}
	}
}

// <tag attr=value ...>: the name, the attributes and whether it is a closing tag
void CHtmlLayout::ParseTag( const CUtlString &tag, CUtlString &name, CUtlVector<Attr> &attrs, bool &bClosing )
{
	int pos = 0;

	bClosing = false;

	if( pos < Len( tag ) && tag[pos] == '/' )
	{
		bClosing = true;
		pos++;
	}

	int end = pos;

	while( end < Len( tag ) && !isspace( (unsigned char)tag[end] ) && tag[end] != '/' )
		end++;

	name = ToLower( Substr( tag, pos, end - pos ) );
	pos = end;

	while( pos < Len( tag ) )
	{
		while( pos < Len( tag ) && ( isspace( (unsigned char)tag[pos] ) || tag[pos] == '/' ) )
			pos++;

		if( pos >= Len( tag ) )
			break;

		int nameEnd = pos;

		while( nameEnd < Len( tag ) && !isspace( (unsigned char)tag[nameEnd] ) && tag[nameEnd] != '=' )
			nameEnd++;

		Attr attr;

		attr.name = ToLower( Substr( tag, pos, nameEnd - pos ) );
		pos = nameEnd;

		while( pos < Len( tag ) && isspace( (unsigned char)tag[pos] ) )
			pos++;

		if( pos < Len( tag ) && tag[pos] == '=' )
		{
			pos++;

			while( pos < Len( tag ) && isspace( (unsigned char)tag[pos] ) )
				pos++;

			if( pos < Len( tag ) && ( tag[pos] == '"' || tag[pos] == '\'' ) )
			{
				char quote = tag[pos++];
				int valueEnd = Find( tag, quote, pos );

				if( valueEnd == -1 )
					valueEnd = Len( tag );

				attr.value = Substr( tag, pos, valueEnd - pos );
				pos = valueEnd + 1;
			}
			else
			{
				int valueEnd = pos;

				while( valueEnd < Len( tag ) && !isspace( (unsigned char)tag[valueEnd] ) )
					valueEnd++;

				attr.value = Substr( tag, pos, valueEnd - pos );
				pos = valueEnd;
			}
		}

		if( !attr.name.IsEmpty() )
			attrs.AddToTail( attr );
	}
}

// The style of an element: what it inherits, what its tag looks like, the rules of the page (by the specificity)
// and the attributes
void CHtmlLayout::ApplyElementStyle( Element &element, const CUtlVector<Attr> &attrs )
{
	const CUtlString &tag = element.tag;
	Style &style = element.style;
	float flParentSize = style.flFontSize;

	// not inherited
	style.bHidden = false;
	style.bBackground = false;
	style.flMargin[0] = style.flMargin[1] = style.flMargin[2] = style.flMargin[3] = 0.0f;

	if( tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "h5" || tag == "h6" )
	{
		static const float s_flSizes[] = { 2.0f, 1.5f, 1.17f, 1.0f, 0.83f, 0.67f };
		static const float s_flMargins[] = { 0.67f, 0.83f, 1.0f, 1.33f, 1.67f, 2.33f };
		int iLevel = tag[1] - '1';

		style.flFontSize = flParentSize * s_flSizes[iLevel];
		style.iWeight = 700;
		style.flMargin[0] = style.flMargin[2] = style.flFontSize * s_flMargins[iLevel];
	}
	else if( tag == "p" || tag == "dl" || tag == "fieldset" || tag == "address" )
	{
		style.flMargin[0] = style.flMargin[2] = style.flFontSize;

		if( tag == "address" )
			style.bItalic = true;
	}
	else if( tag == "ul" || tag == "ol" )
	{
		style.flMargin[0] = style.flMargin[2] = style.flFontSize;
		style.flMargin[3] = 40.0f * s_flZoom;
	}
	else if( tag == "blockquote" )
	{
		style.flMargin[0] = style.flMargin[2] = style.flFontSize;
		style.flMargin[1] = style.flMargin[3] = 40.0f * s_flZoom;
	}
	else if( tag == "dd" )
	{
		style.flMargin[3] = 40.0f * s_flZoom;
	}
	else if( tag == "pre" )
	{
		style.flMargin[0] = style.flMargin[2] = style.flFontSize;
		style.bPre = true;
		style.family = "Courier New";
	}
	else if( tag == "body" )
	{
		style.flMargin[0] = style.flMargin[1] = style.flMargin[2] = style.flMargin[3] = 8.0f * s_flZoom;
	}
	else if( tag == "b" || tag == "strong" )
	{
		style.iWeight = 700;
	}
	else if( tag == "th" )
	{
		style.iWeight = 700;
		style.iAlign = 1;
	}
	else if( tag == "i" || tag == "em" || tag == "cite" || tag == "var" || tag == "dfn" )
	{
		style.bItalic = true;
	}
	else if( tag == "u" || tag == "ins" )
	{
		style.bUnderline = true;
	}
	else if( tag == "a" && FindAttr( attrs, "href" ) )
	{
		// the link color of the browsers
		style.r = 0;
		style.g = 0;
		style.b = 238;
		style.bUnderline = true;
	}
	else if( tag == "small" )
	{
		style.flFontSize = flParentSize / 1.2f;
	}
	else if( tag == "big" )
	{
		style.flFontSize = flParentSize * 1.2f;
	}
	else if( tag == "sub" || tag == "sup" )
	{
		style.flFontSize = flParentSize * 0.83f;
	}
	else if( tag == "code" || tag == "tt" || tag == "kbd" || tag == "samp" )
	{
		style.family = "Courier New";
	}
	else if( tag == "center" )
	{
		style.iAlign = 1;
	}
	else if( tag == "head" || tag == "title" || tag == "script" || tag == "style" )
	{
		style.bHidden = true;
	}

	// the attributes of the old HTML
	const Attr *pAttr;
	int r, g, b;

	if( tag == "font" )
	{
		pAttr = FindAttr( attrs, "color" );

		if( pAttr && ParseColor( pAttr->value, r, g, b ) )
		{
			style.r = r;
			style.g = g;
			style.b = b;
		}

		pAttr = FindAttr( attrs, "face" );

		if( pAttr )
			style.family = ParseFamily( pAttr->value );

		pAttr = FindAttr( attrs, "size" );

		if( pAttr && !pAttr->value.IsEmpty() )
		{
			static const float s_flSizes[] = { 10.0f, 13.0f, 16.0f, 18.0f, 24.0f, 32.0f, 48.0f };
			int iSize = atoi( pAttr->value.Get() );

			// +n and -n are relative to the size 3
			if( pAttr->value[0] == '+' || pAttr->value[0] == '-' )
				iSize += 3;

			if( iSize < 1 )
				iSize = 1;
			else if( iSize > 7 )
				iSize = 7;

			style.flFontSize = s_flSizes[iSize - 1] * s_flZoom;
		}
	}

	if( tag == "body" )
	{
		pAttr = FindAttr( attrs, "bgcolor" );

		if( pAttr && ParseColor( pAttr->value, r, g, b ) )
		{
			style.bBackground = true;
			style.iBackground[0] = r;
			style.iBackground[1] = g;
			style.iBackground[2] = b;
		}

		pAttr = FindAttr( attrs, "text" );

		if( pAttr && ParseColor( pAttr->value, r, g, b ) )
		{
			style.r = r;
			style.g = g;
			style.b = b;
		}
	}

	pAttr = FindAttr( attrs, "align" );

	if( pAttr && IsBlockTag( tag ) )
	{
		CUtlString align = ToLower( pAttr->value );

		if( align == "center" )
			style.iAlign = 1;
		else if( align == "right" )
			style.iAlign = 2;
		else if( align == "left" )
			style.iAlign = 0;
	}

	// the rules of the page: by the specificity, the later one wins
	const Attr *pClass = FindAttr( attrs, "class" );
	const Attr *pId = FindAttr( attrs, "id" );

	CUtlVector<const CssRule *> matched;

	for( int i = 0; i < m_Rules.Count(); i++ )
	{
		const CssRule &rule = m_Rules[i];

		if( !rule.tag.IsEmpty() && rule.tag != tag )
			continue;

		if( !rule.cls.IsEmpty() )
		{
			bool bFound = false;

			if( pClass )
			{
				// the classes are separated with spaces
				CUtlString classes, needle;

				classes.Format( " %s ", pClass->value.Get() );
				needle.Format( " %s ", rule.cls.Get() );

				bFound = Find( classes, needle ) != -1;
			}

			if( !bFound )
				continue;
		}

		if( !rule.id.IsEmpty() && ( !pId || pId->value != rule.id ) )
			continue;

		// stable insertion by the specificity
		int j = matched.Count();

		matched.AddToTail( &rule );

		while( j > 0 && matched[j - 1]->iSpecificity > rule.iSpecificity )
		{
			matched[j] = matched[j - 1];
			j--;
		}

		matched[j] = &rule;
	}

	for( int i = 0; i < matched.Count(); i++ )
		ApplyDeclarations( style, matched[i]->props, flParentSize );

	pAttr = FindAttr( attrs, "style" );

	if( pAttr )
		ApplyDeclarations( style, pAttr->value, flParentSize );
}

void CHtmlLayout::StartLine( void )
{
	// the margin before the first line of a block
	m_y += m_flPendingMargin;
	m_flPendingMargin = 0.0f;

	m_flLineX = 0.0f;
	m_bPendingSpace = false;
}

void CHtmlLayout::FlushLine( void )
{
	if( m_Words.IsEmpty() )
		return;

	float flLineHeight = 0.0f;

	for( int i = 0; i < m_Words.Count(); i++ )
	{
		if( m_Words[i].flLineHeight > flLineHeight )
			flLineHeight = m_Words[i].flLineHeight;
	}

	float flAvailable = AvailableWidth();
	float flX = m_flLeft;

	if( m_Elements.Tail().style.iAlign == 1 )
		flX += ( flAvailable - m_flLineX ) / 2.0f;
	else if( m_Elements.Tail().style.iAlign == 2 )
		flX += flAvailable - m_flLineX;

	if( flX < m_flLeft )
		flX = m_flLeft;

	int iFirstRun = m_pResult->runs.Count();

	for( int i = 0; i < m_Words.Count(); i++ )
	{
		const Word &word = m_Words[i];

		flX += word.flGap;

		float flY = m_y + ( flLineHeight - word.flTall ) / 2.0f;

		// a word of the same style joins the one before it: one draw call for a piece of text
		if( m_pResult->runs.Count() > iFirstRun && word.flGap > 0.0f && !word.bUnderline )
		{
			HtmlRun &previous = m_pResult->runs.Tail();
			int a, b, c;

			// the gap is the space
			word.pFont->getCharABCwide( ' ', a, b, c );

			if( previous.pFont == word.pFont && previous.r == word.r && previous.g == word.g && previous.b == word.b &&
				previous.y == (int)( flY + 0.5f ) && fabs( word.flGap - ( a + b + c ) ) < 0.5f )
			{
				previous.text.Append( " " );
				previous.text.Append( word.text );
				flX += word.flWide;
				continue;
			}
		}

		HtmlRun run;

		run.x = (int)( flX + 0.5f );
		run.y = (int)( flY + 0.5f );
		run.iWide = (int)word.flWide;
		run.text = word.text;
		run.pFont = word.pFont;
		run.r = word.r;
		run.g = word.g;
		run.b = word.b;

		m_pResult->runs.AddToTail( run );

		// the underline is under the baseline
		if( word.bUnderline )
		{
			HtmlRect rect;

			rect.x0 = run.x;
			rect.x1 = run.x + (int)( word.flWide + 0.5f );
			rect.y0 = run.y + (int)( word.flTall * 0.85f );
			rect.y1 = rect.y0 + 1;
			rect.r = word.r;
			rect.g = word.g;
			rect.b = word.b;

			m_pResult->rects.AddToTail( rect );
		}

		flX += word.flWide;
	}

	m_y += flLineHeight;

	m_Words.RemoveAll();
	m_flLineX = 0.0f;
	m_bPendingSpace = false;
}

// <br>: ends the line; on an empty line it is a line of its own
void CHtmlLayout::LineBreak( void )
{
	if( m_Words.IsEmpty() )
	{
		const Style &style = TopStyle();
		vgui::Font *pFont = GetFont( style.family, (int)( style.flFontSize + 0.5f ), style.iWeight, style.bItalic );

		StartLine();
		m_y += ( style.flLineHeight > 0.0f ) ? style.flFontSize * style.flLineHeight : (float)pFont->getTall();
		return;
	}

	FlushLine();
}

void CHtmlLayout::AddWord( const CUtlString &text, bool bNoWrap )
{
	const Style &style = TopStyle();
	int iTall = (int)( style.flFontSize + 0.5f );

	if( iTall < 6 )
		iTall = 6;

	vgui::Font *pFont = GetFont( style.family, iTall, style.iWeight, style.bItalic );
	int iWide, iHeight;

	pFont->getTextSize( text.Get(), iWide, iHeight );

	float flGap = 0.0f;

	if( m_Words.IsEmpty() )
	{
		StartLine();
	}
	else if( m_bPendingSpace )
	{
		flGap = SpaceWidth( pFont );
	}

	flGap += m_flPendingGap;
	m_flPendingGap = 0.0f;

	// the word does not fit: the next line
	if( !m_Words.IsEmpty() && !bNoWrap && m_flLineX + flGap + iWide > AvailableWidth() )
	{
		FlushLine();
		StartLine();
		flGap = 0.0f;
	}

	// the first word of a line has no gap
	if( m_Words.IsEmpty() )
		flGap = 0.0f;

	Word word;

	word.text = text;
	word.pFont = pFont;
	word.r = style.r;
	word.g = style.g;
	word.b = style.b;
	word.bUnderline = style.bUnderline;
	word.flWide = (float)iWide;
	word.flTall = (float)pFont->getTall();

	// the line height is the multiplier of the font size, the normal one is the height of the font
	word.flLineHeight = ( style.flLineHeight > 0.0f ) ? style.flFontSize * style.flLineHeight : word.flTall;
	word.flGap = flGap;

	m_Words.AddToTail( word );

	m_flLineX += flGap + iWide;
	m_bPendingSpace = false;
}

void CHtmlLayout::AddText( const CUtlString &raw )
{
	if( m_iHidden > 0 )
		return;

	CUtlString text = DecodeEntities( raw );

	// the text before <body> is not shown
	if( !m_bBodySeen )
	{
		bool bBlank = true;

		for( int i = 0; i < Len( text ); i++ )
		{
			if( !isspace( (unsigned char)text[i] ) )
				bBlank = false;
		}

		if( bBlank )
			return;
	}

	const Style &style = TopStyle();

	if( style.bPre )
	{
		// the lines of the text are the lines of the page, nothing is collapsed or wrapped
		int pos = 0;

		while( pos <= Len( text ) )
		{
			int end = Find( text, '\n', pos );

			if( end == -1 )
				end = Len( text );

			CUtlString line = Substr( text, pos, end - pos );
			int iLength = Len( line );

			if( iLength > 0 && line[iLength - 1] == '\r' )
				line = Substr( line, 0, iLength - 1 );

			if( !line.IsEmpty() )
				AddWord( line, true );

			if( end < Len( text ) )
				LineBreak();

			pos = end + 1;
		}

		return;
	}

	// the spaces are collapsed: the space between the words is one gap
	int iLength = Len( text );
	int iWord = 0;

	for( int i = 0; i <= iLength; i++ )
	{
		bool bSpace = ( i == iLength ) || isspace( (unsigned char)text[i] ) || (unsigned char)text[i] == 0xA0;

		if( !bSpace )
			continue;

		if( i > iWord )
			AddWord( Substr( text, iWord, i - iWord ), false );

		iWord = i + 1;

		if( i < iLength )
			m_bPendingSpace = !m_Words.IsEmpty();
	}
}

// A block starts on a new line and has the margin above it, the left and right margins are the indent
void CHtmlLayout::OpenBlock( const Style &style )
{
	FlushLine();

	if( style.flMargin[0] > m_flPendingMargin )
		m_flPendingMargin = style.flMargin[0];

	m_flLeft += style.flMargin[3];
	m_flRight += style.flMargin[1];
}

void CHtmlLayout::CloseBlock( const Style &style )
{
	FlushLine();

	if( style.flMargin[2] > m_flPendingMargin )
		m_flPendingMargin = style.flMargin[2];

	m_flLeft -= style.flMargin[3];
	m_flRight -= style.flMargin[1];
}

void CHtmlLayout::CloseTop( void )
{
	Element &element = m_Elements.Tail();

	if( element.bBlock )
		CloseBlock( element.style );

	if( element.bHiddenCounted )
		m_iHidden--;

	m_Elements.RemoveMultipleFromTail( 1 );
}

void CHtmlLayout::CloseElement( const CUtlString &name )
{
	// the element that is closed is the last open one with the name, the ones above it are closed too
	for( int i = (int)m_Elements.Count() - 1; i > 0; i-- )
	{
		if( m_Elements[i].tag == name )
		{
			while( (int)m_Elements.Count() > i )
				CloseTop();

			return;
		}
	}
}

// The tags that close the element before them without a closing tag of their own
void CHtmlLayout::ImplicitClose( const CUtlString &name )
{
	const CUtlString &top = m_Elements.Tail().tag;

	if( top == "p" && IsBlockTag( name ) )
		CloseTop();
	else if( top == "li" && ( name == "li" ) )
		CloseTop();
	else if( ( top == "td" || top == "th" ) && ( name == "td" || name == "th" || name == "tr" ) )
		CloseTop();
	else if( top == "tr" && name == "tr" )
		CloseTop();
}

void CHtmlLayout::OpenElement( const CUtlString &name, const CUtlVector<Attr> &attrs )
{
	ImplicitClose( name );

	// <br> and <hr> do not have the content
	if( name == "br" )
	{
		if( m_iHidden == 0 )
			LineBreak();

		return;
	}

	Element element;

	element.tag = name;
	element.style = m_Elements.Tail().style;
	element.bBlock = IsBlockTag( name );
	element.bHiddenCounted = false;
	element.bList = ( name == "ul" || name == "ol" );
	element.iOrdered = ( name == "ol" ) ? 1 : 0;

	ApplyElementStyle( element, attrs );

	if( name == "body" )
		m_bBodySeen = true;

	// the page has the background of its body
	if( element.style.bBackground && ( name == "body" || name == "html" ) )
	{
		m_pResult->iBackground[0] = element.style.iBackground[0];
		m_pResult->iBackground[1] = element.style.iBackground[1];
		m_pResult->iBackground[2] = element.style.iBackground[2];
	}

	if( element.style.bHidden )
	{
		element.bHiddenCounted = true;
		m_iHidden++;
	}

	if( name == "hr" )
	{
		if( m_iHidden == 0 )
		{
			FlushLine();

			float flMargin = 8.0f * s_flZoom;

			if( flMargin > m_flPendingMargin )
				m_flPendingMargin = flMargin;

			m_y += m_flPendingMargin;
			m_flPendingMargin = 0.0f;

			HtmlRect rect;

			rect.x0 = (int)m_flLeft;
			rect.x1 = (int)( m_iWidth - m_flRight );
			rect.y0 = (int)m_y;
			rect.y1 = rect.y0 + 2;
			rect.r = rect.g = rect.b = 128;

			m_pResult->rects.AddToTail( rect );

			m_y += 2.0f;
			m_flPendingMargin = flMargin;
		}

		if( element.bHiddenCounted )
			m_iHidden--;

		return;
	}

	if( IsVoidTag( name ) )
	{
		if( element.bHiddenCounted )
			m_iHidden--;

		return;
	}

	// the cells of a row are separated
	if( name == "tr" )
		m_iCells = 0;

	if( name == "td" || name == "th" )
	{
		if( m_iCells > 0 && m_iHidden == 0 )
			m_flPendingGap = 24.0f * s_flZoom;

		m_iCells++;
	}

	if( element.bBlock && m_iHidden == 0 )
		OpenBlock( element.style );

	// the first element of the list shows the marker: a bullet or the number
	if( name == "li" && m_iHidden == 0 )
	{
		char szMarker[16] = "\xB7";

		// the number of the item is kept by the list
		for( int i = (int)m_Elements.Count() - 1; i >= 0; i-- )
		{
			if( m_Elements[i].bList )
			{
				if( m_Elements[i].iOrdered > 0 )
					sprintf( szMarker, "%d.", m_Elements[i].iOrdered++ );

				break;
			}
		}

		m_Elements.AddToTail( element );
		AddWord( szMarker, true );
		m_bPendingSpace = true;
		return;
	}

	m_Elements.AddToTail( element );
}

void CHtmlLayout::Run( const CUtlString &html, int iWidth, HtmlResult &result )
{
	m_pResult = &result;
	m_iWidth = iWidth;
	m_y = 0.0f;
	m_flLeft = 0.0f;
	m_flRight = 0.0f;
	m_flPendingMargin = 0.0f;
	m_flPendingGap = 0.0f;
	m_flLineX = 0.0f;
	m_bPendingSpace = false;
	m_iHidden = 0;
	m_iCells = 0;
	m_bBodySeen = false;

	m_Words.RemoveAll();
	m_Elements.RemoveAll();
	m_Rules.RemoveAll();

	result.runs.RemoveAll();
	result.rects.RemoveAll();
	result.iHeight = 0;

	// the page of the browser: black text on white, the serif font of 16 pixels
	Element root;
	Style &style = root.style;

	root.tag = "#root";
	root.bBlock = false;
	root.bHiddenCounted = false;
	root.bList = false;
	root.iOrdered = 0;

	style.r = style.g = style.b = 0;
	style.flFontSize = 16.0f * s_flZoom;
	style.iWeight = 400;
	style.bItalic = false;
	style.bUnderline = false;
	style.bPre = false;
	style.iAlign = 0;
	style.flLineHeight = 0.0f;
	style.family = "Times New Roman";
	style.bHidden = false;
	style.flMargin[0] = style.flMargin[1] = style.flMargin[2] = style.flMargin[3] = 0.0f;
	style.bBackground = false;
	style.iBackground[0] = style.iBackground[1] = style.iBackground[2] = 255;

	m_Elements.AddToTail( root );

	result.iBackground[0] = result.iBackground[1] = result.iBackground[2] = 255;

	int pos = 0;

	while( pos < Len( html ) )
	{
		if( html[pos] != '<' )
		{
			int end = Find( html, '<', pos );

			if( end == -1 )
				end = Len( html );

			AddText( Substr( html, pos, end - pos ) );
			pos = end;
			continue;
		}

		// the comments and the declarations
		if( StartsWith( html.Get() + pos, "<!--" ) )
		{
			int end = Find( html, "-->", pos + 4 );

			pos = ( end == -1 ) ? Len( html ) : end + 3;
			continue;
		}

		if( pos + 1 < Len( html ) && ( html[pos + 1] == '!' || html[pos + 1] == '?' ) )
		{
			int end = Find( html, '>', pos );

			pos = ( end == -1 ) ? Len( html ) : end + 1;
			continue;
		}

		// a quote in an attribute can hold the '>'
		int end = pos + 1;
		char quote = 0;

		while( end < Len( html ) )
		{
			if( quote )
			{
				if( html[end] == quote )
					quote = 0;
			}
			else if( html[end] == '"' || html[end] == '\'' )
			{
				quote = html[end];
			}
			else if( html[end] == '>' )
			{
				break;
			}

			end++;
		}

		CUtlString name;
		CUtlVector<Attr> attrs;
		bool bClosing;

		ParseTag( Substr( html, pos + 1, end - pos - 1 ), name, attrs, bClosing );

		pos = ( end < Len( html ) ) ? end + 1 : Len( html );

		if( name.IsEmpty() )
			continue;

		if( !bClosing && ( name == "style" || name == "script" ) )
		{
			// the content is the rules or the code
			CUtlString lower = ToLower( html );
			CUtlString closing;

			closing.Format( "</%s", name.Get() );

			int close = Find( lower, closing, pos );

			if( close == -1 )
				close = Len( html );

			if( name == "style" )
				ParseCss( Substr( html, pos, close - pos ) );

			int closeEnd = Find( html, '>', close );

			pos = ( closeEnd == -1 ) ? Len( html ) : closeEnd + 1;
			continue;
		}

		if( bClosing )
			CloseElement( name );
		else
			OpenElement( name, attrs );
	}

	FlushLine();

	while( m_Elements.Count() > 1 )
		CloseTop();

	float flBottom = m_y + m_flPendingMargin;

	result.iHeight = (int)( flBottom + 0.5f );
}

}

// The part of the view that is scrolled: draws the text and the lines of the page
class CDoDHtmlContent : public vgui::Panel
{
public:
	CDoDHtmlContent() : Panel( 0, 0, 10, 10 )
	{
		setPaintBackgroundEnabled( false );
	}

	HtmlResult m_Result;

	virtual void paint( void )
	{
		for( int i = 0; i < m_Result.rects.Count(); i++ )
		{
			const HtmlRect &rect = m_Result.rects[i];

			drawSetColor( rect.r, rect.g, rect.b, 0 );
			drawFilledRect( rect.x0, rect.y0, rect.x1, rect.y1 );
		}

		for( int i = 0; i < m_Result.runs.Count(); i++ )
		{
			const HtmlRun &run = m_Result.runs[i];

			drawSetTextFont( run.pFont );
			drawSetTextColor( run.r, run.g, run.b, 0 );
			drawPrintText( run.x, run.y, run.text.Get(), (int)Len( run.text ) );
		}
	}
};

CDoDHtmlView::CDoDHtmlView( int x, int y, int wide, int tall ) : CDoDScrollPanel( x, y, wide, tall )
{
	m_iContentWide = wide;
	m_iBackground[0] = m_iBackground[1] = m_iBackground[2] = 255;

	m_pContent = new CDoDHtmlContent();
	m_pContent->setParent( getClient() );

	AddWheelTarget( m_pContent );

	// the page has its own background
	setPaintBackgroundEnabled( true );
}

void CDoDHtmlView::SetHtmlBounds( int x, int y, int wide, int tall, int barWide )
{
	setBounds( x, y, wide, tall );
	SetBarWidth( barWide );

	// the layout depends on the width
	Relayout();
}

void CDoDHtmlView::SetHtml( const char *pszHtml )
{
	// the text of the page is UTF-8 or the one-byte charset
	CUtlVector<char> converted;

	converted.SetCount( (int)strlen( pszHtml ) + 1 );

	DoD_ConvertText( pszHtml, converted.Base(), converted.Count() );

	m_strHtml = converted.Base();

	Relayout();
	setScrollValue( 0, 0 );
}

void CDoDHtmlView::Relayout( void )
{
	CHtmlLayout layout;

	s_flZoom = HTML_ZOOM;

	// the scroll bar is shown when the page does not fit, like the HTML control of the original: the page is laid out
	// without it first and again, narrower, with it
	m_iContentWide = getWide();

	layout.Run( m_strHtml, m_iContentWide, m_pContent->m_Result );

	if( m_pContent->m_Result.iHeight > getTall() )
	{
		m_iContentWide = getWide() - GetBarWidth();

		layout.Run( m_strHtml, m_iContentWide, m_pContent->m_Result );

		setScrollBarVisible( false, true );
	}
	else
	{
		setScrollBarVisible( false, false );
	}

	m_iBackground[0] = m_pContent->m_Result.iBackground[0];
	m_iBackground[1] = m_pContent->m_Result.iBackground[1];
	m_iBackground[2] = m_pContent->m_Result.iBackground[2];

	int iHeight = m_pContent->m_Result.iHeight;

	m_pContent->setSize( m_iContentWide, iHeight > 1 ? iHeight : 1 );

	validate();
	repaint();
}

void CDoDHtmlView::paintBackground( void )
{
	int wide, tall;

	getPaintSize( wide, tall );

	drawSetColor( m_iBackground[0], m_iBackground[1], m_iBackground[2], 0 );
	drawFilledRect( 0, 0, wide, tall );
}
