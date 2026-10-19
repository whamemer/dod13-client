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
// dodkeyvalues.cpp
//

#include <string.h>
#include <stdlib.h>

#include "hud.h"
#include "cl_util.h"

#include "vgui/dodkeyvalues.h"

static char *CopyString( const char *psz )
{
	char *pszCopy = new char[strlen( psz ) + 1];

	strcpy( pszCopy, psz );

	return pszCopy;
}

// Reads a token: a string in quotes, a brace or a word; returns NULL at the end of the text
static const char *ReadToken( const char *psz, char *pszToken, int iSize, bool &bQuoted )
{
	int iLength = 0;

	bQuoted = false;

	// skip the spaces and the comments
	for( ;; )
	{
		while( *psz && (unsigned char)*psz <= ' ' )
			psz++;

		if( psz[0] == '/' && psz[1] == '/' )
		{
			while( *psz && *psz != '\n' )
				psz++;

			continue;
		}

		break;
	}

	if( !*psz )
		return NULL;

	if( *psz == '"' )
	{
		bQuoted = true;
		psz++;

		while( *psz && *psz != '"' )
		{
			if( iLength < iSize - 1 )
				pszToken[iLength++] = *psz;

			psz++;
		}

		if( *psz == '"' )
			psz++;
	}
	else if( *psz == '{' || *psz == '}' )
	{
		pszToken[iLength++] = *psz++;
	}
	else
	{
		while( *psz && (unsigned char)*psz > ' ' && *psz != '{' && *psz != '}' && *psz != '"' )
		{
			if( iLength < iSize - 1 )
				pszToken[iLength++] = *psz;

			psz++;
		}

		// a quote that only closes the word, there is one in the files ( "textAlignment" west" )
		if( *psz == '"' )
			psz++;
	}

	pszToken[iLength] = '\0';

	return psz;
}

CDoDKeyValues::CDoDKeyValues( const char *pszName )
{
	m_pszName = CopyString( pszName );
	m_pszValue = NULL;
	m_pChild = NULL;
	m_pLastChild = NULL;
	m_pNext = NULL;
}

CDoDKeyValues::~CDoDKeyValues()
{
	CDoDKeyValues *pChild = m_pChild;

	while( pChild )
	{
		CDoDKeyValues *pNext = pChild->m_pNext;

		delete pChild;
		pChild = pNext;
	}

	delete[] m_pszName;
	delete[] m_pszValue;
}

void CDoDKeyValues::SetValue( const char *pszValue )
{
	delete[] m_pszValue;
	m_pszValue = CopyString( pszValue );
}

void CDoDKeyValues::AddChild( CDoDKeyValues *pChild )
{
	if( m_pLastChild )
		m_pLastChild->m_pNext = pChild;
	else
		m_pChild = pChild;

	m_pLastChild = pChild;
}

// Reads the keys of a section up to its closing brace; returns the text after it or NULL at the end of the text
const char *CDoDKeyValues::Parse( const char *pszText, CDoDKeyValues *pParent )
{
	char szKey[256];
	char szValue[1024];
	bool bQuoted;

	for( ;; )
	{
		pszText = ReadToken( pszText, szKey, sizeof( szKey ), bQuoted );

		if( !pszText )
			return NULL;

		if( !bQuoted && szKey[0] == '}' )
			return pszText;

		CDoDKeyValues *pKey = new CDoDKeyValues( szKey );

		pParent->AddChild( pKey );

		pszText = ReadToken( pszText, szValue, sizeof( szValue ), bQuoted );

		if( !pszText )
			return NULL;

		if( !bQuoted && szValue[0] == '{' )
		{
			pszText = Parse( pszText, pKey );

			if( !pszText )
				return NULL;
		}
		else
		{
			pKey->SetValue( szValue );
		}
	}
}

CDoDKeyValues *CDoDKeyValues::LoadFromFile( const char *pszFile )
{
	char *pszData = (char *)gEngfuncs.COM_LoadFile( pszFile, 5, NULL );

	if( !pszData )
		return NULL;

	CDoDKeyValues *pRoot = new CDoDKeyValues( pszFile );

	Parse( pszData, pRoot );

	gEngfuncs.COM_FreeFile( pszData );

	return pRoot;
}

CDoDKeyValues *CDoDKeyValues::FindKey( const char *pszKey ) const
{
	for( CDoDKeyValues *pChild = m_pChild; pChild; pChild = pChild->m_pNext )
	{
		if( !stricmp( pChild->m_pszName, pszKey ) )
			return pChild;
	}

	return NULL;
}

const char *CDoDKeyValues::GetString( const char *pszKey, const char *pszDefault ) const
{
	CDoDKeyValues *pKey = FindKey( pszKey );

	if( pKey && pKey->m_pszValue )
		return pKey->m_pszValue;

	return pszDefault;
}

int CDoDKeyValues::GetInt( const char *pszKey, int iDefault ) const
{
	CDoDKeyValues *pKey = FindKey( pszKey );

	if( pKey && pKey->m_pszValue )
		return atoi( pKey->m_pszValue );

	return iDefault;
}

float CDoDKeyValues::GetFloat( const char *pszKey, float flDefault ) const
{
	CDoDKeyValues *pKey = FindKey( pszKey );

	if( pKey && pKey->m_pszValue )
		return (float)atof( pKey->m_pszValue );

	return flDefault;
}
