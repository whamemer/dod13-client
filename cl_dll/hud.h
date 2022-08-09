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
//#include "com_model.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	

#define		HUDELEM_ACTIVE	1

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
typedef struct particle_shooter_s particle_shooter_t;

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
	virtual		~CHudBase() {}
	virtual int Init( void ) { return 0; }
	virtual int VidInit( void ) { return 0; }
	virtual int Draw( float flTime ) { return 0; }
	virtual void Think( void ) { return; }
	virtual void Reset( void ) { return; }
	virtual void InitHUDData( void ) {}		// called every time a server is connected to
};

struct HUDLIST
{
	CHudBase	*p;
	HUDLIST		*pNext;
};

//
//-----------------------------------------------------
#if USE_VGUI
#include "voice_status.h" // base voice handling class
#endif
#include "hud_spectator.h"

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
	int DrawWList( float flTime );
	int MsgFunc_CurWeapon( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeaponList( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AmmoX( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf );

	int GetCurrentWeaponId( void );

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

private:
	float m_fFade;
	RGBA  m_rgba;
	WEAPON *m_pWeapon;
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

	int MsgFunc_SecAmmoVal( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SecAmmoIcon( const char *pszName, int iSize, void *pbuf );

private:
	enum {
		MAX_SEC_AMMO_VALUES = 4
	};

	int m_HUD_ammoicon; // sprite indices
	int m_iAmmoAmounts[MAX_SEC_AMMO_VALUES];
	float m_fFade;
};


#include "health.h"


#define FADE_TIME 100


//
//-----------------------------------------------------
//
class CHudGeiger: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_Geiger( const char *pszName, int iSize, void *pbuf );
	
private:
	int m_iGeigerRange;
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

#if !USE_VGUI || USE_NOVGUI_MOTD
class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	int MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf );
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
#endif

#if !USE_VGUI || USE_NOVGUI_SCOREBOARD
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
	int MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamScores( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamNames( const char *pszName, int iSize, void *pbuf );
	void DeathMsg( int killer, int victim );

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;

	void GetAllPlayersInfo( void );
};
#endif

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
	void ParseStatusString( int line_num );

	int MsgFunc_StatusText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf );

protected:
	enum
	{ 
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2
	};

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
	int MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf );

private:
	int m_HUD_d_skull;  // sprite index of skull icon
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

	void SelectMenuItem( int menu_item );

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
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
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
	friend class CHudSpectator;

private:
	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
class CHudBattery : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_Battery( const char *pszName,  int iSize, void *pbuf );
	
private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	int m_iBat;
	float m_fFade;
	int m_iHeight;		// width of the battery innards
};

//
//-----------------------------------------------------
//
class CHudFlashlight: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	int MsgFunc_Flashlight( const char *pszName,  int iSize, void *pbuf );
	int MsgFunc_FlashBat( const char *pszName,  int iSize, void *pbuf );

private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	HSPRITE m_hBeam;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	wrect_t *m_prcBeam;
	float m_flBat;	
	int m_iBat;	
	int m_fOn;
	float m_fFade;
	int m_iWidth;		// width of the battery innards
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
	int MsgFunc_TextMsg( const char *pszName, int iSize, void *pbuf );
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
	int MsgFunc_HudText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_GameTitle( const char *pszName, int iSize, void *pbuf );

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	client_textmessage_t		*m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
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

class CHudScope : public CHudBase
{
public:
	int Init( void );
	int MsgFunc_Scope( const char *pszName, int iSize, void *pbuf );
	void SetScope( int weaponId );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	void Think( void );
	void DrawTriApiScope( void );

private:
	int			m_iWeaponId;
	bool		m_bWeaponChanged;
	int			m_nLastWpnId;

	HSPRITE		spring_sprite;
	const struct model_s		*spring_model;
	HSPRITE						k43_sprite;
	const struct model_s		*k43_model;
	HSPRITE						binoc_sprite;
	const struct model_s		*binoc_model;
	HSPRITE						enfield_sprite;
	const struct model_s		*enfield_model;
};

//
//-----------------------------------------------------
//

