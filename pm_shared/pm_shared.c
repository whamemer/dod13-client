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

#include <assert.h>
//#include <stdio.h>  // NULL
#include <math.h>   // sqrt
#include <string.h> // strcpy
#include <stdlib.h> // atoi
#include <ctype.h>  // isspace
#include "mathlib.h"
#if HAVE_TGMATH_H
#include <tgmath.h>
#endif

#include "const.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pm_movevars.h"
#include "pm_debug.h"

// Spectator Mode
int iJumpSpectator;
extern float vJumpOrigin[3];
extern float vJumpAngles[3];

static int pm_shared_initialized = 0;

#if _MSC_VER
#pragma warning( disable : 4305 )
#endif

playermove_t *pmove = NULL;

// Ducking time
#define TIME_TO_DUCK		0.4f
#define VEC_DUCK_HULL_MIN	-18
#define VEC_DUCK_HULL_MAX	18
#define VEC_DUCK_VIEW		12
#define PM_DEAD_VIEWHEIGHT	-8
#define MAX_CLIMB_SPEED		200
#define STUCK_MOVEUP		1
#define STUCK_MOVEDOWN		-1
#define VEC_HULL_MIN		-36
#define VEC_HULL_MAX		36
#define VEC_VIEW		28
#define	STOP_EPSILON		0.1f

#define CTEXTURESMAX		512			// max number of textures loaded
#include "pm_materials.h"

#define STEP_CONCRETE		0		// default step sound
#define STEP_METAL		1		// metal floor
#define STEP_DIRT		2		// dirt, sand, rock
#define STEP_VENT		3		// ventillation duct
#define STEP_GRATE		4		// metal grating
#define STEP_TILE		5		// floor tiles
#define STEP_SLOSH		6		// shallow liquid puddle
#define STEP_WADE		7		// wading in liquid
#define STEP_LADDER		8		// climbing ladder
#define STEP_WOOD		9	
#define STEP_GRAVEL		14
#define STEP_SNOW		15

#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD	(float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

#define PLAYER_LONGJUMP_SPEED		350 // how fast we longjump

#define PLAYER_DUCKING_MULTIPLIER	0.333f

// double to float warning
#if _MSC_VER
#pragma warning(disable : 4244)
#endif

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
// up / down
#define	PITCH		0
// left / right
#define	YAW		1
// fall over
#define	ROLL		2 

#define MAX_CLIENTS	32

#define	CONTENTS_CURRENT_0		-9
#define	CONTENTS_CURRENT_90		-10
#define	CONTENTS_CURRENT_180		-11
#define	CONTENTS_CURRENT_270		-12
#define	CONTENTS_CURRENT_UP		-13
#define	CONTENTS_CURRENT_DOWN		-14

#define CONTENTS_TRANSLUCENT		-15

static vec3_t rgv3tStuckTable[54];
static int rgStuckLast[MAX_CLIENTS][2];

// Texture names
static int gcTextures = 0;
static char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	
static char grgchTextureType[CTEXTURESMAX];

int g_onladder = 0;
int g_jumped = 0;
int g_prone = 0;
float flFallTime;

int IsProne( int i )
{
	return ( i - 1 ) <= 1;
}

int IsMortarDeployed( int i )
{
	return i == 3;
}

static void PM_InitTrace( trace_t *trace, const vec3_t end )
{
	memset( trace, 0, sizeof( *trace ));
	VectorCopy( end, trace->endpos );
	trace->allsolid = true;
	trace->fraction = 1.0f;
}

static void PM_TraceModel( physent_t *pe, float *start, float *end, trace_t *trace )
{
	PM_InitTrace( trace, end );
	pmove->PM_TraceModel(pe, start, end, trace);
}

void PM_SwapTextures( int i, int j )
{
	char chTemp;
	char szTemp[CBTEXTURENAMEMAX];

	strcpy( szTemp, grgszTextureName[i] );
	chTemp = grgchTextureType[i];
	
	strcpy( grgszTextureName[i], grgszTextureName[j] );
	grgchTextureType[i] = grgchTextureType[j];

	strcpy( grgszTextureName[j], szTemp );
	grgchTextureType[j] = chTemp;
}

void PM_SortTextures( void )
{
	// Bubble sort, yuck, but this only occurs at startup and it's only 512 elements...
	//
	int i, j;

	for( i = 0; i < gcTextures; i++ )
	{
		for( j = i + 1; j < gcTextures; j++ )
		{
			if( stricmp( grgszTextureName[i], grgszTextureName[j] ) > 0 )
			{
				// Swap
				//
				PM_SwapTextures( i, j );
			}
		}
	}
}

// ===================== MATERIAL TYPE DETECTION, MAIN ROUTINES ========================
//
// Used to detect the texture the player is standing on, map the
// texture name to a material type.  Play footstep sound based
// on material type.

// open materials.txt,  get size, alloc space,
// save in array.  Only works first time called,
// ignored on subsequent calls.

char *PM_memfgets( byte *pMemFile, int fileSize, int *pFilePos, char *pBuffer, int bufferSize )
{
	// Bullet-proofing
	if( !pMemFile || !pBuffer || !pFilePos)
		return NULL;

	if( *pFilePos >= fileSize )
		return NULL;

	int i = *pFilePos;
	int last = fileSize;

	// fgets always NULL terminates, so only read bufferSize-1 characters
	if( last - *pFilePos > ( bufferSize - 1 ) )
		last = *pFilePos + ( bufferSize - 1 );

	int stop = 0;

	// Stop at the next newline (inclusive) or end of buffer
	while( i < last && !stop )
	{
		if( pMemFile[i] == '\n' )
			stop = 1;
		i++;
	}

	// If we actually advanced the pointer, copy it over
	if( i != *pFilePos )
	{
		// We read in size bytes
		int size = i - *pFilePos;
		// copy it out
		memcpy( pBuffer, pMemFile + *pFilePos, sizeof(byte) * size );

		// If the buffer isn't full, terminate (this is always true)
		if( size < bufferSize )
			pBuffer[size] = 0;

		// Update file pointer
		*pFilePos = i;
		return pBuffer;
	}

	// No data read, bail
	return NULL;
}

void PM_InitTextureTypes( void )
{
	char buffer[512];
	int i, j;
	byte *pMemFile;
	int fileSize, filePos = 0;
	static qboolean bTextureTypeInit = false;

	if( bTextureTypeInit )
		return;

	memset(&( grgszTextureName[0][0] ), 0, sizeof( grgszTextureName ) );
	memset( grgchTextureType, 0, sizeof( grgchTextureType ) );

	gcTextures = 0;

	pMemFile = pmove->COM_LoadFile( "sound/materials.txt", 5, &fileSize );
	if( !pMemFile )
		return;

	memset( buffer, 0, sizeof( buffer ) );

	// for each line in the file...
	while( PM_memfgets( pMemFile, fileSize, &filePos, buffer, 511 ) != NULL && (gcTextures < CTEXTURESMAX ) )
	{
		// skip whitespace
		i = 0;
		while( buffer[i] && isspace( buffer[i] ) )
			i++;

		if( !buffer[i] )
			continue;

		// skip comment lines
		if( buffer[i] == '/' || !isalpha( buffer[i] ) )
			continue;

		// get texture type
		grgchTextureType[gcTextures] = toupper( buffer[i++] );

		// skip whitespace
		while( buffer[i] && isspace( buffer[i] ) )
			i++;
		
		if( !buffer[i] )
			continue;

		// get sentence name
		j = i;
		while( buffer[j] && !isspace( buffer[j] ) )
			j++;

		if( !buffer[j] )
			continue;

		// null-terminate name and save in sentences array
		j = min( j, CBTEXTURENAMEMAX - 1 + i );
		buffer[j] = 0;
		strcpy( &( grgszTextureName[gcTextures++][0] ), &( buffer[i] ) );
	}

	// Must use engine to free since we are in a .dll
	pmove->COM_FreeFile( pMemFile );

	PM_SortTextures();

	bTextureTypeInit = true;
}

char PM_FindTextureType( char *name )
{
	int left, right, pivot;
	int val;

	assert( pm_shared_initialized );

	left = 0;
	right = gcTextures - 1;

	while( left <= right )
	{
		pivot = ( left + right ) / 2;

		val = strnicmp( name, grgszTextureName[pivot], CBTEXTURENAMEMAX - 1 );
		if( val == 0 )
		{
			return grgchTextureType[pivot];
		}
		else if( val > 0 )
		{
			left = pivot + 1;
		}
		else if( val < 0 )
		{
			right = pivot - 1;
		}
	}

	return CHAR_TEX_CONCRETE;
}

