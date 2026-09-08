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
#include "r_efx.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	
#define	HUDELEM_ACTIVE	1

#define CHudMsgFunc(x) int MsgFunc_##x(const char *pszName, int iSize, void *buf)
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
/*class Element
{
public:
	float flTimeCreated;
	Element *next;
	Element *previous;

	Element( float flTime );
};

class Queue
{
public:
	int count;
	int maxelemets;
	float m_flDuration;
	Element *first;
	Element *last;
	Element *current;

	~Queue( void )
	{
		if( last )
		{
			while( last->flTimeCreated + m_flDuration < 99999.0f )
			{
				delete[] last;
				--count;
				last = last->previous;
				current = last->previous;

				if( !last->previous )
				{
					first = NULL;
					current = NULL;
					return;
				}

				last = last->previous;
			}
		}
	}

	bool Full( void );
	bool Add( float flDuration );
	void Update( float flDuration );
};*/

struct pmodel_fx_t
{
	bool bSwitch;
	int iSwitchSeq;
	int iSwitchFrame;
	bool bAnim;
	int iAnimSeq;
	int iAnimFrame;
	int iAnimTargetSeq;
};

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	POSITION  m_pos;
	int   m_type;
	int	  m_iFlags; // active, moving, 
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
//
#include "voice_status.h"
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

	ClipInfo ClipInfoArray[64];
	ClipInfo BritGrenClipInfo;

	int m_HUD_bucket0;
	int m_HUD_selection;
};

//
//-----------------------------------------------------
//
class CHudAmmoSecondary : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);

	CHudMsgFunc( SecAmmoVal );
	CHudMsgFunc( SecAmmoIcon );

private:
	enum {
		MAX_SEC_AMMO_VALUES = 4
	};

	int m_HUD_ammoicon; // sprite indices
	int m_iAmmoAmounts[MAX_SEC_AMMO_VALUES];
	float m_fFade;
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
class CHudDoDCrossHair : CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	//void SetDoDCrosshair( void );
	int Draw( float flTime );
	void Reset( void ) { return; }
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

//
//-----------------------------------------------------
//
class CHudDoDMap : public CHudBase
{
public:
	int VidInit( void );
	void InitHUDData( void ) { return; }
	int Draw( float flTime );
	int Init( void );
	void DrawOverview( void );
	void DrawOverviewLayer( void );
	void DrawOverviewEntities( void );
	void CheckOverviewEntities( void );
	// void CreateMapSprite( float, int, int, int, int, const class Vector &, const class Vector &, class Vector & );
	void HandleMapButton( void );
	void HandleMapZoomButton( void );
	void SetMapState( int mapstate );
	void GetSmallMapOffset( float &x, float &y, float &z );
	bool AddMapEntityToMap( HSPRITE sprite, double lifetime, vec3_t *origin );
	void DrawOverviewIcon( vec3_t *origin, HSPRITE hIcon, int iconScale, vec3_t *angles );

	typedef struct
	{
		vec3_t origin;
		HSPRITE hSprite;
		double killtime;
	} map_icon_t;

	map_icon_t m_ExtraOverviewEntities[32];

private:
	cl_entity_t m_MapTag[64];

	float m_flZoom;
	float m_flZoomTime;
	float m_flSmallMapScale;
	float m_flIdealMapScale;
	float m_flPreviousMapScale;
};

//
//-----------------------------------------------------
//
class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	CHudMsgFunc( MOTD );
	void Scroll( int dir );
	void Scroll( float amount );
	float scroll;
	bool m_bShow;

protected:
	static int MOTD_DISPLAY_TIME;
	char m_szMOTD[MAX_MOTD_LENGTH];

	int m_iLines;
	int m_iMaxLength;
};

//
//-----------------------------------------------------
//
class CHudScoreboard : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawPlayers( int xoffset, float listslot, int nameoffset = 0, const char *team = NULL ); // returns the ypos where it finishes drawing
	void UserCmd_ShowScores( void );
	void UserCmd_HideScores( void );
	CHudMsgFunc( ScoreInfo );
	CHudMsgFunc( TeamInfo );
	CHudMsgFunc( TeamScore );
	CHudMsgFunc( TeamScores );
	CHudMsgFunc( TeamNames );
	void DeathMsg( int killer, int victim );

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;

	void GetAllPlayersInfo( void );
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
	CHudMsgFunc( StatusValue );
	//int GetTargetHealth( void );
	//int GetTargetIndex( void );
	//int GetTargetTeam( void );
	char *GetTargetName( void );
	void CreateEntities( void );
	void DrawEntitiesOverTeam( void );
	void DrawEntitiesOverTarget( void );
	// void UpdateWhosTalking( int, qboolean );
	bool InDeathCamMode( void );
	void Think( void );
	//qboolean IsEntityTalking( int );

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
	cl_entity_t m_TargetHeadModel[64];
	int m_TargetTalking[64];

	HSPRITE m_StatusBarAllieModel;
	HSPRITE m_StatusBarBritishModel;
	HSPRITE m_StatusBarGermanModel;
	HSPRITE m_StatusBarHeadModel;

	cvar_t *hud_centerid;

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];
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
	// void ShowMenu_Votekick( int );
	// void ShowMenu_ListPlayers( int );
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
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1, char *sstr1 = '\0', 
		char *sstr2 = '\0', char *sstr3 = '\0', char *sstr4 = '\0'  );
	int GetTextPrintY( void );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
	friend class CHudSpectator;

	struct cvar_s *m_HUD_saytext;

