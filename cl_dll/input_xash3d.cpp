#include "hud.h"
#include "dod_shared.h"
#include "view.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input_mouse.h"
extern cvar_t		*sensitivity;
extern cvar_t		*in_joystick;

extern kbutton_t	in_strafe;
extern kbutton_t	in_mlook;
extern kbutton_t	in_speed;
extern kbutton_t	in_jlook;
extern kbutton_t	in_forward;
extern kbutton_t	in_back;
extern kbutton_t	in_moveleft;
extern kbutton_t	in_moveright;

extern cvar_t	*m_pitch;
extern cvar_t	*m_yaw;
extern cvar_t	*m_forward;
extern cvar_t	*m_side;
extern cvar_t	*lookstrafe;
extern cvar_t	*lookspring;
extern cvar_t	*cl_pitchdown;
extern cvar_t	*cl_pitchup;
extern cvar_t	*cl_yawspeed;
extern cvar_t	*cl_sidespeed;
extern cvar_t	*cl_forwardspeed;
extern cvar_t	*cl_pitchspeed;
extern cvar_t	*cl_movespeedkey;
cvar_t	*cl_laddermode;


#define F 1U<<0	// Forward
#define B 1U<<1	// Back
#define L 1U<<2	// Left
#define R 1U<<3	// Right
#define T 1U<<4	// Forward stop
#define S 1U<<5	// Side stop

#define BUTTON_DOWN		1
#define IMPULSE_DOWN	2
#define IMPULSE_UP		4

int CL_IsDead( void );
extern vec3_t dead_viewangles;

/*
===========
IN_GetMouseSensitivity
Get mouse sensitivity with sanitization
===========
*/
float IN_GetMouseSensitivity()
{
	// Absurdly high sensitivity values can cause the game to hang, so clamp
	if( sensitivity->value > 10000.0 )
	{
		gEngfuncs.Cvar_SetValue( "sensitivity", 10000.0 );
	}
	else if( sensitivity->value < 0.01 )
	{
		gEngfuncs.Cvar_SetValue( "sensitivity", 0.01 );
	}
	return sensitivity->value;
}

void IN_ToggleButtons( float forwardmove, float sidemove )
{
	static unsigned int moveflags = T | S;

	if( forwardmove )
		moveflags &= ~T;
	else
	{
		//if( in_forward.state || in_back.state ) gEngfuncs.Con_Printf("Buttons pressed f%d b%d\n", in_forward.state, in_back.state);
		if( !( moveflags & T ) )
		{
			//IN_ForwardUp();
			//IN_BackUp();
			//gEngfuncs.Con_Printf("Reset forwardmove state f%d b%d\n", in_forward.state, in_back.state);
			in_forward.state &= ~BUTTON_DOWN;
			in_back.state &= ~BUTTON_DOWN;
			moveflags |= T;
		}
	}
	if( sidemove )
		moveflags &= ~S;
	else
	{
		//gEngfuncs.Con_Printf("l%d r%d\n", in_moveleft.state, in_moveright.state);
		//if( in_moveleft.state || in_moveright.state ) gEngfuncs.Con_Printf("Buttons pressed l%d r%d\n", in_moveleft.state, in_moveright.state);
		if( !( moveflags & S ) )
		{
			//IN_MoverightUp();
			//IN_MoveleftUp();
			//gEngfuncs.Con_Printf("Reset sidemove state f%d b%d\n", in_moveleft.state, in_moveright.state);
			in_moveleft.state &= ~BUTTON_DOWN;
			in_moveright.state &= ~BUTTON_DOWN;
			moveflags |= S;
		}
	}

	if( forwardmove > 0.7f && !( moveflags & F ) )
	{
		moveflags |= F;
		in_forward.state |= BUTTON_DOWN;
	}
	if( forwardmove < 0.7f && ( moveflags & F ) )
	{
		moveflags &= ~F;
		in_forward.state &= ~BUTTON_DOWN;
	}
	if( forwardmove < -0.7f && !( moveflags & B ) )
	{
		moveflags |= B;
		in_back.state |= BUTTON_DOWN;
	}
	if( forwardmove > -0.7f && ( moveflags & B ) )
	{
		moveflags &= ~B;
		in_back.state &= ~BUTTON_DOWN;
	}
	if( sidemove > 0.9f && !( moveflags & R ) )
	{
		moveflags |= R;
		in_moveright.state |= BUTTON_DOWN;
	}
	if( sidemove < 0.9f && ( moveflags & R ) )
	{
		moveflags &= ~R;
		in_moveright.state &= ~BUTTON_DOWN;
	}
	if( sidemove < -0.9f && !( moveflags & L ) )
	{
		moveflags |= L;
		in_moveleft.state |= BUTTON_DOWN;
	}
	if( sidemove > -0.9f && ( moveflags & L ) )
	{
		moveflags &= ~L;
		in_moveleft.state &= ~BUTTON_DOWN;
	}
}

