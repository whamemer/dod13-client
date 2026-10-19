//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/dodspectatorgui.h"
#include "hltv.h"
#include "vgui/idodviewport.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "entity_types.h"

// these are included for the math functions
#include "com_model.h"
#include "demo_api.h"
#include "event_api.h"
#include "studio_util.h"
#include "screenfade.h"

#pragma warning(disable: 4244)

extern "C" int		iJumpSpectator;
extern "C" float	vJumpOrigin[3];
extern "C" float	vJumpAngles[3]; 

extern void V_GetInEyePos( int entity, float * origin, float * angles );
extern void V_ResetChaseCam();
extern void V_GetChasePos( int target, float * cl_angles, float * origin, float * angles );
extern float * GetClientColor( int clientIndex );

extern char *szMapMarkerIcons[15];

extern vec3_t v_origin;		// last view origin
extern vec3_t v_angles;		// last view angle
extern vec3_t v_cl_angles;	// last client/mouse angle
extern vec3_t v_sim_org;	// last sim origin

const char *GetSpectatorLabel( int iMode )
{
	switch( iMode )
	{
	case OBS_CHASE_LOCKED:
		return "#OBS_CHASE_LOCKED";

	case OBS_CHASE_FREE:
		return "#OBS_CHASE_FREE";

	case OBS_ROAMING:
		return "#OBS_ROAMING";

	case OBS_IN_EYE:
		return "#OBS_IN_EYE";

	case OBS_MAP_FREE:
		return "#OBS_MAP_FREE";

	case OBS_MAP_CHASE:
		return "#OBS_MAP_CHASE";

	case OBS_NONE:
	default:
		return "#OBS_NONE";
	}
}

void SpectatorMode( void )
{
	if( gEngfuncs.Cmd_Argc() <= 1 )
	{
		gEngfuncs.Con_Printf( "usage:  spec_mode <Main Mode> [<Inset Mode>]\n" );
		return;
	}

	// SetModes() will decide if command is executed on server or local
	if( gEngfuncs.Cmd_Argc() == 2 )
		gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), -1 );
	else if ( gEngfuncs.Cmd_Argc() == 3 )
		gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), atoi( gEngfuncs.Cmd_Argv( 2 ) )  );
}

void SpectatorToggleInset( void )
{
	gHUD.m_Spectator.SetModes( -1, 0 );
}

void SpectatorSpray( void )
{
	vec3_t forward;
	char string[128];

	if( !gEngfuncs.IsSpectateOnly() )
		return;

	AngleVectors( v_angles, forward, NULL, NULL );
	VectorScale( forward, 128, forward );
	VectorAdd( forward, v_origin, forward );
	pmtrace_t * trace = gEngfuncs.PM_TraceLine( v_origin, forward, PM_TRACELINE_ANYVISIBLE, 2, -1 );
	if( trace->fraction != 1.0f )
	{
		sprintf( string, "drc_spray %.2f %.2f %.2f %i",
			(double)trace->endpos[0], (double)trace->endpos[1], (double)trace->endpos[2], trace->ent );
		gEngfuncs.pfnServerCmd( string );
	}
}

void SpectatorHelp( void )
{
	if( gViewPort )
	{
		gViewPort->ShowVGUIMenu( MENU_SPECHELP );
	}
	else
	{
  		char *text = CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help_Text" );

		if( text )
		{
			while( *text )
			{
				if( *text != 13 )
					gEngfuncs.Con_Printf( "%c", *text );
				text++;
			}
		}
	}
}

void SpectatorMenu( void )
{
	if( gEngfuncs.Cmd_Argc() <= 1 )
	{
		gEngfuncs.Con_Printf( "usage:  spec_menu <0|1>\n" );
		return;
	}

	if( atoi( gEngfuncs.Cmd_Argv( 1 ) ) == 0 )
		gDoDViewPortInterface->DeactivateSpectatorGUI();
	else
		gDoDViewPortInterface->ShowSpectatorGUI();
}

void ToggleScores( void )
{
	if( gHUD.UseVguiScoreBoard() && gViewPort )
	{
		if( gViewPort->IsScoreBoardVisible() )
		{
			gViewPort->HideScoreBoard();
		}
		else
		{
			gViewPort->ShowScoreBoard();
		}
		return;
	}
	if (gHUD.m_Scoreboard.m_iShowscoresHeld) {
		gHUD.m_Scoreboard.UserCmd_HideScores();
	} else {
		gHUD.m_Scoreboard.UserCmd_ShowScores();
	}
}

void SpectatorToggleDrawNames( void )
{
	if( gHUD.m_Spectator.m_drawnames->value == 0.0f )
		gHUD.m_Spectator.m_drawnames->value = 1.0f;
	else
		gHUD.m_Spectator.m_drawnames->value = 0.0f;
}

void SpectatorToggleDrawCone( void )
{
	if( gHUD.m_Spectator.m_drawcone->value == 0.0f )
		gHUD.m_Spectator.m_drawcone->value = 1.0f;
	else
		gHUD.m_Spectator.m_drawcone->value = 0.0f;
}

void SpectatorToggleDrawStatus( void )
{
	if( gHUD.m_Spectator.m_drawstatus->value == 0.0f )
		gHUD.m_Spectator.m_drawstatus->value = 1.0f;
	else
		gHUD.m_Spectator.m_drawstatus->value = 0.0f;
}

