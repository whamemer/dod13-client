#pragma once
#if !defined(DOD_SHARED_H)
#define DOD_SHARED_H

// weapon id
#define WEAPON_NONE				0
#define WEAPON_AMER_KNIFE		1
#define WEAPON_GER_KNIFE		2
#define WEAPON_COLT             3
#define WEAPON_LUGER            4
#define WEAPON_GARAND           5
#define WEAPON_SCOPEDKAR        6
#define WEAPON_THOMPSON         7
#define WEAPON_MP44             8
#define WEAPON_SPRING           9
#define WEAPON_KAR              10
#define WEAPON_BAR              11
#define WEAPON_MP40             12
#define WEAPON_HANDGRENADE      13
#define WEAPON_STICKGRENADE     14
#define WEAPON_STICKGRENADEX    15
#define WEAPON_HANDGRENADEX     16
#define WEAPON_MG42             17
#define WEAPON_CAL30            18
#define	WEAPON_SPADE			19
#define WEAPON_M1CARBINE        20
#define WEAPON_MG34             21
#define WEAPON_GREASEGUN        22
#define WEAPON_FG42             23
#define WEAPON_K43              24
#define WEAPON_ENFIELD          25
#define WEAPON_STEN             26
#define WEAPON_BREN             27
#define WEAPON_WEBLEY           28
#define WEAPON_BAZOOKA          29
#define WEAPON_PSCHRECK         30
#define WEAPON_PIAT             31

#define WEAPON_BINOC            33

// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define BREN_WEIGHT
#define MG34_WEIGHT
#define MG42_WEIGHT
#define CAL30_WEIGHT
#define BAR_WEIGHT
#define AMERKNIFE_WEIGHT        0
#define GERKNIFE_WEIGHT		    0
#define SPADE_WEIGHT		    0
#define COLT_WEIGHT
#define LUGER_WEIGHT
#define WEBLEY_WEIGHT
#define ENFIELD_WEIGHT
#define FG42_WEIGHT

// weapon clip/carry ammo capacities
#define _55MM_MAX_CARRY                 // BREN, BAR, FG42, SCOPED FG42
#define _66MM_MAX_CARRY                 // MG34
#define _66MM_GER_MAX_CARRY             // MG42
#define _66MM_US_MAX_CARRY              // 30CAL
#define _12MM_MAX_CARRY                 // COLT, LUGER, WEBLEY
#define _16MM_MAX_CARRY                 // ENFIELD, SCOPED ENFIELD

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

#define BREN_MAX_CLIP
#define MG34_MAX_CLIP
#define MG42_MAX_CLIP
#define CAL30_MAX_CLIP
#define BAR_MAX_CLIP
#define AMERKNIFE_MAX_CLIP		WEAPON_NOCLIP
#define GERKNIFE_MAX_CLIP		WEAPON_NOCLIP
#define SPADE_MAX_CLIP			WEAPON_NOCLIP
#define COLT_MAX_CLIP
#define LUGER_MAX_CLIP
#define WEBLEY_MAX_CLIP
#define ENFIELD_MAX_CLIP
#define FG42_MAX_CLIP

// the default amount of ammo that comes with each gun when it spawns
#define BREN_DEFAULT_GIVE      30
#define MG34_DEFAULT_GIVE      75
#define MG42_DEFAULT_GIVE      250
#define CAL30_DEFAULT_GIVE
#define BAR_DEFAULT_GIVE
#define MELEE_DEFAULT_GIVE     0
#define COLT_DEFAULT_GIVE
#define LUGER_DEFAULT_GIVE
#define WEBLEY_DEFAULT_GIVE
#define ENFIELD_DEFAULT_GIVE
#define FG42_DEFAULT_GIVE

