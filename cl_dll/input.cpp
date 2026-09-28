//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// cl.input.c  -- builds an intended movement command to send to the server

//xxxxxx Move bob and pitch drifting code here and other stuff from view if needed

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
extern "C"
{
#include "kbutton.h"
}
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "view.h"
#include "dod_shared.h"
#include <string.h>
#include <ctype.h>

extern "C" 
{
	struct kbutton_s DLLEXPORT *KB_Find( const char *name );
	void DLLEXPORT CL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
	void DLLEXPORT HUD_ShutdownInput( void );
	int DLLEXPORT HUD_Key_Event( int eventcode, int keynum, const char *pszCurrentBinding );
}

int g_JoyForward;
int g_JoySide;

extern int g_iAlive;
extern int g_weaponselect;
extern int g_iMovetype;
extern cl_enginefunc_t gEngfuncs;

// Defined in pm_math.c
extern "C" float anglemod( float a );

void IN_Init( void );
void IN_Move( float frametime, usercmd_t *cmd );
void IN_Shutdown( void );
void V_Init( void );
void VectorAngles( const float *forward, float *angles );
int CL_ButtonBits( int );

// xxx need client dll function to get and clear impuse
extern cvar_t *in_joystick;

static float flPlaySprintSoundTime, flProneView;
static int flProneViewM;

int	in_impulse = 0;
int	in_cancel = 0;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;

cvar_t	*lookstrafe;
cvar_t	*lookspring;
cvar_t	*cl_pitchup;
cvar_t	*cl_pitchdown;
cvar_t	*cl_upspeed;

cvar_t  *cl_bulletejects;
cvar_t  *dodcredits;

cvar_t  *cl_hud_objectives;
cvar_t  *cl_hud_objtimer;
cvar_t  *cl_hud_reinforcements;
cvar_t  *cl_hud_health;
cvar_t  *cl_hud_ammo;
cvar_t  *cl_hud_msgs;

cvar_t  *cl_cutscenes;
cvar_t  *cl_dynamiclights;
cvar_t  *cl_identiconmode;
cvar_t  *cl_drawmodels;
cvar_t  *cl_drawplayermodels;;

cvar_t  *cl_dynamic_xhair;
cvar_t  *cl_xhair_style;

cvar_t  *cl_fog_density;
cvar_t  *cl_fog_start;
cvar_t  *cl_fog_end;
cvar_t  *cl_fog;
cvar_t  *cl_fog_red;
cvar_t  *cl_fog_green;
cvar_t  *cl_fog_blue;

cvar_t	*cl_forwardspeed;
cvar_t	*cl_backspeed;
cvar_t	*cl_sidespeed;
float   g_flSideSpeed;
cvar_t	*cl_movespeedkey;
cvar_t	*cl_yawspeed;
cvar_t	*cl_pitchspeed;
cvar_t	*cl_anglespeedkey;
cvar_t	*cl_vsmoothing;
cvar_t  *cl_particlefx;


/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/

kbutton_t	in_mlook;
kbutton_t	in_klook;
kbutton_t	in_jlook;
kbutton_t	in_left;
kbutton_t	in_right;
kbutton_t	in_forward;
kbutton_t	in_back;
kbutton_t	in_lookup;
kbutton_t	in_lookdown;
kbutton_t	in_moveleft;
kbutton_t	in_moveright;
kbutton_t	in_strafe;
kbutton_t	in_speed;
kbutton_t	in_use;
kbutton_t	in_jump;
kbutton_t	in_attack;
kbutton_t	in_attack2;
kbutton_t	in_special;
kbutton_t	in_up;
kbutton_t	in_down;
kbutton_t	in_duck;
kbutton_t	in_reload;
kbutton_t	in_alt1;
kbutton_t	in_score;
kbutton_t	in_break;
kbutton_t	in_graph;  // Display the netgraph
kbutton_t   in_map;
kbutton_t   in_mapzoom;

typedef struct kblist_s
{
	struct kblist_s *next;
	kbutton_t *pkey;
	char name[32];
} kblist_t;

kblist_t *g_kbkeys = NULL;

/*
============
KB_ConvertString

Removes references to +use and replaces them with the keyname in the output string.  If
 a binding is unfound, then the original text is retained.
NOTE:  Only works for text with +word in it.
============
*/
int KB_ConvertString( char *in, char **ppout )
{
	char sz[4096];
	char binding[64];
	char *p;
	char *pOut;
	char *pEnd;
	const char *pBinding;

	if( !ppout )
		return 0;

	*ppout = NULL;
	p = in;
	pOut = sz;
	while( *p )
	{
		if( *p == '+' )
		{
			pEnd = binding;
			while( *p && ( isalnum( *p ) || ( pEnd == binding ) ) && ( ( pEnd - binding ) < 63 ) )
			{
				*pEnd++ = *p++;
			}

			*pEnd = '\0';

			pBinding = NULL;
			if( binding[1] != '\0' )
			{
				// See if there is a binding for binding?
				pBinding = gEngfuncs.Key_LookupBinding( binding + 1 );
			}

			if( pBinding )
			{
				*pOut++ = '[';
				pEnd = (char *)pBinding;
			}
			else
			{
				pEnd = binding;
			}

			while( *pEnd )
			{
				*pOut++ = *pEnd++;
			}

			if( pBinding )
			{
				*pOut++ = ']';
			}
		}
		else
		{
			*pOut++ = *p++;
		}
	}

	*pOut = '\0';

	pOut = (char *)malloc( strlen( sz ) + 1 );
	strcpy( pOut, sz );
	*ppout = pOut;

	return 1;
}