void SpectatorToggleAutoDirector( void )
{
	if( gHUD.m_Spectator.m_autoDirector->value == 0.0f )
		gHUD.m_Spectator.m_autoDirector->value = 1.0f;
	else
		gHUD.m_Spectator.m_autoDirector->value = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CHudSpectator::Init()
{
	gHUD.AddHudElem( this );

	m_iFlags |= HUD_ACTIVE;
	m_flNextObserverInput = 0.0f;
	m_zoomDelta = 0.0f;
	m_moveDelta = 0.0f;
	m_chatEnabled = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );
	iJumpSpectator = 0;

	memset( &m_OverviewData, 0, sizeof(m_OverviewData) );
	memset( &m_OverviewEntities, 0, sizeof(m_OverviewEntities) );
	m_lastPrimaryObject = m_lastSecondaryObject = 0;

	gEngfuncs.pfnAddCommand( "spec_mode", SpectatorMode );
	gEngfuncs.pfnAddCommand( "spec_decal", SpectatorSpray );
	gEngfuncs.pfnAddCommand( "spec_help", SpectatorHelp );
	gEngfuncs.pfnAddCommand( "spec_menu", SpectatorMenu );
	gEngfuncs.pfnAddCommand( "togglescores", ToggleScores );
	gEngfuncs.pfnAddCommand( "spec_drawnames", SpectatorToggleDrawNames );
	gEngfuncs.pfnAddCommand( "spec_drawcone", SpectatorToggleDrawCone );
	gEngfuncs.pfnAddCommand( "spec_drawstatus", SpectatorToggleDrawStatus );
	gEngfuncs.pfnAddCommand( "spec_autodirector", SpectatorToggleAutoDirector );

	m_drawnames = gEngfuncs.pfnRegisterVariable( "spec_drawnames_internal", "1", 0 );
	m_drawcone = gEngfuncs.pfnRegisterVariable( "spec_drawcone_internal", "1", 0 );
	m_drawstatus = gEngfuncs.pfnRegisterVariable( "spec_drawstatus_internal", "1", 0 );
	m_autoDirector = gEngfuncs.pfnRegisterVariable( "spec_autodirector_internal", "1", 0 );
	m_pip = gEngfuncs.pfnRegisterVariable( "spec_pip", "1", 0 );
	m_scoreboard = gEngfuncs.pfnRegisterVariable( "spec_scoreboard", "0", 0 );

	if( !m_drawnames || !m_drawcone || !m_drawstatus || !m_autoDirector || !m_pip || !m_scoreboard )
	{
		gEngfuncs.Con_Printf( "ERROR! Couldn't register all spectator variables.\n" );
		return 0;
	}

	ClearVoiceIconFlags();

	default_fov = gEngfuncs.pfnGetCvarPointer( "default_fov" );

	// the inset window of the spectator GUI is hidden
	if( gViewPort )
		gViewPort->SpectatorGUIEnableInsetView( 0 );

	return 1;
}

//-----------------------------------------------------------------------------
// UTIL_StringToVector originally from ..\dlls\util.cpp, slightly changed
//-----------------------------------------------------------------------------
void UTIL_StringToVector( float * pVector, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	strlcpy( tempString, pString, sizeof( tempString ));

	pstr = pfront = tempString;

	for( j = 0; j < 3; j++ )		
	{
		pVector[j] = atof( pfront );

		while( *pstr && *pstr != ' ' )
			pstr++;
		if( !( *pstr ) )
			break;
		pstr++;
		pfront = pstr;
	}

	if( j < 2 )
	{
		for( j = j + 1;j < 3; j++ )
			pVector[j] = 0;
	}
}

int UTIL_FindEntityInMap( const char *name, float *origin, float *angle )
{
	int			n, found = 0;
	char			keyname[256];
	char			token[2048];

	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( 0 );	// get world model

	if( !pEnt )
		return 0;

	if( !pEnt->model )
		return 0;

	char *data = pEnt->model->entities;

	while( data )
	{
		data = gEngfuncs.COM_ParseFile( data, token );

		if( ( token[0] == '}' ) || ( token[0] == 0 ) )
			break;

		if( !data )
		{
			gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
			return 0;
		}

		if( token[0] != '{' )
		{
			gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: expected {\n" );
			return 0;
		}

		// we parse the first { now parse entities properties		
		while( 1 )
		{	
			// parse key
			data = gEngfuncs.COM_ParseFile( data, token );
			if( token[0] == '}' )
				break; // finish parsing this entity

			if( !data )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
				return 0;
			}

			strcpy( keyname, token );

			// another hack to fix keynames with trailing spaces
			n = strlen( keyname );
			while( n && keyname[n - 1] == ' ' )
			{
				keyname[n - 1] = 0;
				n--;
			}

			// parse value	
			data = gEngfuncs.COM_ParseFile( data, token );
			if( !data )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
				return 0;
			}
	
			if( token[0] == '}' )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: closing brace without data\n" );
				return 0;
			}

			if( !strcmp( keyname, "classname" ) )
			{
				if( !strcmp( token, name ) )
				{
					found = 1;	// thats our entity
				}
			}

			if( !strcmp( keyname, "angle" ) )
			{
				float y = atof( token );

				if( y >= 0 )
				{
					angle[0] = 0.0f;
					angle[1] = y;
				}
				else if( (int)y == -1 )
				{
					angle[0] = -90.0f;
					angle[1] = 0.0f;;
				}
				else
				{
					angle[0] = 90.0f;
					angle[1] = 0.0f;
				}

				angle[2] = 0.0f;
			}

			if( !strcmp( keyname, "angles" ) )
			{
				UTIL_StringToVector( angle, token );
			}

			if( !strcmp( keyname, "origin" ) )
			{
				UTIL_StringToVector( origin, token );
			}
		} // while (1)

		if( found )
			return 1;
	}

	return 0;	// we search all entities, but didn't found the correct
}

//-----------------------------------------------------------------------------
// SetSpectatorStartPosition(): 
// Get valid map position and 'beam' spectator to this position
//-----------------------------------------------------------------------------
void CHudSpectator::SetSpectatorStartPosition()
{
	// search for info_player start
	if( !UTIL_FindEntityInMap( "info_player_observer", m_cameraOrigin, m_cameraAngles ) &&
		!UTIL_FindEntityInMap( "info_player_start", m_cameraOrigin, m_cameraAngles ) &&
		!UTIL_FindEntityInMap( "info_player_allies", m_cameraOrigin, m_cameraAngles ) &&
		!UTIL_FindEntityInMap( "info_player_axis", m_cameraOrigin, m_cameraAngles ) )
	{
		// jump to 0,0,0 if no better position was found
		VectorCopy( vec3_origin, m_cameraOrigin );
		VectorCopy( vec3_origin, m_cameraAngles );
	}

	VectorCopy( m_cameraOrigin, vJumpOrigin );
	VectorCopy( m_cameraAngles, vJumpAngles );

	iJumpSpectator = 1;	// jump anyway
}

