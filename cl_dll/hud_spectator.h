//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined(HUD_SPECTATOR_H)
#define HUD_SPECTATOR_H

#include "cl_entity.h"
#include "interpolation.h"

#define INSET_OFF			0
#define	INSET_CHASE_FREE		1
#define	INSET_IN_EYE			2
#define	INSET_MAP_FREE			3
#define	INSET_MAP_CHASE			4

#define	MAX_OVERVIEW_ENTITIES		128
#define MAX_SPEC_HUD_MESSAGES		8

#define OVERVIEW_TILE_SIZE		128		// don't change this
#define OVERVIEW_MAX_LAYERS		1

#define OBS_DRAW_HEALTH		(1 << 0)
#define OBS_DRAW_NAMES		(1 << 1)
#define OBS_DRAW_CONE		(1 << 2)
#define OBS_DRAW_STATUS		(1 << 3)
#define OBS_AUTO_DIRECTOR	(1 << 4)
#define OBS_PIP_ALLOWED		(1 << 5)
#define OBS_STAMINA_DRAW	(1 << 6)
#define OBS_ALLOW_ROAMING	(1 << 7)
#define OBS_ALLOW_CHASE		(1 << 8)
#define OBS_ALLOW_INEYE		(1 << 9)

extern void VectorAngles( const float *forward, float *angles );
extern "C" void NormalizeAngles( float *angles );

//-----------------------------------------------------------------------------
// Purpose: Handles the drawing of the spectator stuff (camera & top-down map and all the things on it )
//-----------------------------------------------------------------------------

typedef struct overviewInfo_s
{
	char		map[64];	// cl.levelname or empty
	vec3_t		origin;		// center of map
	float		zoom;		// zoom of map images
	int		layers;		// how may layers do we have
	float		layersHeights[OVERVIEW_MAX_LAYERS];
	char		layersImages[OVERVIEW_MAX_LAYERS][255];
	qboolean	rotated;	// are map images rotated (90 degrees) ?

	int		insetWindowX;
	int		insetWindowY;
	int		insetWindowHeight;
	int		insetWindowWidth;
} overviewInfo_t;

typedef struct overviewEntity_s
{
	HSPRITE					hSprite;
	struct cl_entity_s *	entity;
	double					killTime;
	bool					b_dodMapTag;
	vec3_t					origin;
} overviewEntity_t;

class CHudSpectator : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	void InitHUDData( void );

	int  ToggleInset( bool allowOff );
	void CheckSettings( void );
	bool AddOverviewEntityToList( HSPRITE sprite, cl_entity_t *ent, double killTime );
	void DeathMessage( int victim );
	bool AddOverviewEntity( int type, struct cl_entity_s *ent, const char *modelname );
	void CheckOverviewEntities( void );
	void DrawOverview( void );
	void DrawOverviewEntities( void );
	// void GetMapPosition( float *returnvec );
	void DrawOverviewLayer( void );
	void LoadMapSprites( void );
	bool ParseOverviewFile( void );
	bool IsActivePlayer( cl_entity_t *ent );
	void SetModes( int iMainMode, int iInsetMode );
	void HandleButtonsDown( int ButtonPressed );
	void HandleButtonsUp( int ButtonPressed );
	void FindNextPlayer( bool bReverse );
	void DirectorMessage( int iSize, void *pbuf );
	void SetSpectatorStartPosition();
	bool AddOverviewEntityToMap( HSPRITE sprite, cl_entity_t *ent, double killTime, vec3_t origin );
	void FindPlayer( const char *name );
	void AddVoiceIconToPlayerEnt( int index );
	HSPRITE GetMarkerSPR( int marker );
	void ClearVoiceIconFlags( void );
	bool ShouldSetVoiceIcon( int index );

	int                        m_hsprMapMarkers[15];
	int                        m_iDrawCycle;
	struct client_textmessage_s m_HUDMessages[MAX_SPEC_HUD_MESSAGES];
	char                       m_HUDMessageText[MAX_SPEC_HUD_MESSAGES][128];
	int                        m_lastHudMessage;
	overviewInfo_t             m_OverviewData;
	overviewEntity_t           m_OverviewEntities[MAX_OVERVIEW_ENTITIES];
	int                        m_iObserverFlags;
	int                        m_iSpectatorNumber;
	float                      m_mapZoom;
	vec3_t                     m_mapOrigin;

	cvar_t *m_drawnames;
	cvar_t *m_drawcone;
	cvar_t *m_drawstatus;
	cvar_t *m_autoDirector;
	cvar_t *m_pip;
	cvar_t *m_scoreboard;
	cvar_t *default_fov;

	HSPRITE                    m_hsprAllieZone;
	HSPRITE                    m_hsprAxisZone;
	HSPRITE                    m_hsprCapZone;
	HSPRITE                    m_hsprTnT;
	HSPRITE                    m_hsprBanglr;
	HSPRITE                    m_hsprGrenade;
	HSPRITE                    m_hsprStick;
	HSPRITE                    m_hsprCustom;
	HSPRITE                    m_hsprVoiceIcon;
	HSPRITE                    m_hsprSpeakerIcon;
	HSPRITE                    m_hsprAllieLight;
	HSPRITE                    m_hsprAxisLight;
	HSPRITE                    m_hsprBritLight;

	qboolean                   m_chatEnabled;
	vec3_t                     m_cameraOrigin;
	vec3_t                     m_cameraAngles;
	model_t					   *m_MapSprite;
	vec3_t                     m_vPlayerPos[64]; 

	HSPRITE                    m_hsprUnkownMap;
	HSPRITE                    m_hsprBeam;
	HSPRITE                    m_hsprCamera;
	HSPRITE                    m_hsprPlayer;
	HSPRITE                    m_hsprCameraAllies;
	HSPRITE                    m_hsprCameraAxis;
	HSPRITE                    m_hsprCameraBrit;
	HSPRITE                    m_hsprCameraSpec;
	bool                       m_bAddDrawIconNextFrame[64];

private:
	HSPRITE                    m_hsprPlayerDead;
	HSPRITE                    m_hsprViewcone;
	float                      m_flNextObserverInput;
	float                      m_zoomDelta;
	float                      m_moveDelta;
	int                        m_lastPrimaryObject;
	int                        m_lastSecondaryObject;
};
#endif // SPECTATOR_H