// The amount of ammo given to a player by an ammo item.
#define AMMO_BRENCLIP_GIVE		BREN_MAX_CLIP
#define AMMO_MG34CLIP_GIVE      MG34_MAX_CLIP
#define AMMO_MG42CLIP_GIVE      MG42_MAX_CLIP
#define AMMO_CAL30CLIP_GIVE     CAL30_MAX_CLIP
#define AMMO_BARCLIP_GIVE       BAR_MAX_CLIP
#define AMMO_COLTCLIP_GIVE      COLT_MAX_CLIP
#define AMMO_LUGERCLIP_GIVE     LUGER_MAX_CLIP
#define AMMO_WEBLEYCLIP_GIVE    WEBLEY_MAX_CLIP
#define AMMO_ENFIELDCLIP_GIVE   ENFIELD_MAX_CLIP
#define AMOO_FG42CLIP_GIVE      FG42_MAX_CLIP

typedef enum
{
	BULLET_PLAYER_COLT,
	BULLET_PLAYER_LUGER,
	BULLET_PLAYER_GARAND,
	BULLET_PLAYER_SCOPEDKAR,
	BULLET_PLAYER_THOMPSON,
	BULLET_PLAYER_MP44,
	BULLET_PLAYER_SPRING,
	BULLET_PLAYER_KAR,
	BULLET_PLAYER_BAR,
	BULLET_PLAYER_MP40,
	BULLET_PLAYER_MG42,
	BULLET_PLAYER_MG34,
	BULLET_PLAYER_30CAL,
	BULLET_PLAYER_M1CARBINE,
	BULLET_PLAYER_GREASEGUN,
	BULLET_PLAYER_FG42,
	BULLET_PLAYER_K43,
	BULLET_PLAYER_ENFIELD,
	BULLET_PLAYER_STEN,
	BULLET_PLAYER_BREN,
	BULLET_PLAYER_WEBLEY
} Bullet_dod;

enum VOICECOMSLOTS_e
{
    SLOT_VOICECOMMAND = 0,
    SLOT_TEXT = 1,
    SLOT_GER_ALLTEXT = 2,
    SLOT_BRIT_ALLTEXT = 3
};

//melee weapon
#define KNIFE_BODYHIT_VOLUME    128
#define KNIFE_WALLHIT_VOLUME    512

enum KNIFE_e
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH1 = 1,
    KNIFE_SLASH2 = 2,
    KNIFE_DRAW = 3
};

enum GREASEGUN_e
{
    GREASEGUN_IDLE = 0,
    GREASEGUN_RELOAD = 1,
    GREASEGUN_DRAW = 2,
    GREASEGUN_SHOOT1 = 3,
    GREASEGUN_SHOOT2 = 4,
    GREASEGUN_IDLE_EMPTY = 5,
    GREASEGUN_FAST_DRAW = 6
};