//-----------------------------------------------------------------------------
// Purpose: Loads new icons
//-----------------------------------------------------------------------------
int CHudSpectator::VidInit()
{
	m_hsprPlayerDead	= SPR_Load( "sprites/iplayerdead.spr" );
	m_hsprUnkownMap		= SPR_Load( "sprites/tile.spr" );
	m_hsprBeam		= SPR_Load( "sprites/laserbeam.spr" );
	m_hsprCamera		= SPR_Load( "sprites/camera.spr" );
	m_hsprAllieLight	= SPR_Load( "sprites/aplayer.spr" );
	m_hsprAxisLight		= SPR_Load( "sprites/gplayer.spr" );
	m_hsprAllieZone		= SPR_Load( "sprites/aflag.spr" );
	m_hsprAxisZone		= SPR_Load( "sprites/gflag.spr" );
	m_hsprCapZone		= SPR_Load( "sprites/wflag.spr" );
	m_hsprTnT		= SPR_Load( "sprites/tnt.spr" );
	m_hsprBanglr		= SPR_Load( "sprites/banglr.spr" );
	m_hsprStick		= SPR_Load( "sprites/stick_hltv.spr" );
	m_hsprGrenade		= SPR_Load( "sprites/grenade_hltv.spr" );
	m_hsprCameraAllies	= SPR_Load( "sprites/allies_camera.spr" );
	m_hsprCameraAxis	= SPR_Load( "sprites/axis_camera.spr" );
	m_hsprCameraBrit	= SPR_Load( "sprites/brit_camera.spr" );
	m_hsprCameraSpec	= SPR_Load( "sprites/spec_camera.spr" );
	m_hsprBritLight		= SPR_Load( "sprites/bplayer.spr" );
	m_hsprVoiceIcon		= SPR_Load( "sprites/mapsprites/voiceIcon.spr" );
	m_hsprSpeakerIcon	= SPR_Load( "sprites/mapsprites/speakerIcon.spr" );

	for( int i = 0; i < 15; i++ )
		m_hsprMapMarkers[i] = SPR_Load( szMapMarkerIcons[i] );

	ClearVoiceIconFlags();

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : flTime -
//			intermission -
//-----------------------------------------------------------------------------
int CHudSpectator::Draw( float flTime )
{
	if( !g_iUser1 )
	{
		// the spectator GUI is not needed any more
		if( gViewPort && gViewPort->IsSpectatorGUIVisible() )
			gViewPort->HideSpectatorGUI();

		return 0;
	}

	return 1;
}

void CHudSpectator::DirectorMessage( int iSize, void *pbuf )
{
	float f1;
	char *string;

	BEGIN_READ( pbuf, iSize );

	int cmd = READ_BYTE();

	switch( cmd )	// director command byte
	{
		case DRC_CMD_START:
			// now we have to do some things clientside, since the proxy doesn't know our mod
			g_iPlayerClass = 0;
			g_iTeamNumber = 0;

			// fake a InitHUD & ResetHUD message
			gHUD.MsgFunc_InitHUD( NULL, 0, NULL );
			gHUD.MsgFunc_ResetHUD( NULL, 0, NULL );
			break;
		case DRC_CMD_EVENT: // old director style message
			m_lastPrimaryObject = READ_WORD();
			m_lastSecondaryObject = READ_WORD();
			m_iObserverFlags = READ_LONG();

			if( m_autoDirector->value )
			{
				if( ( g_iUser2 != m_lastPrimaryObject) || ( g_iUser3 != m_lastSecondaryObject ) )
					V_ResetChaseCam();

				g_iUser2 = m_lastPrimaryObject;
				g_iUser3 = m_lastSecondaryObject;
			}

			// gEngfuncs.Con_Printf( "Director Camera: %i %i\n", firstObject, secondObject );
			break;
		case DRC_CMD_MODE:
			if( m_autoDirector->value )
			{
				SetModes( READ_BYTE(), -1 );
			}
			break;
		case DRC_CMD_CAMERA:
			// the camera of the game has no field of view, no target and no waypoints
			if( m_autoDirector->value )
			{
				vJumpOrigin[0] = READ_COORD();	// position
				vJumpOrigin[1] = READ_COORD();
				vJumpOrigin[2] = READ_COORD();

				vJumpAngles[0] = READ_COORD();	// view angle
				vJumpAngles[1] = READ_COORD();
				vJumpAngles[2] = READ_COORD();

				gEngfuncs.SetViewAngles( vJumpAngles );
				iJumpSpectator = 1;
			}
			break;
		case DRC_CMD_MESSAGE:
			{
				client_textmessage_t * msg = &m_HUDMessages[m_lastHudMessage];

				msg->effect = READ_BYTE();		// effect

				UnpackRGB( (int&)msg->r1, (int&)msg->g1, (int&)msg->b1, READ_LONG() );	// color
				msg->r2 = msg->r1;
				msg->g2 = msg->g1;
				msg->b2 = msg->b1;
				msg->a2 = msg->a1 = 0xFF;	// not transparent

				msg->x = READ_FLOAT();	// x pos
				msg->y = READ_FLOAT();	// y pos

				msg->fadein = READ_FLOAT();	// fadein
				msg->fadeout = READ_FLOAT();	// fadeout
				msg->holdtime = READ_FLOAT();	// holdtime
				msg->fxtime = READ_FLOAT();	// fxtime;

				strlcpy( m_HUDMessageText[m_lastHudMessage], READ_STRING(), 128 );

				msg->pMessage = m_HUDMessageText[m_lastHudMessage];
				msg->pName = "HUD_MESSAGE";

				gHUD.m_Message.MessageAdd( msg );

				m_lastHudMessage++;
				m_lastHudMessage %= MAX_SPEC_HUD_MESSAGES;
			}
			break;
		case DRC_CMD_SOUND:
			string = READ_STRING();
			f1 =  READ_FLOAT();

			// gEngfuncs.Con_Printf("DRC_CMD_FX_SOUND: %s %.2f\n", string, f1 );
			gEngfuncs.pEventAPI->EV_PlaySound( 0, v_origin, CHAN_BODY, string, f1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case DRC_CMD_TIMESCALE:
			READ_FLOAT();
			break;
		case DRC_CMD_STATUS:
			READ_LONG(); // total number of spectator slots
			m_iSpectatorNumber = READ_LONG(); // total number of spectator
			READ_WORD(); // total number of relay proxies
			gViewPort->UpdateSpectatorPanel();
			break;
		case DRC_CMD_BANNER:
			// gEngfuncs.Con_DPrintf( "GUI: Banner %s\n",READ_STRING() ); // name of banner tga eg gfx/temp/7454562234563475.tga
			gViewPort->SetSpectatorBanner( READ_STRING() );
			gViewPort->UpdateSpectatorPanel();
			break;
		case DRC_CMD_STUFFTEXT:
			gEngfuncs.pfnFilteredClientCmd( READ_STRING() );
			break;
		default:
			gEngfuncs.Con_DPrintf( "CHudSpectator::DirectorMessage: unknown command %i.\n", cmd );
			break;
	}
}

void CHudSpectator::FindNextPlayer( bool bReverse )
{
	int		iStart;
	cl_entity_t * pEnt = NULL;

	// if we are NOT in HLTV mode, spectator targets are set on server
	if( !gEngfuncs.IsSpectateOnly() )
	{
		char cmdstring[32];
		// forward command to server
		sprintf( cmdstring, "follownext %i", bReverse );
		gEngfuncs.pfnServerCmd( cmdstring );
		return;
	}

	if( g_iUser2 )
		iStart = g_iUser2;
	else
		iStart = 1;

	g_iUser2 = 0;

	int iCurrent = iStart;

	int iDir = bReverse ? -1 : 1;

	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if( iCurrent > MAX_PLAYERS )
			iCurrent = 1;
		if( iCurrent < 1 )
			iCurrent = MAX_PLAYERS;

		pEnt = gEngfuncs.GetEntityByIndex( iCurrent );

		if( !IsActivePlayer( pEnt ) )
			continue;

		g_iUser2 = iCurrent;
		break;
	} while( iCurrent != iStart );

	// Did we find a target?
	if( !g_iUser2 )
	{
		gEngfuncs.Con_DPrintf( "No observer targets.\n" );
		// take save camera position
		VectorCopy( m_cameraOrigin, vJumpOrigin );
		VectorCopy( m_cameraAngles, vJumpAngles );
	}
	else
	{
		// use new entity position for roaming
		VectorCopy( pEnt->origin, vJumpOrigin );
		VectorCopy( pEnt->angles, vJumpAngles );
	}

	if( gViewPort )
		gViewPort->UpdateSpectatorPanel();

	iJumpSpectator = 1;
}

void CHudSpectator::FindPlayer( const char *name )
{
	// if we are NOT in HLTV mode, spectator targets are set on server
	if( !gEngfuncs.IsSpectateOnly() )
	{
		char cmdstring[128];
		// forward command to server
		snprintf( cmdstring, 127, "follow \"%s\"", name );
		cmdstring[127] = 0;
		gEngfuncs.pfnServerCmd( cmdstring );
		return;
	}

	g_iUser2 = 0;

	cl_entity_t * pEnt = NULL;

	for( int i = 1; i < MAX_PLAYERS; i++ )
	{
		pEnt = gEngfuncs.GetEntityByIndex( i );

		if( !IsActivePlayer( pEnt ) )
			continue;

		gEngfuncs.pfnGetPlayerInfo( i, &g_PlayerInfoList[i] );

		if( !stricmp( g_PlayerInfoList[pEnt->index].name, name ) )
		{
			g_iUser2 = i;
			break;
		}
	}

	// Did we find a target?
	if( !g_iUser2 )
	{
		gEngfuncs.Con_DPrintf( "No observer targets.\n" );
		// take save camera position
		VectorCopy( m_cameraOrigin, vJumpOrigin );
		VectorCopy( m_cameraAngles, vJumpAngles );
	}
	else
	{
		// use new entity position for roaming
		VectorCopy( pEnt->origin, vJumpOrigin );
		VectorCopy( pEnt->angles, vJumpAngles );
	}

	if( gViewPort )
		gViewPort->UpdateSpectatorPanel();

	iJumpSpectator = 1;
}

void CHudSpectator::HandleButtonsDown( int ButtonPressed )
{
	double time = gEngfuncs.GetClientTime();

	int newMainMode = -1;
	int newInsetMode = m_pip->value;

	// gEngfuncs.Con_Printf( " HandleButtons:%i\n", ButtonPressed );

	if( !gViewPort )
		return;

	//Not in intermission.
	if( gHUD.m_iIntermission )
		 return;

	if( !g_iUser1 )
		return; // dont do anything if not in spectator mode

	// don't handle buttons during normal demo playback
	if( gEngfuncs.pDemoAPI->IsPlayingback() && !gEngfuncs.IsSpectateOnly() )
		return;
	// Slow down mouse clicks.
	if( m_flNextObserverInput > time )
		return;

	// enable spectator screen
	if( ButtonPressed & IN_DUCK )
	{
		if( !gViewPort->IsSpectatorBarVisible() )
			gViewPort->ShowSpectatorGUIBar();
		else
			gViewPort->SpectatorGUIDuckKey();	// VGUI2 closes the bar with the key press in CSpectatorGUI::OnKeyCodePressed
	}

	//  'Use' would change the inset window mode, the game has no inset window
	if( ButtonPressed & IN_USE )
		newInsetMode = 0;

	// if not in HLTV mode, buttons are handled server side
	if( gEngfuncs.IsSpectateOnly() )
	{
		// Jump changes main window modes
		if( ButtonPressed & IN_JUMP )
		{
			if( g_iUser1 == OBS_CHASE_LOCKED )
				newMainMode = OBS_CHASE_FREE;
			else if( g_iUser1 == OBS_CHASE_FREE )
				newMainMode = OBS_IN_EYE;
			else if( g_iUser1 == OBS_IN_EYE )
				newMainMode = OBS_ROAMING;
			else if( g_iUser1 == OBS_ROAMING )
				newMainMode = OBS_CHASE_LOCKED;
			else
				newMainMode = OBS_CHASE_FREE;
		}

		// Attack moves to the next player
		if( ButtonPressed & ( IN_ATTACK | IN_ATTACK2 ) )
		{
			FindNextPlayer( ( ButtonPressed & IN_ATTACK2 ) ? true : false );

			if( g_iUser1 == OBS_ROAMING )
			{
				gEngfuncs.SetViewAngles( vJumpAngles );
				iJumpSpectator = 1;
			}
			// lease directed mode if player want to see another player
			m_autoDirector->value = 0.0f;
		}
	}

	SetModes( newMainMode, newInsetMode );

	m_flNextObserverInput = time + 0.2;
}

void CHudSpectator::HandleButtonsUp( int ButtonPressed )
{
	if( !gViewPort )
		return;

	if( !gViewPort->IsSpectatorGUIVisible() )
		return; // dont do anything if not in spectator mode

	if( ButtonPressed & ( IN_FORWARD | IN_BACK ) )
		m_zoomDelta = 0.0f;

	if( ButtonPressed & ( IN_MOVELEFT | IN_MOVERIGHT ) )
		m_moveDelta = 0.0f;
}

void CHudSpectator::SetModes( int iNewMainMode, int iNewInsetMode )
{
	// if value == -1 keep old value
	if( iNewMainMode == -1 )
		iNewMainMode = g_iUser1;

	if( iNewInsetMode == -1 )
		iNewInsetMode = (int)m_pip->value;

	// inset mode is handled only clients side
	m_pip->value = iNewInsetMode;

	// only the modes 1..4 are in the game: chase locked, chase free, roaming, in eye
	if( iNewMainMode < OBS_CHASE_LOCKED || iNewMainMode > OBS_IN_EYE )
	{
		gEngfuncs.Con_Printf( "Invalid spectator mode.\n" );
		return;
	}

	if( iNewMainMode == g_iUser1 )
	{
		if( gViewPort )
			gViewPort->UpdateSpectatorPanel();

		return;
	}

	// if we are NOT in HLTV mode, main spectator mode is set on server
	if( !gEngfuncs.IsSpectateOnly() )
	{
		char cmdstring[32];

		// the forced camera of the server keeps the director
		if( iNewMainMode == OBS_CHASE_FREE && ( !gDoDViewPortInterface || gDoDViewPortInterface->GetForceCamera() != 2 ) )
			m_autoDirector->value = 0.0f;

		// forward command to server
		sprintf( cmdstring, "specmode %i", iNewMainMode );
		gEngfuncs.pfnServerCmd( cmdstring );
		return;
	}

	if( !g_iUser2 && ( iNewMainMode != OBS_ROAMING ) )	// make sure we have a target
	{
		// choose last Director object if still available
		if( IsActivePlayer( gEngfuncs.GetEntityByIndex( m_lastPrimaryObject ) ) )
		{
			g_iUser2 = m_lastPrimaryObject;
			g_iUser3 = m_lastSecondaryObject;
		}
		else
			FindNextPlayer( false ); // find any target
	}

	switch( iNewMainMode )
	{
		case OBS_CHASE_LOCKED:
			g_iUser1 = OBS_CHASE_LOCKED;
			break;
		case OBS_CHASE_FREE:
			g_iUser1 = OBS_CHASE_FREE;
			m_autoDirector->value = 0.0f;
			break;
		case OBS_ROAMING:	// jump to the eyes of the target
			g_iUser1 = OBS_ROAMING;
			if( g_iUser2 )
			{
				V_GetInEyePos( g_iUser2, vJumpOrigin, vJumpAngles );
				gEngfuncs.SetViewAngles( vJumpAngles );
				iJumpSpectator = 1;
			}
			break;
		case OBS_IN_EYE:
			g_iUser1 = OBS_IN_EYE;
			break;
	}

	char string[128];
	sprintf( string, "#Spec_Mode%d", g_iUser1 );
	sprintf( string, "%c%s", HUD_PRINTCENTER, CHudTextMessage::BufferedLocaliseTextString( string ) );
	gHUD.m_TextMessage.MsgFunc_TextMsg( NULL, strlen( string ) + 1, string );

	if( gViewPort )
		gViewPort->UpdateSpectatorPanel();
}

bool CHudSpectator::IsActivePlayer( cl_entity_t *ent )
{
	if( ent && ent->player )
	{
		gEngfuncs.pfnGetPlayerInfo( ent->index, &g_PlayerInfoList[ent->index] );

		if( ent->curstate.solid != SOLID_NOT && ent != gEngfuncs.GetLocalPlayer() )
			return g_PlayerInfoList[ent->index].name != NULL;
	}

	return false;
}

bool CHudSpectator::ParseOverviewFile()
{
	char filename[512] = { 0 };
	char levelname[256] = { 0 };
	char token[1024] = { 0 };
	float height;
	bool ret = false;

	char *afile = NULL, *pfile = NULL;

	memset( &m_OverviewData, 0, sizeof(m_OverviewData) );

	// fill in standrd values
	m_OverviewData.insetWindowX = 4;	// upper left corner
	m_OverviewData.insetWindowY = 4;
	m_OverviewData.insetWindowHeight = 180;
	m_OverviewData.insetWindowWidth = 240;
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom = 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0] = 0.0f;
	strcpy( m_OverviewData.map, gEngfuncs.pfnGetLevelName() );

	if( m_OverviewData.map[0] == '\0' )
		return ret; // not active yet

	strcpy( levelname, m_OverviewData.map + 5 );
	levelname[strlen( levelname ) - 4] = 0;

	sprintf( filename, "overviews/%s.txt", levelname );

	afile = pfile = (char *)gEngfuncs.COM_LoadFile( filename, 5, NULL );

	if( !pfile )
	{
		gEngfuncs.Con_Printf( "Couldn't open file %s. Using default values for overiew mode.\n", filename );
		return ret;
	}

	while( true )
	{
		pfile = gEngfuncs.COM_ParseFile( pfile, token );

		if( !pfile )
			break;

		if( !stricmp( token, "global" ) )
		{
			// parse the global data
			pfile = gEngfuncs.COM_ParseFile( pfile, token );
			if( stricmp( token, "{" ) ) 
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename );
				goto end;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			while( stricmp( token, "}") )
			{
				if( !stricmp( token, "zoom" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.zoom = atof( token );
				} 
				else if( !stricmp( token, "origin" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token ); 
					m_OverviewData.origin[0] = atof( token );
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.origin[1] = atof( token );
					pfile = gEngfuncs.COM_ParseFile( pfile, token ); 
					m_OverviewData.origin[2] = atof( token );
				}
				else if( !stricmp( token, "rotated" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token ); 
					m_OverviewData.rotated = atoi( token );
				}
				else if( !stricmp( token, "inset" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowX = atof( token );
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowY = atof( token );
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowWidth = atof( token );
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowHeight = atof( token );
				}
				else
				{
					gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename, token );
					goto end;
				}

				pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
			}
		}
		else if( !stricmp( token, "layer" ) )
		{
			// parse a layer data
			if( m_OverviewData.layers == OVERVIEW_MAX_LAYERS )
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. ( too many layers )\n", filename );
				goto end;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			if( stricmp( token, "{" ) ) 
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename );
				goto end;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			while( stricmp( token, "}") )
			{
				if( !stricmp( token, "image" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					strcpy( m_OverviewData.layersImages[m_OverviewData.layers], token );
				} 
				else if ( !stricmp( token, "height" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token ); 
					height = atof( token );
					m_OverviewData.layersHeights[m_OverviewData.layers] = height;
				}
				else
				{
					gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename, token );
					goto end;
				}

				pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
			}

			m_OverviewData.layers++;
		}
	}

	m_mapZoom = m_OverviewData.zoom;
	m_mapOrigin = m_OverviewData.origin;

	ret = true;
