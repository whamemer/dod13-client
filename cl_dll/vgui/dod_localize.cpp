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
// dod_localize.cpp
//
// Xash3D FWGS has no VGUI2, so the localization that the original gets from vgui2::localize() is read here from
// resource/dod_english.txt by hand
//

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

#include "utlmap.h"
#include "utlstring.h"
#include "utlvector.h"

static CUtlOrderedMap<CUtlString, CUtlString> g_Tokens;

static CUtlString LowerCase( const char *psz )
{
	CUtlString s( psz );

	s.ToLower();

	return s;
}

static void AppendUTF8( CUtlVector<char> &dest, unsigned int ch )
{
	if( ch < 0x80 )
	{
		dest.AddToTail( (char)ch );
	}
	else if( ch < 0x800 )
	{
		dest.AddToTail( (char)( 0xC0 | ( ch >> 6 ) ) );
		dest.AddToTail( (char)( 0x80 | ( ch & 0x3F ) ) );
	}
	else if( ch < 0x10000 )
	{
		dest.AddToTail( (char)( 0xE0 | ( ch >> 12 ) ) );
		dest.AddToTail( (char)( 0x80 | ( ( ch >> 6 ) & 0x3F ) ) );
		dest.AddToTail( (char)( 0x80 | ( ch & 0x3F ) ) );
	}
	else
	{
		dest.AddToTail( (char)( 0xF0 | ( ch >> 18 ) ) );
		dest.AddToTail( (char)( 0x80 | ( ( ch >> 12 ) & 0x3F ) ) );
		dest.AddToTail( (char)( 0x80 | ( ( ch >> 6 ) & 0x3F ) ) );
		dest.AddToTail( (char)( 0x80 | ( ch & 0x3F ) ) );
	}
}

// the localization files are UTF-16LE with a byte order mark
static void ToUTF8( const unsigned char *pData, int iLength, CUtlVector<char> &out )
{
	int i = 0;

	out.RemoveAll();

	if( iLength >= 2 && pData[0] == 0xFF && pData[1] == 0xFE )
	{
		i = 2;

		for( ; i + 1 < iLength; i += 2 )
		{
			unsigned int ch = pData[i] | ( pData[i + 1] << 8 );

			if( ch >= 0xD800 && ch < 0xDC00 && i + 3 < iLength )
			{
				unsigned int lo = pData[i + 2] | ( pData[i + 3] << 8 );

				if( lo >= 0xDC00 && lo < 0xE000 )
				{
					ch = 0x10000 + ( ( ch - 0xD800 ) << 10 ) + ( lo - 0xDC00 );
					i += 2;
				}
			}

			AppendUTF8( out, ch );
		}
	}
	else
	{
		if( iLength >= 3 && pData[0] == 0xEF && pData[1] == 0xBB && pData[2] == 0xBF )
			i = 3;

		out.AddMultipleToTail( iLength - i, (const char *)pData + i );
	}
}

static void AppendSpan( CUtlString &dest, const char *text, int start, int end )
{
	if( end > start )
		dest.Append( text + start, end - start );
}

// reads the next quoted string; returns false at the end of the text, a brace is returned as a one-character token
static bool NextToken( const char *text, int length, int &pos, CUtlString &token, bool &bQuoted )
{
	token.Clear();
	bQuoted = false;

	while( pos < length )
	{
		char c = text[pos];

		if( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
		{
			pos++;
		}
		else if( c == '/' && pos + 1 < length && text[pos + 1] == '/' )
		{
			while( pos < length && text[pos] != '\n' )
				pos++;
		}
		else
		{
			break;
		}
	}

	if( pos >= length )
		return false;

	if( text[pos] == '"' )
	{
		int start;

		bQuoted = true;
		pos++;
		start = pos;

		while( pos < length && text[pos] != '"' )
		{
			if( text[pos] == '\\' && pos + 1 < length )
			{
				AppendSpan( token, text, start, pos );
				pos++;

				switch( text[pos] )
				{
				case 'n':
					token.AppendChar( '\n' );
					break;
				case 't':
					token.AppendChar( '\t' );
					break;
				default:
					token.AppendChar( text[pos] );
					break;
				}

				start = pos + 1;
			}

			pos++;
		}

		AppendSpan( token, text, start, pos );

		pos++;
		return true;
	}

	if( text[pos] == '{' || text[pos] == '}' )
	{
		token.AppendChar( text[pos++] );
		return true;
	}

	int start = pos;

	while( pos < length && text[pos] != ' ' && text[pos] != '\t' && text[pos] != '\r' && text[pos] != '\n' )
		pos++;

	AppendSpan( token, text, start, pos );

	return true;
}

int DoD_LoadLocalization( const char *pszFileName, bool bAppend )
{
	int iLength = 0;
	byte *pFile = gEngfuncs.COM_LoadFile( (char *)pszFileName, 5, &iLength );

	if( !bAppend )
		g_Tokens.RemoveAll();

	if( !pFile )
		return (int)g_Tokens.Count();

	CUtlVector<char> text;

	ToUTF8( pFile, iLength, text );
	gEngfuncs.COM_FreeFile( pFile );

	int pos = 0;
	int iDepth = 0;
	bool bInTokens = false;
	CUtlString token;
	bool bQuoted;

	// "lang" { "Language" "English" "Tokens" { "key" "value" ... } }
	while( NextToken( text.Base(), text.Count(), pos, token, bQuoted ) )
	{
		if( !bQuoted && token == "{" )
		{
			iDepth++;
		}
		else if( !bQuoted && token == "}" )
		{
			if( bInTokens && iDepth == 2 )
				bInTokens = false;

			iDepth--;
		}
		else if( bInTokens )
		{
			CUtlString value;
			bool bValueQuoted;

			if( !NextToken( text.Base(), text.Count(), pos, value, bValueQuoted ) )
				break;

			g_Tokens.InsertOrReplace( LowerCase( token ), value );
		}
		else if( bQuoted && LowerCase( token ) == "tokens" )
		{
			// the opening brace follows
			if( NextToken( text.Base(), text.Count(), pos, token, bQuoted ) && token == "{" )
			{
				iDepth++;
				bInTokens = true;
			}
		}
	}

	return (int)g_Tokens.Count();
}

const char *DoD_FindLocalized( const char *pszToken )
{
	if( !pszToken )
		return NULL;

	if( *pszToken == '#' )
		pszToken++;

	int i = g_Tokens.Find( LowerCase( pszToken ) );

	if( g_Tokens.IsValidIndex( i ) )
		return g_Tokens[i].Get();

	client_textmessage_t *pMsg = gEngfuncs.pfnTextMessageGet( pszToken );

	if( pMsg && pMsg->pMessage )
		return pMsg->pMessage;

	return NULL;
}

void DoD_ConstructLocalized( char *pszDest, int iDestSize, const char *pszFormat, const char *pszArg1, const char *pszArg2, const char *pszArg3, const char *pszArg4 )
{
	const char *pszArgs[4] = { pszArg1, pszArg2, pszArg3, pszArg4 };
	int iLen = 0;

	for( const char *p = pszFormat; *p && iLen < iDestSize - 1; )
	{
		if( p[0] == '%' && p[1] == 's' && p[2] >= '1' && p[2] <= '4' )
		{
			const char *pszArg = pszArgs[p[2] - '1'];

			if( pszArg )
			{
				while( *pszArg && iLen < iDestSize - 1 )
					pszDest[iLen++] = *pszArg++;
			}

			p += 3;
		}
		else
		{
			pszDest[iLen++] = *p++;
		}
	}

	pszDest[iLen] = 0;
}