/*
============
KB_Find

Allows the engine to get a kbutton_t directly ( so it can check +mlook state, etc ) for saving out to .cfg files
============
*/
struct kbutton_s DLLEXPORT *KB_Find( const char *name )
{
	kblist_t *p;
	p = g_kbkeys;
	while( p )
	{
		if( !stricmp( name, p->name ) )
			return p->pkey;

		p = p->next;
	}
	return NULL;
}

/*
============
KB_Add

Add a kbutton_t * to the list of pointers the engine can retrieve via KB_Find
============
*/
void KB_Add( const char *name, kbutton_t *pkb )
{
	kblist_t *p;	
	kbutton_t *kb;

	kb = KB_Find( name );

	if( kb )
		return;

	p = (kblist_t *)malloc( sizeof(kblist_t) );
	memset( p, 0, sizeof(*p) );

	strcpy( p->name, name );
	p->pkey = pkb;

	p->next = g_kbkeys;
	g_kbkeys = p;
}

/*
============
KB_Init

Add kbutton_t definitions that the engine can query if needed
============
*/
void KB_Init( void )
{
	g_kbkeys = NULL;

	KB_Add( "in_graph", &in_graph );
	KB_Add( "in_mlook", &in_mlook );
	KB_Add( "in_jlook", &in_jlook );
}

/*
============
KB_Shutdown

Clear kblist
============
*/
void KB_Shutdown( void )
{
	kblist_t *p, *n;
	p = g_kbkeys;
	while( p )
	{
		n = p->next;
		free( p );
		p = n;
	}
	g_kbkeys = NULL;
}

/*
============
KeyDown
============
*/
void KeyDown( kbutton_t *b )
{
	int	k;
	char	*c;

	c = gEngfuncs.Cmd_Argv( 1 );
	if( c[0] )
		k = atoi( c );
	else
		k = -1;		// typed manually at the console for continuous down

	if( k == b->down[0] || k == b->down[1] )
		return;		// repeating key
	
	if( !b->down[0] )
		b->down[0] = k;
	else if( !b->down[1] )
		b->down[1] = k;
	else
	{
		gEngfuncs.Con_DPrintf( "Three keys down for a button '%c' '%c' '%c'!\n", b->down[0], b->down[1], c );
		return;
	}
	
	if( b->state & 1 )
		return;		// still down
	b->state |= 1 + 2;	// down + impulse down
}

/*
============
ForceKeyUp
============
*/
void ForceKeyUp( kbutton_t *b )
{
	int state = b->state;
	b->down[0] = 0;
	b->down[1] = 0;

	if( state & 1 )
		b->state = ( state & ~1 ) | 4;
}