end:
	gEngfuncs.COM_FreeFile( afile );

	return ret;
}

void CHudSpectator::LoadMapSprites()
{
	// right now only support for one map layer
	if( m_OverviewData.layers > 0 )
	{
		m_MapSprite = gEngfuncs.LoadMapSprite( m_OverviewData.layersImages[0] );
	}
	else
		m_MapSprite = NULL; // the standard "unkown map" sprite will be used instead
}

void CHudSpectator::DrawOverviewLayer()
{
	float screenaspect, xs, ys, xStep, yStep, x, y, z;
	int ix, iy, i, xTiles, yTiles, frame;

	qboolean	 hasMapImage = m_MapSprite ? TRUE : FALSE;
	model_t		*dummySprite = (struct model_s *)gEngfuncs.GetSpritePointer( m_hsprUnkownMap );

	if( hasMapImage )
	{
		i = m_MapSprite->numframes / ( 4 * 3 );
		i = sqrt( float( i ) );
		xTiles = i * 4;
		yTiles = i * 3;
	}
	else
	{
		xTiles = 8;
		yTiles = 6;
	}

	SCREENINFO screenInfo;

	screenInfo.iSize = sizeof( SCREENINFO );
	gEngfuncs.pfnGetScreenInfo( &screenInfo );

	screenaspect = (float)screenInfo.iWidth / (float)screenInfo.iHeight;

	xs = m_OverviewData.origin[0];
	ys = m_OverviewData.origin[1];
	z  = ( 90.0f - v_angles[0] ) / 90.0f;		
	z *= m_OverviewData.layersHeights[0]; // gOverviewData.z_min - 32;	

	// i = r_overviewTexture + ( layer * OVERVIEW_X_TILES * OVERVIEW_Y_TILES );

	gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );

	frame = 0;

	// rotated view ?
	if( m_OverviewData.rotated )
	{
		xStep = ( 2 * 4096.0f / m_OverviewData.zoom ) / xTiles;
		yStep = -( 2 * 4096.0f / ( m_OverviewData.zoom* screenaspect ) ) / yTiles;

		y = ys + ( 4096.0f / ( m_OverviewData.zoom * screenaspect ) );

		for( iy = 0; iy < yTiles; iy++ )
		{
			x = xs - ( 4096.0f / (m_OverviewData.zoom ) );

			for( ix = 0; ix < xTiles; ix++ )
			{
				if( hasMapImage )
					gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );
				else
					gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

				gEngfuncs.pTriAPI->Begin( TRI_QUADS );
					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					gEngfuncs.pTriAPI->Vertex3f( x, y, z );

					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
					gEngfuncs.pTriAPI->Vertex3f( x + xStep, y + yStep, z );

					gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
					gEngfuncs.pTriAPI->Vertex3f( x, y + yStep, z);
				gEngfuncs.pTriAPI->End();
				gEngfuncs.pTriAPI->RenderMode( kRenderNormal );

				frame++;
				x += xStep;
			}

			y += yStep;
		}
	} 
	else
	{
		xStep = -( 2 * 4096.0f / m_OverviewData.zoom ) / xTiles;
		yStep = -( 2 * 4096.0f / ( m_OverviewData.zoom* screenaspect ) ) / yTiles;

		x = xs + ( 4096.0f / ( m_OverviewData.zoom * screenaspect ) );

		for( ix = 0; ix < yTiles; ix++ )
		{
			y = ys + ( 4096.0f / ( m_OverviewData.zoom ) );	

			for( iy = 0; iy < xTiles; iy++ )	
			{
				if( hasMapImage )
					gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );
				else
					gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

				gEngfuncs.pTriAPI->Begin( TRI_QUADS );
					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					gEngfuncs.pTriAPI->Vertex3f( x, y, z );

					gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
					gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
					gEngfuncs.pTriAPI->Vertex3f( x + xStep, y + yStep, z );

					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					gEngfuncs.pTriAPI->Vertex3f( x, y + yStep, z );
				gEngfuncs.pTriAPI->End();
				gEngfuncs.pTriAPI->RenderMode( kRenderNormal );

				frame++;

				y += yStep;
			}

			x += xStep;
		}
	}
}

