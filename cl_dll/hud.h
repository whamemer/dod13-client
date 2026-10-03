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
class Element
{
public:
	float flTimeCreated;
	Element *next;
	Element *previous;

	Element( float Time )
	{
		flTimeCreated = Time;
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

	~Queue( void )
	{
		Update( 99999.0f );
	}

	bool Full( void ) 
	{ 
		return ( count >= maxelements );
	}

	bool Add( float flTime )
	{
		if( Full() )
			return false;

		Element *pNewElement = new Element( flTime );

		if( !pNewElement )
			return false;

		if( !first )
		{
			first = pNewElement;
			last = pNewElement;
			current = pNewElement;
		}
		else
		{
			pNewElement->previous = last;
			last->next = pNewElement;
			last = pNewElement;
			current = pNewElement;
		}

		count++;
		return true;
	}


	void Update( float flCurrentTime )
	{
		Element *last;
		Element *previous;

		last = this->last;

		if( last )
		{
			while( last->flTimeCreated - flCurrentTime < 0.0f )
			{
				previous = last->previous;

				delete last;

				count--;
				this->last = previous;
				current = previous;

				if( !previous )
				{
					first = NULL;
					current = NULL;
					return;
				}

				last = previous;
			}

			Element *pCurr = first;

			while( pCurr )
			{
				pCurr->flTimeCreated -= flCurrentTime;
				pCurr = pCurr->next;
			}
		}
	}
};

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
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void ) { /*Nothing.*/ }

	CHudMsgFunc( ClanTimer );
	bool ShouldDrawCrossHair( void );
	float GetCurrentWeaponAccuracy( void );
	void DrawClanTimer( float flTime );
	void DrawSpectatorCrossHair( void );
	void DrawDynamicCrossHair( void );
	void DrawCustomCrossHair( int style );
	int GetCrossHairWidth( void );

private:
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
#define MAX_EXTRA_MAP_ICONS 32 

#pragma pack(push, 1)

class CHudDoDMap : public CHudBase
{
public:
	typedef struct
	{
		vec3_t origin;
		HSPRITE hSprite;
		double killtime;
	} map_icon_t;

	map_icon_t m_ExtraOverviewEntities[MAX_EXTRA_MAP_ICONS];

	int VidInit( void );
	void InitHUDData( void );
	int Draw( float flTime );
	int Init( void );

	void DrawOverview( void );
	void DrawOverviewLayer( void );
	void DrawOverviewEntities( void );
	void CheckOverviewEntities( void );
	void HandleMapButton( void );
	void HandleMapZoomButton( void );
	void SetMapState( int mapstate );
	void GetSmallMapOffset( float &x, float &y, float &z );
	bool AddMapEntityToMap( HSPRITE sprite, double lifetime, vec3_t origin );
	void DrawOverviewIcon( vec3_t origin, HSPRITE hIcon, int iconScale, vec3_t angles );

private:
	cl_entity_t m_MapTag[64];

	float m_flZoom;
	float m_flZoomTime;
	float m_flSmallMapScale;
	float m_flIdealMapScale;
	float m_flPreviousMapScale;
};

#pragma pack(pop)

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
	char *GetTargetName( void );
	void CreateEntities( void );
	void DrawEntitiesOverTeam( void );
	void DrawEntitiesOverTarget( void );
	bool InDeathCamMode( void );
	void Think( void );

	int GetTargetHealth( void ) { return m_iHealth; }
	int GetTargetIndex( void ) { return m_iTargetIndex; }
	int GetTargetTeam( void ) { return m_iTargetTeam; }

	void UpdateWhosTalking( int index, qboolean bTalking )
	{
		if( index < 1 || index > MAX_PLAYERS )
			return;

		m_TargetTalking[index] = bTalking;
	}

	qboolean IsEntityTalking( int index )
	{
		if( index < 1 || index > MAX_PLAYERS )
			return false;

		if( m_TargetTalking[index] )
			return true;

		return false;
	}

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
};