/*
============
KeyUp
============
*/
void KeyUp( kbutton_t *b )
{
	int	k;
	char	*c;
	
	c = gEngfuncs.Cmd_Argv( 1 );
	if( c[0] )
		k = atoi(c);
	else
	{
		// typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	if( b->down[0] == k )
		b->down[0] = 0;
	else if( b->down[1] == k )
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	if( b->down[0] || b->down[1] )
	{
		//Con_Printf ( "Keys down for button: '%c' '%c' '%c' (%d,%d,%d)!\n", b->down[0], b->down[1], c, b->down[0], b->down[1], c );
		return;		// some other key is still holding it down
	}

	if( !( b->state & 1 ) )
		return;		// still up (this should not happen)

	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

/*
============
HUD_Key_Event

Return 1 to allow engine to process the key, otherwise, act on it as needed
============
*/
int DLLEXPORT HUD_Key_Event( int down, int keynum, const char *pszCurrentBinding )
{
	return 1;
}

bool DoDAreWeAlive( void )
{
	if( g_iAlive )
		return ( g_iUser1 == 0 );

	return false;
}

bool IsAttackPressed( void )
{
	return in_attack.state & 1;
}

bool DoDAreWeAttacking( void )
{
	if( IsAttackPressed() )
		return true;

	return ( in_attack2.state & 1 ) != 0;
}

bool DoDAreWeMoving( void )
{
	return ( gHUD.m_vecVelocity.Length() > 0.0f );
}

extern float i_ProneCounter, g_fStamina;

float DoDGetSpeed( void )
{
	float maxspd = gEngfuncs.GetClientMaxspeed();

	if( maxspd < 10.0f )
		return maxspd;

	if( !DoDAreWeAlive() )
		return 350.0f;

	if( !g_iUser2 )
	{
		float flTargetSpeed = 330.0f;
		if( in_speed.state & 1 )
		{
			flTargetSpeed = 220.0f;
		}

		float spd = flTargetSpeed;

		if( DoDAreWeMoving() && ( in_attack.state & 1 ) != 0 )
		{
			int weaponId = gHUD.GetCurrentWeaponId();

			if( weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_MG34 )
			{
				if( gHUD.m_iClipSize > 0 )
				{
					if( maxspd > 50.0f )
						spd = -50.0f;
					else
						spd = maxspd;
				}
			}
		}

		if( spd != -50.0f )
		{
			if( i_ProneCounter <= gHUD.m_flTime || g_iUser3 )
			{
				if( gHUD.IsInMGDeploy() )
					spd = -99.0f;
				else
					spd = spd - 100.0f;
			}
			else
			{
				float percent = i_ProneCounter / 1.5f;
				int newspd = spd - ( percent * spd );
				spd = ( float ) newspd;
			}
		}

		float flStaminaModifier = 100.0f;

		if( g_fStamina <= 100.0f )
			flStaminaModifier = g_fStamina;

		maxspd = spd + flStaminaModifier;
	}

	if( maxspd >= 0.0f )
		return maxspd;

	return 0.0f;
}

int g_iinjump;

bool DoDHandleButtons( int ButtonPressed )
{
	if( DoDAreWeAlive() )
	{
		int weaponId = gHUD.GetCurrentWeaponId();
		return true;
	}

	return false;
}

void IN_BreakDown( void )
{
	KeyDown( &in_break );
}

void IN_BreakUp( void )
{
	KeyUp( &in_break );
}

void IN_KLookDown( void )
{
	KeyDown( &in_klook );
}

void IN_KLookUp( void )
{
	KeyUp( &in_klook );
}

void IN_JLookDown( void )
{
	KeyDown( &in_jlook );
}

void IN_JLookUp( void )
{
	KeyUp( &in_jlook );
}

void IN_MLookDown( void )
{
	KeyDown( &in_mlook );
}

void IN_UpDown( void )
{
	if( !DoDAreWeAlive() || !gHUD.IsInMGDeploy() )
		KeyDown( &in_up );
}


void IN_UpUp( void )
{
	KeyUp( &in_up );
}

void IN_DownDown( void )
{
	if( !DoDAreWeAlive() || !gHUD.IsInMGDeploy() )
		KeyDown( &in_down );
}

extern int g_iDeadFlag;

void IN_MapUp( void )
{
	KeyUp( &in_map );
}

void IN_MapDown( void )
{
	KeyDown( &in_map );
	gHUD.m_DoDMap.HandleMapButton();
}

void IN_MapZoomUp( void )
{
	KeyUp( &in_mapzoom );
}

void IN_MapZoomDown( void )
{
	KeyDown( &in_mapzoom );
	gHUD.m_DoDMap.HandleMapZoomButton();
}

void IN_DownUp( void )
{
	KeyUp( &in_down );
}

void IN_LeftDown( void )
{
	if( !DoDHandleButtons( IN_LEFT ) )
		KeyDown( &in_left );
}

void IN_LeftUp( void )
{
	KeyUp( &in_left );
}

void IN_RightDown( void )
{
	if( !DoDHandleButtons( IN_RIGHT ) )
		KeyDown( &in_right );
}

void IN_RightUp( void )
{
	KeyUp( &in_right );
}

void IN_ForwardDown( void )
{
	if( !DoDHandleButtons( IN_FORWARD ) )
		KeyDown( &in_forward );

	gHUD.m_Spectator.HandleButtonsDown( IN_FORWARD );
}

void IN_ForwardUp( void )
{
	KeyUp( &in_forward );
	gHUD.m_Spectator.HandleButtonsUp( IN_FORWARD );
}

void IN_BackDown( void )
{
	if( !DoDHandleButtons( IN_BACK ) )
		KeyDown( &in_back );

	gHUD.m_Spectator.HandleButtonsDown( IN_BACK );
}

void IN_BackUp( void )
{
	KeyUp( &in_back );
	gHUD.m_Spectator.HandleButtonsUp( IN_BACK );
}

void IN_LookupDown( void )
{
	if( !DoDAreWeAlive() || ( !gHUD.IsInMGDeploy() && ( in_attack.state & 1 ) == 0 ) )
		KeyDown( &in_lookup );
}

void IN_LookupUp( void )
{
	KeyUp( &in_lookup );
}

void IN_LookdownDown( void )
{
	if( !DoDAreWeAlive() || ( !gHUD.IsInMGDeploy() && ( in_attack.state & 1 ) == 0 ) )
		KeyDown( &in_lookdown );
}

void IN_LookdownUp( void )
{
	KeyUp( &in_lookdown );
}

void IN_MoveleftDown( void )
{
	if( !DoDAreWeAlive() || !gHUD.IsInMGDeploy() )
	{
		KeyDown( &in_moveleft );
	}
	else
	{
		gHUD.m_Spectator.HandleButtonsDown( IN_MOVELEFT );
	}
}

void IN_MoveleftUp( void )
{
	KeyUp( &in_moveleft );
	gHUD.m_Spectator.HandleButtonsUp( IN_MOVELEFT );
}

void IN_MoverightDown( void )
{
	if( !DoDAreWeAlive() || !gHUD.IsInMGDeploy() )
	{
		KeyDown( &in_moveright );
	}
	else
	{
		gHUD.m_Spectator.HandleButtonsDown( IN_MOVERIGHT );
	}
}

void IN_MoverightUp( void )
{
	KeyUp( &in_moveright );
	gHUD.m_Spectator.HandleButtonsUp( IN_MOVERIGHT );
}

void IN_SpeedDown( void )
{
	if( DoDHandleButtons( IN_RUN ) )
		return;

	KeyDown( &in_speed );

	if( g_iUser1 )
	{
		gHUD.i_specmenutoggle++;

		if( !gHUD.m_bInfiniteLives && ( gHUD.i_specmenutoggle - 2 ) <= 1 )
			gHUD.i_specmenutoggle = 4;
		else if( gHUD.i_specmenutoggle > 4 )
			gHUD.i_specmenutoggle = 0;
	}
}

void IN_SpeedUp( void )
{
	KeyUp( &in_speed );
}

void IN_StrafeDown( void )
{
	KeyDown( &in_strafe );
}

void IN_StrafeUp( void )
{
	KeyUp( &in_strafe );
}

// needs capture by hud/vgui also
extern void __CmdFunc_InputPlayerSpecial( void );

void IN_Attack2Down( void )
{
	if( !DoDHandleButtons( IN_ATTACK2 ) )
		KeyDown( &in_attack2 );
	else
		gHUD.m_Spectator.HandleButtonsDown( IN_ATTACK2 );
}

void IN_Attack2Up( void )
{
	KeyUp( &in_attack2 );
}

void IN_UseDown( void )
{
	KeyDown( &in_use );
	gHUD.m_Spectator.HandleButtonsDown( IN_USE );
}

void IN_UseUp( void )
{
	KeyUp( &in_use );
}

void IN_JumpDown( void )
{
	if( !DoDHandleButtons( IN_JUMP ) )
		KeyDown( &in_jump );
	else
		gHUD.m_Spectator.HandleButtonsDown( IN_JUMP );
}

void IN_JumpUp( void )
{
	KeyUp( &in_jump );
}

void IN_DuckDown( void )
{
	if( !DoDHandleButtons( IN_DUCK ) )
		KeyDown( &in_duck );
	else
		gHUD.m_Spectator.HandleButtonsDown( IN_DUCK );
}

void IN_DuckUp( void )
{
	KeyUp( &in_duck );
}

void IN_ReloadDown( void )
{
	if( !DoDHandleButtons( IN_RELOAD ) )
		KeyDown( &in_reload );
}

void IN_ReloadUp( void )
{
	KeyUp( &in_reload );
}

void IN_Alt1Down( void )
{
	KeyDown( &in_alt1 );
}

void IN_Alt1Up( void )
{
	KeyUp( &in_alt1 );
}

void IN_GraphDown( void )
{
	KeyDown( &in_graph );
}

void IN_GraphUp( void )
{
	KeyUp( &in_graph );
}

void IN_AttackDown( void )
{
	if( !DoDHandleButtons( IN_ATTACK ) )
		KeyDown( &in_attack );
	else
		gHUD.m_Spectator.HandleButtonsDown( IN_ATTACK );
}

void IN_AttackUp( void )
{
	KeyUp( &in_attack );
	in_cancel = 0;
}

void IN_SpecialDown( void )
{
	KeyDown( &in_special );
}

void IN_SpecialUp( void )
{
	KeyUp( &in_special );
}

// Special handling
void IN_Cancel( void )
{
	in_cancel = 1;
}

void IN_Impulse( void )
{
	in_impulse = atoi( gEngfuncs.Cmd_Argv( 1 ) );
}

void IN_ScoreDown( void )
{
	KeyDown( &in_score );

	gHUD.m_Scoreboard.UserCmd_ShowScores();
}

void IN_ScoreUp( void )
{
	KeyUp( &in_score );

	gHUD.m_Scoreboard.UserCmd_HideScores();
}

void IN_MLookUp( void )
{
	KeyUp( &in_mlook );
}

/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CL_KeyState( kbutton_t *key )
{
	float		val = 0.0;
	int		impulsedown, impulseup, down;

	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;

	if( impulsedown && !impulseup )
	{
		// pressed and held this frame?
		val = down ? 0.5 : 0.0;
	}

	if( impulseup && !impulsedown )
	{
		// released this frame?
		val = down ? 0.0 : 0.0;
	}

	if( !impulsedown && !impulseup )
	{
		// held the entire frame?
		val = down ? 1.0 : 0.0;
	}

	if( impulsedown && impulseup )
	{
		if( down )
		{
			// released and re-pressed this frame
			val = 0.75;	
		}
		else
		{
			// pressed and released this frame
			val = 0.25;	
		}
	}

	// clear impulses
	key->state &= 1;		
	return val;
}

extern int g_iWeaponFlags;

void GetWeaponRecoilAmount( int weaponId, float *const flPitchRecoil, float *const flYawRecoil )
{
	*flPitchRecoil = 0.0f;
	*flYawRecoil = 0.4f;

	switch( weaponId )
	{
	case WEAPON_COLT:
	case WEAPON_LUGER:
	case WEAPON_WEBLEY:
		*flPitchRecoil = 0.2f;
		break;
	case WEAPON_GARAND:
	case WEAPON_KAR:
		*flPitchRecoil = 1.5f;
		break;
	case WEAPON_SCOPEDKAR:
	case WEAPON_SPRING:
	case WEAPON_BAR:
	case WEAPON_MG42:
	case WEAPON_CAL30:
	case WEAPON_MG34:
	case WEAPON_FG42:
	case WEAPON_BREN:
		*flPitchRecoil = 1.0f;
		break;
	case WEAPON_THOMPSON:
	case WEAPON_MP40:
	case WEAPON_GREASEGUN:
	case WEAPON_STEN:
		*flPitchRecoil = 0.4f;
		break;
	case WEAPON_MP44:
		*flPitchRecoil = 0.6f;
		break;
	case WEAPON_M1CARBINE:
		*flPitchRecoil = 0.5f;
		break;
	case WEAPON_K43:
		*flPitchRecoil = 1.3f;
		break;
	case WEAPON_ENFIELD:
		if( g_iWeaponFlags & WPNSTATE_SCOPED )
			*flPitchRecoil = 1.0f;
		else
			*flPitchRecoil = 1.5f;
		break;
	case WEAPON_BAZOOKA:
	case WEAPON_PSCHRECK:
	case WEAPON_PIAT:
		*flPitchRecoil = 2.0f;
		break;
	default:
		*flPitchRecoil = 0.0f;
		*flYawRecoil = 0.0f;
		break;
	}

	if( gHUD.IsInMGDeploy() &&
		( weaponId == WEAPON_MG42 || weaponId == WEAPON_CAL30 || weaponId == WEAPON_FG42 ||
			weaponId == WEAPON_BAR || weaponId == WEAPON_BREN || weaponId == WEAPON_MG34 ) )
	{
		*flPitchRecoil = 0.0f;
		*flYawRecoil = 0.0f;
	}
	else
	{
		float flModifier = 1.0f;

		if( gHUD.IsProne() && weaponId != WEAPON_MG42 && weaponId != WEAPON_CAL30 && weaponId != WEAPON_MG34 )
			flModifier = 0.25f;

		else if( in_duck.state & 1 )
			flModifier = 0.5f;

		*flPitchRecoil *= flModifier;
		*flYawRecoil *= flModifier;
	}
}

/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
void CL_AdjustAngles( float frametime, float *viewangles )
{
	if( !DoDAreWeAlive() )
		return;

	if( ( in_attack.state & 1 ) && ( in_attack2.state & 1 ) )
	{
		KeyUp( &in_attack );
		in_cancel = 0;
		KeyUp( &in_attack2 );
	}

	int weaponId = gHUD.GetCurrentWeaponId();
	float speed;

	if( in_speed.state & 1 )
		speed = frametime * cl_anglespeedkey->value;
	else
		speed = frametime;

	if( !( in_strafe.state & 1 ) )
	{
		viewangles[YAW] -= speed * cl_yawspeed->value * CL_KeyState( &in_right );
		viewangles[YAW] += speed * cl_yawspeed->value * CL_KeyState( &in_left );
		viewangles[YAW] = anglemod( viewangles[YAW] );
	}

	if( in_klook.state & 1 )
	{
		V_StopPitchDrift();
		viewangles[PITCH] -= speed * cl_pitchspeed->value * CL_KeyState( &in_forward );
		viewangles[PITCH] += speed * cl_pitchspeed->value * CL_KeyState( &in_back );
	}

	float up = CL_KeyState( &in_lookup );
	float down = CL_KeyState( &in_lookdown );

	viewangles[PITCH] -= speed * cl_pitchspeed->value * up;
	viewangles[PITCH] += speed * cl_pitchspeed->value * down;

	if( up != 0.0f || down != 0.0f )
		V_StopPitchDrift();

	if( viewangles[PITCH] > cl_pitchdown->value )
	{
		viewangles[PITCH] = cl_pitchdown->value;
	}
	if( viewangles[PITCH] < -cl_pitchup->value )
	{
		viewangles[PITCH] = -cl_pitchup->value;
	}

	if( viewangles[ROLL] > 50.0f )
	{
		viewangles[ROLL] = 50.0f;
	}
	if( viewangles[ROLL] < -50.0f )
	{
		viewangles[ROLL] = -50.0f;
	}

	if( DoDAreWeAlive() && ( gHUD.m_iHideHUDDisplay & 5 ) == 0 && !g_iUser2 )
	{
		if( DoDAreWeMoving() && g_iUser3 > 0 && g_iVuser1x != 2 )
		{
			if( flProneView >= 2.5f )
			{
				flProneViewM = 0;
			}
			else if( flProneView <= -2.5f )
			{
				flProneViewM = 1;
			}

			if( flProneViewM == 0 )
			{
				viewangles[YAW] -= 0.1f;
				flProneView -= 0.1f;
			}
			else
			{
				viewangles[YAW] += 0.1f;
				flProneView += 0.1f;
			}
		}
		else
		{
			if( flProneView != 0.0f && ( g_iUser3 <= 0 || g_iVuser1x == 2 ) )
			{
				if( flProneView < 0.0f && flProneView > -0.1f )
				{
					flProneView = 0.0f;
				}
				else if( flProneView > 0.0f && flProneView < 0.1f )
				{
					flProneView = 0.0f;
				}
				else if( flProneView > 0.0f )
				{
					viewangles[YAW] -= 0.1f;
					flProneView -= 0.1f;
				}
				else if( flProneView < 0.0f )
				{
					viewangles[YAW] += 0.1f;
					flProneView += 0.1f;
				}
			}
		}

		if( gHUD.m_iFOV <= 89 && weaponId != WEAPON_BINOC && weaponId != WEAPON_BINOCULARS && !g_iVuser1z )
		{
			float t = gEngfuncs.GetClientTime();

			float x = cosf( t ) * frametime;
			float y = cosf( t + t ) * ( frametime + frametime );

			float scale = 0.2f;
			if( !( in_duck.state & 1 ) )
			{
				scale = ( g_iUser3 <= 0 ) ? 0.5f : 0.1f;
			}

			viewangles[PITCH] += y * scale;
			viewangles[YAW] += x * scale;
		}

		float flPitchRecoil = 0.0f;
		float flYawRecoil = 0.0f;

		gHUD.PopRecoil( frametime, flPitchRecoil, flYawRecoil );

		if( flPitchRecoil > 0.0f )
		{
			viewangles[PITCH] -= flPitchRecoil;
			viewangles[YAW] += flYawRecoil;
		}
	}
}

/*
================
CL_CreateMove

Send the intended movement message to the server
if active == 1 then we are 1) not playing back demos ( where our commands are ignored ) and
2 ) we have finished signing on to server
================
*/
void DLLEXPORT CL_CreateMove( float frametime, struct usercmd_s *cmd, int active )
{
	int weaponId = gHUD.GetCurrentWeaponId();
	static vec3_t oldangles;

	bool bIsWeaponAGrenade = ( weaponId == WEAPON_HANDGRENADE || weaponId == WEAPON_STICKGRENADE ||
		weaponId == WEAPON_STICKGRENADEX || weaponId == WEAPON_HANDGRENADEX );

	if( DoDAreWeAlive() && !bIsWeaponAGrenade )
	{
		bool bIsMovingInProne = ( g_iUser3 && ( ( in_forward.state & 1 ) || ( in_back.state & 1 ) ||
			( in_left.state & 1 ) || ( in_right.state & 1 ) ||
			( in_moveleft.state & 1 ) || ( in_moveright.state & 1 ) ) );

		if( ( i_ProneCounter > gHUD.m_flTime ) || ( in_speed.state & 1 ) || ( in_jump.state & 1 ) ||
			g_iinjump || bIsMovingInProne || ( g_iMovetype == 5 ) )
		{
			if( in_attack.state & 1 )
				in_attack.down[1] = 0; in_attack.down[0] = 0; in_attack.state = 4;

			if( in_attack2.state & 1 )
				in_attack2.down[1] = 0; in_attack2.down[0] = 0; in_attack2.state = 4;

			if( in_reload.state & 1 )
				in_reload.down[1] = 0; in_reload.down[0] = 0; in_reload.state = 4;

		}
	}

	if( active )
	{
		vec3_t viewangles;

		gEngfuncs.GetViewAngles( ( float * ) viewangles );

		CL_AdjustAngles( frametime, viewangles );
		memset( cmd, 0, sizeof( *cmd ) );

		gEngfuncs.SetViewAngles( ( float * ) viewangles );

		float spd = DoDGetSpeed();

		if( !DoDAreWeAlive() )
		{
			cmd->forwardmove = 400.0f * CL_KeyState( &in_forward );
			cmd->forwardmove -= 400.0f * CL_KeyState( &in_back );

			cmd->sidemove = 400.0f * CL_KeyState( &in_moveright );
			cmd->sidemove -= 400.0f * CL_KeyState( &in_moveleft );

			cmd->upmove = cl_upspeed->value * CL_KeyState( &in_up );
			cmd->upmove -= cl_upspeed->value * CL_KeyState( &in_down );
		}
		else if( !g_iUser2 )
		{
			if( in_speed.state & 1 )
			{
				float flStrafeWalkSpeed = spd - 45.0f;

				cmd->sidemove = flStrafeWalkSpeed * CL_KeyState( &in_moveright );
				cmd->sidemove -= flStrafeWalkSpeed * CL_KeyState( &in_moveleft );

				cmd->forwardmove = spd * CL_KeyState( &in_forward );
				cmd->forwardmove -= spd * CL_KeyState( &in_back );

				cmd->upmove = CL_KeyState( &in_up ) * spd;
				cmd->upmove -= CL_KeyState( &in_down ) * spd;
			}
			else
			{
				cmd->sidemove = spd * CL_KeyState( &in_moveright );
				cmd->sidemove -= spd * CL_KeyState( &in_moveleft );

				cmd->forwardmove = spd * CL_KeyState( &in_forward );
				cmd->forwardmove -= spd * CL_KeyState( &in_back );

				cmd->upmove = cl_upspeed->value * CL_KeyState( &in_up );
				cmd->upmove -= cl_upspeed->value * CL_KeyState( &in_down );
			}
		}

		if( spd != 0.0f )
		{
			float fmov = sqrt( ( cmd->forwardmove * cmd->forwardmove ) + ( cmd->sidemove * cmd->sidemove ) + ( cmd->upmove * cmd->upmove ) );

			if( fmov > spd )
			{
				float fratio = spd / fmov;
				cmd->forwardmove *= fratio;
				cmd->sidemove *= fratio;
				cmd->upmove *= fratio;
			}
		}

		IN_Move( frametime, cmd );
	}

	cmd->impulse = in_impulse;
	in_impulse = 0;
	cmd->weaponselect = g_weaponselect;
	g_weaponselect = 0;
	cmd->buttons = CL_ButtonBits( 1 );

	CVoiceStatus *ClientVoice = GetClientVoice();
	if( ClientVoice && ClientVoice->IsInSquelchMode() )
		cmd->buttons &= ~IN_ATTACK;

	if( in_joystick->value )
	{
		if( cmd->forwardmove > 0 )
			cmd->buttons |= IN_FORWARD;
		else if( cmd->forwardmove < 0 )
			cmd->buttons |= IN_BACK;
	}

	vec3_t final_angles;
	gEngfuncs.GetViewAngles( ( float * ) final_angles );

	if( g_iDeadFlag && ( !g_iUser1 || g_iUser3 ) )
	{
		VectorCopy( oldangles, cmd->viewangles );
	}
	else
	{
		VectorCopy( final_angles, cmd->viewangles );
		VectorCopy( final_angles, oldangles );
	}
}

/*
============
CL_IsDead

Returns 1 if health is <= 0
============
*/
int CL_IsDead( void )
{
	return gHUD.m_Icons.m_iHealth <= 0;;
}

/*
============
CL_ButtonBits

Returns appropriate button info for keyboard and mouse state
Set bResetState to 1 to clear old state info
============
*/
int CL_ButtonBits( int bResetState )
{
	int bits = 0;

	if( in_attack.state & 3 )
	{
		if( gHUD.m_MOTD.m_bShow )
			gHUD.m_MOTD.Reset();
		else
			bits |= IN_ATTACK;
	}

	if( in_duck.state & 3 )			bits |= IN_DUCK;
	if( in_jump.state & 3 )			bits |= IN_JUMP;
	if( in_forward.state & 3 )		bits |= IN_FORWARD;
	if( in_back.state & 3 )			bits |= IN_BACK;
	if( in_use.state & 3 )			bits |= IN_USE;
	if( in_cancel )					bits |= IN_CANCEL;
	if( in_left.state & 3 )			bits |= IN_LEFT;
	if( in_right.state & 3 )		bits |= IN_RIGHT;
	if( in_moveleft.state & 3 )		bits |= IN_MOVELEFT;
	if( in_moveright.state & 3 )	bits |= IN_MOVERIGHT;
	if( in_attack2.state & 3 )		bits |= IN_ATTACK2;
	if( in_reload.state & 3 )		bits |= IN_RELOAD;
	if( in_alt1.state & 3 )			bits |= IN_ALT1;
	if( in_score.state & 3 )		bits |= IN_SCORE;
	if( in_speed.state & 3 )		bits |= IN_RUN;

	if( CL_IsDead() || gHUD.m_iIntermission )
		bits |= IN_SCORE;

	if( g_JoyForward )				bits |= IN_FORWARD;
	if( g_JoySide )					bits |= IN_MOVELEFT;

	if( bResetState )
	{
		in_map.state &= ~2;
		in_special.state &= ~2;
		in_attack.state &= ~2;
		in_moveright.state &= ~2;
		in_attack2.state &= ~2;
		in_reload.state &= ~2;
		in_duck.state &= ~2;
		in_alt1.state &= ~2;
		in_score.state &= ~2;
		in_jump.state &= ~2;
		in_forward.state &= ~2;
		in_back.state &= ~2;
		in_use.state &= ~2;
		in_left.state &= ~2;
		in_right.state &= ~2;
		in_moveleft.state &= ~2;
		in_speed.state &= ~2;
	}

	return bits;
}

/*
============
CL_ResetButtonBits

============
*/
void CL_ResetButtonBits( int bits )
{
	int bitsNew = CL_ButtonBits( 0 ) ^ bits;

	// Has the attack button been changed
	if( bitsNew & IN_ATTACK )
	{
		// Was it pressed? or let go?
		if( bits & IN_ATTACK )
		{
			KeyDown( &in_attack );
		}
		else
		{
			// totally clear state
			in_attack.state &= ~7;
		}
	}
}

/*
============
InitInput
============
*/
void InitInput( void )
{
	gEngfuncs.pfnAddCommand( "+moveup", IN_UpDown );
	gEngfuncs.pfnAddCommand( "-moveup", IN_UpUp );
	gEngfuncs.pfnAddCommand( "+movedown", IN_DownDown );
	gEngfuncs.pfnAddCommand( "-movedown", IN_DownUp );
	gEngfuncs.pfnAddCommand( "+left", IN_LeftDown );
	gEngfuncs.pfnAddCommand( "-left", IN_LeftUp );
	gEngfuncs.pfnAddCommand( "+right", IN_RightDown );
	gEngfuncs.pfnAddCommand( "-right", IN_RightUp );
	gEngfuncs.pfnAddCommand( "+forward", IN_ForwardDown );
	gEngfuncs.pfnAddCommand( "-forward", IN_ForwardUp );
	gEngfuncs.pfnAddCommand( "+back", IN_BackDown );
	gEngfuncs.pfnAddCommand( "-back", IN_BackUp );
	gEngfuncs.pfnAddCommand( "+lookup", IN_LookupDown );
	gEngfuncs.pfnAddCommand( "-lookup", IN_LookupUp );
	gEngfuncs.pfnAddCommand( "+lookdown", IN_LookdownDown );
	gEngfuncs.pfnAddCommand( "-lookdown", IN_LookdownUp );
	gEngfuncs.pfnAddCommand( "+strafe", IN_StrafeDown );
	gEngfuncs.pfnAddCommand( "-strafe", IN_StrafeUp );
	gEngfuncs.pfnAddCommand( "+moveleft", IN_MoveleftDown );
	gEngfuncs.pfnAddCommand( "-moveleft", IN_MoveleftUp );
	gEngfuncs.pfnAddCommand( "+moveright", IN_MoverightDown );
	gEngfuncs.pfnAddCommand( "-moveright", IN_MoverightUp );
	gEngfuncs.pfnAddCommand( "+speed", IN_SpeedDown );
	gEngfuncs.pfnAddCommand( "-speed", IN_SpeedUp );
	gEngfuncs.pfnAddCommand( "+attack", IN_AttackDown );
	gEngfuncs.pfnAddCommand( "-attack", IN_AttackUp );
	gEngfuncs.pfnAddCommand( "+attack2", IN_Attack2Down );
	gEngfuncs.pfnAddCommand( "-attack2", IN_Attack2Up );
	gEngfuncs.pfnAddCommand( "+use", IN_UseDown );
	gEngfuncs.pfnAddCommand( "-use", IN_UseUp );
	gEngfuncs.pfnAddCommand( "+jump", IN_JumpDown );
	gEngfuncs.pfnAddCommand( "-jump", IN_JumpUp );
	gEngfuncs.pfnAddCommand( "impulse", IN_Impulse );
	gEngfuncs.pfnAddCommand( "+klook", IN_KLookDown );
	gEngfuncs.pfnAddCommand( "-klook", IN_KLookUp );
	gEngfuncs.pfnAddCommand( "+mlook", IN_MLookDown );
	gEngfuncs.pfnAddCommand( "-mlook", IN_MLookUp );
	gEngfuncs.pfnAddCommand( "+jlook", IN_JLookDown );
	gEngfuncs.pfnAddCommand( "-jlook", IN_JLookUp );
	gEngfuncs.pfnAddCommand( "+duck", IN_DuckDown );
	gEngfuncs.pfnAddCommand( "-duck", IN_DuckUp );
	gEngfuncs.pfnAddCommand( "+reload", IN_ReloadDown );
	gEngfuncs.pfnAddCommand( "-reload", IN_ReloadUp );
	gEngfuncs.pfnAddCommand( "+alt1", IN_Alt1Down );
	gEngfuncs.pfnAddCommand( "-alt1", IN_Alt1Up );
	gEngfuncs.pfnAddCommand( "+score", IN_ScoreDown );
	gEngfuncs.pfnAddCommand( "-score", IN_ScoreUp );
	gEngfuncs.pfnAddCommand( "+showscores", IN_ScoreDown );
	gEngfuncs.pfnAddCommand( "-showscores", IN_ScoreUp );
	gEngfuncs.pfnAddCommand( "+graph", IN_GraphDown );
	gEngfuncs.pfnAddCommand( "-graph", IN_GraphUp );
	gEngfuncs.pfnAddCommand( "+break", IN_BreakDown );
	gEngfuncs.pfnAddCommand( "-break", IN_BreakUp );

	gEngfuncs.pfnAddCommand( "+special", IN_SpecialDown );
	gEngfuncs.pfnAddCommand( "-special", IN_SpecialUp );
	gEngfuncs.pfnAddCommand( "+map", IN_MapDown );
	gEngfuncs.pfnAddCommand( "-map", IN_MapUp );
	gEngfuncs.pfnAddCommand( "+mapzoom", IN_MapZoomDown );
	gEngfuncs.pfnAddCommand( "-mapzoom", IN_MapZoomUp );

	lookstrafe = gEngfuncs.pfnRegisterVariable( "lookstrafe", "0", FCVAR_ARCHIVE );
	lookspring = gEngfuncs.pfnRegisterVariable( "lookspring", "0", FCVAR_ARCHIVE );
	cl_anglespeedkey = gEngfuncs.pfnRegisterVariable( "cl_anglespeedkey", "0.67", 0 );
	cl_yawspeed = gEngfuncs.pfnRegisterVariable( "cl_yawspeed", "210", 0 );
	cl_pitchspeed = gEngfuncs.pfnRegisterVariable( "cl_pitchspeed", "225", 0 );
	cl_upspeed = gEngfuncs.pfnRegisterVariable( "cl_upspeed", "320", 0 );
	cl_forwardspeed = gEngfuncs.pfnRegisterVariable( "cl_forwardspeed", "400", FCVAR_ARCHIVE );
	cl_backspeed = gEngfuncs.pfnRegisterVariable( "cl_backspeed", "400", FCVAR_ARCHIVE );
	cl_sidespeed = gEngfuncs.pfnRegisterVariable( "cl_sidespeed", "400", 0 );
	cl_movespeedkey = gEngfuncs.pfnRegisterVariable( "cl_movespeedkey", "0.3", 0 );
	cl_pitchup = gEngfuncs.pfnRegisterVariable( "cl_pitchup", "89", 0 );
	cl_pitchdown = gEngfuncs.pfnRegisterVariable( "cl_pitchdown", "89", 0 );
	cl_vsmoothing = gEngfuncs.pfnRegisterVariable( "cl_vsmoothing", "0.05", FCVAR_ARCHIVE );

	m_pitch = gEngfuncs.pfnRegisterVariable( "m_pitch", "0.022", FCVAR_ARCHIVE );
	m_yaw = gEngfuncs.pfnRegisterVariable( "m_yaw", "0.022", FCVAR_ARCHIVE );
	m_forward = gEngfuncs.pfnRegisterVariable( "m_forward", "1", FCVAR_ARCHIVE );
	m_side = gEngfuncs.pfnRegisterVariable( "m_side", "0.8", FCVAR_ARCHIVE );

	cl_bulletejects = gEngfuncs.pfnRegisterVariable( "cl_bulletejects", "1", FCVAR_ARCHIVE );
	dodcredits = gEngfuncs.pfnRegisterVariable( "dodcredits", "0", FCVAR_ARCHIVE );
	cl_hud_objectives = gEngfuncs.pfnRegisterVariable( "cl_hud_objectives", "1", FCVAR_ARCHIVE );
	cl_hud_objtimer = gEngfuncs.pfnRegisterVariable( "cl_hud_objtimer", "1", FCVAR_ARCHIVE );
	cl_hud_reinforcements = gEngfuncs.pfnRegisterVariable( "cl_hud_reinforcements", "1", FCVAR_ARCHIVE );
	cl_hud_health = gEngfuncs.pfnRegisterVariable( "cl_hud_health", "1", FCVAR_ARCHIVE );
	cl_hud_ammo = gEngfuncs.pfnRegisterVariable( "cl_hud_ammo", "1", FCVAR_ARCHIVE );
	cl_cutscenes = gEngfuncs.pfnRegisterVariable( "cl_cutscenes", "1", FCVAR_ARCHIVE );
	cl_dynamiclights = gEngfuncs.pfnRegisterVariable( "cl_dynamiclights", "1", FCVAR_ARCHIVE );
	cl_identiconmode = gEngfuncs.pfnRegisterVariable( "cl_identiconmode", "2", FCVAR_ARCHIVE );
	cl_drawmodels = gEngfuncs.pfnRegisterVariable( "cl_drawmodels", "1", FCVAR_ARCHIVE );
	cl_drawplayermodels = gEngfuncs.pfnRegisterVariable( "cl_drawplayermodels", "1", FCVAR_ARCHIVE );
	cl_hud_msgs = gEngfuncs.pfnRegisterVariable( "cl_hud_msgs", "2", FCVAR_ARCHIVE );
	cl_particlefx = gEngfuncs.pfnRegisterVariable( "cl_particlefx", "2", FCVAR_ARCHIVE );

	cl_fog = gEngfuncs.pfnRegisterVariable( "cl_fog", "1", FCVAR_ARCHIVE );
	cl_fog_density = gEngfuncs.pfnRegisterVariable( "cl_fog_density", "0.0", FCVAR_ARCHIVE );
	cl_fog_start = gEngfuncs.pfnRegisterVariable( "cl_fog_start", "10.0", FCVAR_ARCHIVE );
	cl_fog_end = gEngfuncs.pfnRegisterVariable( "cl_fog_end", "3500.0", FCVAR_ARCHIVE );
	cl_fog_red = gEngfuncs.pfnRegisterVariable( "cl_fog_red", "192", FCVAR_ARCHIVE );
	cl_fog_green = gEngfuncs.pfnRegisterVariable( "cl_fog_green", "192", FCVAR_ARCHIVE );
	cl_fog_blue = gEngfuncs.pfnRegisterVariable( "cl_fog_blue", "192", FCVAR_ARCHIVE );

	cl_dynamic_xhair = gEngfuncs.pfnRegisterVariable( "cl_dynamic_xhair", "1", FCVAR_ARCHIVE );
	cl_xhair_style = gEngfuncs.pfnRegisterVariable( "cl_xhair_style", "0", FCVAR_ARCHIVE );

	// Initialize third person camera controls.
	CAM_Init();
	// Initialize inputs
	IN_Init();
	// Initialize keyboard
	KB_Init();
	// Initialize view system
	V_Init();
}

/*
============
ShutdownInput
============
*/
void ShutdownInput( void )
{
	IN_Shutdown();
	KB_Shutdown();
}

void DLLEXPORT HUD_ShutdownInput( void )
{
	ShutdownInput();
}