void CHudSpectator::DrawOverviewEntities()
{
	int			i;
	struct model_s *hSpriteModel;
	vec3_t			origin, angles, point, forward, right, left, up, world, screen, offset;
	float			x, y, z, r, g, b, sizeScale = 4.0f;
	cl_entity_t *	ent;
	float rmatrix[3][4];	// transformation matrix

	float			zScale = ( 90.0f - v_angles[0] ) / 90.0f;

	z = m_OverviewData.layersHeights[0] * zScale;
	r = g = b = 1.0f;

	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	for( i = 0; i < MAX_PLAYERS; i++ )
		m_vPlayerPos[i][2] = -1;	// mark as invisible 

	// draw all players
	for( i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if( !m_OverviewEntities[i].hSprite )
			continue;

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_OverviewEntities[i].hSprite );
		ent = m_OverviewEntities[i].entity;

		gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );
		gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );

		// see R_DrawSpriteModel
		// draws players sprite
		AngleVectors( ent->angles, right, up, NULL );

		VectorCopy( ent->origin,origin );

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

		gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		VectorMA( origin, 16.0f * sizeScale, up, point );
		VectorMA( point, 16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

		VectorMA( origin, 16.0f * sizeScale, up, point );
		VectorMA( point, -16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
		VectorMA( origin, -16.0f * sizeScale, up, point );
		VectorMA( point, -16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
		VectorMA( origin, -16.0f * sizeScale, up, point );
		VectorMA( point, 16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->End();

		if( !ent->player )
			continue;

		// draw line under player icons
		origin[2] *= zScale;

		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_hsprBeam );
		gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );

		gEngfuncs.pTriAPI->Color4f( r, g, b, 0.3f );

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4.0f, origin[1] + 4.0f, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4.0f, origin[1] - 4.0f, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4.0f, origin[1] - 4.0f, z );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4.0f, origin[1] + 4.0f, z );
		gEngfuncs.pTriAPI->End();

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4.0f, origin[1] + 4.0f, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4.0f, origin[1] - 4.0f, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4.0f, origin[1] - 4.0f, z );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4.0f, origin[1] + 4.0f, z );
		gEngfuncs.pTriAPI->End();

		// calculate screen position for name and infromation in hud::draw()
		if( gEngfuncs.pTriAPI->WorldToScreen( origin, screen ) )
			continue;	// object is behind viewer

		screen[0] = XPROJECT( screen[0] );
		screen[1] = YPROJECT( screen[1] );
		screen[2] = 0.0f;

		// calculate some offset under the icon
		origin[0] += 32.0f;
		origin[1] += 32.0f;

		gEngfuncs.pTriAPI->WorldToScreen( origin, offset );

		offset[0] = XPROJECT( offset[0] );
		offset[1] = YPROJECT( offset[1] );
		offset[2] = 0.0f;

		VectorSubtract( offset, screen, offset );

		int playerNum = ent->index - 1;

		m_vPlayerPos[playerNum][0] = screen[0];	
		m_vPlayerPos[playerNum][1] = screen[1] + Length( offset );	
		m_vPlayerPos[playerNum][2] = 1;	// mark player as visible 
	}

	if( !m_pip->value || !m_drawcone->value )
		return;

	// get current camera position and angle
	if( m_pip->value == INSET_IN_EYE || g_iUser1 == OBS_IN_EYE )
	{
		V_GetInEyePos( g_iUser2, origin, angles );
	}
	else if( m_pip->value == INSET_CHASE_FREE || g_iUser1 == OBS_CHASE_FREE )
	{
		V_GetChasePos( g_iUser2, v_cl_angles, origin, angles );
	}
	else if( g_iUser1 == OBS_ROAMING )
	{
		VectorCopy( v_sim_org, origin );
		VectorCopy( v_cl_angles, angles );
	}
	else
		V_GetChasePos( g_iUser2, NULL, origin, angles );

	// draw camera sprite
	x = origin[0];
	y = origin[1];
	z = origin[2];

	angles[0] = 0; // always show horizontal camera sprite

	hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_hsprCamera );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );

	gEngfuncs.pTriAPI->Color4f( r, g, b, 1.0f );

	AngleVectors( angles, forward, NULL, NULL );
	VectorScale( forward, 512.0f, forward );

	offset[0] = 0.0f; 
	offset[1] = 45.0f; 
	offset[2] = 0.0f; 

	AngleMatrix( offset, rmatrix );
	VectorTransform( forward, rmatrix, right );

	offset[1]= -45.0f;
	AngleMatrix( offset, rmatrix );
	VectorTransform( forward, rmatrix , left );

	gEngfuncs.pTriAPI->Begin( TRI_TRIANGLES );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( x + right[0], y + right[1], ( z + right[2] ) * zScale);

		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( x, y, z * zScale );

		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( x + left[0], y + left[1], ( z + left[2] ) * zScale );
	gEngfuncs.pTriAPI->End ();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

