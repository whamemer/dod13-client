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

#include <map>
#include <string>

#include "hud.h"
#include "cl_util.h"
#include "dod_localize.h"

static std::map<std::string, std::string> g_Tokens;

static std::string LowerCase( const char *psz )
{
	std::string s( psz );

	for( size_t i = 0; i < s.size(); i++ )
	{
		if( s[i] >= 'A' && s[i] <= 'Z' )
			s[i] += 'a' - 'A';
	}

	return s;
}

static void AppendUTF8( std::string &dest, unsigned int ch )
{
	if( ch < 0x80 )
	{
		dest += (char)ch;
	}
	else if( ch < 0x800 )
	{
		dest += (char)( 0xC0 | ( ch >> 6 ) );
		dest += (char)( 0x80 | ( ch & 0x3F ) );
	}
	else if( ch < 0x10000 )
	{
		dest += (char)( 0xE0 | ( ch >> 12 ) );
		dest += (char)( 0x80 | ( ( ch >> 6 ) & 0x3F ) );
		dest += (char)( 0x80 | ( ch & 0x3F ) );
	}
	else
	{
		dest += (char)( 0xF0 | ( ch >> 18 ) );
		dest += (char)( 0x80 | ( ( ch >> 12 ) & 0x3F ) );
		dest += (char)( 0x80 | ( ( ch >> 6 ) & 0x3F ) );
		dest += (char)( 0x80 | ( ch & 0x3F ) );
	}
}

// the localization files are UTF-16LE with a byte order mark
static std::string ToUTF8( const unsigned char *pData, int iLength )
{
	std::string out;
	int i = 0;

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

		out.assign( (const char *)pData + i, iLength - i );
	}

	return out;
}

// reads the next quoted string; returns false at the end of the text, a brace is returned as a one-character token
static bool NextToken( const std::string &text, size_t &pos, std::string &token, bool &bQuoted )
{
	token.clear();
	bQuoted = false;

	while( pos < text.size() )
	{
		char c = text[pos];

		if( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
		{
			pos++;
		}
		else if( c == '/' && pos + 1 < text.size() && text[pos + 1] == '/' )
		{
			while( pos < text.size() && text[pos] != '\n' )
				pos++;
		}
		else
		{
			break;
		}
	}

	if( pos >= text.size() )
		return false;

	if( text[pos] == '"' )
	{
		bQuoted = true;
		pos++;

		while( pos < text.size() && text[pos] != '"' )
		{
			if( text[pos] == '\\' && pos + 1 < text.size() )
			{
				pos++;

				switch( text[pos] )
				{
				case 'n':
					token += '\n';
					break;
				case 't':
					token += '\t';
					break;
				default:
					token += text[pos];
					break;
				}
			}
			else
			{
				token += text[pos];
			}

			pos++;
		}

		pos++;
		return true;
	}

	if( text[pos] == '{' || text[pos] == '}' )
	{
		token += text[pos++];
		return true;
	}

	while( pos < text.size() && text[pos] != ' ' && text[pos] != '\t' && text[pos] != '\r' && text[pos] != '\n' )
		token += text[pos++];

	return true;
}

int DoD_LoadLocalization( const char *pszFileName, bool bAppend )
{
	int iLength = 0;
	byte *pFile = gEngfuncs.COM_LoadFile( (char *)pszFileName, 5, &iLength );

	if( !bAppend )
		g_Tokens.clear();

	if( !pFile )
		return (int)g_Tokens.size();

	std::string text = ToUTF8( pFile, iLength );
	gEngfuncs.COM_FreeFile( pFile );

	size_t pos = 0;
	int iDepth = 0;
	bool bInTokens = false;
	std::string token, key;
	bool bQuoted;

	// "lang" { "Language" "English" "Tokens" { "key" "value" ... } }
	while( NextToken( text, pos, token, bQuoted ) )
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
			std::string value;
			bool bValueQuoted;

			key = token;

			if( !NextToken( text, pos, value, bValueQuoted ) )
				break;

			g_Tokens[LowerCase( key.c_str() )] = value;
		}
		else if( bQuoted && LowerCase( token.c_str() ) == "tokens" )
		{
			// the opening brace follows
			if( NextToken( text, pos, token, bQuoted ) && token == "{" )
			{
				iDepth++;
				bInTokens = true;
			}
		}
	}

	return (int)g_Tokens.size();
}

const char *DoD_FindLocalized( const char *pszToken )
{
	if( !pszToken )
		return NULL;

	if( *pszToken == '#' )
		pszToken++;

	std::map<std::string, std::string>::const_iterator it = g_Tokens.find( LowerCase( pszToken ) );

	if( it != g_Tokens.end() )
		return it->second.c_str();

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