enum CAL30_e
{
    CAL30_UPIDLE = 0,
    CAL30_UPIDLE8 = 1,
    CAL30_UPIDLE7 = 2,
    CAL30_UPIDLE6 = 3,
    CAL30_UPIDLE5 = 4,
    CAL30_UPIDLE4 = 5,
    CAL30_UPIDLE3 = 6,
    CAL30_UPIDLE2 = 7,
    CAL30_UPIDLE1 = 8,
    CAL30_UPIDLEEMPTY = 9,
    CAL30_DOWNIDLE = 10,
    CAL30_DOWNIDLE8 = 11,
    CAL30_DOWNIDLE7 = 12,
    CAL30_DOWNIDLE6 = 13,
    CAL30_DOWNIDLE5 = 14,
    CAL30_DOWNIDLE4 = 15,
    CAL30_DOWNIDLE3 = 16,
    CAL30_DOWNIDLE2 = 17,
    CAL30_DOWNIDLE1 = 18,
    CAL30_DOWNIDLEEMPTY = 19,
    CAL30_DOWNTOUP = 20,
    CAL30_DOWNTOUP8 = 21,
    CAL30_DOWNTOUP7 = 22,
    CAL30_DOWNTOUP6 = 23,
    CAL30_DOWNTOUP5 = 24,
    CAL30_DOWNTOUP4 = 25,
    CAL30_DOWNTOUP3 = 26,
    CAL30_DOWNTOUP2 = 27,
    CAL30_DOWNTOUP1 = 28,
    CAL30_DOWNTOUPEMPTY = 29,
    CAL30_UPTODOWN = 30,
    CAL30_UPTODOWN8 = 31,
    CAL30_UPTODOWN7 = 32,
    CAL30_UPTODOWN6 = 33,
    CAL30_UPTODOWN5 = 34,
    CAL30_UPTODOWN4 = 35,
    CAL30_UPTODOWN3 = 36,
    CAL30_UPTODOWN2 = 37,
    CAL30_UPTODOWN1 = 38,
    CAL30_UPTODOWNEMPTY = 39,
    CAL30_UPSHOOT = 40,
    CAL30_UPSHOOT8 = 41,
    CAL30_UPSHOOT7 = 42,
    CAL30_UPSHOOT6 = 43,
    CAL30_UPSHOOT5 = 44,
    CAL30_UPSHOOT4 = 45,
    CAL30_UPSHOOT3 = 46,
    CAL30_UPSHOOT2 = 47,
    CAL30_UPSHOOT1 = 48,
    CAL30_DOWNSHOOT = 49,
    CAL30_DOWNSHOOT8 = 50,
    CAL30_DOWNSHOOT7 = 51,
    CAL30_DOWNSHOOT6 = 52,
    CAL30_DOWNSHOOT5 = 53,
    CAL30_DOWNSHOOT4 = 54,
    CAL30_DOWNSHOOT3 = 55,
    CAL30_DOWNSHOOT2 = 56,
    CAL30_DOWNSHOOT1 = 57,
    CAL30_RELOAD = 58
};

enum BAR_e
{
    BAR_UP_IDLE = 0,
    BAR_UP_RELOA = 1,
    BAR_UP_DRAW = 2,
    BAR_UP_SHOOT = 3,
    BAR_UP_TO_DOWN = 4,
    BAR_DOWN_IDLE = 5,
    BAR_DOWN_RELOAD = 6,
    BAR_DOWN_SHOOT = 7,
    BAR_DOWN_TO_UP = 8
};

enum BREN_e
{
    BREN_UP_IDLE = 0,
    BREN_UP_RELOAD = 1,
    BREN_UP_DRAW = 2,
    BREN_UP_SHOOT = 3,
    BREN_UP_TO_DOWN = 4,
    BREN_DOWN_IDLE = 5,
    BREN_DOWN_RELOAD = 6,
    BREN_DOWN_SHOOT = 7,
    BREN_DOWN_TO_UP = 8
};

enum COLT_e
{
    COLT_IDLE = 0,
    COLT_SHOOT = 1,
    COLT_SHOOT2 = 2,
    COLT_RELOAD_EMPTY = 3,
    COLT_RELOAD = 4,
    COLT_DRAW = 5,
    COLT_SHOOT_EMPTY = 6,
    COLT_IDLE_EMPTY = 7
};

enum LUGER_e
{
    LUGER_IDLE = 0,
    LUGER_IDLE2 = 1,
    LUGER_IDLE3 = 2,
    LUGER_SHOOT = 3,
    LUGER_SHOOT_EMPTY = 4,
    LUGER_RELOAD_EMPTY = 5,
    LUGER_RELOAD = 6,
    LUGER_DRAW = 7,
    LUGER_IDLE_EMPTY = 8
};

enum WEBLEY_e
{
    WEBLEY_IDLE = 0,
    WEBLEY_SHOOT = 1,
    WEBLEY_RELOAD = 2,
    WEBLEY_DRAW = 3
};