void CHudSpectator::DrawOverview()
{
	/* Nothing. */
}

void CHudSpectator::CheckOverviewEntities()
{
	double time = gEngfuncs.GetClientTime();

	// removes old entities from list
	for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		// remove entity from list if it is too old
		if( m_OverviewEntities[i].killTime < time )
		{
			memset( &m_OverviewEntities[i], 0, sizeof(overviewEntity_t) );
		}
	}
}

bool CHudSpectator::AddOverviewEntity( int type, cl_entity_s *ent, const char *modelname )
{
	HSPRITE	hSprite = 0;
	double  duration = -1.0;	// duration -1 means show it only this frame;

	if( !ent )
		return false;

	if( type == ET_PLAYER )
	{
		if( ent->curstate.solid != SOLID_NOT && ent->curstate.movetype != MOVETYPE_TOSS && ent->curstate.movetype != MOVETYPE_NONE )
		{
			if( g_PlayerExtraInfo[ent->index].teamnumber == 1 )
				hSprite = gHUD.m_bBritish ? m_hsprBritLight : m_hsprAllieLight;
			else
				hSprite = m_hsprAxisLight;
		}
		else
			return false;	// it's an spectator
	}
	else
		return false;

	return AddOverviewEntityToList( hSprite, ent, gEngfuncs.GetClientTime() + duration );
}