void PM_PlayStepSound( int step, float fvol )
{
	static int iSkipStep = 0;
	int irand;
	vec3_t hvel;

	pmove->iStepLeft = !pmove->iStepLeft;

	if( !pmove->runfuncs )
		return;

	int iMovementState = pmove->iuser3;

	if( ( iMovementState == 1 || iMovementState == 2 ) || iMovementState == 3 )
		return;

	VectorCopy( pmove->velocity, hvel );
	hvel[2] = 0.0f;

	if( !g_onladder && Length( hvel ) <= 100.0f )
		return;

	irand = pmove->RandomLong( 0, 1 ) + ( pmove->iStepLeft * 2 );

	switch( step )
	{
	default:
	case STEP_CONCRETE:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_step1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_step3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_step2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_step4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_METAL:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_DIRT:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_VENT:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_GRATE:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_TILE:
		if( !pmove->RandomLong( 0, 4 ) )
			irand = 4;
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 4: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile5.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_SLOSH:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_WADE:
		if( iSkipStep != 0 )
		{
			if( iSkipStep == 3 )
				iSkipStep = 0;
			else
				iSkipStep++;

			switch( irand )
			{
			case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
			case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
			case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
			case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
			}
		}
		else
		{
			iSkipStep = 1;
		}
		break;
	case STEP_LADDER:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_WOOD:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wood1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wood3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wood2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_wood4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_GRAVEL:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_gravel1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_gravel3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_gravel2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_gravel4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	case STEP_SNOW:
		switch( irand )
		{
		case 0: pmove->PM_PlaySound( CHAN_BODY, "player/pl_snow1.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: pmove->PM_PlaySound( CHAN_BODY, "player/pl_snow3.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2: pmove->PM_PlaySound( CHAN_BODY, "player/pl_snow2.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3: pmove->PM_PlaySound( CHAN_BODY, "player/pl_snow4.wav", fvol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		break;
	}
}

int PM_MapTextureTypeStepType( char chTextureType )
{
	switch( chTextureType )
	{
		default:
		case CHAR_TEX_CONCRETE:
			return STEP_CONCRETE;	
		case CHAR_TEX_METAL:
			return STEP_METAL;	
		case CHAR_TEX_DIRT:
			return STEP_DIRT;	
		case CHAR_TEX_VENT:
			return STEP_VENT;	
		case CHAR_TEX_GRATE:
			return STEP_GRATE;	
		case CHAR_TEX_TILE:
			return STEP_TILE;
		case CHAR_TEX_SLOSH:
			return STEP_SLOSH;
		case CHAR_TEX_WOOD:
			return STEP_WOOD;
		case CHAR_TEX_GRAVEL:
			return STEP_GRAVEL;
		case CHAR_TEX_SNOW:
			return STEP_SNOW;
	}
}

/*
====================
PM_CatagorizeTextureType

Determine texture info for the texture we are standing on.
====================
*/
void PM_CatagorizeTextureType( void )
{
	vec3_t start, end;
	const char *pTextureName;

	VectorCopy( pmove->origin, start );
	VectorCopy( pmove->origin, end );

	// Straight down
	end[2] -= 64;

	// Fill in default values, just in case.
	pmove->sztexturename[0] = '\0';
	pmove->chtexturetype = CHAR_TEX_CONCRETE;

	pTextureName = pmove->PM_TraceTexture( pmove->onground, start, end );
	if( !pTextureName )
		return;

	// strip leading '-0' or '+0~' or '{' or '!'
	if( *pTextureName == '-' || *pTextureName == '+' )
		pTextureName += 2;

	if( *pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ' )
		pTextureName++;
	// '}}'
	
	strcpy( pmove->sztexturename, pTextureName);
	pmove->sztexturename[CBTEXTURENAMEMAX - 1] = 0;

	// get texture type
	pmove->chtexturetype = PM_FindTextureType( pmove->sztexturename );	
}

void PM_UpdateStepSound( void )
{
	int fWalking;
	float fvol;
	vec3_t knee;
	vec3_t feet;
	float height;
	float speed;
	float velrun;
	float velwalk;
	float flduck;
	int fLadder;
	int step;
	int bSilentLadder;

	if( pmove->flTimeStepSound > 0 )
		return;

	PM_CatagorizeTextureType();

	speed = Length( pmove->velocity );
	fLadder = ( pmove->movetype == MOVETYPE_FLY );

	if( ( pmove->flags & FL_DUCKING ) || fLadder )
	{
		flduck = 100.0f;
		velrun = 80.0f;
		velwalk = 60.0f;
	}
	else
	{
		flduck = 0.0f;
		velrun = 210.0f;
		velwalk = 81.0f;
	}

	if( pmove->onground == -1 && !fLadder )
		return;

	if( ( Length( pmove->velocity ) > 0.0f ) && ( speed >= velwalk || !pmove->flTimeStepSound ) )
	{
		float flBaseTime = 400.0f;

		VectorCopy( pmove->origin, knee );
		VectorCopy( pmove->origin, feet );

		height = pmove->player_maxs[pmove->usehull][2] - pmove->player_mins[pmove->usehull][2];

		knee[2] = pmove->origin[2] - ( 0.3f * height );
		feet[2] = pmove->origin[2] - ( 0.5f * height );

		if( fLadder )
		{
			step = STEP_LADDER;
			flBaseTime = 350.0f;

			bSilentLadder = ( ( pmove->flags & 0x40000 ) == 0 );

			if( !bSilentLadder )
				flBaseTime = 700.0f;
		}
		else if( pmove->PM_PointContents( knee, NULL ) == CONTENTS_WATER )
		{
			step = STEP_WADE;
			flBaseTime = 600.0f;
		}
		else if( pmove->PM_PointContents( feet, NULL ) == CONTENTS_WATER )
		{
			step = STEP_SLOSH;
			flBaseTime = ( speed >= velrun ) ? 300.0f : 400.0f;
		}
		else
		{
			step = PM_MapTextureTypeStepType( pmove->chtexturetype );

			switch( pmove->chtexturetype )
			{
			case CHAR_TEX_DIRT:
				flBaseTime = ( speed >= velrun ) ? 300.0f : 400.0f;
				break;
			default:
				flBaseTime = ( speed >= velrun ) ? 300.0f : 400.0f;
				break;
			}
		}

		pmove->flTimeStepSound = flBaseTime + flduck;
		pmove->iStepLeft = !pmove->iStepLeft;

		if( pmove->runfuncs )
			PM_PlayStepSound( step, fvol );
	}
}

/*
================
PM_AddToTouched

Add's the trace result to touch list, if contact is not already in list.
================
*/
qboolean PM_AddToTouched( pmtrace_t tr, vec3_t impactvelocity )
{
	int i;

	for( i = 0; i < pmove->numtouch; i++ )
	{
		if( pmove->touchindex[i].ent == tr.ent )
			break;
	}
	if( i != pmove->numtouch )  // Already in list.
		return false;

	VectorCopy( impactvelocity, tr.deltavelocity );

	if( pmove->numtouch >= MAX_PHYSENTS )
		pmove->Con_DPrintf( "Too many entities were touched!\n" );

	pmove->touchindex[pmove->numtouch++] = tr;
	return true;
}

/*
================
PM_CheckVelocity

See if the player has a bogus velocity value.
================
*/
void PM_CheckVelocity( void )
{
	int i;

//
// bound velocity
//
	for( i = 0; i < 3; i++ )
	{
		// See if it's bogus.
		if( IS_NAN( pmove->velocity[i] ) )
		{
			pmove->Con_Printf( "PM  Got a NaN velocity %i\n", i );
			pmove->velocity[i] = 0;
		}
		if( IS_NAN( pmove->origin[i] ) )
		{
			pmove->Con_Printf( "PM  Got a NaN origin on %i\n", i );
			pmove->origin[i] = 0;
		}

		// Bound it.
		if( pmove->velocity[i] > pmove->movevars->maxvelocity )
		{
			pmove->Con_DPrintf( "PM  Got a velocity too high on %i\n", i );
			pmove->velocity[i] = pmove->movevars->maxvelocity;
		}
		else if( pmove->velocity[i] < -pmove->movevars->maxvelocity )
		{
			pmove->Con_DPrintf( "PM  Got a velocity too low on %i\n", i );
			pmove->velocity[i] = -pmove->movevars->maxvelocity;
		}
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags:
0x01 == floor
0x02 == step / wall
==================
*/
int PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce )
{
	float backoff;
	float change;
	float angle;
	int i, blocked;
	
	angle = normal[2];

	blocked = 0x00;            // Assume unblocked.
	if( angle > 0 )      // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;
	if( !angle )         // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = DotProduct( in, normal ) * overbounce;

	for( i = 0; i < 3; i++ )
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		// If out velocity is too small, zero it out.
		if( out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON )
			out[i] = 0;
	}

	// Return blocking flags.
	return blocked;
}

void PM_AddCorrectGravity( void )
{
	float ent_gravity;

	if( pmove->waterjumptime )
		return;

	if( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0f;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * 0.5f * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;

	PM_CheckVelocity();
}

void PM_FixupGravityVelocity( void )
{
	float ent_gravity;

	if( pmove->waterjumptime )
		return;

	if( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0f;

	// Get the correct velocity for the end of the dt 
  	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * pmove->frametime * 0.5f );

	PM_CheckVelocity();
}

/*
============
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
int PM_FlyMove( void )
{
	int bumpcount, numbumps;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity, original_velocity;
	vec3_t new_velocity;
	int i, j;
	pmtrace_t trace;
	vec3_t end;
	float time_left, allFraction;
	int blocked;

	numbumps = 4;           // Bump up to four times

	blocked = 0;           // Assume not blocked
	numplanes = 0;           // and not sliding along any planes
	VectorCopy( pmove->velocity, original_velocity );  // Store original velocity
	VectorCopy( pmove->velocity, primal_velocity );

	allFraction = 0;
	time_left = pmove->frametime;   // Total time for this movement operation.

	for( bumpcount = 0; bumpcount < numbumps; bumpcount++ )
	{
		if( !pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2] )
			break;

		// Assume we can move all the way from the current origin to the end point.
		for( i = 0; i < 3; i++ )
			end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

		trace = pmove->PM_PlayerTrace( pmove->origin, end, PM_NORMAL, pmove->usehull );

		allFraction += trace.fraction;

		if( trace.allsolid )
		{	// entity is trapped in another solid
			VectorCopy( vec3_origin, pmove->velocity );
			return 4;
		}

		if( trace.fraction > 0 )
		{	// actually covered some distance
			VectorCopy( trace.endpos, pmove->origin );
			VectorCopy( pmove->velocity, original_velocity );
			numplanes = 0;
		}

		if( trace.fraction == 1 )
			break;		// moved the entire distance

		PM_AddToTouched( trace, pmove->velocity );

		if( trace.plane.normal[2] > 0.7f )
		{
			blocked |= 1; // floor
		}
		if( !trace.plane.normal[2] )
		{
			blocked |= 2; // step / wall
		}

		time_left -= time_left * trace.fraction;

		if( numplanes >= MAX_CLIP_PLANES )
		{
			VectorCopy( vec3_origin, pmove->velocity );
			break;
		}

		VectorCopy( trace.plane.normal, planes[numplanes] );
		numplanes++;

		if( numplanes == 1 && pmove->movetype == MOVETYPE_WALK && ( ( pmove->onground == -1 ) || ( pmove->friction != 1 ) ) )
		{
			for( i = 0; i < numplanes; i++ )
			{
				if( planes[i][2] > 0.7f )
				{
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
					VectorCopy( new_velocity, original_velocity );
				}
				else
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1.0f + pmove->movevars->bounce * ( 1.0f - pmove->friction ) );
			}

			VectorCopy( new_velocity, pmove->velocity );
			VectorCopy( new_velocity, original_velocity );
		}
		else
		{
			for( i = 0; i < numplanes; i++ )
			{
				PM_ClipVelocity( original_velocity, planes[i], pmove->velocity, 1 );
				for( j = 0; j < numplanes; j++ )
					if( j != i )
					{
						if( DotProduct( pmove->velocity, planes[j] ) < 0 )
							break;	// not ok
					}
				if( j == numplanes )  // Didn't have to clip, so we're ok
					break;
			}

			if( i != numplanes )
			{
				// go along this plane
			}
			else
			{	// go along the crease
				if( numplanes != 2 )
				{
					VectorCopy( vec3_origin, pmove->velocity );
					break;
				}
				CrossProduct( planes[0], planes[1], dir );
				d = DotProduct( dir, pmove->velocity );
				VectorScale( dir, d, pmove->velocity );
			}

			if( DotProduct( pmove->velocity, primal_velocity ) <= 0 )
			{
				VectorCopy( vec3_origin, pmove->velocity );
				break;
			}
		}
	}

	if( allFraction == 0 )
	{
		VectorCopy( vec3_origin, pmove->velocity );
	}

	return blocked;
}

/*
==============
PM_Accelerate
==============
*/
void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed;

	// Dead player's don't accelerate
	if( pmove->dead )
		return;

	// If waterjumping, don't accelerate
	if( pmove->waterjumptime )
		return;

	// See if we are changing direction a bit
	currentspeed = DotProduct( pmove->velocity, wishdir );

	// Reduce wishspeed by the amount of veer.
	addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if( addspeed <= 0 )
		return;

	// Determine amount of accleration.
	accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;
	
	// Cap at addspeed
	if( accelspeed > addspeed )
		accelspeed = addspeed;
	
	// Adjust velocity.
	for( i = 0; i < 3; i++ )
	{
		pmove->velocity[i] += accelspeed * wishdir[i];	
	}
}

/*
=====================
PM_WalkMove

Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
======================
*/
void PM_WalkMove( void )
{
	//int clip;
	int oldonground;
	int i;

	vec3_t wishvel;
	float spd;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;

	vec3_t dest; //, start;
	vec3_t original, originalvel;
	vec3_t down, downvel;
	float downdist, updist;

	pmtrace_t trace;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;

	VectorNormalize( pmove->forward );  // Normalize remainder of vectors.
	VectorNormalize( pmove->right );    // 

	for( i = 0; i < 2; i++ )       // Determine x and y parts of velocity
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;

	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy( wishvel, wishdir );   // Determine maginitude of speed of move
	wishspeed = VectorNormalize( wishdir );

	//
	// Clamp to server defined max speed
	//
	if( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed / wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}

	// Set pmove velocity
	pmove->velocity[2] = 0;
	PM_Accelerate( wishdir, wishspeed, pmove->movevars->accelerate );
	pmove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	spd = Length( pmove->velocity );

	if( spd < 1.0f )
	{
		VectorClear( pmove->velocity );
		return;
	}

	// If we are not moving, do nothing
	//if( !pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2] )
	//	return;

	oldonground = pmove->onground;

	// first try just moving to the destination
	dest[0] = pmove->origin[0] + pmove->velocity[0] * pmove->frametime;
	dest[1] = pmove->origin[1] + pmove->velocity[1] * pmove->frametime;
	dest[2] = pmove->origin[2];

	// first try moving directly to the next spot
	//VectorCopy( dest, start );
	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, pmove->usehull );
	// If we made it all the way, then copy trace end
	//  as new player position.
	if( trace.fraction == 1 )
	{
		VectorCopy( trace.endpos, pmove->origin );
		return;
	}

	// Don't walk up stairs if not on ground.
	if( oldonground == -1 && pmove->waterlevel  == 0 )
		return;

	if( pmove->waterjumptime )	// If we are jumping out of water, don't do anything more.
		return;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	VectorCopy( pmove->origin, original );	// Save out original pos &
	VectorCopy( pmove->velocity, originalvel ); // velocity.

	// Slide move
	//clip = PM_FlyMove();
	PM_FlyMove();

	// Copy the results out
	VectorCopy( pmove->origin, down );
	VectorCopy( pmove->velocity, downvel );

	// Reset original values.
	VectorCopy( original, pmove->origin );

	VectorCopy( originalvel, pmove->velocity );

	// Start out up one stair height
	VectorCopy( pmove->origin, dest );
	dest[2] += pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, pmove->usehull );
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, pmove->origin );
	}

	// slide move the rest of the way.
	//clip = PM_FlyMove();
	PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy( pmove->origin, dest );
	dest[2] -= pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, pmove->usehull );

	// If we are not on the ground any more then
	//  use the original movement attempt
	if( trace.plane.normal[2] < 0.7f )
		goto usedown;

	// If the trace ended up in empty space, copy the end
	//  over to the origin.
	if( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, pmove->origin );
	}
	// Copy this origion to up.
	VectorCopy( pmove->origin, pmove->up );

	// decide which one went farther
	downdist = ( down[0] - original[0] ) * ( down[0] - original[0] )
			+ ( down[1] - original[1] ) * ( down[1] - original[1] );
	updist = ( pmove->up[0] - original[0] ) * ( pmove->up[0] - original[0] )
			+ ( pmove->up[1]   - original[1] ) * ( pmove->up[1] - original[1] );

	if( downdist > updist )
	{
usedown:
		VectorCopy( down, pmove->origin );
		VectorCopy( downvel, pmove->velocity );
	} else // copy z value from slide move
		pmove->velocity[2] = downvel[2];
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction( void )
{
	float speed, newspeed, control;
	float friction;
	vec3_t start, stop;
	pmtrace_t trace;

	if( pmove->waterjumptime != 0.0f )
		return;

	speed = Length( pmove->velocity );
	if( speed < 0.1f )
		return;

	// apply ground friction
	if( pmove->onground != -1 )  // On an entity that is the ground
	{
		vec3_t start, stop;
		pmtrace_t trace;

		VectorMA( pmove->origin, 16.0f / speed, pmove->velocity, start );
		VectorAdd( pmove->origin, pmove->player_mins[pmove->usehull], start );
		VectorCopy( start, stop );

		stop[2] = start[2] - 34.0f;
		trace = pmove->PM_PlayerTrace( start, stop, PM_NORMAL, pmove->usehull );

		if( trace.fraction == 1.0f )
			friction = pmove->movevars->friction * pmove->movevars->edgefriction;
		else
			friction = pmove->movevars->friction;

		friction *= pmove->friction;

		control = ( speed < pmove->movevars->stopspeed ) ? pmove->movevars->stopspeed : speed;

		float drop = control * friction * pmove->frametime;

		newspeed = speed - drop;
		if( newspeed < 0.0f )
			newspeed = 0.0f;

		newspeed /= speed;
		VectorScale( pmove->velocity, newspeed, pmove->velocity );
	}
}