//
//-----------------------------------------------------
//
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

#define MAX_CONTROL_POINTS 12

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
	void SetNumPlayersInArea( int point, int team, int numlayers, int required );
	void SetVisible( int point, int visible );
	void UpdateObjectiveIcons( void );
	void CalcIconLocations( void );
	void DrawDigit( int digit, int x, int y );

	float GetObjectiveTime( void ) { return m_fTimerSeconds; }
	void SetWaveTime( float flTime ) { m_flWaveTime = flTime; }
	float GetWaveTime( void ) { return m_flWaveTime; }
	void SetWaveStatus( int iStatus ) { m_iWaveStatus = iStatus; }
	int GetWaveStatus( void ) { return m_iWaveStatus; }
	void SetWarmupMode( bool bWarmupMode ) { m_bWarmupMode = bWarmupMode; }

	bool IsPointValid( int point )
	{
		if( point < 0 || point >= MAX_CONTROL_POINTS )
			return false;

		return m_eControlPoints[point].valid;
	}

	void PlayersInArea( int point, int team )
	{
		if( point < 0 || point >= MAX_CONTROL_POINTS )
			return;

		if( m_eControlPoints[point].visible )
		{
			m_eControlPoints[point].occupyingteam = team;
		}
	}

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

	char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	char *BufferedLocaliseTextString( const char *msg );
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
		client_textmessage_t *pMessage;
		unsigned int font;
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
	void MessageAdd( client_textmessage_t *newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time, unsigned int font );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );
	void HintMessageAdd( const char *pText );

private:
	client_message_t			m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t *m_pGameTitle;

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
	CHudMsgFunc( CameraView );
	int Draw( float flTime );
	int VidInit( void );
};

//
//-----------------------------------------------------
//
struct ClientArea {
	int     nStatus;
	HSPRITE hSpr;
};

class CHudDodIcons : public CHudBase {
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
#define MAX_SHOOTERS	64

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
	CHudMsgFunc( PShoot );

private:
	particle_shooter_s m_sShooters[MAX_SHOOTERS];
	int m_iNumShooters;
};

//
//-----------------------------------------------------
//
#define MAX_ENV_MODELS 192

#define SF_ENVMODEL_ANIMATION 0x10

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

	int GetNumModels( void ) { return m_iNumEnvModels; }

	env_model_t *GetModel( int iModel )
	{
		if( iModel < 0 || iModel >= MAX_ENV_MODELS )
			return NULL;

		return &m_sEnvModels[iModel];
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

	void SetRainSprite( model_s *pModel ) { m_pRainSprite = pModel; }
	void SetSnowSprite( model_s *pModel ) { m_pSnowSprite = pModel; }
	void SetSplashSprite( model_s *pModel ) { m_pSplashSprite = pModel; }
	void SetRippleSprite( model_s *pModel ) { m_pRippleSprite = pModel; }

	model_s *GetRainSprite( void ) { return m_pRainSprite; }
	model_s *GetSnowSprite( void ) { return m_pSnowSprite; }
	model_s *GetSplashSprite( void ) { return m_pSplashSprite; }
	model_s *GetRippleSprite( void ) { return m_pRippleSprite; }

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

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	void EnableIcon( char *pszIconName, unsigned char red, unsigned char green, unsigned char blue );
	void DisableIcon( char *pszIconName );

private:
	icon_sprite_t m_IconList[MAX_ICONSPRITES];
};

//
//-----------------------------------------------------
//
#define VGUI2_CHAR_BUFFER 512

class CHudVGUI2Print : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	int DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b );
	int DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b );

	void VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );

	int GetHudFontHeight( void );
	void GetStringSize( const char *string, int *width, int *height );

