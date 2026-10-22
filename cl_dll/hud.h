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
#include "cvardef.h"
#include "const.h"
#include "com_model.h"
#include "cl_entity.h"
#include "r_efx.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	

#define		HUDELEM_ACTIVE	1

// Declarations of the user message handlers and user commands of the HUD classes
#define CHudMsgFunc(x) int MsgFunc_##x(const char *pszName, int iSize, void *pbuf)
#define CHudMsgFuncV(x) void MsgFunc_##x(const char *pszName, int iSize, void *pbuf)
#define CHudUserCmd(x) void UserCmd_##x()

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
	POSITION  m_pos;
	int   m_type;
	int	  m_iFlags; // active, moving, 
	int	  m_iSafeAnchor; // the edges of the screen that the element is attached to (SAFE_*, hud_safearea.h)
	virtual		~CHudBase() {}
	virtual int Init( void ) { return 0; }
	virtual int VidInit( void ) { return 0; }
	virtual int Draw( float flTime ) { return 0; }
	virtual void Think( void ) { return; }
	virtual void Reset( void ) { return; }
	virtual void PlayerDied( void ) { return; }
	virtual void InitHUDData( void ) {}		// called every time a server is connected to
};

struct HUDLIST
{
	CHudBase	*p;
	HUDLIST		*pNext;
};

//
//-----------------------------------------------------
#include "voice_status.h" // base voice handling class
#include "hud_spectator.h"

//
//-----------------------------------------------------
//
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
	CHudMsgFunc( CurWeapon );
	CHudMsgFunc( WeaponList );
	CHudMsgFunc( AmmoX );
	CHudMsgFunc( AmmoShort );
	CHudMsgFunc( AmmoPickup );
	CHudMsgFunc( WeapPickup );
	CHudMsgFunc( ItemPickup );
	CHudMsgFunc( HideWeapon );
	CHudMsgFunc( ReloadDone );

	void SlotInput( int iSlot );
	CHudUserCmd( Slot1 );
	CHudUserCmd( Slot2 );
	CHudUserCmd( Slot3 );
	CHudUserCmd( Slot4 );
	CHudUserCmd( Slot5 );
	CHudUserCmd( Slot6 );
	CHudUserCmd( Slot7 );
	CHudUserCmd( Slot8 );
	CHudUserCmd( Slot9 );
	CHudUserCmd( Slot10 );
	CHudUserCmd( Close );
	CHudUserCmd( NextWeapon );
	CHudUserCmd( PrevWeapon );

	ClipInfo *GetCurrentGun( WEAPON *pw );
	int GetCurrentWeaponId( void );

private:
	float m_fFade;
	RGBA  m_rgba;
	WEAPON *m_pWeapon;
	int m_HUD_clip;
	int m_HUD_clipempty;
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

	ClipInfo ClipInfoArray[MAX_WEAPONS];
	ClipInfo BritGrenClipInfo;
};


#define FADE_TIME 100


//
//-----------------------------------------------------
//
class CHudTrain : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	CHudMsgFunc( Train );

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
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	virtual void Reset( void );
	CHudMsgFunc( ClanTimer );

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

#define MAX_EXTRA_MAP_ICONS 32

class CHudDoDMap : public CHudBase
{
public:
	typedef struct
	{
		vec3_t origin;
		HSPRITE hSprite;
		double killTime;
	} map_icon_t;

	virtual int VidInit( void );
	virtual void InitHUDData( void );
	virtual int Draw( float flTime );
	virtual int Init( void );
	void DrawOverview( void );
	void DrawOverviewLayer( void );
	void DrawOverviewEntities( void );
	void CheckOverviewEntities( void );
	void HandleMapButton( void );
	void HandleMapZoomButton( void );
	void SetMapState( int mapstate );
	void GetSmallMapOffset( float &x, float &y, float &z );
	bool AddMapEntityToMap( HSPRITE sprite, double lifeTime, Vector origin );
	void DrawOverviewIcon( Vector origin, HSPRITE hIcon, int iconScale, Vector angles );

	map_icon_t m_ExtraOverviewEntities[MAX_EXTRA_MAP_ICONS];

private:
	float m_flZoom;
	float m_flZoomTime;
	float m_flSmallMapScale;
	float m_flIdealMapScale;
	float m_flPreviousMapScale;
};

class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	bool HandleMOTDMessage( const char *pszName, int iSize, void *pbuf );
	void Scroll( int dir );
	void Scroll( float amount );
	float scroll;
	bool m_bShow;

	char m_szMOTD[MAX_MOTD_LENGTH];
