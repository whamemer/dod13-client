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
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//
#pragma once
#if !defined(HUD_H)
#define HUD_H
#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010 //255,160,0
#define RGB_GREENISH 0x0000A000 //0,160,0

#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"

#include "cl_entity.h"
#include "r_efx.h"

#include "com_model.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	

#define	HUDELEM_ACTIVE 1

typedef struct
{
	int x, y;
} POSITION;

enum 
{ 
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16
};

typedef struct
{
	unsigned char r, g, b, a;
} RGBA;

struct HUDLIST
{
	CHudBase *p;
	HUDLIST *pNext;
};

struct ClipInfo
{
	int weapon_id;
	int full_index;
	int empty_index;
	int extra_index;

	HSPRITE FullSprite;
	HSPRITE EmptySprite;
	HSPRITE ExtraSprite;

	wrect_t *FullArea;
	wrect_t *EmptyArea;
	wrect_t *ExtraArea;

	float lastDrop;
	float subseqDrop;
};

typedef struct cvar_s cvar_t;

#define HUD_ACTIVE	1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH		32

#define	MAX_MOTD_LENGTH				1536

#define MAX_SERVERNAME_LENGTH	64
#define MAX_TEAMNAME_SIZE 32

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	POSITION m_pos;
	int m_type;
	int m_iFlags;

	virtual ~CHudBase() {}
	virtual int Init( void ) { return 0; }
	virtual int VidInit( void ) { return 0; }
	virtual int Draw( float flTime ) { return 0; }
	virtual void Think( void ) { return; }
	virtual void Reset( void ) { return; }
	virtual void PlayerDied( void ) {}
	virtual void InitHUDData( void ) {}
};

#include "hud_spectator.h"
#include "mortarhud.h"