enum ENFIELD_e
{
    ENFIELD_IDLE = 0,
    ENFIELD_SHOOT = 1,
    ENFIELD_RELOAD = 2,
    ENFIELD_DRAW = 3,
    ENFIELD_RELOADLONG = 4,
    ENFIELD_SWING = 5
};

enum SCOPEDENFIELD_e
{
    SCOPEDENFIELD_IDLE = 0,
    SCOPEDENFIELD_SHOOT = 1,
    SCOPEDENFIELD_RELOAD = 2,
    SCOPEDENFIELD_DRAW = 3,
    SCOPEDENFIELD_RELOADLONG = 4
};

enum FG42_e
{
    FG42_UP_IDLE = 0,
    FG42_UP_RELOAD = 1,
    FG42_UP_DRAW = 2,
    FG42_UP_SHOOT = 3,
    FG42_UP_TO_DOWN = 4,
    FG42_DOWN_IDLE = 5,
    FG42_DOWN_RELOAD = 6,
    FG42_DOWN_SHOOT = 7,
    FG42_DOWN_TO_UP = 8,
    FG42_LOWER_FOR_ZOOM = 9
};

enum GARAND_e
{
    GARAND_IDLE = 0,
    GARAND_SHOOT1 = 1,
    GARAND_SHOOT2 = 2,
    GARAND_SHOOT3 = 3,
    GARAND_SHOOT_EMPTY = 4,
    GARAND_RELOAD = 5,
    GARAND_DRAW = 6,
    GARAND_IDLE_EMPTY = 7,
    GARAND_DRAW_EMPTY = 8,
    GARAND_SMASH = 9,
    GARAND_SMASH_EMPTY = 10
};

enum K43_e
{
    K43_IDLE = 0,
    K43_SHOOT1 = 1,
    K43_SHOOT2 = 2,
    K43_RELOAD = 3,
    K43_DRAW = 4,
    K43_SHOOT_EMPTY = 5,
    K43_SMASH = 6
};

enum KAR_e
{
    KAR_IDLE = 0,
    KAR_SHOOT = 1,
    KAR_RELOAD = 2,
    KAR_DRAW = 3,
    KAR_SWING = 4,
    KAR_SLASH = 5
};

enum M1CARBINE_e
{
    M1CARBINE_IDLE = 0,
    M1CARBINE_RELOAD = 1,
    M1CARBINE_DRAW = 2,
    M1CARBINE_SHOOT = 3,
    M1CARBINE_FAST_DRAW = 4
};

enum MG34_e
{
    MG34_DOWNIDLE = 0,
    MG34_DOWNIDLEEMPTY = 1,
    MG34_DOWNTOUP = 2,
    MG34_DOWNTOUPEMPTY = 3,
    MG34_DOWNSHOOT = 4,
    MG34_DOWNSHOOTEMPTY = 5,
    MG34_UPIDLE = 6,
    MG34_UPIDLEEMPTY = 7,
    MG34_UPTODOWN = 8,
    MG34_UPTODOWNEMPTY = 9,
    MG34_UPSHOOT = 10,
    MG34_UPSHOOTEMPTY = 11,
    MG34_RELOAD = 12
};

