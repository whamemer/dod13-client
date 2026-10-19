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
// dodkeyvalues.h
//
// A small reader of the KeyValues text files of the original (Resource/UI/*.res, Resource/Classes/*.res):
// the sections and the "key" "value" pairs, the comments are //. The files of the game are loaded through the
// filesystem of the engine.
//

#ifndef DODKEYVALUES_H
#define DODKEYVALUES_H

class CDoDKeyValues
{
public:
	~CDoDKeyValues();

	// the root has the sections of the file as its sub keys; NULL when the file does not exist
	static CDoDKeyValues *LoadFromFile( const char *pszFile );

	const char *GetName( void ) const { return m_pszName; }

	// the value of a sub key, the names are not case sensitive
	const char *GetString( const char *pszKey, const char *pszDefault = "" ) const;
	int GetInt( const char *pszKey, int iDefault = 0 ) const;
	float GetFloat( const char *pszKey, float flDefault = 0.0f ) const;

	CDoDKeyValues *FindKey( const char *pszKey ) const;
	CDoDKeyValues *GetFirstSubKey( void ) const { return m_pChild; }
	CDoDKeyValues *GetNextKey( void ) const { return m_pNext; }

private:
	CDoDKeyValues( const char *pszName );

	void SetValue( const char *pszValue );
	void AddChild( CDoDKeyValues *pChild );

	static const char *Parse( const char *pszText, CDoDKeyValues *pParent );

	char *m_pszName;
	char *m_pszValue;
	CDoDKeyValues *m_pChild;
	CDoDKeyValues *m_pLastChild;
	CDoDKeyValues *m_pNext;
};

#endif // DODKEYVALUES_H