//
//-----------------------------------------------------
//
class CHudAmmo : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Think( void );
	void Reset( void );
	void PlayerDied( void );
	int DrawWeaponList( float flTime );
	int DrawWList( float flTime );
	int MsgFunc_CurWeapon( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeaponList( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AmmoX( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AmmoShort( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ReloadDone( const char *pszName, int iSize, void *pbuf );

	void SlotInput( int iSlot );
	void _cdecl UserCmd_Slot1( void );
	void _cdecl UserCmd_Slot2( void );
	void _cdecl UserCmd_Slot3( void );
	void _cdecl UserCmd_Slot4( void );
	void _cdecl UserCmd_Slot5( void );
	void _cdecl UserCmd_Slot6( void );
	void _cdecl UserCmd_Slot7( void );
	void _cdecl UserCmd_Slot8( void );
	void _cdecl UserCmd_Slot9( void );
	void _cdecl UserCmd_Slot10( void );
	void _cdecl UserCmd_Close( void );
	void _cdecl UserCmd_NextWeapon( void );
	void _cdecl UserCmd_PrevWeapon( void );

	ClipInfo *GetCurrentGun( WEAPON *pw );

	int GetCurrentWeaponId( void );

private:
	float m_fFade;

	RGBA m_rgba;

	WEAPON *m_pWeapon;

	int m_HUD_clip;
	int m_HUD_clipemty;

	wrect_t *m_prc1;
	wrect_t *m_prc2;

	int m_iHeight;
	int m_iClipHeight;

	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;

	HSPRITE MGBarrelHUD;
	wrect_t *MGBarrelHUDArea;
	HSPRITE MGBarrel2HUD;
	wrect_t *MGBarrel2HUDArea;

	ClipInfo ClipInfoArray[64];
	ClipInfo BritGrenClipInfo;
};

//
//-----------------------------------------------------
//
class CHudTrain : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_Train( const char *pszName, int iSize, void *pbuf );

private:
	HSPRITE m_hSprite;
	int m_iPos;
};

//
//-----------------------------------------------------
//
class CHudDoDCrossHair : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void SetDoDCrosshair( void );
	int Draw( float flTime );
	void Reset( void );
	int MsgFunc_ClanTimer( const char *pszName, int iSize, void *pbuf );

	bool ShouldDrawCrossHair( void );
	float GetCurrentWeaponAccuracy( void );
	void DrawClanTimer( float flTime );
	void DrawSpectatorCrossHair( void );
	void DrawDynamicCrossHair( void );
	void DrawCustomCrossHair( int style );
	int GetCrossHairWidth( void );

	float m_fClanTimer;

	int i_xSet;
	float f_xPos;
	float f_yPos;
	float f_xLPos;
	float f_xRPos;

	vec3_t v_evPunch;

	int m_MoveWidth;
	int m_MoveHeigth;
	int m_MoveWidthD;
	int m_MoveHeightD;

	HSPRITE CrossSprite2horiz;
	wrect_t *CrossArea2horiz;
	HSPRITE CrossSprite2vert;
	wrect_t *CrossArea2vert;
	HSPRITE CrossSprite2dot;
	wrect_t *CrossArea2dot;

	HSPRITE m_hCrosshair;
	wrect_t m_crosshairRect;

	HSPRITE m_hCustomCrosshair;
	wrect_t m_customCrosshairRect;

	int m_iXPos;
	int m_iYPos;

	float m_fMoveTime;

	int m_flLastSetTime;
	int m_iLastXHairWidth;
};

//
//-----------------------------------------------------
//
class CHudDoDMap : public CHudBase
{
public:
	int VidInit( void );
	void InitHUDData( void );
	int Draw( float flTime );
	int Init( void );

	void DrawOverview( void );
	void DrawOverviewLayer( void );
	void DrawOverviewEntities( void );
	void CheckOverviewEntities( void );

	//void CreateMapSprite(  ); WHAMER: TODO
	void HandleMapButton( void );
	void HandleMapZoomButton( void );
	void SetMapState( int mapstate );
	void GetSmallMapOffset( const float *x, const float *y, const float *z );
	bool AddMapEntityToMap( HSPRITE sprite, double lifeTime, Vector origin );
	void DrawOverviewIcon( Vector origin, HSPRITE hIcon, int iconScale, Vector angles );

	typedef struct
	{
		vec3_t origin;
		HSPRITE hSprite;
		double killTime;
	} map_icon_t;
	
	map_icon_t m_ExtraOverviewEntities[32];

private:
	cl_entity_s m_MapTag[64];

	float m_flZoom;
	float m_flZoomTime;
	float m_flSmallMapScale;
	float m_flIdealMapScale;
	float m_flPreviousMapScale;
};

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	int MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf );

	int GetTargetHealth( void );
	int GetTargetIndex( void );
	int GetTargetTeam( void );
	char *GetTargetName( void );

	void CreateEntities( void );
	void DrawEntitiesOverTeam( void );
	void DrawEntitiesOverTarget( void );

	void UpdateWhosTalking( int update, qboolean talk );
	bool InDeathCamMode( void );
	void Think( void );
	qboolean IsEntityTalking( int talk );

protected:
	enum
	{ 
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2
	};

	float m_flNextUpdateTime;

	int m_iTargetTeam;
	int m_iTargetIndex;
	int m_iHealth;
	cl_entity_s m_TargetHeadModel[64];
	int m_TargetTalking[64];

	HSPRITE m_StatusBarAllieModel;
	HSPRITE m_StatusBarBritishModel;
	HSPRITE m_StatusBarGermanModel;
	HSPRITE m_StatusBarHeadModel;

	cvar_t *hud_centerid;
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS + 1];
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];
extern team_info_t			g_TeamInfo[MAX_TEAMS + 1];
extern int					g_IsSpectator[MAX_PLAYERS + 1];

//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf );

private:
	int m_HUD_d_skull;
	cvar_t *hud_deathnotice_time;
};

//
//-----------------------------------------------------
//
class CHudMenu : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	int MsgFunc_ShowMenu( const char *pszName, int iSize, void *pbuf );

	void ShowMenu_Votekick( int vote );
	void ShowMenu_ListPlayers( int list );

	void SelectMenuItem( int menu_item );

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;

	int CanCancel;
};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_SayText( const char *pszName, int iSize, void *pbuf );
	void SayTextPrint( const wchar_t *pszBuf, int iBufSize, int clientIndex );
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
	int GetTextPrintY( void );
	friend class CHudSpectator;