void CHudSpectator::DeathMessage( int victim )
{
	// find out where the victim is
	cl_entity_t *pl = gEngfuncs.GetEntityByIndex( victim );

	if( pl && pl->player )
	{
		AddOverviewEntityToList( m_hsprPlayerDead, pl, gEngfuncs.GetClientTime() + 2.0f );

		// the player has died, the observer flags of the death cam
		if( pl->index == gEngfuncs.GetLocalPlayer()->index )
		{
			m_iObserverFlags = 544;
			V_ResetChaseCam();
		}
	}
}

bool CHudSpectator::AddOverviewEntityToList( HSPRITE sprite, cl_entity_t *ent, double killTime )
{
	for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		// the entity is in the list already, it lives longer now
		if( m_OverviewEntities[i].entity == ent )
		{
			m_OverviewEntities[i].killTime = killTime;
			return true;
		}

		// find empty entity slot
		if( m_OverviewEntities[i].entity == NULL )
		{
			m_OverviewEntities[i].entity = ent;
			m_OverviewEntities[i].hSprite = sprite;
			m_OverviewEntities[i].killTime = killTime;
			return true;
		}
	}

	return false;	// maximum overview entities reached
}

bool CHudSpectator::AddOverviewEntityToMap( HSPRITE sprite, cl_entity_t *ent, double killTime, Vector origin )
{
	for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		// find empty entity slot
		if( m_OverviewEntities[i].entity == NULL )
		{
			m_OverviewEntities[i].entity = ent;
			m_OverviewEntities[i].hSprite = sprite;
			m_OverviewEntities[i].killTime = killTime;
			m_OverviewEntities[i].b_dodMapTag = true;
			m_OverviewEntities[i].origin = origin;
			return true;
		}
	}

	return false;	// maximum overview entities reached
}