void FWGSInput::IN_ClientMoveEvent( float forwardmove, float sidemove )
{
	//gEngfuncs.Con_Printf("IN_MoveEvent\n");

	ac_forwardmove += forwardmove;
	ac_sidemove += sidemove;
	ac_movecount++;
}

void FWGSInput::IN_ClientLookEvent( float relyaw, float relpitch )
{
	rel_yaw += relyaw;
	rel_pitch += relpitch;
}

/*
===========
IN_DoDLimitView

The view of the deployed machine gun (and of the deployed prone) is limited
to a window around the angles it was deployed at: 20 degrees up and down,
45 degrees to the sides. The original does it in IN_MouseMove.
===========
*/
static void IN_DoDLimitView( Vector &viewangles )
{
	cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
	CHudDoDCrossHair &crosshair = gHUD.m_DoDCrossHair;
	float yaw, left, right, tempL, tempR;

	if( !pplayer || g_iUser1 || ( pplayer->curstate.iuser3 != STANCE_PRONE_DEPLOYED && pplayer->curstate.vuser1[0] != SANDBAG_DEPLOYED ) )
	{
		crosshair.i_xSet = 0;
		return;
	}

	// the window is computed for the yaw in 0..360
	while( viewangles[YAW] < 0.0f )
		viewangles[YAW] += 360.0f;

	while( viewangles[YAW] >= 360.0f )
		viewangles[YAW] -= 360.0f;

	if( !crosshair.i_xSet )
	{
		float pitch = viewangles[PITCH];
		float y = viewangles[YAW];

		if( pitch < -10.0f )
		{
			pitch = -10.0f;
			viewangles[PITCH] = pitch;
		}

		crosshair.i_xSet = 1;
		crosshair.f_xPos = y;
		crosshair.f_yPos = pitch;

		if( y <= 314.0f )
			crosshair.f_xLPos = y + 45.0f;
		else
			crosshair.f_xLPos = y - 314.0f;

		if( y >= 45.0f || y == 0.0f )
		{
			if( y == 0.0f )
				crosshair.f_xRPos = 314.0f;
			else
				crosshair.f_xRPos = y - 45.0f;
		}
		else
		{
			crosshair.f_xRPos = (float)( 359 - (int)( 45.0f - y ) );
		}
	}

	if( viewangles[PITCH] > crosshair.f_yPos + 20.0f )
		viewangles[PITCH] = crosshair.f_yPos + 20.0f;

	if( viewangles[PITCH] < crosshair.f_yPos - 20.0f )
		viewangles[PITCH] = crosshair.f_yPos - 20.0f;

	yaw = viewangles[YAW];
	left = crosshair.f_xLPos;
	right = crosshair.f_xRPos;
	tempL = fabsf( yaw - left );
	tempR = fabsf( yaw - right );

	// the window wraps around the zero when the right limit is greater than the left one
	if( yaw <= left )
	{
		if( yaw < right && yaw < left && tempL > tempR )
			yaw = right;
	}
	else if( yaw > right )
	{
		if( tempR > tempL )
			yaw = left;
	}
	else if( yaw < right )
	{
		if( tempR > tempL )
			yaw = left;
		else if( tempL > tempR )
			yaw = right;
	}

	viewangles[YAW] = yaw;
}