void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if( pmove->dead )
		return;
	if( pmove->waterjumptime )
		return;

	// Cap speed
	//wishspd = VectorNormalize( pmove->wishveloc );

	if( wishspd > 30 )
		wishspd = 30;
	// Determine veer amount
	currentspeed = DotProduct( pmove->velocity, wishdir );
	// See how much to add
	addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if( addspeed <= 0 )
		return;
	// Determine acceleration speed after acceleration

	accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
	// Cap it
	if( accelspeed > addspeed )
		accelspeed = addspeed;

	// Adjust pmove vel.
	for( i = 0; i < 3; i++ )
	{
		pmove->velocity[i] += accelspeed * wishdir[i];
	}
}

/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove( void )
{
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	vec3_t dest;
	vec3_t temp;
	pmtrace_t trace;

	float speed, newspeed, addspeed, accelspeed;

	for( i = 0; i < 3; i++ )
		wishvel[i] = pmove->forward[i] * pmove->cmd.forwardmove + pmove->right[i] * pmove->cmd.sidemove;

	// Sinking after no other movement occurs
	if( !pmove->cmd.forwardmove && !pmove->cmd.sidemove && !pmove->cmd.upmove )
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += pmove->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	if( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed / wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}

	wishspeed *= 0.8f;

	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	VectorCopy( pmove->velocity, temp );
	speed = VectorNormalize( temp );
	if( speed )
	{
		newspeed = speed - pmove->frametime * speed * pmove->movevars->friction * pmove->friction;

		if( newspeed < 0 )
			newspeed = 0;
		VectorScale( pmove->velocity, newspeed / speed, pmove->velocity );
	}
	else
		newspeed = 0;

	if( wishspeed >= 0.1f )
	{
		addspeed = wishspeed - newspeed;
		if( addspeed > 0 )
		{
			VectorNormalize( wishvel );
			accelspeed = pmove->movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
			if( accelspeed > addspeed )
				accelspeed = addspeed;

			for( i = 0; i < 3; i++ )
				pmove->velocity[i] += accelspeed * wishvel[i];
		}
	}

	VectorMA( pmove->origin, pmove->frametime, pmove->velocity, dest );

	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, pmove->usehull );

	if( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, pmove->origin );
	}
	else
	{
		PM_FlyMove();
	}
}