private:
	struct cvar_s *m_HUD_saytext;
	struct cvar_s *m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
struct control_point_t
{
	int entindex;
	bool valid;

	int visible;
	int owner;
	int nextOwner;

	float animtime;
	float totaltime;

	int numplayers;
	int requiredplayers;
	int occupyingteam;

	int m_iIcons[3];
	rect_s m_rAreas[3];

	vec3_t m_rOrigin;

	int m_iMapXPos;
	int m_iMapYPos;
};

class CObjectiveIcons : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Think( void );

	int MsgFunc_InitObj( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SetObj( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_StartProg( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_StartProgF( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ProgUpdate( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CancelProg( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TimerStatus( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_PlayersIn( const char *pszName, int iSize, void *pbuf );

	void ClearAllCapPoints( void );
	void ChangeCapPoint( int point, int newowner, int timedCap );
	void StartCapProgress( int point, int newOwner, float time );
	void CancelCapProgress( int point, int owner );

	void SetNumPlayersInArea( int point, int team, int numplayers, int required );

	void SetVisible( int point, int visible );

	void UpdateObjectiveIcons( void );
	void GetObjectiveTime( void );

	float GetWaveTime( void );
	void SetWaveTime( float time );
	void SetWaveStatus( int status );
	int GetWaveStatus( void );

	void CalcIconLocations( void );

	void SetWarmupMode( bool mode );

	bool IsPointValid( int valid );
	void DrawDigit( int digit, int x, int y );

	wrect_t m_iconarea;
	HSPRITE TimerHUD;
	wrect_t *TimerHUDArea;

private:
	float m_fTimerSeconds;
	float m_flWaveTime;

	int m_Init;
	int m_Set;
	int m_StartProgress;
	int m_CancelProgress;
	int m_iWaveStatus;

	float m_fLastTime;

	control_point_t m_eControlPoints[12];

	int m_nTimerStatus;
	int m_TimerIcons[11];

	wrect_t *m_TimerAreas[11];

	wrect_t m_topwrect;
	wrect_t m_bottomwrect;

	HSPRITE m_TopNumber;
	HSPRITE m_BottomNumber;

	bool m_bWarmupMode;
};

//
//-----------------------------------------------------
//
class CHudTextMessage : public CHudBase
{
public:
	int Init( void );
	static char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	static char *BufferedLocaliseTextString( const char *msg );
	const char *LookupString( const char *msg_name, int *msg_dest = NULL );
	int MsgFunc_TextMsg( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CapMsg( const char *pszName, int iSize, void *pbuf );
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t	*pMessage;
	float	time;
	int x, y;
	int	totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

class CHudMessage : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_HudText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_GameTitle( const char *pszName, int iSize, void *pbuf );

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd( client_textmessage_t *newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

	void HintMessageAdd( const char *pText );

	//vgui2::HFont GetFont( void ); WHAMER: TODO: need vgui2 :(

private:
	client_textmessage_t		*m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;

	unsigned long m_Fonts[3];
};

//
//-----------------------------------------------------
//
class CHudScope : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Think( void );
	int MsgFunc_Scope( const char *pszName, int iSize, void *pbuf );
	void Reset( void );
	void DrawTriApiScope( void );
	void SetScope( int weaponId );

private:
	int m_iWeaponId;
	bool m_bWeaponChanged;
	int m_nLastWpnId;

	HSPRITE spring_sprite;
	model_s *spring_model;
	HSPRITE k43_sprite;
	model_s *k43_model;
	HSPRITE binoc_sprite;
	model_s *binoc_model;
	HSPRITE enfield_sprite;
	model_s *enfield_model;
};

//
//-----------------------------------------------------
//
class CHudDoDCommon : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );

	int MsgFunc_GameRules( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ParaLand( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf );

	int Draw( float flTime );
	int VidInit( void );

	int MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf );
};

//
//-----------------------------------------------------
//
struct ClientArea
{
	int nStatus;
	HSPRITE hSpr;
};

class CHudDodIcons : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	void PlayerDied( void );

	int MsgFunc_Health( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_Object( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ClientAreas( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ClCorpse( const char *pszName, int iSize, void *pbuf );

	int Draw( float flTime );
	void MapMarkerPosted( void );
	void ActivateHintBacking( int team, float flTime );
	void GetHintMessageLocation( const int *x, const int *y );
	void StartDrawingCredits( void );
	void DrawCredits( float flTime );
	void DrawMarkerIcon( HSPRITE pSpr );
	void DrawObjectiveTimer( void );
	void UpdateReinforcementTimer( void );

	int m_iHealth;

	char *m_szObjectIcon;
	HSPRITE m_hObjectSpr;
	wrect_t *m_wObjectArea;

	int sprite_array[5];
	wrect_t *area_array[5];

	int m_iKey;

	HSPRITE MapMarkerSprite;
	wrect_t *MapMarkerArea;
	float m_fLastMapMarkerTime;

private:
	ClientArea m_Areas[128];

	HSPRITE IconMGDeploy;
	wrect_t *IconMGDeployArea;
	HSPRITE MainHUD;
	wrect_t *MainHUDArea;
	HSPRITE ObjectivesHUD;
	wrect_t *ObjectivesHUDArea;
	HSPRITE ReinforcementsHUD;
	wrect_t *ReinforcementsHUDArea;
	HSPRITE HeartHUD;
	wrect_t *HeartHUDArea;
	HSPRITE StaminaBarHUD;
	wrect_t *StaminaBarHUDArea;
	HSPRITE HealthBarHUD;
	wrect_t *HealthBarHUDArea;

	int i_HeartFrame;
	float f_HeartTime;
	float f_HeartSpeed;

	HSPRITE HealthOverlayHUD;
	wrect_t *HealthOverlayHUDArea;

	int m_iIconHeight;
	int m_iIconWidth;

	float m_flHintDieTime;
	int m_iHintTeam;

	int m_iMsgX;
	int m_iMsgY;

	int m_hHintHeads[3];
	wrect_t *m_rectHintHeads[3];
	HSPRITE m_hHorizHintBacking;
	HSPRITE m_hVertHintBacking;
	wrect_t *m_rectHorizHintBacking;
	wrect_t *m_rectVertHintBacking;

	int m_iCreditName;
	float m_flCreditChangeTime;
	HSPRITE m_hsprSelectedMarker;
	float m_flDrawSelectedMarkerTime;
};

//
//-----------------------------------------------------
//
struct particle_shooter_s
{
	int id;
	int iNumParticles;
	int iParticlesRemaining;

	float fParticleLife;
	float fFireDelay;

	int iFlags;
	vec3_t vOrigin, vVelocity;

	float fVariance;
	float fNextShootTime;

	int iSprite;
	char szSprite[128];
	model_s *pSprite;

	int iColour[3];

	float fGravity;
	float fFadeSpeed;
	float fSize;
	float fDampingTime;
	float fDampingVel;

	int iSpinDegPerSec;

	float fBrightness;
	float fScaleSpeed;

	int iFramerate;
	int iRenderMode;
	int iState;
};


class CParticleShooter : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );
	void AddParticleSystem( particle_shooter_s *pShooter );

	int MsgFunc_PReg( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_PShoot( const char *pszName, int iSize, void *pbuf );

private:
	particle_shooter_s m_sShooters[64];
	int m_iNumShooters;
};

//
//-----------------------------------------------------
//
// WHAMER: TODO
struct env_model_t
{
	const char *szModel[64];
	const char *szSequence[64];

	int iBody;

	vec3_t vecOrigin, vecAngles;

	int spawnflags;
	int frame;
	int sequence;
	int iModel;
	int pModel;
};


class CClientEnvModel : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );
	void RemoveAllModels( void );
	void AddEnvModel( env_model_t *pModel );
	int GetNumModels( void );
	env_model_t *GetModel( int model );

private:
	TEMPENTITY *m_teEnvModelTE;
	env_model_t m_sEnvModels[192];
	int m_iNumEnvModels;
};

//
//-----------------------------------------------------
//
class CWeatherManager : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );

	void CreateRainParticle( float *origin );
	void CreateSnowParticle( float *origin );

	void SetRainSprite( model_t *model );
	void SetSnowSprite( model_t *model );
	void SetSplashSprite( model_t *model );
	void SetRippleSprite( model_t *model );

	model_t *GetRainSprite( void );
	model_t *GetSnowSprite( void );
	model_t *GetSplashSprite( void );
	model_t *GetRippleSprite( void );

private:
	model_t *m_pRainSprite;
	model_t *m_pSnowSprite;
	model_t *m_pSplashSprite;
	model_t *m_pRippleSprite;
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH	24

class CHudStatusIcons : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	int MsgFunc_StatusIcon( const char *pszName, int iSize, void *pbuf );

	enum
	{
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4
	};
	
	void EnableIcon( const char *pszIconName, unsigned char red, unsigned char green, unsigned char blue );
	void DisableIcon( const char *pszIconName );

private:
	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];
};

//
//-----------------------------------------------------
//
// WHAMER: TODO: need vgui2 :(
class CHudVGUI2Print : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );

	int DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b );
	int DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b );

	int DrawVGUI2String( wchar_t *msg, int x, int y, float r, float g, float b );
	int DrawVGUI2StringReverse( wchar_t *msg, int x, int y, float r, float g, float b );

	void VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );

	int GetHudFontHeight( void );

	void GetStringSize( const wchar_t *string, int *width, int *height );

	int Draw( float flTime );

	//vgui2::HFont GetFont( void );