private:
	float m_flVGUI2StringTime;
	char  m_szCharBuf[VGUI2_CHAR_BUFFER];
	float m_fR, m_fG, m_fB;
	int   m_iX, m_iY;

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
	void GetTrajectory( vec3_t launchPos, vec3_t targetPos, float *pitch1, float *pitch2, float *yaw );
	void CalculateTrajectory( vec3_t launchPos, vec3_t targetPos, float *pitch1, float *pitch2, float *yaw );
	trajectory_t *AddTrajectory( vec3_t targetPos );
	void InvalidateAllTrajectories( void );

	enum
	{
		MAX_TRAJECTORIES = 32,
		TRAJECTORY_LIFETIME = 30
	};

private:
	vec3_t m_vecLaunchPos;
	trajectory_t m_Trajectories[MAX_TRAJECTORIES];
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
class CHud {
private:
	HUDLIST			   *m_pHudList;
	HSPRITE            m_hsprLogo;
	int                m_iLogo;
	client_sprite_t	   *m_pSpriteList;
	int                m_iSpriteCount;
	int                m_iSpriteCountAllRes;

public:
	HSPRITE            m_hsprCursor;
	float              m_flTime;
	float              m_fOldTime;

	double             m_flTimeDelta;
	Vector             m_vecOrigin;
	Vector             m_vecAngles;
	Vector             m_vecVelocity;
	int                m_iKeyBits;
	int                m_iHideHUDDisplay;
	int                m_Teamplay;
	int                m_iRes;
	cvar_t			   *m_pCvarStealMouse;
	cvar_t			   *m_pCvarDraw;
	float              m_flMouseSensitivity;

	int                m_PlayerFOV[64];
	bool               m_bAllieParatrooper;
	bool               m_bAllieInfiniteLives;
	bool               m_bAxisParatrooper;
	bool               m_bAxisInfiniteLives;
	bool               m_bParatrooper;
	bool               m_bInfiniteLives;
	bool               m_bBritish;

	int                m_iRoundState;
	int                m_iSensLevel;
	int                m_iFontHeight;
	int                m_iFontEngineHeight;
	HSPRITE			   *m_rghSprites;
	wrect_t			   *m_rgrcRects;
	char			   *m_rgszSpriteNames;
	char               m_szTeamNames[5][MAX_TEAMNAME_SIZE];

	int                m_iFOV;
	int                m_iMapX;
	int                m_iMapY;
	int                m_iMapWidth;
	int                m_iMapHeight;
	int                m_iSmallMapX;
	int                m_iSmallMapY;
	int                m_iSmallMapWidth;
	int                m_iSmallMapHeight;

	CHudScope          m_Scope;
	CHudDodIcons       m_Icons;
	CObjectiveIcons    m_ObjectiveIcons;
	CParticleShooter   m_PShooter;
	CClientEnvModel    m_CEnvModel;
	CWeatherManager    m_Weather;
	CHudAmmo           m_Ammo;
	CHudSayText        m_SayText;
	CHudSpectator      m_Spectator;
	CHudTrain          m_Train;
	CHudMessage        m_Message;
	CHudStatusBar      m_StatusBar;
	CHudDeathNotice    m_DeathNotice;
	CHudMenu           m_Menu;
	CHudTextMessage    m_TextMessage;
	CHudStatusIcons    m_StatusIcons;
	CHudDoDCrossHair   m_DoDCrossHair;
	CHudDoDMap         m_DoDMap;
	CHudDoDCommon      m_DoDCommon;
	CHudVGUI2Print     m_VGUI2Print;
	CMortarHud         m_MortarHud;

	SCREENINFO         m_scrinfo;
	int                m_iWeaponBits;
	int                m_fPlayerDead;
	int                m_iIntermission;
	int                g_iClip;
	bool               m_bAutoReloadComplete;

	int                g_pModel;
	float              g_NextAttack;
	int                m_HUD_number_0;
	int                m_MG_number_0;
	float              i_MusicFadeCounter;
	float              i_MusicFadeCounter2;
	int                i_MusicPlay;
	int                i_specmenutoggle;
	int                i_Recoil;
	float              i_TempFSpeed;
	float              i_TempBSpeed;
	float              i_TempSSpeed;