enum MG42_e
{
    MG42_UPIDLE = 0,
    MG42_UPIDLE8 = 1,
    MG42_UPIDLE7 = 2,
    MG42_UPIDLE6 = 3,
    MG42_UPIDLE5 = 4,
    MG42_UPIDLE4 = 5,
    MG42_UPIDLE3 = 6,
    MG42_UPIDLE2 = 7,
    MG42_UPIDLE1 = 8,
    MG42_UPIDLEEMPTY = 9,
    MG42_DOWNIDLE = 10,
    MG42_DOWNIDLE8 = 11,
    MG42_DOWNIDLE7 = 12,
    MG42_DOWNIDLE6 = 13,
    MG42_DOWNIDLE5 = 14,
    MG42_DOWNIDLE4 = 15,
    MG42_DOWNIDLE3 = 16,
    MG42_DOWNIDLE2 = 17,
    MG42_DOWNIDLE1 = 18,
    MG42_DOWNIDLEEMPTY = 19,
    MG42_DOWNTOUP8 = 20,
    MG42_DOWNTOUP7 = 21,
    MG42_DOWNTOUP6 = 22,
    MG42_DOWNTOUP5 = 23,
    MG42_DOWNTOUP4 = 24,
    MG42_DOWNTOUP3 = 25,
    MG42_DOWNTOUP2 = 26,
    MG42_DOWNTOUP1 = 27,
    MG42_DOWNTOUPEMPTY = 28,
    MG42_UPTODOWN = 29,
    MG42_UPTODOWN8 = 30,
    MG42_UPTODOWN7 = 31,
    MG42_UPTODOWN6 = 32,
    MG42_UPTODOWN5 = 33,
    MG42_UPTODOWN4 = 34,
    MG42_UPTODOWN3 = 35,
    MG42_UPTODOWN2 = 36,
    MG42_UPTODOWN1 = 37,
    MG42_UPTODOWNEMPTY = 38,
    MG42_UPSHOOT = 39,
    MG42_UPSHOOT8 = 40,
    MG42_UPSHOOT7 = 41,
    MG42_UPSHOOT6 = 42,
    MG42_UPSHOOT5 = 43,
    MG42_UPSHOOT4 = 44,
    MG42_UPSHOOT3 = 45,
    MG42_UPSHOOT2 = 46,
    MG42_UPSHOOT1 = 47,
    MG42_DOWNSHOOT = 48,
    MG42_DOWNSHOOT8 = 49,
    MG42_DOWNSHOOT7 = 50,
    MG42_DOWNSHOOT6 = 51,
    MG42_DOWNSHOOT5 = 52,
    MG42_DOWNSHOOT4 = 53,
    MG42_DOWNSHOOT3 = 54,
    MG42_DOWNSHOOT2 = 55,
    MG42_DOWNSHOOT1 = 56,
    MG42_RELOAD = 57
};

enum MP40_e
{
    MP40_IDLE = 0,
    MP40_RELOAD = 1,
    MP40_DRAW = 2,
    MP40_SHOOT1 = 3,
    MP40_SHOOT2 = 4,
    MP40_IDLEEMPTY = 5
};

enum MP44_e
{
    MP44_IDLE = 0,
    MP44_RELOAD = 1,
    MP44_DRAW = 2,
    MP44_SHOOT1 = 3
};

enum SCOPEDKAR_e
{
    SCOPEDKAR_IDLE = 0,
    SCOPEDKAR_SHOOT = 1,
    SCOPEDKAR_RELOAD = 2,
    SCOPEDKAR_DRAW = 3,
    SCOPEDKAR_STABSLASH = 4,
    SCOPEDKAR_STAB = 5,
    SCOPEDKAR_LOWER_FOR_ZOOM = 6
};

enum SPRING_e
{
    SPRING_IDLE = 0,
    SPRING_SHOOT1 = 1,
    SPRING_SHOOT2 = 2,
    SPRING_RELOAD = 3,
    SPRING_DRAW = 4,
    SPRING_xxx = 5,
    SPRING_LOWER_FOR_ZOOM = 6
};

enum STEN_e
{
    STEN_IDLE = 0,
    STEN_RELOAD = 1,
    STEN_DRAW = 2,
    STEN_SHOOT = 3
};

enum THOMPSON_e
{
    THOMPSON_IDLE = 0,
    THOMPSON_RELOAD = 1,
    THOMPSON_DRAW = 2,
    THOMPSON_SHOOT1 = 3,
    THOMPSON_SHOOT2 = 4,
    THOMPSON_IDLE_EMPTY = 5,
    THOMPSON_PRONE_BACK = 6,
    THOMPSON_PRONE_FORWARD = 7,
    THOMPSON_PRONE_STILL = 8
};