private:
	float m_flVGUI2StringTime;
	wchar_t m_wCharBuf[512];

	float m_fR, m_fG, m_fB;
	int m_iX, m_iY;

	unsigned long m_Fonts[3];
};

//
//-----------------------------------------------------
//
class Element
{
	float flTimeCreated;
	
	Element *next;
	Element *previous;

	Element( float fl );
};

class Queue
{
	int count;
	int maxelements;
	float m_flDuration;

	Element *first;
	Element *last;
	Element *current;

	bool Full( void );
	bool Add( float fl );
	void Update( float fl );
};

//
//-----------------------------------------------------
//
class CHud
{
private:
	HUDLIST *m_pHudList;
	HSPRITE m_hsprLogo;
	int m_iLogo;

	client_sprite_t *m_pSpriteList;
	int m_iSpriteCount;
	int m_iSpriteCountAllRes;

public:
	HSPRITE m_hsprCursor;

	float m_flTime;
	float m_fOldTime;
	double m_flTimeDelta;

	vec3_t m_vecOrigin;
	vec3_t m_vecAngles;
	vec3_t m_vecVelocity;

	int m_iKeyBits;
	int m_iHideHUDDisplay;
	int m_Teamplay;
	int m_iRes;

	cvar_t *m_pCvarStealMouse;
	cvar_t *m_pCvarDraw;