protected:
	static int MOTD_DISPLAY_TIME;

	int m_iLines;
	int m_iMaxLength;
};

class CHudScoreboard : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawPlayers( int xoffset, float listslot, int nameoffset = 0, const char *team = NULL ); // returns the ypos where it finishes drawing
	CHudUserCmd( ShowScores );
	CHudUserCmd( HideScores );
	CHudMsgFunc( ScoreInfo );
	CHudMsgFunc( TeamInfo );
	CHudMsgFunc( TeamScore );
	CHudMsgFunc( TeamScores );
	CHudMsgFunc( TeamNames );
	void DeathMsg( int killer, int victim );
	void RebuildTeams();
	void UpdateTeams();
	int BestTeam();

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;
};

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
	enum
	{
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2
	};

public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	virtual void Reset( void );
	CHudMsgFunc( StatusValue );

	int GetTargetHealth( void ) { return m_iHealth; }
	int GetTargetIndex( void ) { return m_iTargetIndex; }
	int GetTargetTeam( void ) { return m_iTargetTeam; }
	char *GetTargetName( void );

	void CreateEntities( void );
	void DrawEntitiesOverTeam( void );
	void DrawEntitiesOverTarget( void );
	void UpdateWhosTalking( int entindex, qboolean bTalking );
	bool InDeathCamMode( void );
	virtual void Think( void );

	qboolean IsEntityTalking( int index )
	{
		if( (unsigned int)( index - 1 ) >= MAX_PLAYERS )
			return 0;

		return m_TargetTalking[index];
	}

protected:
	float m_flNextUpdateTime;
	int m_iTargetTeam;
	int m_iTargetIndex;
	int m_iHealth;

	// the entities are indexed by the number of the player (1..64)
	cl_entity_t m_TargetHeadModel[MAX_PLAYERS + 1];
	int m_TargetTalking[MAX_PLAYERS + 1];

	HSPRITE m_StatusBarAllieModel;
	HSPRITE m_StatusBarBritishModel;
	HSPRITE m_StatusBarGermanModel;
	HSPRITE m_StatusBarHeadModel;

	cvar_t *hud_centerid;
};

struct extra_player_info_t
{
	short frags;
	short objscore;
	short deaths;
	short playerclass;
	short teamnumber;
	int status;
	char teamname[MAX_TEAM_NAME];
	float showhealth;
	int health;
	int teamId;
	bool dead;
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

extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS + 1];	   // player info from the engine
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];   // additional player info sent directly to the client dll

// the animation events of the player models (7000 switches the sequence at a frame, 7002 plays another sequence in the middle)
typedef struct pmodel_fx_s
{
	bool bSwitch;
	int iSwitchSeq;
	int iSwitchFrame;
	bool bAnim;
	int iAnimSeq;
	int iAnimFrame;
	int iAnimTargetSeq;
} pmodel_fx_t;

extern pmodel_fx_t			g_PModelFxInfo[MAX_PLAYERS + 1];
extern team_info_t			g_TeamInfo[MAX_TEAMS + 1];
extern int					g_IsSpectator[MAX_PLAYERS + 1];

//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	virtual int Init( void );
	virtual void InitHUDData( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	CHudMsgFunc( DeathMsg );

private:
	int m_HUD_d_skull;  // sprite index of skull icon
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
	CHudMsgFunc( ShowMenu );

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
	CHudMsgFunc( SayText );
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
	int GetTextPrintY( void );
	friend class CHudSpectator;

	struct cvar_s *	m_HUD_saytext;

private:
	struct cvar_s *	m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
#define MAX_CONTROL_POINTS 12

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
	wrect_t m_rAreas[3];
	vec3_t m_rOrigin;
	int m_iMapXPos;
	int m_iMapYPos;
};

class CObjectiveIcons : public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	virtual void Think( void );
	CHudMsgFunc( InitObj );
	CHudMsgFunc( SetObj );
	CHudMsgFunc( StartProg );
	CHudMsgFunc( StartProgF );
	CHudMsgFunc( ProgUpdate );
	CHudMsgFunc( CancelProg );
	CHudMsgFunc( TimerStatus );
	CHudMsgFunc( PlayersIn );
	void ClearAllCapPoints( void );
	void ChangeCapPoint( int point, int newowner, int timedCap );
	void StartCapProgress( int point, int newOwner, float time );
	void CancelCapProgress( int point, int owner );
	void SetNumPlayersInArea( int point, int team, int num, int required );
	void SetVisible( int point, int visible );
	void UpdateObjectiveIcons( void );
	float GetObjectiveTime( void ) { return m_fTimerSeconds; }
	float GetWaveTime( void ) { return m_flWaveTime; }
	void SetWaveTime( float time ) { m_flWaveTime = time; }
	void SetWaveStatus( int status ) { m_iWaveStatus = status; }
	int GetWaveStatus( void ) { return m_iWaveStatus; }
	void CalcIconLocations( void );
	void SetWarmupMode( bool bWarmup ) { m_bWarmupMode = bWarmup; }
	bool IsPointValid( int point ) { return m_eControlPoints[point].valid; }
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
	control_point_t m_eControlPoints[MAX_CONTROL_POINTS];
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
	CHudMsgFunc( TextMsg );
	CHudMsgFunc( CapMsg );
};