/*
===================
PM_AirMove

===================
*/
void PM_AirMove( void )
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;
	// Renormalize
	VectorNormalize( pmove->forward );
	VectorNormalize( pmove->right );

	// Determine x and y parts of velocity
	for( i = 0; i < 2; i++ )
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	// Zero out z part of velocity
	wishvel[2] = 0;

	 // Determine maginitude of speed of move
	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	// Clamp to server defined max speed
	if( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed/wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}
	
	PM_AirAccelerate( wishdir, wishspeed, pmove->movevars->airaccelerate );

	// Add in any base velocity to the current velocity.
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	PM_FlyMove();
}

qboolean PM_InWater( void )
{
	return ( pmove->waterlevel > 1 );
}

/*
=============
PM_CheckWater

Sets pmove->waterlevel and pmove->watertype values.
=============
*/
qboolean PM_CheckWater( void )
{
	vec3_t point;
	int cont;
	int truecont;
	float height;
	float heightover2;

	// Pick a spot just above the players feet.
	point[0] = pmove->origin[0] + ( pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0] ) * 0.5f;
	point[1] = pmove->origin[1] + ( pmove->player_mins[pmove->usehull][1] + pmove->player_maxs[pmove->usehull][1] ) * 0.5f;
	point[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2] + 1;

	// Assume that we are not in water at all.
	pmove->waterlevel = 0;
	pmove->watertype = CONTENTS_EMPTY;

	// Grab point contents.
	cont = pmove->PM_PointContents (point, &truecont );
	// Are we under water? (not solid and not empty?)
	if( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
	{
		// Set water type
		pmove->watertype = cont;

		// We are at least at level one
		pmove->waterlevel = 1;

		point[2] = ( pmove->player_mins[pmove->usehull][2] + pmove->player_maxs[pmove->usehull][2] ) * 0.5f + pmove->origin[2];

		cont = pmove->PM_PointContents( point, NULL );
		// If that point is also under water...
		if( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
		{
			// Set a higher water level.
			pmove->waterlevel = 2;

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = pmove->origin[2] + pmove->view_ofs[2];

			cont = pmove->PM_PointContents( point, NULL );
			if( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT ) 
				pmove->waterlevel = 3;  // In over our eyes
		}

		// Adjust velocity based on water current, if any.
		if( ( truecont <= CONTENTS_CURRENT_0 ) && ( truecont >= CONTENTS_CURRENT_DOWN ) )
		{
			// The deeper we are, the stronger the current.
			static vec3_t current_table[] =
			{
				{1, 0, 0},
				{0, 1, 0},
				{-1, 0, 0},
				{0, -1, 0},
				{0, 0, 1},
				{0, 0, -1}
			};

			VectorMA( pmove->basevelocity, 50.0*pmove->waterlevel, current_table[CONTENTS_CURRENT_0 - truecont], pmove->basevelocity );
		}
	}

	return pmove->waterlevel > 1;
}

/*
=============
PM_CatagorizePosition
=============
*/
void PM_CatagorizePosition( void )
{
	vec3_t point;
	pmtrace_t tr;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid	

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	PM_CheckWater();

	point[0] = pmove->origin[0];
	point[1] = pmove->origin[1];
	point[2] = pmove->origin[2] - 2;

	if( pmove->velocity[2] > 180 )   // Shooting up really fast.  Definitely not on ground.
	{
		pmove->onground = -1;
	}
	else
	{
		// Try and move down.
		tr = pmove->PM_PlayerTrace( pmove->origin, point, PM_NORMAL, -1 );
		// If we hit a steep plane, we are not on ground
		if( tr.plane.normal[2] < 0.7f )
			pmove->onground = -1;	// too steep
		else
			pmove->onground = tr.ent;  // Otherwise, point to index of ent under us.

		// If we are on something...
		if( pmove->onground != -1 )
		{
			// Then we are not in water jump sequence
			pmove->waterjumptime = 0;
			// If we could make the move, drop us down that 1 pixel
			if( pmove->waterlevel < 2 && !tr.startsolid && !tr.allsolid )
				VectorCopy( tr.endpos, pmove->origin );
		}

		// Standing on an entity other than the world
		if( tr.ent > 0 ) // So signal that we are touching something.
		{
			PM_AddToTouched( tr, pmove->velocity );
		}
	}
}

/*
=================
PM_GetRandomStuckOffsets

When a player is stuck, it's costly to try and unstick them
Grab a test offset for the player based on a passed in index
=================
*/
int PM_GetRandomStuckOffsets( int nIndex, int server, vec3_t offset )
{
	// Last time we did a full
	int idx;
	idx = rgStuckLast[nIndex][server]++;

	VectorCopy( rgv3tStuckTable[idx % 54], offset );

	return ( idx % 54 );
}

void PM_ResetStuckOffsets( int nIndex, int server )
{
	rgStuckLast[nIndex][server] = 0;
}

/*
=================
NudgePosition

If pmove->origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/
#define PM_CHECKSTUCK_MINTIME 0.05f  // Don't check again too quickly.

int PM_CheckStuck( void )
{
	vec3_t base;
	vec3_t offset;
	vec3_t test;
	int hitent;
	int idx;
	float fTime;
	int i;
	pmtrace_t traceresult;

	static float rgStuckCheckTime[MAX_CLIENTS][2];

	hitent = pmove->PM_TestPlayerPosition( pmove->origin, &traceresult );
	if( hitent == -1 )
	{
		PM_ResetStuckOffsets( pmove->player_index, pmove->server );
		return 0;
	}

	VectorCopy( pmove->origin, base );

	if( !( pmove->server && pmove->multiplayer ) )
	{
		if( ( hitent == 0 ) || ( pmove->physents[hitent].model != NULL ) )
		{
			int nReps = 0;
			PM_ResetStuckOffsets( pmove->player_index, pmove->server );
			do
			{
				i = PM_GetRandomStuckOffsets( pmove->player_index, pmove->server, offset );

				VectorAdd( base, offset, test );
				if( pmove->PM_TestPlayerPosition( test, &traceresult ) == -1 )
				{
					PM_ResetStuckOffsets( pmove->player_index, pmove->server );

					if( i > 26 )
					{
						VectorCopy( test, pmove->origin );
					}
					return 0;
				}
				nReps++;
			} while( nReps < 54 );
		}
	}

	if( pmove->server )
		idx = 0;
	else
		idx = 1;

	fTime = pmove->Sys_FloatTime();
	if( rgStuckCheckTime[pmove->player_index][idx] >= ( fTime - 0.05f ) )
	{
		return 1;
	}
	rgStuckCheckTime[pmove->player_index][idx] = fTime;

	pmove->PM_StuckTouch( hitent, &traceresult );

	i = PM_GetRandomStuckOffsets( pmove->player_index, pmove->server, offset );

	VectorAdd( base, offset, test );
	if( ( hitent = pmove->PM_TestPlayerPosition( test, NULL ) ) == -1 )
	{
		PM_ResetStuckOffsets( pmove->player_index, pmove->server );

		if( i > 26 )
		{
			VectorCopy( test, pmove->origin );
		}
		return 0;
	}

	if( ( pmove->cmd.buttons & 7 ) && ( pmove->physents[hitent].player != 0 ) )
	{
		float x, y, z;
		float xystep = 8.0f;
		float zstep = 18.0f;
		float xyminmax = xystep;
		float zminmax = 4.0f * zstep;

		for( z = 0.0f; z <= zminmax; z += zstep )
		{
			for( x = -xyminmax; x <= xyminmax; x += xystep )
			{
				for( y = -xyminmax; y <= xyminmax; y += xystep )
				{
					VectorCopy( base, test );
					test[0] += x;
					test[1] += y;
					test[2] += z;

					if( pmove->PM_TestPlayerPosition( test, NULL ) == -1 )
					{
						VectorCopy( test, pmove->origin );
						PM_ResetStuckOffsets( pmove->player_index, pmove->server );
						return 0;
					}
				}
			}
		}
	}

	return 1;
}

/*
===============
PM_SpectatorMove
===============
*/
void PM_SpectatorMove( void )
{
	float speed, drop, friction, control, newspeed;
	//float accel;
	float currentspeed, addspeed, accelspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	// this routine keeps track of the spectators psoition
	// there a two different main move types : track player or moce freely (OBS_ROAMING)
	// doesn't need excate track position, only to generate PVS, so just copy
	// targets position and real view position is calculated on client (saves server CPU)
	
	if( pmove->iuser1 == OBS_ROAMING )
	{
		// jump only in roaming mode
		if( iJumpSpectator )
		{
			VectorCopy( vJumpOrigin, pmove->origin );
			VectorCopy( vJumpAngles, pmove->angles );
			VectorCopy( vec3_origin, pmove->velocity );
			iJumpSpectator	= 0;
			return;
		}
		// Move around in normal spectator method
		speed = Length( pmove->velocity );
		if( speed < 1 )
		{
			VectorCopy( vec3_origin, pmove->velocity );
		}
		else
		{
			drop = 0;

			friction = pmove->movevars->friction * 1.5f;	// extra friction
			control = speed < pmove->movevars->stopspeed ? pmove->movevars->stopspeed : speed;
			drop += control * friction*pmove->frametime;

			// scale the velocity
			newspeed = speed - drop;
			if( newspeed < 0 )
				newspeed = 0;
			newspeed /= speed;

			VectorScale( pmove->velocity, newspeed, pmove->velocity );
		}

		// accelerate
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		VectorNormalize( pmove->forward );
		VectorNormalize( pmove->right );

		for( i = 0; i < 3; i++ )
		{
			wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
		}
		wishvel[2] += pmove->cmd.upmove;

		VectorCopy( wishvel, wishdir );
		wishspeed = VectorNormalize( wishdir );

		//
		// clamp to server defined max speed
		//
		if( wishspeed > pmove->movevars->spectatormaxspeed )
		{
			VectorScale( wishvel, pmove->movevars->spectatormaxspeed/wishspeed, wishvel );
			wishspeed = pmove->movevars->spectatormaxspeed;
		}

		currentspeed = DotProduct( pmove->velocity, wishdir );
		addspeed = wishspeed - currentspeed;
		if( addspeed <= 0 )
			return;

		accelspeed = pmove->movevars->accelerate * pmove->frametime * wishspeed;
		if( accelspeed > addspeed )
			accelspeed = addspeed;

		for( i = 0; i < 3; i++ )
			pmove->velocity[i] += accelspeed*wishdir[i];

		// move
		VectorMA( pmove->origin, pmove->frametime, pmove->velocity, pmove->origin );
	}
	else
	{
		// all other modes just track some kind of target, so spectator PVS = target PVS

		int target;

		// no valid target ?
		if( pmove->iuser2 <= 0 )
			return;

		// Find the client this player's targeting
		for( target = 0; target < pmove->numphysent; target++ )
		{
			if( pmove->physents[target].info == pmove->iuser2 )
				break;
		}

		if( target == pmove->numphysent )
			return;

		// use targets position as own origin for PVS
		VectorCopy( pmove->physents[target].angles, pmove->angles );
		VectorCopy( pmove->physents[target].origin, pmove->origin );

		// no velocity
		VectorCopy( vec3_origin, pmove->velocity );
	}
}

/*
==================
PM_SplineFraction

Use for ease-in, ease-out style interpolation (accel/decel)
Used by ducking code.
==================
*/
float PM_SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void PM_FixPlayerCrouchStuck( int direction )
{
	int hitent;
	int i;
	vec3_t test;

	hitent = pmove->PM_TestPlayerPosition( pmove->origin, NULL );
	if( hitent == -1 )
		return;

	VectorCopy( pmove->origin, test );
	for( i = 0; i < 36; i++ )
	{
		pmove->origin[2] += direction;
		hitent = pmove->PM_TestPlayerPosition( pmove->origin, NULL );
		if( hitent == -1 )
			return;
	}

	VectorCopy( test, pmove->origin ); // Failed
}

void PM_UnDuck( void )
{
	pmtrace_t trace;
	vec3_t newOrigin;

	VectorCopy( pmove->origin, newOrigin );

	if( pmove->onground != -1 )
	{
		for( int i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
		}
	}

	trace = pmove->PM_PlayerTrace( pmove->origin, newOrigin, PM_NORMAL, pmove->usehull );

	if( !trace.startsolid )
	{
		pmove->usehull = 0;

		trace = pmove->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, pmove->usehull );
		if( trace.startsolid )
		{
			pmove->usehull = 1;
			return;
		}

		pmove->flags &= ~FL_DUCKING;
		pmove->bInDuck = false;
		pmove->view_ofs[2] = 22.0f;
		pmove->flDuckTime = 0;

		VectorCopy( newOrigin, pmove->origin );

		// Recatagorize position since ducking can change origin
		PM_CatagorizePosition();
	}
}


