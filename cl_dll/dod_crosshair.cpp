#include "hud.h"

int CHudDoDCrossHair::Init( void )
{
    int iWidth, m_iXPos, m_iYPos, m_iFlags;
    float m_fMoveTime;

    gEngfuncs.pfnHookUserMsg( "ClanTimer", __MsgFunc_ClanTimer );
    CHud::AddHudElem( &gHUD );
    iWidth = gHUD.m_scrinfo.iWidth;
    m_iYPos = gHUD.m_scrinfo.iHeight / 2;
    m_iXPos = iWidth / 2;
    m_iFlags |= 1u;
    m_fMoveTime = 0.0f;

    return 1;
}

int CHudDoDCrossHair::VidInit( void )
{
    
}

void CHudDoDCrossHair::Reset( void )
{

}

int CHudDoDCrossHair::MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf )
{

}

int CHudDoDCrossHair::Draw( float fltime )
{

}

void CHudDoDCrossHair::DrawDynamicCrossHair( void )
{

}

void CHudDoDCrossHair::DrawCustomCrossHair( int style )
{

}

int CHudDoDCrossHair::GetCrossHairWidth( void )
{

}

float CHudDoDCrossHair::GetCurrentWeaponAccuracy( void )
{

}

bool CHudDoDCrossHair::ShouldDrawCrossHair( void )
{

}

void CHudDoDCrossHair::DrawSpectatorCrossHair( void )
{

}

void CHudDoDCrossHair::DrawClanTimer( float fltime )
{

}