	float m_flMouseSensitivity;

	int m_PlayerFOV[64];

	bool m_bAllieParatrooper;
	bool m_bAllieInfiniteLives;
	bool m_bAxisParatrooper;
	bool m_bAxisInfiniteLives;
	bool m_bParatrooper;
	bool m_bInfiniteLives;
	bool m_bBritish;

	int m_iRoundState;
	int m_iSensLevel;
	int m_iFontHeight;
	int m_iFontEngineHeight;

	void SetSensitivity( int level );
	int DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth( int iNumber, int iFlags );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );
	void PostMortarValue( float value );
	int GetMinimapZoomLevel( void );
	int ZoomMinimap( void );

private:
	HSPRITE *m_rghSprites;
	wrect_t *m_rgrcRects;

public:
	HSPRITE GetSprite( int index ) 
	{
		return ( index < 0 ) ? 0 : m_rghSprites[index];
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects[index];
	}
	
	int GetSpriteIndex( const char *SpriteName );

	char *m_rgszSpriteNames;
	char m_szTeamNames[5][32];

	int m_iFOV;
	int m_iMapX;
	int m_iMapY;
	int m_iMapWidth;
	int m_iMapHeight;
	int m_iSmallMapX;
	int m_iSmallMapY;
	int m_iSmallMapWidth;
	int m_iSmallMapHeight;

	CHudScope m_Scope;
	CHudDodIcons m_Icons;
	CObjectiveIcons m_ObjectiveIcons;
	CParticleShooter m_PShooter;
	CClientEnvModel m_CEnvModel;
	CWeatherManager m_Weather;
	CHudAmmo m_Ammo;
	CHudSayText m_SayText;
	CHudSpectator m_Spectator;
	CHudTrain m_Train;
	CHudMessage m_Message;
	CHudStatusBar m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudMenu m_Menu;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudDoDCrossHair m_DoDCrossHair;
	CHudDoDMap m_DoDMap;
	CHudDoDCommon m_DoDCommon;
	CHudVGUI2Print m_VGUI2Print;
	CMortarHud m_MortarHud;

	void Init( void );
	void VidInit( void );
	void Think( void );
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount( 0 ), m_pHudList( NULL ) {}  
	~CHud();

	int _cdecl MsgFunc_Damage( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Logo( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_ResetHUD( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_YouDied( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_ZoomStatus( const char *pszName,  int iSize, void *pbuf );
	void _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_HLTV( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_RoundState( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_TimeLeft( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_UseSound( const char *pszName,  int iSize, void *pbuf );

	SCREENINFO m_scrinfo;

	int m_iWeaponBits;
	int m_fPlayerDead;
	int m_iIntermission;
	int g_iClip;

	bool m_bAutoReloadComplete;

	int g_pModel;
	float g_NextAttack;

	int m_HUD_number_0;
	int m_MG_number_0;

	float i_MusicFadeCounter;
	float i_MusicFadeCounter2;
	int i_MusicPlay;
	int i_specmenutoggle;
	int i_Recoil;
	float i_TempFSpeed;
	float i_TempBSpeed;
	float i_TempSSpeed;

	int m_iWaterLevel;
	int m_iClipSize;

	float m_flPlaySprintSoundTime;

	int m_iMinimapState;

	float m_fRoundEndsTime;
	float m_fMortarDeployTime;
	float m_fMortarUnDeployTime;

	cvar_t *_cl_minimap;
	cvar_t *_cl_minimapzoom;
	cvar_t *zoom_sensitivity_ratio;
	cvar_t *hud_takesshots;
	cvar_t *r_drawentities;
	cvar_t *cl_lw;
	cvar_t *cl_pitchup;
	cvar_t *cl_pitchdown;
	cvar_t *crosshair;
	cvar_t *max_rubble;
	cvar_t *_ah;
	cvar_t *cl_corpsestay;
	cvar_t *developer;
	cvar_t *cl_hudfont;
	cvar_t *hud_fastswitch;

	float m_flPitchRecoilAccumulator;
	float m_flYawRecoilAccumulator;
	float m_flRecoilTimeRemaining;

	void AddHudElem( CHudBase *p );
	float GetSensitivity( void );
	void SetWaterLevel( int level );
	int GetWaterLevel( void );
	int GetCurrentWeaponId( void );
	char *GetTeamName( int );
	int GetMinimapState( void );
	void SetMinimapState( int state );
	void PlaySoundOnChan( char *name, float fVol, int chan );
	void InitMapBounds( void );
	void GetMapBounds( const int *x, const int *y, const int *w, const int *h );

	bool IsInMGDeploy( void );
	bool IsProneDeployed( void );
	bool IsSandbagDeployed( void );
	bool IsProne( void );
	bool IsDucking( void );
	bool IsInMortarDeploy( void );

	void SetMortarDeployTime( void );
	float GetMortarDeployTime( void );
	void SetMortarUnDeployTime( void );
	float GetMortarUnDeployTime( void );

	bool IsTeamPara( int team );

	char *GetPlayerClassName( int playerclass );

	void DoRecoil( int weapon_id );
	void GetWeaponRecoilAmount( int weaponId, const float *flPitchRecoil, const float *flYawRecoil );
	void SetRecoilAmount( float flPitchRecoil, float flYawRecoil );
	void PopRecoil( float frametime, const float *flPitchRecoil, const float *flYawRecoil );
};

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
#endif