void PM_Duck( void )
{
	int i;
	float time;
	float duckFraction;

	if( pmove->iuser3 == 3 )
	{
		pmove->cmd.buttons |= IN_DUCK;
	}

	int buttonsChanged = ( pmove->oldbuttons ^ pmove->cmd.buttons );
	int nButtonPressed = buttonsChanged & pmove->cmd.buttons;

	if( pmove->cmd.buttons & IN_DUCK )
	{
		pmove->oldbuttons |= IN_DUCK;
	}
	else
	{
		pmove->oldbuttons &= ~IN_DUCK;
	}

	if( pmove->dead )
		return;

	if( ( pmove->cmd.buttons & IN_DUCK ) || pmove->bInDuck || ( pmove->flags & FL_DUCKING ) )
	{
		pmove->cmd.forwardmove *= 0.333f;
		pmove->cmd.sidemove *= 0.333f;
		pmove->cmd.upmove *= 0.333f;

		if( pmove->cmd.buttons & IN_DUCK )
		{
			if( ( nButtonPressed & IN_DUCK ) && !( pmove->flags & FL_DUCKING ) )
			{
				pmove->flDuckTime = 1000;
				pmove->bInDuck = true;
			}

			time = max( 0.0f, ( 1.0f - ( float ) pmove->flDuckTime / 1000.0f ) );

			if( pmove->bInDuck )
			{
				if( ( ( float ) pmove->flDuckTime / 1000.0f <= 0.6f ) || ( pmove->onground == -1 ) )
				{
					pmove->usehull = 1;
					pmove->view_ofs[2] = 18.0f;
					pmove->flags |= FL_DUCKING;
					pmove->bInDuck = false;

					if( pmove->onground != -1 )
					{
						pmove->origin[0] -= ( pmove->player_mins[1][0] - pmove->player_mins[0][0] );
						pmove->origin[1] -= ( pmove->player_mins[1][1] - pmove->player_mins[0][1] );
						pmove->origin[2] -= ( pmove->player_mins[1][2] - pmove->player_mins[0][2] );

						PM_FixPlayerCrouchStuck( 0 );
						PM_CatagorizePosition();
					}
				}
				else
				{
					if( pmove->onground != -1 )
					{
						float t = time * 2.5f;
						float flSpline = 3.0f * ( t * t ) - 2.0f * ( t * t * t );

						pmove->view_ofs[2] = 0.0f * flSpline + ( 1.0f - flSpline ) * 22.0f;
					}
					else
					{
						pmove->usehull = 1;
						pmove->view_ofs[2] = 18.0f;
						pmove->flags |= FL_DUCKING;
						pmove->bInDuck = false;
					}
				}
			}
		}
		else
		{
			if( pmove->bInDuck || ( pmove->flags & FL_DUCKING ) )
			{
				PM_UnDuck();
			}
		}
	}
}

void PM_UnProne( void )
{
	pmtrace_t trace;
	vec3_t newOrigin;

	VectorCopy( pmove->origin, newOrigin );

	if( pmove->onground != -1 )
	{
		newOrigin[0] = pmove->origin[0] + pmove->player_maxs[0][0] - pmove->player_mins[0][0];
		newOrigin[1] = pmove->origin[1] + pmove->player_maxs[0][1] - pmove->player_mins[0][1];
		newOrigin[2] = pmove->origin[2] + pmove->player_maxs[0][2] - pmove->player_mins[0][2];
	}

	trace = pmove->PM_PlayerTrace( pmove->origin, newOrigin, PM_NORMAL, pmove->usehull );

	if( !trace.startsolid )
	{
		pmove->usehull = 0;
		trace = pmove->PM_PlayerTrace( pmove->origin, newOrigin, PM_NORMAL, pmove->usehull );

		if( trace.startsolid )
		{
			pmove->usehull = 1;
		}
		else
		{
			pmove->bInDuck = false;
			pmove->flags &= ~FL_DUCKING;
			pmove->view_ofs[2] = 22.0f;
			pmove->flDuckTime = 0.0f;

			VectorCopy( newOrigin, pmove->origin );

			PM_CatagorizePosition();
		}
	}
}

void PM_Prone( void )
{
	float time;
	float duckFraction;
	int flags;
	int onground;

	if( pmove->dead || ( pmove->iuser3 - 1 ) > 1 )
		return;

	pmove->usehull = 1;
	flags = pmove->flags;

	if( flags & FL_DUCKING )
	{
		time = pmove->flDuckTime / 1000.0f;
		duckFraction = 1.0f - time;
		if( duckFraction < 0.0f )
		{
			duckFraction = 0.0f;
		}
	}
	else
	{
		pmove->bInDuck = true;
		pmove->onground = -1;
		duckFraction = 0.0f;
		time = 1.0f;
		pmove->flDuckTime = 1000.0f;
	}

	if( pmove->bInDuck )
	{
		if( time > 0.6f )
		{
			if( pmove->onground == -1 )
			{
				pmove->flags |= FL_DUCKING;
				pmove->view_ofs[2] = -6.0f;
				pmove->bInDuck = false;
			}
			else
			{
				float fMore = PM_SplineFraction( duckFraction, 2.5f );

				pmove->view_ofs[2] = -24.0f * fMore + ( 1.0f - fMore ) * 22.0f;
			}
		}
		else
		{
			onground = pmove->onground;
			pmove->flags |= FL_DUCKING;
			pmove->view_ofs[2] = -6.0f;
			pmove->bInDuck = false;

			if( onground != -1 )
			{
				vec3_t delta;

				VectorSubtract( pmove->player_maxs[1], pmove->player_mins[0], delta );
				VectorSubtract( pmove->origin, delta, pmove->origin );

				PM_FixPlayerCrouchStuck( 0 );
				PM_CatagorizePosition();
			}
		}
	}
}

