//========= Copyright (c) 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_STATUS_HUD_H
#define VOICE_STATUS_HUD_H
#pragma once

#include "voice_status.h"

#include "cl_entity.h"
#include "cdll_int.h"

class CVoiceLabel
{
public:
    // WHAMER: TODO: need vgui2
    class VoiceVGUILabel //: public vgui2::Label
    {
    public:
        VoiceVGUILabel() {}
        ~VoiceVGUILabel();
    private:
        //virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );
    };

    CVoiceLabel() {}
    ~CVoiceLabel();

    //void SetFgColor( Color color );
    //void SetBgColor( Color color );
    void SetVisible( bool state );
    bool GetVisible( void );
    void GetContentSize( const int *wide, const int *tall );
    void SetBounds( int x, int y, int wide, int tall );
    void SetClientIndex( int index );
    int GetClientIndex( void );
    void SetLocation( const char *location );
    void SetPlayerName( const char *name );
    void RebuildLabelText( void );

private:
    VoiceVGUILabel *m_pLabel;

    int m_clientindex;

    wchar_t *m_locationString;
    char *m_playerName;
};

class CVoiceStatusHud : public IVoiceHud, public CHudBase
{
public:
    CVoiceStatusHud() {}
    ~CVoiceStatusHud();

    virtual int Init( IVoiceStatusHelper *pHelper, IVoiceStatus *pStatus );
    virtual int VidInit( void );
    virtual void CreateEntities( void );
    virtual void UpdateLocation( int entindex, const char *location );
    virtual void UpdateSpeakerStatus( int entindex, bool bTalking );
    CVoiceLabel *FindVoiceLabel( int clientindex );
    CVoiceLabel *GetFreeVoiceLabel( void );
    void RepositionLabels( void );

private:
    cl_entity_s m_VoiceHeadModels[32];
    HSPRITE m_VoiceHeadModel;

    float m_VoiceHeadModelHeight;
    IVoiceStatusHelper *m_pHelper;
    IVoiceStatus *m_pStatus;
    CVoiceLabel m_Labels;

    //ImagePanel *m_pLocalPlayerTalkIcon;
};
#endif // VOICE_STATUS_HUD_H