//
//-----------------------------------------------------
//

class CHudMessage : public CHudBase
{
public:
	typedef struct
	{
		client_textmessage_t	*pMessage;
		unsigned int			font;
	} client_message_t;

	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	CHudMsgFunc( HudText );
	CHudMsgFunc( GameTitle );

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time, int hintMessage, unsigned int font );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time, unsigned int font );
	void MessageScanStart( void );
	void MessageScanNextChar( unsigned int font );
	void Reset( void );
	void HintMessageAdd( const char *pText );
	unsigned long GetFont( void );

private:
	client_message_t			m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
	unsigned long m_Fonts[3];	// the VGUI2 fonts of the original, there is only the font of the engine here
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH	24

//
//-----------------------------------------------------
//
class CHudScope : public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	virtual void Think( void );
	CHudMsgFunc( Scope );
	virtual void Reset( void );
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
	virtual int Init( void );
	virtual void InitHUDData( void );
	CHudMsgFunc( GameRules );
	CHudMsgFunc( ParaLand );
	CHudMsgFunc( ResetSens );
	virtual int Draw( float flTime );
	virtual int VidInit( void );
	CHudMsgFunc( CameraView );
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
	virtual int Init( void );
	virtual int VidInit( void );
	virtual void Reset( void );
	virtual void PlayerDied( void );
	CHudMsgFunc( Health );
	CHudMsgFunc( Object );
	CHudMsgFunc( ClientAreas );
	CHudMsgFunc( ClCorpse );
	virtual int Draw( float flTime );
	void MapMarkerPosted( void );
	void ActivateHintBacking( int team, float flTime );
	void GetHintMessageLocation( int &x, int &y );
	void StartDrawingCredits( void );
	void DrawCredits( float flTime );
	void DrawMarkerIcon( HSPRITE pSpr );

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

class CTrajectoryList;

class CMortarHud : public CHudBase
{
public:
	CMortarHud( void );
	virtual ~CMortarHud( void );
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw( float flTime );
	void CalculateFireAngle( float *pitch, float *yaw );
	void DrawPredictedMortarImpactSite( void );

	class CTrajectoryList *m_TrajectoryList;
};

//
//-----------------------------------------------------
//
#define MAX_SHOOTERS 64

// the particle_shooter entities of the map
typedef struct particle_shooter_s
{
	int id;
	int iNumParticles;
	int iParticlesRemaining;
	float fParticleLife;
	float fFireDelay;
	int iFlags;
	vec3_t vOrigin;
	vec3_t vVelocity;
	float fVariance;
	float fNextShootTime;
	int iSprite;
	char szSprite[128];
	model_t *pSprite;
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
} particle_shooter_t;

class CParticleShooter : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );

	void AddParticleSystem( particle_shooter_t *pShooter );
	CHudMsgFunc( PShoot );

private:
	particle_shooter_t m_sShooters[MAX_SHOOTERS];
	int m_iNumShooters;
};

//
//-----------------------------------------------------
//
#define MAX_ENV_MODELS 192

#define SF_ENVMODEL_OFF 1
#define SF_ENVMODEL_DROPTOFLOOR 2
#define SF_ENVMODEL_ANIMATION 0x10

struct env_model_t
{
	char szModel[64];
	char szSequence[64];
	int iBody;
	vec3_t vecOrigin;
	vec3_t vecAngles;
	int spawnflags;
	float frame;
	int sequence;
	int iModel;
	model_t *pModel;
};

class CClientEnvModel : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );

	void RemoveAllModels( void );
	void AddEnvModel( env_model_t *pModel );

	int GetNumModels( void )
	{
		return m_iNumEnvModels;
	}

	env_model_t *GetModel( int i )
	{
		return &m_sEnvModels[i];
	}