void PM_LadderMove( physent_t *pLadder )
{
	vec3_t ladderCenter;
	trace_t trace;
	qboolean onFloor;
	vec3_t floor;
	vec3_t modelmins, modelmaxs;

	if( pmove->movetype == MOVETYPE_NOCLIP )
		return;

	int iMovementState = pmove->iuser3;

	if( iMovementState == 1 || iMovementState == 2 )
		return;

	pmove->PM_GetModelBounds( pLadder->model, modelmins, modelmaxs );

	VectorAdd( modelmins, modelmaxs, ladderCenter );
	VectorScale( ladderCenter, 0.5, ladderCenter );

	pmove->movetype = MOVETYPE_FLY;

	if( pLadder->iuser1 == 1 )
		pmove->flags |= 0x40000;
	else
		pmove->flags &= ~0x40000;

	// On ladder, convert movement to be relative to the ladder
	VectorCopy( pmove->origin, floor );
	floor[2] += pmove->player_mins[pmove->usehull][2] - 1;

	int iContentsUnderFeet = pmove->PM_PointContents( floor, NULL );

	if( iContentsUnderFeet == CONTENTS_SOLID )
		onFloor = true;
	else
		onFloor = false;

	pmove->gravity = 0;
	PM_TraceModel( pLadder, pmove->origin, ladderCenter, &trace );
	if( trace.fraction != 1.0f )
	{
		float forward = 0, right = 0;
		vec3_t vpn, v_right;
		float flSpeed = MAX_CLIMB_SPEED;

		// they shouldn't be able to move faster than their maxspeed
		if( flSpeed > pmove->maxspeed )
			flSpeed = pmove->maxspeed;

		AngleVectors( pmove->angles, vpn, v_right, NULL );

		if( pmove->flags & FL_DUCKING )
			flSpeed *= PLAYER_DUCKING_MULTIPLIER;
		if( pmove->cmd.buttons & IN_BACK )
			forward -= flSpeed;
		if( pmove->cmd.buttons & IN_FORWARD )
			forward += flSpeed;
		if( pmove->cmd.buttons & IN_MOVELEFT )
			right -= flSpeed;
		if( pmove->cmd.buttons & IN_MOVERIGHT )
			right += flSpeed;

		if( pmove->cmd.buttons & IN_JUMP )
		{
			pmove->movetype = MOVETYPE_WALK;
			VectorScale( trace.plane.normal, 270, pmove->velocity );
		}
		else
		{
			if( forward != 0 || right != 0 )
			{
				vec3_t velocity, perp, cross, lateral, tmp;
				float normal;

				VectorScale( vpn, forward, velocity );
				VectorMA( velocity, right, v_right, velocity );

				VectorClear( tmp );
				tmp[2] = 1;
				CrossProduct( tmp, trace.plane.normal, perp );
				VectorNormalize( perp );

				normal = DotProduct( velocity, trace.plane.normal );
				VectorScale( trace.plane.normal, normal, cross );

				VectorSubtract( velocity, cross, lateral );

				CrossProduct( trace.plane.normal, perp, tmp );
				VectorMA( lateral, -normal, tmp, pmove->velocity );

				if( onFloor && normal > 0 )	// On ground moving away from the ladder
				{
					VectorMA( pmove->velocity, MAX_CLIMB_SPEED, trace.plane.normal, pmove->velocity );
				}
				else if( iContentsUnderFeet == CONTENTS_WATER && normal > 0 )
				{
					VectorMA( pmove->velocity, 200.0f, trace.plane.normal, pmove->velocity );
				}
			}
			else
			{
				VectorClear( pmove->velocity );
			}
		}
	}
}

physent_t *PM_Ladder( void )
{
	int i;
	physent_t *pe;
	hull_t *hull;
	int num;
	vec3_t test;

	for( i = 0; i < pmove->nummoveent; i++ )
	{
		pe = &pmove->moveents[i];

		if( pe->model && (modtype_t)pmove->PM_GetModelType( pe->model ) == mod_brush && pe->skin == CONTENTS_LADDER )
		{

			hull = (hull_t *)pmove->PM_HullForBsp( pe, test );
			num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			VectorSubtract( pmove->origin, test, test );

			// Test the player's hull for intersection with this model
			if( pmove->PM_HullPointContents( hull, num, test ) != CONTENTS_EMPTY )
				continue;

			return pe;
		}
	}

	return NULL;
}

void PM_WaterJump( void )
{
	if( pmove->waterjumptime > 10000 )
	{
		pmove->waterjumptime = 10000;
	}

	if( !pmove->waterjumptime )
		return;

	pmove->waterjumptime -= pmove->cmd.msec;
	if( pmove->waterjumptime < 0 || !pmove->waterlevel )
	{
		pmove->waterjumptime = 0;
		pmove->flags &= ~FL_WATERJUMP;
	}

	pmove->velocity[0] = pmove->movedir[0];
	pmove->velocity[1] = pmove->movedir[1];
}

/*
============
PM_AddGravity

============
*/
void PM_AddGravity( void )
{
	float ent_gravity;

	if( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0f;

	// Add gravity incorrectly
	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;
	PM_CheckVelocity();
}

/*
============
PM_PushEntity

Does not change the entities velocity at all
============
*/
pmtrace_t PM_PushEntity( vec3_t push )
{
	pmtrace_t trace;
	vec3_t end;

	VectorAdd( pmove->origin, push, end );

	trace = pmove->PM_PlayerTrace( pmove->origin, end, PM_NORMAL, pmove->usehull );

	VectorCopy( trace.endpos, pmove->origin );

	// So we can run impact function afterwards.
	if( trace.fraction < 1.0f && !trace.allsolid )
	{
		PM_AddToTouched( trace, pmove->velocity );
	}

	return trace;
}

/*
============
PM_Physics_Toss()

Dead player flying through air., e.g.
============
*/
void PM_Physics_Toss( void )
{
	pmtrace_t trace;
	vec3_t move;
	float backoff;

	PM_CheckWater();

	if( pmove->velocity[2] > 0 )
	{
		pmove->onground = -1;
	}
	else
	{
		if( pmove->onground != -1 )
		{
			if( VectorCompare( pmove->basevelocity, vec3_origin ) && VectorCompare( pmove->velocity, vec3_origin ) )
				return;
		}
	}

	PM_CheckVelocity();

	// add gravity
	if( pmove->movetype != MOVETYPE_FLY && pmove->movetype != MOVETYPE_BOUNCEMISSILE && pmove->movetype != MOVETYPE_FLYMISSILE )
	{
		float flGravity = ( pmove->gravity != 0.0f ) ? pmove->gravity : 1.0f;
		float flNewVertVel = pmove->velocity[2] - flGravity * pmove->movevars->gravity * pmove->frametime;

		pmove->velocity[2] = flNewVertVel + ( pmove->frametime * pmove->basevelocity[2] );
		pmove->basevelocity[2] = 0.0f;

		PM_CheckVelocity();
	}

	// move origin
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	PM_CheckVelocity();
	VectorScale( pmove->velocity, pmove->frametime, move );
	VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );

	trace = PM_PushEntity( move );

	PM_CheckVelocity();

	if( trace.allsolid )
	{
		pmove->onground = trace.ent;
		VectorCopy( vec3_origin, pmove->velocity );
		return;
	}

	if( trace.fraction == 1 )
	{
		PM_CheckWater();
		return;
	}

	if( pmove->movetype == MOVETYPE_BOUNCE )
		backoff = 2.0f - pmove->friction;
	else if( pmove->movetype == MOVETYPE_BOUNCEMISSILE )
		backoff = 2.0f;
	else
		backoff = 1.0f;

	PM_ClipVelocity( pmove->velocity, trace.plane.normal, pmove->velocity, backoff );

	// stop if on ground
	if( trace.plane.normal[2] > 0.7f )
	{
		float vel;

		if( pmove->velocity[2] < pmove->movevars->gravity * pmove->frametime )
		{
			pmove->onground = trace.ent;
			pmove->velocity[2] = 0;
		}

		vel = DotProduct( pmove->velocity, pmove->velocity );

		if( vel < 900.0f || ( pmove->movetype != MOVETYPE_BOUNCE && pmove->movetype != MOVETYPE_BOUNCEMISSILE ) )
		{
			pmove->onground = trace.ent;
			VectorCopy( vec3_origin, pmove->velocity );
		}
		else
		{
			VectorScale( pmove->velocity, ( 1.0f - trace.fraction ) * pmove->frametime * 0.9f, move );
			trace = PM_PushEntity( move );
		}

	}

	// check for in water
	PM_CheckWater();
}

/*
====================
PM_NoClip

====================
*/
void PM_NoClip( void )
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	//float currentspeed, addspeed, accelspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	VectorNormalize( pmove->forward ); 
	VectorNormalize( pmove->right );

	for( i = 0; i < 3; i++ )       // Determine x and y parts of velocity
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	wishvel[2] += pmove->cmd.upmove;

	VectorMA (pmove->origin, pmove->frametime, wishvel, pmove->origin );

	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	// gravity, etc.
	VectorClear( pmove->velocity );
}

// Only allow bunny jumping up to 1.2x server / player maxspeed setting
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.2f

//-----------------------------------------------------------------------------
// Purpose: Corrects bunny jumping ( where player initiates a bunny jump before other
//  movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
//  running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
//  movement logic does.
//-----------------------------------------------------------------------------
void PM_PreventMegaBunnyJumping( void )
{
	// Current player speed
	float spd;
	// If we have to crop, apply this cropping fraction to velocity
	float fraction;
	// Speed at which bunny jumping is limited
	float maxscaledspeed;

	maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;

	// Don't divide by zero
	if( maxscaledspeed <= 0.0f )
		return;

	spd = Length( pmove->velocity );

	if( spd <= maxscaledspeed )
		return;

	fraction = ( maxscaledspeed / spd ) * 0.8f; //Returns the modifier for the velocity
	
	VectorScale( pmove->velocity, fraction, pmove->velocity ); //Crop it down!.
}

