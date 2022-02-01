#pragma once
#if !defined(DOD_SHARED_H)
#define DOD_SHARED_H

// DOD weapon id
#define WEAPON_NONE				0
#define WEAPON_AMER_KNIFE		1
#define WEAPON_GER_KNIFE		2
#define	WEAPON_SPADE			19

// DOD weapon weight factors (for auto-switching)   (-1 = noswitch)
#define AMERKNIFE_WEIGHT	0
#define GERKNIFE_WEIGHT		0
#define SPADE_WEIGHT		0

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

#define AMERKNIFE_MAX_CLIP		WEAPON_NOCLIP
#define GERKNIFE_MAXCLIP		WEAPON_NOCLIP
#define SPADE_MAXCLIP			WEAPON_NOCLIP

// the default amount of ammo that comes with each gun when it spawns
#define MELEE_DEFAULT_GIVE     0

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

class CMeleeWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( int weapon_id );
	void Precache( void );
	int AddToplayer( CBasePlayer *pPlayer );
	BOOL Deploy( void );
	void Holster( int skiplocal );
	void PrimaryAttack( void );
	void Smack( void );
	void SwingAgain( void );
	int Swing( int fFirst );
	int Stab( int fFirst );
	void WeaponIdle( void );

    BOOL CanDrop( void ) { return FALSE; }
    int Classify( void ) { return 1; }
    int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
    void SecondaryAttack( void ) { ; }
    int ItemSlot( void ) { return 0; }
    BOOL UseDecrement( void ) { return TRUE; }

	TraceResult m_trHit;
	int m_iSwing;

private:
	unsigned short m_iFireEvent;
};

class CAmerKnife : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
	BOOL UseDecrement( void ) { return TRUE; }
};

class CSpade : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
    BOOL UseDecrement( void ) { return TRUE; }
};

class CGerKnife : public CMeleeWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int GetSlashAnim( int m_iSwing );
	int GetDrawAnim( void );
	int GetIdleAnim( void );
    BOOL UseDecrement( void ) { return TRUE; }
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
    BREN_DOWN_SHOO = 7,
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

class CPistol : public CBasePlayerWeapon
{
public:
    void Spawn( void );
    void PrimaryAttack( void );
    //BOOL Deploy( void );
    void Reload( void );
    void WeaponIdle( void );
    int GetIdleAnim( void );
    int GetDrawAnim( void );
    int GetReloadAnim( void );
    
    void SecondaryAttack( void ) { ; }
    BOOL CanHolster( void ) { return TRUE; }
    int iItemSlot( void ) { return 1; }
    BOOL UseDecrement( void ) { return TRUE; }
    int Classify( void ) { return 8; }
    BOOL CanDrop( void ) { return FALSE; }
};

class CColt : public CPistol
{

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

class CLuger : public CPistol
{
    
};

enum WEBLEY_e
{
    WEBLEY_IDLE = 0,
    WEBLEY_SHOOT = 1,
    WEBLEY_RELOAD = 2,
    WEBLEY_DRAW = 3
};

class CWEBLEY : public CPistol
{

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
    FG42_DOWN_SHOO = 7,
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
    MG42_DOWNTOUPMG42_DOWNTOUP8 = 20,
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

#endif // DOD_SHARED_H