class CHudDoDCommon : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float fltime );
	int MsgFunc_GameRules( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ResetSens( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_CameraView( const char *pszName, int iSize, void *pbuf );

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

private:
	int m_MoveWidth;
	int m_MoveHeigth;
	int m_MoveWidthD;
	int m_MoveHeigthD;
	int i_xSet;
	int m_iXPos;
	int m_iYPos;
	int m_iLastXHairWidth;

	float f_xPos;
	float f_yPos;
	float f_xLPos;
	float f_xRPos;
	float m_fClanTimer;
	float m_fMoveTime;
	float m_flLastSetTime;

	vec3_t v_evPunch;

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
};

//
//-----------------------------------------------------
//

class CHudDoDMap : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void InitHUDData( void );
	int Draw( float flTime );
	void DrawOverview( void );
	void DrawOverviewLayer( void );
	void DrawOverviewEntities( void );
	void CheckOverviewEntities( void );
	void HandleMapButton( void );
	void HandleMapZoomButton( void );
	void SetMapState( void );
	void GetSmallMapOffset( float *x, float *y, float *z );
	bool AddMapEntityToMap( HSPRITE sprite, double lifeTime, vec3_t *p_origin );
	void DrawOverviewIcon( vec3_t *p_origin, HSPRITE hIcon, int iconScale, vec3_t *p_angles );

private:
	struct cl_entity_s m_MapTag[64];
	float m_flZoom;
	float m_flZoomTime;
	float m_flSmallMapScale;
	float m_flIdealMapScale;
	float m_flPreviousMapScale;

	typedef struct
	{
		vec3_t origin;
		HSPRITE hSprite;
		double killTime;
	} map_icon_t;
	
	map_icon_t m_ExtraOverviewEntities[32];

};

//
//-----------------------------------------------------
//

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
	void GetHintMessageLocation( int *x, int *y );
	void StartDrawingCredits( void );
	void DrawCredits( float flTime );
	void DrawMarkerIcon( HSPRITE pSpr );

private:
	int iHealth;
	int sprite_array[5];
	int m_iKey;
	int i_HeartFrame;
	int m_iIconHeight;
	int m_iIconWidth;
	int m_iHintTeam;
	int m_iMsgX;
	int m_iMsgY;
	int m_hHintHeads[3];
	int m_iCreditName;

	float m_fLastMapMarkerTime;
	float f_HeartTime;
	float f_HeartSpeed;
	float m_flHintDieTime;
	float m_flCreditChangeTime;
	float m_flDrawSelectedMarkerTime;

	char *m_szObjectIcon;

	HSPRITE m_hObjectSpr;
	HSPRITE MapMarkerSprite;
	HSPRITE m_hHorizHintBacking;
	HSPRITE m_hVertHintBacking;
	HSPRITE m_hsprSelectedMarker;

	wrect_t *m_wObjectArea;
	wrect_t *area_array[5];
	wrect_t *MapMarkerArea;
	wrect_t *m_rectHintHeads[3];
	wrect_t *m_rectHorizHintBacking;
	wrect_t *m_rectVertHintBacking;

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
	wrect_t StaminaBarHUDArea;
	HSPRITE HealthBarHUD;
	wrect_t *HealthBarHUDArea;
	HSPRITE HealthOverlayHUD;
	wrect_t *HealthOverlayHUDArea;

	struct ClientArea
	{
		int nStatus;
		HSPRITE hSpr;
	};
	
	struct ClientArea m_Areas[128];

};

//
//-----------------------------------------------------
//
// TODO: WHAMER: need vgui2 :(
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
	void GetStringSize( wchar_t *string, int *width, int *height );
	int Draw( float flTime );
	//vgui2::HFont GetFont( void ); 

private:
	int m_iX;
	int m_iY;

	unsigned long m_Fonts[3];

	float m_flVGUI2StringTime;
	float m_fR;
	float m_fG;
	float m_fB;

	wchar_t m_wCharBuf[512];

};

//
//-----------------------------------------------------
//

class CClientEnvModel : public CHudBase
{
public:
	typedef struct
	{
		// TODO: WHAMER
	} env_model_t;

	int Init( void );
	int VidInit( void );
	void Think( void );
	void RemoveAllModels( void );
	void AddEnvModel( env_model_t *pModel );

private:
	struct tempent_s *m_teEnvModelTE;
	env_model_t *m_sEnvModels[192];

};

//
//-----------------------------------------------------
//

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
	void DrawDigit( int digit, int x, int y );