private:
	TEMPENTITY *m_teEnvModelTE;
	env_model_t m_sEnvModels[MAX_ENV_MODELS];
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

	void SetRainSprite( model_t *pRain ) { m_pRainSprite = pRain; }
	void SetSnowSprite( model_t *pSnow ) { m_pSnowSprite = pSnow; }
	void SetSplashSprite( model_t *pSplash ) { m_pSplashSprite = pSplash; }
	void SetRippleSprite( model_t *pRipple ) { m_pRippleSprite = pRipple; }

	model_t *GetRainSprite( void ) { return m_pRainSprite; }
	model_t *GetSnowSprite( void ) { return m_pSnowSprite; }
	model_t *GetSplashSprite( void ) { return m_pSplashSprite; }
	model_t *GetRippleSprite( void ) { return m_pRippleSprite; }

private:
	model_t *m_pRainSprite;
	model_t *m_pSnowSprite;
	model_t *m_pSplashSprite;
	model_t *m_pRippleSprite;
};

//
//-----------------------------------------------------
//
class CHudStatusIcons : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	CHudMsgFunc( StatusIcon );

	enum
	{
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4
	};
	
	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
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
// The original draws its localized text with VGUI2 fonts (wchar_t); here the text goes through
// DoD_FindLocalized() and the engine console font
#define VGUI2_CHAR_BUFFER 2048

class CHudVGUI2Print : public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );

	int DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b );
	int DrawLocalizedString( const char *msg, int x, int y, float r, float g, float b ); // wchar_t version of the original
	int DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b );
	int DrawLocalizedStringReverse( const char *msg, int x, int y, float r, float g, float b ); // wchar_t version of the original
	void VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );
	int GetHudFontHeight( void );
	void GetStringSize( const char *string, int *width, int *height );
	virtual int Draw( float flTime );

private:
	float m_flVGUI2StringTime;
	char m_szBuf[VGUI2_CHAR_BUFFER];
	float m_fR;
	float m_fG;
	float m_fB;
	int m_iX;
	int m_iY;
};

//
//-----------------------------------------------------
//
class CHud
{
private:
	HUDLIST						*m_pHudList;
	HSPRITE						m_hsprLogo;
	int							m_iLogo;
	client_sprite_t				*m_pSpriteList;
	int							m_iSpriteCount;
	int							m_iSpriteCountAllRes;

public:
	HSPRITE						m_hsprCursor;
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	vec3_t	m_vecOrigin;
	vec3_t	m_vecAngles;
	vec3_t	m_vecVelocity;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_Teamplay;
	int		m_iRes;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;

	float						m_flMouseSensitivity;
	int		m_PlayerFOV[MAX_PLAYERS];

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
	void SetSensitivity( int level ) { m_iSensLevel = level; }
	int DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth( int iNumber, int iFlags );
	int DrawHudStringLen( const char *szIt );
	void DrawDarkRectangle( int x, int y, int wide, int tall );

	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );

	int GetMinimapZoomLevel( void );
	int ZoomMinimap( void );

private:
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

