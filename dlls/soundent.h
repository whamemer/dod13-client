//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( SOUNDENTH )
#define SOUNDENTH

class CSound
{
    void Clear( void );
    void Reset( void );
    BOOL FIsSound( void );
    BOOL FIsScent( void );

    Vector m_vecOrigin;
    int m_iType;
    int m_iVolume;
    float m_flExpireTime;
    int m_iNext;
    int m_iNextAudible;
};
#endif // !SOUNDENTH