private:
	int m_Init;
	int m_Set;
	int m_StartProgress;
	int m_CancelProgress;
	int m_iWaveStatus;
	int m_nTimerStatus;
	int m_TimerIcons[11];

	float m_fTimerSeconds;
	float m_flWaveTime;
	float m_fLastTime;

	wrect_t m_iconarea;
	wrect_t *TimerHUDArea;
	wrect_t *m_TimerAreas[11];
	wrect_t m_topwrect;
	wrect_t m_bottomwrect;

	HSPRITE TimerHUD;
	HSPRITE m_TopNumber;
	HSPRITE m_BottomNumber;

	bool m_bWarmupMode;

	struct control_point_t
	{
		int entindex;
		int visible;
		int owner;
		int nextOwner;
		int numplayers;
		int requiredplayers;
		int occupyingteam;
		int m_iIcons[3];
		int m_iMapXPos;
		int m_iMapYPos;
		
		struct rect_s m_rAreas[3];

		vec3_t m_rOrigin;

		float animtime;
		float totaltime;

		bool valid;
	};
	
	control_point_t m_eControlPoints[12];
};

//
//-----------------------------------------------------
//

class CParticleShooter : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Think( void );
	void AddParticleSystem( particle_shooter_t *pShooter );
	int MsgFunc_PReg( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_PShoot( const char *pszName, int iSize, void *pbuf );

private:
	int m_iNumShooters;

	struct particle_shooter_s
	{
		int id;
		int iNumParticles;
		int iParticlesRemaining;
		int iFlags;
		int iSprite;
		int iColour[3];
		int iSpinDegPerSec;
		int iFramerate;
		int iRenderModel;
		int iState;

		float fParticleLife;
		float fFireDelay;
		float fVariance;
		float fNextShootTime;
		float fGravity;
		float fFadeSpeed;
		float fSize;
		float fDampingTime;
		float fDampingVel;
		float fBrightness;
		float fScaleSpeed;

		char szSprite[128];

		vec3_t vOrigin;
		vec3_t vVelocity;

		model_s *pSprite;
	};
	
	particle_shooter_s m_sShooters[64];

};

//
//-----------------------------------------------------
// TODO: WHAMER: need Particleman :(

class CWeatherManager : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void CreateRainParticle( void );
	void CreateSnowParticle( void );

private:
	model_s *m_pRainSprite;
	model_s *m_pSnowSprite;
	model_s *m_pSplashSprite;
	model_s *m_pRippleSprite;

};

//
//-----------------------------------------------------
//

class CTrajectoryList
{
public:
	enum
	{
		MAX_TRAJECTORIES = 32,
		TRAJECTORY_LIFETIME = 30
	};

	typedef struct
	{
		// TODO: WHAMER
	} trajectory_t;

	void GetTrajectory( vec3_t *p_launchPos, vec3_t *p_targetPos, float *pitch1, float *pitch2, float *yaw );
	void CalculateTrajectory( vec3_t *p_launchPos, vec3_t *p_targetPos, float *pitch1, float *pitch2, float *yaw );
	trajectory_t *AddTrajectory( vec3_t *p_targetPos );
	void InvalidateAllTrajectories( void );

private:
	vec3_t m_vecLaunchPos;

	trajectory_t m_Trajectories[MAX_TRAJECTORIES];
};

class CMortarHud : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void CalculateFireAngle( float *pitch, float *yaw );
	void DrawPredictedMortarImpactSite( void );

private:
	CTrajectoryList *m_TrajectoryList;

};

//
//-----------------------------------------------------
//

class Element
{
public:
	float flTimeCreated;
	Element *next;
	Element *previous;
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
	float						m_flMouseSensitivity;
	int							m_iConcussionEffect; 

public:
	HSPRITE						m_hsprCursor;
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	Vector  m_vecVelocity;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	int		m_Teamplay;
	int		m_iRes;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;

	float m_flPitchRecoilAccumulator;
	float m_flYawRecoilAccumulator;
	float m_flRecoilTimeRemaining;