private:
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
	wrect_t m_iconarea;
	HSPRITE TimerHUD;
	wrect_t *TimerHUDArea;

	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Think( void );

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
	// void PlayersInArea( int, int );
	void SetNumPlayersInArea( int point, int team, int numlayers, int required );
	void SetVisible( int point, int visible );
	void UpdateObjectiveIcons( void );
	float GetObjectiveTime( void );
	float GetWaveTime( void );
	// void SetWaveTime( float );
	// void SetWaveStatus( int );
	// int GetWaveStatus( void );
	void CalcIconLocations( void );
	// void SetWarmupMode( bool );
	// bool IsPointValid( int );
	void DrawDigit( int digit, int x, int y );

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
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	CHudMsgFunc( HudText );
	CHudMsgFunc( GameTitle );

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time, int hintMessage, unsigned int font );
	void MessageAdd( client_textmessage_t *newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );
	void HintMessageAdd( const char *pText );
	// vgui2::HFont GetFont( void );

	typedef struct
	{
		client_textmessage_t *pMessage;
		unsigned int font;
	} client_message_t;

private:
	client_message_t			m_pMessages[maxHUDMessages];
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
	CHudMsgFunc( Scope );
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
	CHudMsgFunc( GameRules );
	CHudMsgFunc( ResetSens );
	//int MsgFunc_ParaLand( const char *pszName, int iSize, void *pbuf );
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
	CHudMsgFunc( Health );
	CHudMsgFunc( Object );
	CHudMsgFunc( ClientAreas );
	CHudMsgFunc( ClCorpse );
	int Draw( float flTime );
	void MapMarkerPosted( void );
	void ActivateHintBacking( int team, float flTime );
	void GetHintMessageLocation( int &x, int &y );
	void StartDrawingCredits( void );
	void DrawCredits( float flTime );
	void DrawMarkerIcon( HSPRITE pSpr );
	//void DrawObjectiveTimer( void );
	//void UpdateReinforcementTimer( void );

	int m_iHealth;
	char *m_szObjectIcon;
	HSPRITE m_hObjectSpr;
	wrect_t *m_wObjectArea;
	int sprite_array[5];
	wrect_t *area_array;
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
} particle_shooter_t;

class CParticleShooter : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );
	void AddParticleSystem( particle_shooter_t *pShooter );
	//int MsgFunc_PReg( const char *pszName, int iSize, void *pbuf );
	CHudMsgFunc( PShoot );

private:
	particle_shooter_s m_sShooters[64];
	int m_iNumShooters;
};

//
//-----------------------------------------------------
//
struct env_model_t
{
	char szModel[64];
	char szSequence[64];
	int iBody;
	vec3_t vecOrigin, vecAngles;
	int spawnflags;
	float frame;
	int sequence;
	int iModel;
	model_s *pModel;
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
	env_model_t *GetModel( int iModel );

private:
	TEMPENTITY *m_teEnvModelTE;
	env_model_t m_sEnvModels[192];
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
	//void SetRainSprite( model_s *pModel );
	//void SetSnowSprite( model_s *pModel );
	//void SetSplashSprite( model_s *pModel );
	//void SetRippleSprite( model_s *pModel );
	//model_s *GetRainSprite( void );
	//model_s *GetSnowSprite( void );
	//model_s *GetSplashSprite( void );
	//model_s *GetRippleSprite( void );

private:
	model_s *m_pRainSprite;
	model_s *m_pSnowSprite;
	model_s *m_pSplashSprite;
	model_s *m_pRippleSprite;
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
	// vgui2::HFont GetFont( void );

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
struct trajectory_t
{
	float vTargetPos[3];
	float fPitch1;
	float fPitch2;
	float fYaw;
	float fLastUsedTime;
};

class CTrajectoryList
{
public:
	CTrajectoryList( void );
	~CTrajectoryList( void );
	void GetTrajectory( vec3_t *launchPos, vec3_t *targetPos, float *pitch1, float *pitch2, float *yaw );
	void CalculateTrajectory( vec3_t *launchPos, vec3_t *targetPos, float *pitch1, float *pitch2, float *yaw );
	trajectory_t *AddTrajectory( vec3_t *p_targetPos );
	void InvalidateAllTrajectories( void );