	int                m_iWaterLevel;
	int                m_iClipSize;
	float              m_flPlaySprintSoundTime;
	int                m_iMinimapState;
	float              m_fRoundEndsTime;
	float              m_fMortarDeployTime;
	float              m_fMortarUnDeployTime;

	cvar_t			   *_cl_minimap;
	cvar_t			   *_cl_minimapzoom;
	cvar_t			   *zoom_sensitivity_ratio;
	cvar_t			   *hud_takesshots;
	cvar_t			   *r_drawentities;
	cvar_t			   *cl_lw = NULL;
	cvar_t			   *cl_pitchup;
	cvar_t			   *cl_pitchdown;
	cvar_t			   *crosshair;
	cvar_t			   *max_rubble;
	cvar_t			   *_ah;
	cvar_t			   *cl_corpsestay;
	cvar_t			   *developer;
	cvar_t			   *cl_hudfont;
	cvar_t			   *hud_fastswitch;

	float              m_flPitchRecoilAccumulator;
	float              m_flYawRecoilAccumulator;
	float              m_flRecoilTimeRemaining;

public:
	HSPRITE GetSprite( int index ) 
	{
		return ( index < 0 ) ? 0 : m_rghSprites[index];
	}

	wrect_t &GetSpriteRect( int index ) 
	{
		return m_rgrcRects[index];
	}

	void SetFOV( int fov ) 
	{ 
		m_iFOV = fov; 
	}

	int GetFOV() 
	{ 
		return m_iFOV; 
	}

	int GetSpriteIndex( const char *SpriteName );

	CHud() : m_iSpriteCount( 0 ), m_pHudList( NULL ) {}
	~CHud();

	int DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth( int iNumber, int iFlags );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );
	void PostMortarValue( float value );
	int GetMinimapZoomLevel( void );
	int ZoomMinimap( void );

	void Init( void );
	void VidInit( void );
	void Think( void );
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHudMsgFunc( Damage );
	CHudMsgFunc( Logo );
	CHudMsgFunc( ResetHUD );
	CHudMsgFunc( YouDied );
	CHudMsgFunc( ZoomStatus );
	CHudMsgFuncV( InitHUD );
	CHudMsgFunc( SetFOV );
	CHudMsgFunc( HLTV );
	CHudMsgFunc( RoundState );
	CHudMsgFunc( TimeLeft );
	CHudMsgFuncV( UseSound );

	void AddHudElem( CHudBase *p );
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
	void DoRecoil( int weapon_id );
	void GetWeaponRecoilAmount( int weaponId, float &flPitchRecoil, float &flYawRecoil );
	void SetRecoilAmount( float flPitchRecoil, float flYawRecoil );
	void PopRecoil( float frametime, float &flPitchRecoil, float &flYawRecoil );

	void GetAllPlayersInfo( void );

	int DrawHudStringLen( const char *szIt );
	void DrawDarkRectangle( int x, int y, int wide, int tall );

	CHudScoreboard     m_Scoreboard;
	CHudMOTD           m_MOTD;
};

void ClientSetSensitivity( int level );
bool ShouldShowBlood( void );
int EV_BloodPuffMsg( const char *pszName, int iSize, void *pbuf );
void EV_BloodPuff( float *org );
int EV_HandSignalMsg( const char *pszName, int iSize, void *pbuf );

extern CHud gHUD;
extern Queue g_RubbleQueue;

extern int g_iPlayerClass, g_iTeamNumber;
extern int g_iUser1, g_iUser2, g_iUser3;

extern float g_fUser4;
extern int g_iVuser1x;
extern int g_iVuser1z;
extern int g_iMovetype;
extern int g_iEffects;
extern int g_iOnlyClientDraw;
extern float g_lastFOV;
extern float g_fStamina;
extern int g_iWeaponBits2;
#endif