public:
	HSPRITE GetSprite( int index )
	{
		return ( index < 0 ) ? 0 : m_rghSprites[index];
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects[index];
	}

	inline bool IsHL25( void )
	{
		// a1ba: only HL25 have higher resolution HUD spritesheets
		// and only accept HUD style changes if user has allowed HD sprites
		return m_iMaxRes > 640 && m_pAllowHD->value;
	}

	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rghSprites[] array

	char	m_szTeamNames[5][MAX_TEAMNAME_SIZE];
	int		m_iFOV;

	// minimap bounds
	int		m_iMapX;
	int		m_iMapY;
	int		m_iMapWidth;
	int		m_iMapHeight;
	int		m_iSmallMapX;
	int		m_iSmallMapY;
	int		m_iSmallMapWidth;
	int		m_iSmallMapHeight;

	// the size of the screen of the HUD that the bounds were made for
	int		m_iMapBoundsWidth;
	int		m_iMapBoundsHeight;

	CHudScope		m_Scope;
	CHudDoDCommon	m_DoDCommon;
	CHudDodIcons	m_Icons;
	CHudDoDMap		m_DoDMap;
	CObjectiveIcons	m_ObjectiveIcons;
	CParticleShooter	m_PShooter;
	CClientEnvModel	m_CEnvModel;
	CWeatherManager	m_Weather;
	CHudAmmo		m_Ammo;
	CHudSayText		m_SayText;
	CHudSpectator	m_Spectator;
	CHudTrain		m_Train;
	CHudMessage		m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudMenu		m_Menu;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudDoDCrossHair	m_DoDCrossHair;
	CMortarHud		m_MortarHud;
	CHudVGUI2Print	m_VGUI2Print;

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;
	int g_iClip;
	bool m_bAutoReloadComplete;
	int g_pModel;
	float g_NextAttack;

	// sprite indexes
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

	void Init( void );
	void VidInit( void );
	void Think( void );
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}
	~CHud();			// destructor, frees allocated memory

	// user messages
	CHudMsgFunc( Damage );
	CHudMsgFunc( Logo );
	CHudMsgFunc( ResetHUD );
	CHudMsgFunc( YouDied );
	CHudMsgFuncV( InitHUD );
	CHudMsgFunc( SetFOV );
	CHudMsgFunc( HLTV );
	CHudMsgFunc( RoundState );
	CHudMsgFunc( TimeLeft );
	CHudMsgFunc( UseSound );

	void AddHudElem( CHudBase *p );

	float GetSensitivity( void );

	void SetWaterLevel( int level );
	int GetWaterLevel( void );

	int GetCurrentWeaponId( void );

	char *GetTeamName( int team );

	int GetMinimapState( void );
	void SetMinimapState( int state );

	void PlaySoundOnChan( char *name, float fVol, int chan );

	void InitMapBounds( void );
	void GetMapBounds( int &x, int &y, int &w, int &h );
	void GetMapViewBounds( int &x, int &y, int &w, int &h );

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
	void PostMortarValue( float value );

	bool IsTeamPara( int team );
	char *GetPlayerClassName( int playerclass );

	void SetFOV( int fov ) { m_iFOV = fov; }
	int GetFOV( void ) { return m_iFOV; }

	void DoRecoil( int weapon_id );
	void GetWeaponRecoilAmount( int weaponId, float &flPitchRecoil, float &flYawRecoil );
	void SetRecoilAmount( float flPitchRecoil, float flYawRecoil );
	void PopRecoil( float frametime, float &flPitchRecoil, float &flYawRecoil );

	// hlsdk-portable leftovers (freevgui scoreboard/MOTD, HL-only HUD elements, HD HUD)
	int		m_iMaxRes;
	int		m_iHudNumbersYOffset;
	cvar_t  *m_pAllowHD;
	cvar_t  *m_pCvarMOTDVGUI;
	cvar_t  *m_pCvarScoreboardVGUI;
	int m_iNoConsolePrint;

	CHudScoreboard	m_Scoreboard;
	CHudMOTD	m_MOTD;

	void GetAllPlayersInfo( void );

	bool UseVguiMOTD();
	bool UseVguiScoreBoard();
};

class Element
{
public:
	float flTimeCreated;
	Element *next;
	Element *previous;

	Element( float flTime )
	{
		flTimeCreated = flTime;
		next = NULL;
		previous = NULL;
	}
};

class Queue
{
public:
	int count;
	int maxelements;
	float m_flDuration;
	Element *first;
	Element *last;
	Element *current;

	Queue()
	{
		count = 0;
		maxelements = 60;
		m_flDuration = 5.0f;
		first = NULL;
		last = NULL;
		current = NULL;
	}

	~Queue()
	{
		Update( 99999.0f );
	}

	bool Full( void )
	{
		return count >= maxelements;
	}

	bool Add( float flTime )
	{
		if( Full() )
			return false;

		if( first )
		{
			current->next = new Element( flTime );
			current->next->previous = current;
			current = current->next;
			last = current;
		}
		else
		{
			first = new Element( flTime );
			first->next = NULL;
			first->previous = NULL;
			last = first;
			current = first;
		}

		count++;
		return true;
	}

	void Update( float flCurrentTime )
	{
		Element *temp;

		if( !last )
			return;

		while( flCurrentTime > last->flTimeCreated + m_flDuration )
		{
			temp = last->previous;
			delete last;
			count--;
			last = temp;
			current = temp;

			if( !temp )
			{
				first = NULL;
				current = NULL;
				break;
			}
		}
	}
};

extern Queue g_RubbleQueue;

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
extern int g_iVuser1x;
extern int g_iVuser1z;
extern int g_iMovetype;
extern int g_iOnlyClientDraw;
extern int g_iEffects;
extern float g_fStamina;
extern int g_iWeaponBits2;
extern int g_iWeaponFlags;
extern int g_iAlive;
extern int g_iDeadFlag;
extern float flBoltHideXHair;
extern float g_flWeaponHeat;
extern cvar_t *cl_autoreload;

bool ShowHudElement( int i_hudElement );

void ClientSetSensitivity( int level );
float DoDGunGoOnOffScreen( float v_modeloffscreen, struct ref_params_s *pparams );
#endif