	enum
	{
		MAX_TRAJECTORIES = 32,
		TRAJECTORY_LIFETIME = 30
	};

private:
	vec3_t m_vecLaunchPos;
	trajectory_t m_Trajectories[32];
};

class CMortarHud : public CHudBase
{
public:
	CMortarHud( void );
	~CMortarHud( void );
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void CalculateFireAngle( float *pitch, float *yaw );
	void DrawPredictedMortarImpactSite( void );

	CTrajectoryList *m_TrajectoryList;
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
	int							m_iConcussionEffect; 

public:
	HSPRITE						m_hsprCursor;
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	Vector	m_vecVelocity;
	float	m_flMouseSensitivity;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	int		m_PlayerFOV[64];
	int		m_Teamplay;
	int		m_iRes;
	int		m_iMaxRes;
	int		m_iHudNumbersYOffset;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;
	cvar_t  *m_pAllowHD;

	bool	m_bAllieParatrooper;
	bool	m_bAllieInfiniteLives;
	bool	m_bAxisParatrooper;
	bool	m_bAxisInfiniteLives;
	bool	m_bParatrooper;
	bool	m_bInfiniteLives;
	bool	m_bBritish;

	int		m_iRoundState;
	int		m_iSensLevel;
	int		m_iFontHeight;
	int		m_iFontEngineHeight;
	char	m_szTeamNames[5][32];

	int		m_iMapX;
	int		m_iMapY;
	int		m_iMapWidth;
	int		m_iMapHeight;
	int		m_iSmallMapX;
	int		m_iSmallMapY;
	int		m_iSmallMapWidth;
	int		m_iSmallMapHeight;

	int DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth( int iNumber, int iFlags );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );
	void PostMortarValue( float value );
	int GetMinimapZoomLevel( void );
	int ZoomMinimap( void );
	int DrawHudStringLen( const char *szIt );
	void DrawDarkRectangle( int x, int y, int wide, int tall );

	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	struct cvar_s *default_fov;
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

	CHudScope		m_Scope;
	CHudDodIcons	m_Icons;
	CObjectiveIcons	m_ObjectiveIcons;
	CParticleShooter	m_PShooter;
	CClientEnvModel m_CEnvModel;
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
	CHudStatusIcons	m_StatusIcons;
	CHudDoDCrossHair	m_DoDCrossHair;
	CHudDoDMap		m_DoDMap;
	CHudDoDCommon	m_DoDCommon;
	CHudVGUI2Print	m_VGUI2Print;
	CMortarHud		m_MortarHud;
	CHudAmmoSecondary	m_AmmoSecondary;

	CHudScoreboard	m_Scoreboard;
	CHudMOTD	m_MOTD;

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}  
	~CHud();			// destructor, frees allocated memory

	// user messages

	CHudMsgFunc( Damage );
	CHudMsgFunc( GameMode );
	CHudMsgFunc( Logo );
	CHudMsgFunc( ResetHUD );
	CHudMsgFunc( InitHUD );
	CHudMsgFunc( ViewMode );
	CHudMsgFunc( SetFOV );
	CHudMsgFunc( Concuss );

	CHudMsgFunc( YouDied );
	CHudMsgFunc( HLTV );
	CHudMsgFunc( RoundState );
	CHudMsgFunc( Timeleft );
	CHudMsgFunc( UseSound );

	//int _cdecl MsgFunc_ZoomStatus( const char *pszName, int iSize, void *pbuf );

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

	int m_iNoConsolePrint;

	void AddHudElem( CHudBase *p );
	// void SetFOV( int, int );
	float GetSensitivity( void );
	void SetWaterLevel( int level );
	int GetWaterLevel( void );
	int GetCurrentWeaponId( void );
	int GetClipSize( void );
	char *GetTeamName( int team );
	int GetMinimapState( void );
	void SetMinimapState( int state );
	void PlaySoundOnChan( char *name, float fVol, int chan );
	void InitMapBounds( void );
	void GetMapBounds( int &x, int &y, int &w, int &h );
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
	// void SetFOV( int );
	int GetFOV( void );
	void DoRecoil( int weapon_id );
	void GetWeaponRecoilAmount( int weaponId, float &flPitchRecoil, float &flYawRecoil );
	void SetRecoilAmount( float flPitchRecoil, float flYawRecoil );
	void PopRecoil( float frametime, float &flPitchRecoil, float &flYawRecoil );

	void GetAllPlayersInfo( void );
};

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
extern int g_iVuser1x, g_iVuser1z;
#endif