	int m_iFontHeight;
	int DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString( int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth( int iNumber, int iFlags );
	int DrawHudStringLen( const char *szIt );
	void DrawDarkRectangle( int x, int y, int wide, int tall );

	int GetCurrentWeaponId( void );

private:
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
	
	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rghSprites[] array

	CHudAmmo		m_Ammo;
	CHudHealth		m_Health;
	CHudSpectator		m_Spectator;
	CHudGeiger		m_Geiger;
	CHudBattery		m_Battery;
	CHudTrain		m_Train;
	CHudFlashlight	m_Flash;
	CHudMessage		m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudSayText		m_SayText;
	CHudMenu		m_Menu;
	CHudAmmoSecondary	m_AmmoSecondary;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudScope		m_Scope;
	CHudDodIcons	m_Icons;
	CObjectiveIcons	m_ObjectiveIcons;
	CParticleShooter	m_PShooter;
	CClientEnvModel	m_CEnvModel;
	CWeatherManager	m_Weather;
	CHudDoDCrossHair	m_DoDCrossHair;
	CHudDoDMap		m_DoDMap;
	CHudDoDCommon	m_DoDCommon;
	CHudVGUI2Print	m_VGUI2Print;
	CMortarHud		m_MortarHud;
#if !USE_VGUI || USE_NOVGUI_SCOREBOARD
	CHudScoreboard	m_Scoreboard;
#endif
#if !USE_VGUI || USE_NOVGUI_MOTD
	CHudMOTD	m_MOTD;
#endif

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}  
	~CHud();			// destructor, frees allocated memory

	// user messages
	int _cdecl MsgFunc_Damage( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_GameMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Logo( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_ResetHUD( const char *pszName,  int iSize, void *pbuf );
	void _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	void _cdecl MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV( const char *pszName,  int iSize, void *pbuf );
	int _cdecl MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf );

	int _cdecl MsgFunc_BloodPuff( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_CurMarker( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Frags( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_HandSignal( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_MapMarker( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_ObjScore( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_PClass( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_PStatus( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_PTeam( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_HLTV( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_RoundState( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_ScoreInfoLong( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_ScoreShort( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_TimeLeft( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_UseSound( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_WaveStatus( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_WaveTime( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_WideScreen( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_YouDied( const char *pszName, int iSize, void *pbuf );

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;

	// sprite indexes
	int m_HUD_number_0;

	int m_iNoConsolePrint;

	// DOD
	bool m_bAllieParatrooper;
	bool m_bAllieInfiniteLives;
	bool m_bAxisParatrooper;
	bool m_bAxisInfiniteLives;
	bool m_bParatrooper;
	bool m_bInfiniteLives;
	bool m_bBritish;

	int i_Recoil;
	int g_iWeaponFlags;
	int g_iClip;
	int m_iRoundState;
	int m_iSensLevel;
	int m_iWaterLevel;

	char m_szTeamNames[5][32];

	Queue g_RubbleQueue;

	cvar_t *_cl_minimap;
	cvar_t *_cl_minimapzoom;
	cvar_t *zoom_sensitivity_ratio;
	cvar_t *hud_takesshots;
	cvar_t *max_rubble;
	cvar_t *_ah;
	cvar_t *cl_corpsestay;
	cvar_t *cl_hudfont;

	void AddHudElem( CHudBase *p );

	float GetSensitivity();

	void GetAllPlayersInfo( void );

	// DOD

	void GetWeaponRecoilAmount( int weapon_id, float *flPitchRecoil, float *flYawRecoil );
	void DoRecoil( int weapon_id );
	void GetMapBounds( int *x, int *y, int *w, int *h );
	void InitMapBounds( void );
	void PlaySoundOnChan( char *name, float fVol, int chan );
	void PopRecoil( float frametime, float *flPitchRecoil, float *flYawRecoil );
	void PostMortarValue( float value );
	void SetMinimapState( int state );
	void SetMortarDeployTime( void );
	void SetMortarUnDeployTime( void );
	void SetRecoilAmount( float flPitchRecoil, float flYawRecoil );
	void SetWaterLevel( int level );
	void VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b );

	int GetMinimapState( void );
	int GetMinimapZoomLevel( void );
	int GetWaterLevel( void );
	int ZoomMinimap( void );

	float GetMortarDeployTime( void );

	char *GetPlayerClassName( int playerclass );
	char *GetTeamName( int team );

	bool IsDucking( void );
	bool IsInMGDeploy( void );
	bool IsInMortarDeploy( void );
	bool IsProne( void );
	bool IsProneDeployed( void );
	bool IsSandbagDeployed( void );
	bool IsTeamPara( void );
};

extern CHud gHUD;
extern CHudScope gHUDScope;
extern CHudAmmo gHUDAmmo;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;

extern int g_iVuser1x;
extern int g_iVuser1z;
#endif