enum BAZOOKA_e
{
    BAZOOKA_IDLE = 0,
    BAZOOKA_DRAW = 1,
    BAZOOKA_AIMED = 2,
    BAZOOKA_FIRE = 3,
    BAZOOKA_RAISE = 4,
    BAZOOKA_LOWER = 5,
    BAZOOKA_RELOAD_AIME = 6,
    BAZOOKA_RELOAD_IDLE = 7
};

enum PIAT_e
{
    PIAT_IDLE = 0,
    PIAT_IDLE_EMPTY = 1,
    PIAT_DRAW = 2,
    PIAT_DRAW_EMPTY = 3,
    PIAT_AIMED = 4,
    PIAT_AIMED_EMPTY = 5,
    PIAT_FIRE = 6,
    PIAT_RAISE = 7,
    PIAT_RAISE_EMPTY = 8,
    PIAT_LOWER = 9,
    PIAT_LOWER_EMPTY = 10,
    PIAT_RELOAD_IDLE = 11,
    PIAT_RELOAD_AIMED = 12
};

enum PSCHRECK_e
{
    PSCHRECK_IDLE = 0,
    PSCHRECK_DRAW = 1,
    PSCHRECK_AIMED = 2,
    PSCHRECK_FIRE = 3,
    PSCHRECK_RAISE = 4,
    PSCHRECK_LOWER = 5,
    PSCHRECK_RELOAD_AIMED = 6,
    PSCHRECK_RELOAD_IDLE = 7
};

enum Mortar_e
{
    MORTAR_DEPLOYED_ROUND_UP = 0,
    MORTAR_DEPLOYED_ROUND_DOWN = 1,
    MORTAR_DEPLOYED_ROUND_READY = 2,
    MORTAR_DEPLOYED_FIRE = 3,
    MORTAR_IDLE = 4,
    MORTAR_DEPLOY = 5,
    MORTAR_DEPLOYED_IDLE = 6,
    MORTAR_UNDEPLOY = 7,
    MORTAR_DRAW = 8
};

typedef struct DodClassInfo_s
{
    int team;
    int speed;
    int weapons;
    int weapons2;
    int numgrenades;
    int armskin;
    int headgroup;
    int helmetgroup;
    int geargroup;
    int bodygroup;
    int hairgroup;

    char selectcmd[32];
    char classname[128];
    char modelname[128];
    char limitcvar[32];
} DodClassInfo_t;

DodClassInfo_t g_ClassInfo[28];
DodClassInfo_t g_ParaClassInfo[21];

struct p_wpninfo_s
{
    char pmodel[32];
    char vmodel[32];
    char wmodel[32];
    char leftreload[32];
    char bipodup[32];
    char bipoddown[32];
    char sandbagreload[32];
    char pronereload[32];
    char szAnimExt[16];
    char szAnimReloadExt[16];

    float anim_reloadtime;
    float anim_firedelay;
    float anim_reloadtime2;
    float anim_firetime2;
    float anim_drawtime;
    float anim_up2downtime;
    float anim_down2uptime;
    float base_accuracy;
    float accuracy_penalty;
    float base_accuracy2;

    int ammo_default;
    int ammo_maxcarry;
    int ammo_maxclip;
    int ammo_give;
    int misc_weight;
} p_wpninfo_t;

p_wpninfo_s P_WpnInfo[41];

int iVoiceToHandSignal[29];

cvar_t *pcvar_classlimitcvars[28];

char weaponnames[64][64];

char *s_HandSignalSubtitles[25][3];
char *sHandSignals[30];
char *sHelmetModels[7];
char *sPlayerModelFiles[6];
char *s_BRITVoiceFiles[28];
char *s_GERVoiceFiles[29];
char *s_USVoiceFiles[28];
char *s_VoiceCommands[29][4];
char *szMapMarkerIcons[15];

#endif // DOD_SHARED_H