void CHudSpectator::CheckSettings()
{
	// the game has no inset window
	m_pip->value = INSET_OFF;

	// check chat mode
	if( m_chatEnabled != (gHUD.m_SayText.m_HUD_saytext->value != 0) )
	{
		// hud_saytext changed
		m_chatEnabled = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );

		if( gEngfuncs.IsSpectateOnly() )
		{
			// tell proxy our new chat mode
			char chatcmd[256];
			safe_snprintf( chatcmd, sizeof( chatcmd ), "ignoremsg %i", m_chatEnabled ? 0 : 1 );
			gEngfuncs.pfnServerCmd( chatcmd );
		}
	}
}

int CHudSpectator::ToggleInset( bool allowOff )
{
	return 0;
}

void CHudSpectator::Reset()
{
	// Reset HUD
	if( strcmp( m_OverviewData.map, gEngfuncs.pfnGetLevelName() ) )
	{
		// update level overview if level changed
		ParseOverviewFile();
		LoadMapSprites();
	}

	memset( &m_OverviewEntities, 0, sizeof(m_OverviewEntities) );

	SetSpectatorStartPosition();
}

void CHudSpectator::InitHUDData()
{
	m_lastPrimaryObject = m_lastSecondaryObject = 0;
	iJumpSpectator = 0;
	g_iUser2 = 0;
	m_lastHudMessage = 0;
	m_flNextObserverInput = 0.0f;
	m_iSpectatorNumber = 0;
	g_iUser1 = 0;

	memset( &m_OverviewData, 0, sizeof(m_OverviewData));
	memset( &m_OverviewEntities, 0, sizeof(m_OverviewEntities));

	m_autoDirector->value = 1.0f;

	Reset();

	SetModes( OBS_CHASE_FREE, INSET_OFF );

	g_iUser2 = 0; // fake not target until first camera command

	// reset HUD FOV
	gHUD.m_iFOV = default_fov ? (int)default_fov->value : 90;
}

HSPRITE CHudSpectator::GetMarkerSPR( int marker )
{
	if( (unsigned int)marker <= 14 )
		return m_hsprMapMarkers[marker];

	return 0;
}

void CHudSpectator::AddVoiceIconToPlayerEnt( int index )
{
	if( (unsigned int)( index - 1 ) < MAX_PLAYERS )
		m_bAddDrawIconNextFrame[index - 1] = true;
}

void CHudSpectator::ClearVoiceIconFlags( void )
{
	for( int i = 0; i < MAX_PLAYERS; i++ )
		m_bAddDrawIconNextFrame[i] = false;
}

bool CHudSpectator::ShouldSetVoiceIcon( int index )
{
	if( (unsigned int)( index - 1 ) < MAX_PLAYERS && m_bAddDrawIconNextFrame[index - 1] )
	{
		m_bAddDrawIconNextFrame[index - 1] = false;
		return true;
	}

	return false;
}