// Rotate camera and add move values to usercmd
void FWGSInput::IN_Move( float frametime, usercmd_t *cmd )
{
	vec3_t viewangles;
	bool fLadder = false;

	if( gHUD.m_iIntermission )
		return; // we can't move during intermission

	if( cl_laddermode->value != 2 )
	{
		cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
		if( pplayer )
			fLadder = pplayer->curstate.movetype == MOVETYPE_FLY;
	}
	//if(ac_forwardmove || ac_sidemove)
	//gEngfuncs.Con_Printf("Move: %f %f %f %f\n", ac_forwardmove, ac_sidemove, rel_pitch, rel_yaw);
	// the original stops the pitch drift while the mouse look is held;
	// here the mouse look is always on, so the view never drifts to the horizon
	V_StopPitchDrift();
	if( CL_IsDead() )
	{
		viewangles = dead_viewangles; // HACKHACK: see below
	}
	else
	{
		gEngfuncs.GetViewAngles( viewangles );
	}
	float mouse_sensitivity = gHUD.GetSensitivity() != 0 ? gHUD.GetSensitivity() : IN_GetMouseSensitivity();
	rel_yaw *= mouse_sensitivity;
	rel_pitch *= mouse_sensitivity;
	viewangles[YAW] += rel_yaw;
	if( fLadder )
	{
		if( cl_laddermode->value == 1 )
			viewangles[YAW] -= ac_sidemove * 5;
		ac_sidemove = 0;
	}
	if( gHUD.m_MOTD.m_bShow )
		gHUD.m_MOTD.scroll += rel_pitch;
	else
		viewangles[PITCH] += rel_pitch;

	if( !CL_IsDead() )
		IN_DoDLimitView( viewangles );

	if( viewangles[PITCH] > cl_pitchdown->value )
		viewangles[PITCH] = cl_pitchdown->value;
	if( viewangles[PITCH] < -cl_pitchup->value )
		viewangles[PITCH] = -cl_pitchup->value;
	
	// HACKHACK: change viewangles directly in viewcode, 
	// so viewangles when player is dead will not be changed on server
	if( !CL_IsDead() )
	{
		gEngfuncs.SetViewAngles( viewangles );
	}

	dead_viewangles = viewangles; // keep them actual
	if( ac_movecount )
	{
		IN_ToggleButtons( ac_forwardmove / ac_movecount, ac_sidemove / ac_movecount );

		if( ac_forwardmove )
			cmd->forwardmove = ac_forwardmove * cl_forwardspeed->value / ac_movecount;
		if( ac_sidemove )
			cmd->sidemove  = ac_sidemove * cl_sidespeed->value / ac_movecount;
		if( ( in_speed.state & 1 ) && ( ac_sidemove || ac_forwardmove ) )
		{
			cmd->forwardmove *= cl_movespeedkey->value;
			cmd->sidemove *= cl_movespeedkey->value;
		}
	}

	ac_sidemove = ac_forwardmove = rel_pitch = rel_yaw = 0;
	ac_movecount = 0;
}

void FWGSInput::IN_MouseEvent( int mstate )
{
	static int mouse_oldbuttonstate;
	// perform button actions
	for( int i = 0; i < 5; i++ )
	{
		int iButton = 1 << i;

		if( ( mstate & iButton ) && !( mouse_oldbuttonstate & iButton ) )
		{
			// a click on a window with the cursor is not a command of the game, the engine hands it to VGUI as well
			if( m_bVisibleMouse )
				continue;

			mouse_oldbuttonstate |= iButton;
			gEngfuncs.Key_Event( K_MOUSE1 + i, 1 );
		}
		else if( !( mstate & iButton ) && ( mouse_oldbuttonstate & iButton ) )
		{
			// the release of a sent button always gets through, or the command stays pressed
			mouse_oldbuttonstate &= ~iButton;
			gEngfuncs.Key_Event( K_MOUSE1 + i, 0 );
		}
	}
}

void FWGSInput::IN_SetVisibleMouse( bool visible )
{
	m_bVisibleMouse = visible;
}

// Stubs

void FWGSInput::IN_ClearStates( void )
{
	//gEngfuncs.Con_Printf( "IN_ClearStates\n" );
}

void FWGSInput::IN_ActivateMouse( void )
{
	//gEngfuncs.Con_Printf( "IN_ActivateMouse\n" );
}

void FWGSInput::IN_DeactivateMouse( void )
{
	//gEngfuncs.Con_Printf( "IN_DeactivateMouse\n" );
}

void FWGSInput::IN_Accumulate( void )
{
	//gEngfuncs.Con_Printf( "IN_Accumulate\n" );
}

void FWGSInput::IN_Commands( void )
{
	//gEngfuncs.Con_Printf( "IN_Commands\n" );
}

void FWGSInput::IN_Shutdown( void )
{
	/* Nothing. */
}

// Register cvars and reset data
void FWGSInput::IN_Init( void )
{
	sensitivity = gEngfuncs.pfnRegisterVariable( "sensitivity", "3", FCVAR_ARCHIVE | FCVAR_FILTERSTUFFTEXT );
	in_joystick = gEngfuncs.pfnRegisterVariable( "joystick", "0", FCVAR_ARCHIVE );
	cl_laddermode = gEngfuncs.pfnRegisterVariable( "cl_laddermode", "2", FCVAR_ARCHIVE );
	ac_forwardmove = ac_sidemove = rel_yaw = rel_pitch = 0;
	m_bVisibleMouse = false;
}