/*
=============
PM_Jump
=============
*/
void PM_Jump( void )
{
	int i;
	qboolean bunnyjump = false;
	qboolean tfc = false;
	qboolean cansuperjump = false;

	if( pmove->dead )
	{
		pmove->oldbuttons |= IN_JUMP;
		return;
	}

	int iMovementState = pmove->iuser3;

	if( ( iMovementState == 1 || iMovementState == 2 ) || iMovementState == 3 )
		return;

	tfc = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "tfc" ) ) == 1 ? true : false;

	if( tfc && ( pmove->deadflag == 5 ) )
	{
		return;
	}

	if( pmove->waterjumptime )
	{
		pmove->waterjumptime -= pmove->cmd.msec;
		if( pmove->waterjumptime < 0 )
		{
			pmove->waterjumptime = 0;
		}
		return;
	}

	if( pmove->waterlevel >= 2 )
	{
		pmove->onground = -1;

		if( pmove->watertype == CONTENTS_WATER )
			pmove->velocity[2] = 100;
		else if( pmove->watertype == CONTENTS_SLIME )
			pmove->velocity[2] = 80;
		else
			pmove->velocity[2] = 50;

		if( pmove->flSwimTime <= 0 )
		{
			pmove->flSwimTime = 1000;
			switch( pmove->RandomLong( 0, 3 ) )
			{
			case 0:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 1:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 2:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 3:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			}
		}

		return;
	}

	if( pmove->onground == -1 )
	{
		pmove->oldbuttons |= IN_JUMP;
		return;
	}

	if( pmove->oldbuttons & IN_JUMP )
		return;

	pmove->onground = -1;

	float flMaxAllowedSpeed = 1.2f * pmove->maxspeed;

	if( flMaxAllowedSpeed > 0.0f )
	{
		float flCurrentSpeed = Length( pmove->velocity );
		if( flMaxAllowedSpeed < flCurrentSpeed )
		{
			float flScale = ( flMaxAllowedSpeed / flCurrentSpeed ) * 0.8f;
			VectorScale( pmove->velocity, flScale, pmove->velocity );
		}
	}

	if( !( pmove->flags & FL_FROZEN ) )
	{
		if( tfc )
		{
			pmove->PM_PlaySound( CHAN_BODY, "player/plyrjmp8.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
		}
		else
		{
			pmove->PM_PlaySound( CHAN_BODY, "player/jump.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
		}
	}

	cansuperjump = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "slj" ) ) == 1 ? true : false;

	if( ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
	{
		if( cansuperjump && ( pmove->cmd.buttons & IN_DUCK ) && ( pmove->flDuckTime > 0 ) && Length( pmove->velocity ) > 50 )
		{
			pmove->punchangle[0] = -5;

			for( i = 0; i < 2; i++ )
			{
				pmove->velocity[i] = pmove->forward[i] * 350.0f * 1.6f;
			}

			pmove->velocity[2] = 299.33258f;
		}
		else
		{
			pmove->velocity[2] = 268.32816f;
		}
	}
	else
	{
		float flStamina = pmove->fuser4;
		float flJumpBase = 30.0f;

		if( flStamina < 60.0f )
		{
			flJumpBase = 30.0f * ( flStamina / 60.0f );
		}

		pmove->velocity[2] = sqrt( ( flJumpBase + 15.0f ) * 1600.0f );
	}

	if( pmove->waterjumptime == 0.0f )
	{
		float flGravityValue = ( pmove->gravity != 0.0f ) ? pmove->gravity : 1.0f;
		pmove->velocity[2] -= flGravityValue * pmove->movevars->gravity * pmove->frametime * 0.5f;
		PM_CheckVelocity();
	}

	VectorClear( pmove->basevelocity );

	pmove->oldbuttons |= IN_JUMP;
}

/*
=============
PM_CheckWaterJump
=============
*/
#define WJ_HEIGHT 8
void PM_CheckWaterJump( void )
{
	vec3_t vecStart, vecEnd;
	vec3_t flatforward;
	vec3_t flatvelocity;
	float curspeed;
	pmtrace_t tr;
	int savehull;

	// Already water jumping.
	if( pmove->waterjumptime )
		return;

	// Don't hop out if we just jumped in
	if( pmove->velocity[2] < -180 )
		return; // only hop out if we are moving up

	// See if we are backing up
	flatvelocity[0] = pmove->velocity[0];
	flatvelocity[1] = pmove->velocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize( flatvelocity );

	// see if near an edge
	flatforward[0] = pmove->forward[0];
	flatforward[1] = pmove->forward[1];
	flatforward[2] = 0;
	VectorNormalize( flatforward );

	// Are we backing into water from steps or something?  If so, don't pop forward
	if( curspeed != 0.0f && ( DotProduct( flatvelocity, flatforward ) < 0.0f ) )
		return;

	VectorCopy( pmove->origin, vecStart );
	vecStart[2] += WJ_HEIGHT;

	VectorMA( vecStart, 24, flatforward, vecEnd );

	// Trace, this trace should use the point sized collision hull
	savehull = pmove->usehull;
	pmove->usehull = 2;
	tr = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, pmove->usehull );
	if( tr.fraction < 1.0f && fabs( tr.plane.normal[2] ) < 0.1f )  // Facing a near vertical wall?
	{
		vecStart[2] += pmove->player_maxs[savehull][2] - WJ_HEIGHT;
		VectorMA( vecStart, 24, flatforward, vecEnd );
		VectorMA( vec3_origin, -50, tr.plane.normal, pmove->movedir );

		tr = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, pmove->usehull );
		if( tr.fraction == 1.0f )
		{
			pmove->waterjumptime = 2000;
			pmove->velocity[2] = 225;
			pmove->oldbuttons |= IN_JUMP;
			pmove->flags |= FL_WATERJUMP;
		}
	}

	// Reset the collision hull
	pmove->usehull = savehull;
}

void PM_CheckFalling( void )
{
	float fVol = 0.5f;

	if( pmove->onground == -1 )
		return;

	if( pmove->dead )
	{
		pmove->flFallVelocity = 0.0f;
		return;
	}

	if( pmove->flFallVelocity < 350.0f )
	{
		pmove->flFallVelocity = 0.0f;
		return;
	}

	if( pmove->waterlevel > 0 )
	{
		pmove->flFallVelocity = 0.0f;
		return;
	}

	if( pmove->flFallVelocity > 580.0f )
	{
		pmove->PM_PlaySound( CHAN_BODY, "player/pl_fallpain.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
	}
	else if( pmove->flFallVelocity > 290.0f )
	{
		if( atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "tfc" ) ) == 1 )
		{
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_fallpain.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
		}
	}

	pmove->flTimeStepSound = 0;

	if( !( pmove->flags & 0x1000 ) )
		PM_UpdateStepSound();

	pmove->iStepLeft = !pmove->iStepLeft;

	if( pmove->runfuncs )
		PM_PlayStepSound( PM_MapTextureTypeStepType( pmove->chtexturetype ), fVol );

	pmove->punchangle[2] = 0.013f * pmove->flFallVelocity;

	if( pmove->punchangle[0] > 8.0f )
		pmove->punchangle[0] = 8.0f;

	pmove->flFallVelocity = 0.0f;
}

/*
=================
PM_PlayWaterSounds

=================
*/
void PM_PlayWaterSounds( void )
{
	// Did we enter or leave water?
	if( ( pmove->oldwaterlevel == 0 && pmove->waterlevel != 0 ) || ( pmove->oldwaterlevel != 0 && pmove->waterlevel == 0 ) )
	{
		switch( pmove->RandomLong( 0, 3 ) )
		{
		case 0:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 1:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 2:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 3:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		}
	}
}

/*
===============
PM_CalcRoll

===============
*/
float PM_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
{
	float sign;
	float side;
	float value;
	vec3_t forward, right, up;

	AngleVectors( angles, forward, right, up );

	side = DotProduct( velocity, right );

	sign = side < 0 ? -1 : 1;

	side = fabs( side );

	value = rollangle;

	if( side < rollspeed )
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}

	return side * sign;
}

/*
=============
PM_DropPunchAngle

=============
*/
void PM_DropPunchAngle( vec3_t punchangle )
{
	float len;
	
	len = VectorNormalize( punchangle );
	len -= ( 10.0f + len * 0.5f ) * pmove->frametime;
	len = max( len, 0.0f );
	VectorScale( punchangle, len, punchangle );
}

/*
==============
PM_CheckParamters

==============
*/
void PM_CheckParamters( void )
{
	float spd;
	float maxspeed;
	vec3_t v_angle;

	spd = ( pmove->cmd.forwardmove * pmove->cmd.forwardmove ) + ( pmove->cmd.sidemove * pmove->cmd.sidemove ) +
		( pmove->cmd.upmove * pmove->cmd.upmove );
	spd = sqrt( spd );

	maxspeed = pmove->clientmaxspeed;

	if( maxspeed != 0.0f )
	{
		pmove->maxspeed = min( maxspeed, pmove->maxspeed );
	}

	if( ( spd != 0.0f ) && ( spd > pmove->maxspeed ) )
	{
		float fRatio = pmove->maxspeed / spd;
		pmove->cmd.forwardmove *= fRatio;
		pmove->cmd.sidemove *= fRatio;
		pmove->cmd.upmove *= fRatio;
	}

	if( ( pmove->flags & 0x1001000 ) || pmove->dead )
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove = 0;
		pmove->cmd.upmove = 0;
	}

	PM_DropPunchAngle( pmove->punchangle );

	// Take angles from command.
	if( !pmove->dead )
	{
		VectorCopy( pmove->cmd.viewangles, v_angle );
		VectorAdd( v_angle, pmove->punchangle, v_angle );

		// Set up view angles.
		pmove->angles[ROLL] = PM_CalcRoll( v_angle, pmove->velocity, pmove->movevars->rollangle, pmove->movevars->rollspeed ) * 4;
		pmove->angles[PITCH] = v_angle[PITCH];
		pmove->angles[YAW] = v_angle[YAW];
	}
	else
	{
		VectorCopy( pmove->oldangles, pmove->angles );
	}

	// Set dead player view_offset
	if( pmove->dead )
	{
		pmove->view_ofs[2] = -8.0f;
	}

	// Adjust client view angles to match values used on server.
	if( pmove->angles[YAW] > 180.0f )
	{
		pmove->angles[YAW] -= 360.0f;
	}
}

void PM_ReduceTimers( void )
{
	if( pmove->flTimeStepSound > 0 )
	{
		pmove->flTimeStepSound -= pmove->cmd.msec;
		if( pmove->flTimeStepSound < 0 )
		{
			pmove->flTimeStepSound = 0;
		}
	}
	if( pmove->flDuckTime > 0 )
	{
		pmove->flDuckTime -= pmove->cmd.msec;
		if( pmove->flDuckTime < 0 )
		{
			pmove->flDuckTime = 0;
		}
	}
	if( pmove->flSwimTime > 0 )
	{
		pmove->flSwimTime -= pmove->cmd.msec;
		if( pmove->flSwimTime < 0 )
		{
			pmove->flSwimTime = 0;
		}
	}
}

/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/
void PM_PlayerMove( qboolean server )
{
	physent_t *pLadder = NULL;

	// Are we running server code?
	pmove->server = server;

	// Adjust speeds etc.
	PM_CheckParamters();

	// Assume we don't touch anything
	pmove->numtouch = 0;

	// # of msec to apply movement
	pmove->frametime = pmove->cmd.msec * 0.001f;

	PM_ReduceTimers();

	// Convert view angles to vectors
	AngleVectors( pmove->angles, pmove->forward, pmove->right, pmove->up );

	// Special handling for spectator and observers. (iuser1 is set if the player's in observer mode)
	if( pmove->spectator || pmove->iuser1 > 0 )
	{
		PM_SpectatorMove();
		PM_CatagorizePosition();
		return;
	}

	// Always try and unstick us unless we are in NOCLIP mode
	if( pmove->movetype != MOVETYPE_NOCLIP && pmove->movetype != MOVETYPE_NONE )
	{
		if( PM_CheckStuck() )
		{
			PM_FixPlayerCrouchStuck( server );
			return;
		}
	}

	// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
	PM_CatagorizePosition();

	// Store off the starting water level
	pmove->oldwaterlevel = pmove->waterlevel;

	// If we are not on ground, store off how fast we are moving down
	if( pmove->onground == -1 )
	{
		pmove->flFallVelocity = -pmove->velocity[2];
	}

	g_onladder = 0;

	if( !pmove->dead && !( pmove->flags & 0x1000000 ) )
	{
		pLadder = PM_Ladder();
		if( pLadder )
		{
			g_onladder = 1;
		}
	}

	if( pmove->flTimeStepSound <= 0 && ( pmove->flags & 0x1000 ) == 0 )
	{
		PM_UpdateStepSound();
	}

	PM_Prone();

	int iMovementState = pmove->iuser3;

	if( iMovementState != 1 && iMovementState != 2 )
	{
		PM_Duck();
	}

	if( pmove->dead || ( pmove->flags & 0x1000000 ) != 0 )
	{
		if( pLadder )
		{
			PM_LadderMove( pLadder );
		}
		else if( pmove->movetype != MOVETYPE_WALK && pmove->movetype != MOVETYPE_NOCLIP )
		{
			pmove->movetype = MOVETYPE_WALK;
		}
	}

	// Handle movement
	switch( pmove->movetype )
	{
	default:
		pmove->Con_DPrintf( "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server );
		break;
	case MOVETYPE_NONE:
		break;
	case MOVETYPE_NOCLIP:
		PM_NoClip();
		break;
	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		PM_Physics_Toss();
		break;
	case MOVETYPE_FLY:
		PM_CheckWater();

		if( pmove->cmd.buttons & IN_JUMP )
		{
			if( !pLadder )
			{
				if( pmove->dead )
					pmove->oldbuttons |= IN_JUMP;
				else
					PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );
		PM_FlyMove();
		VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );
		break;
	case MOVETYPE_WALK:
		if( pmove->waterlevel <= 1 )
		{
			if( pmove->waterjumptime != 0.0f )
			{
				PM_WaterJump();
				PM_FlyMove();
				PM_CheckWater();
				return;
			}

			float flGravity = ( pmove->gravity != 0.0f ) ? pmove->gravity : 1.0f;
			pmove->velocity[2] = ( pmove->velocity[2] - flGravity * pmove->movevars->gravity * 0.5f * pmove->frametime ) + ( pmove->frametime * pmove->basevelocity[2] );
			pmove->basevelocity[2] = 0.0f;

			PM_CheckVelocity();
		}

		if( pmove->waterjumptime == 0.0f )
		{
			if( pmove->waterlevel >= 2 )
			{
				if( pmove->waterlevel == 2 )
				{
					PM_CheckWaterJump();
				}

				if( pmove->velocity[2] < 0 && pmove->waterjumptime )
				{
					pmove->waterjumptime = 0;
				}

				if( pmove->cmd.buttons & IN_JUMP )
				{
					if( pmove->dead )
						pmove->oldbuttons |= IN_JUMP;
					else
						PM_Jump();
				}
				else
				{
					pmove->oldbuttons &= ~IN_JUMP;
				}

				PM_WaterMove();

				VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );
				PM_CatagorizePosition();
			}
			else
			{
				if( pmove->cmd.buttons & IN_JUMP )
				{
					if( !pLadder )
					{
						PM_Jump();
					}
				}
				else
				{
					pmove->oldbuttons &= ~IN_JUMP;
				}

				if( pmove->onground == -1 )
				{
					if( !g_jumped )
					{
						g_jumped = 1;
						flFallTime = 1000.0f;
					}
				}
				else if( !g_jumped )
				{
					g_jumped = 0;
					pmove->PM_PlaySound( CHAN_BODY, "player/jumplanding.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
					flFallTime = 0.0f;
				}
				else
				{
					pmove->velocity[2] = 0.0f;
					PM_Friction();
				}

				PM_CheckVelocity();

				if( pmove->onground != -1 )
				{
					PM_WalkMove();
				}
				else
				{
					PM_AirMove();
				}

				PM_CatagorizePosition();

				VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );
				PM_CheckVelocity();

				if( pmove->waterlevel <= 1 && pmove->waterjumptime == 0.0f )
				{
					float flGravity = ( pmove->gravity != 0.0f ) ? pmove->gravity : 1.0f;
					pmove->velocity[2] = pmove->velocity[2] - flGravity * pmove->movevars->gravity * pmove->frametime * 0.5f;
					PM_CheckVelocity();
				}

				if( pmove->onground != -1 )
				{
					pmove->velocity[2] = 0;
				}

				PM_CheckFalling();
			}

			PM_PlayWaterSounds();
		}
		break;
	}
}

void PM_CreateStuckTable( void )
{
	float x, y, z;
	int idx;
	int i;
	float zi[3];

	memset( rgv3tStuckTable, 0, 54 * sizeof(vec3_t) );

	idx = 0;
	// Little Moves.
	x = y = 0;
	// Z moves
	for( z = -0.125f; z <= 0.125f; z += 0.125f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for( y = -0.125f; y <= 0.125f; y += 0.125f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for( x = -0.125f; x <= 0.125f; x += 0.125f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for( x = - 0.125f; x <= 0.125f; x += 0.250f )
	{
		for( y = - 0.125f; y <= 0.125f; y += 0.250f )
		{
			for( z = - 0.125f; z <= 0.125f; z += 0.250f )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;
	zi[0] = 0.0f;
	zi[1] = 1.0f;
	zi[2] = 6.0f;

	for( i = 0; i < 3; i++ )
	{
		// Z moves
		z = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for( y = -2.0f; y <= 2.0f; y += 2.0f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for( x = -2.0f; x <= 2.0f; x += 2.0f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for( i = 0 ; i < 3; i++ )
	{
		z = zi[i];

		for( x = -2.0f; x <= 2.0f; x += 2.0f )
		{
			for( y = -2.0f; y <= 2.0f; y += 2.0f )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
}

/*
This modume implements the shared player physics code between any particular game and 
the engine.  The same PM_Move routine is built into the game .dll and the client .dll and is
invoked by each side as appropriate.  There should be no distinction, internally, between server
and client.  This will ensure that prediction behaves appropriately.
*/

void PM_Move( struct playermove_s *ppmove, int server )
{
	pmove = ppmove;

	PM_PlayerMove( ( server != 0 ) ? true : false );

	if( pmove->onground != -1 )
	{
		pmove->flags |= FL_ONGROUND;
	}
	else
	{
		pmove->flags &= ~FL_ONGROUND;
	}

	if( !( pmove->multiplayer && atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "fr" ) ) == 0 ) && pmove->movetype == MOVETYPE_WALK )
	{
		pmove->friction = 1.0f;

	}

	if( pmove->movetype == MOVETYPE_WALK )
	{
		pmove->friction = 1.0f;
	}
}

int PM_GetVisEntInfo( int ent )
{
	if( ent >= 0 && ent <= pmove->numvisent )
	{
		return pmove->visents[ent].info;
	}
	return -1;
}

int PM_GetPhysEntInfo( int ent )
{
	if( ent >= 0 && ent <= pmove->numphysent )
	{
		return pmove->physents[ent].info;
	}
	return -1;
}

void PM_Init( struct playermove_s *ppmove )
{
	assert( !pm_shared_initialized );

	pmove = ppmove;

	PM_CreateStuckTable();
	PM_InitTextureTypes();

	pm_shared_initialized = 1;
}
