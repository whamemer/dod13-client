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

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_dod.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include "dod_shared.h"
#include "tri.h"

#include "pm_shared.h"

#include <string.h>

#ifndef SND_CHANGE_PITCH
#define SND_CHANGE_PITCH ( 1 << 7 )
#endif

extern char *sHelmetModels[];

static int tracerCount[32];

extern cvar_t *cl_dynamiclights;

cvar_t *cl_numshotrubble;

BEAM *pBeam;
BEAM *pBeam2;
TEMPENTITY *pFlare;

enum GUNTYPE_e
{
	PISTOL = 1,
	RIFLE,
	SUBMACHINE,
	MG
};

extern "C"
{
	void EV_FireMelee( struct event_args_s *args );
	void EV_FireColt( struct event_args_s *args );
	void EV_FireLuger( struct event_args_s *args );
	void EV_FireMG42( struct event_args_s *args );
	void EV_FireMG34( struct event_args_s *args );
	void EV_Fire30CAL( struct event_args_s *args );
	void EV_FireGarand( struct event_args_s *args );
	void EV_FireM1Carbine( struct event_args_s *args );
	void EV_FireScopedKar( struct event_args_s *args );
	void EV_FireThompson( struct event_args_s *args );
	void EV_FireMP44( struct event_args_s *args );
	void EV_FireGreaseGun( struct event_args_s *args );
	void EV_FireFG42( struct event_args_s *args );
	void EV_FireK43( struct event_args_s *args );
	void EV_FireEnfield( struct event_args_s *args );
	void EV_FireSten( struct event_args_s *args );
	void EV_FireBren( struct event_args_s *args );
	void EV_FireWebley( struct event_args_s *args );
	void EV_PopHelmet( struct event_args_s *args );
	void EV_Knife( struct event_args_s *args );
	void EV_FireMP40( struct event_args_s *args );
	void EV_FireSpring( struct event_args_s *args );
	void EV_FireKar( struct event_args_s *args );
	void EV_FireBAR( struct event_args_s *args );
	void EV_FireBazooka( struct event_args_s *args );
	void EV_FirePschreck( struct event_args_s *args );
	void EV_FirePIAT( struct event_args_s *args );
	void EV_FireMortar( struct event_args_s *args );
	void EV_FireScopedEnfield( struct event_args_s *args );
	void EV_PlayWhizz( struct event_args_s *args );
	void EV_RoundReset( struct event_args_s *args );
	void EV_DoDCamera( struct event_args_s *args );
	void EV_SparkShower( struct event_args_s *args );
	void EV_Bubbles( struct event_args_s *args );
	void EV_BubbleTrails( struct event_args_s *args );
	void EV_BulletTracers( struct event_args_s *args );
	void EV_BloodStream( struct event_args_s *args );
	void EV_BloodSprite( struct event_args_s *args );
	void EV_Pain( struct event_args_s *args );
	void EV_BodyDamage( struct event_args_s *args );
	void EV_RoundReleaseSound( struct event_args_s *args );
	void EV_TrainPitchAdjust( struct event_args_s *args );
	void EV_MortarShell( struct event_args_s *args );
	void EV_Smoke( struct event_args_s *args );
	void EV_Explosion( struct event_args_s *args );
	void EV_USVoice( struct event_args_s *args );
	void EV_GERVoice( struct event_args_s *args );
	void EV_Overheat( struct event_args_s *args );
	void EV_RocketTrail( struct event_args_s *args );
}

int EV_GetWeaponBody( void )
{
	int team = g_iTeamNumber;

	if( g_iUser1 == OBS_IN_EYE )
		team = gEngfuncs.GetEntityByIndex( g_iUser2 )->curstate.team;

	if( team == 1 )
		return gHUD.m_bBritish;

	return 2;
}

void EV_ResetAnimationEvents( int index )
{
	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( index );

	ent->baseline.iuser1 = 0;
	ent->baseline.iuser2 = 0;
	ent->baseline.iuser3 = 0;
	ent->baseline.iuser4 = 0;
}

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
float EV_DOD_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	const char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[64];
	char szbuffer[64];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	chTextureType = 0;

	// Player
	if( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );

		if( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if( *pTextureName == '-' || *pTextureName == '+' )
				pTextureName += 2;

			if( *pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ' )
				pTextureName++;

			// '}}'
			strcpy( szbuffer, pTextureName );
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );
		}
	}

	switch( chTextureType )
	{
	default:
		fvol = 0.9f;
		fvolbar = 0.6f;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case 'H':
	case CHAR_TEX_METAL:
		fvol = 0.9f;
		fvolbar = 0.3f;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:
		fvol = 0.9f;
		fvolbar = 0.1f;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GRATE:
		fvol = 0.9f;
		fvolbar = 0.5f;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		rgsz[2] = "player/pl_duct2.wav";
		rgsz[3] = "player/pl_grate4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_TILE:
		fvol = 0.8f;
		fvolbar = 0.2f;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH:
		fvol = 0.9f;
		fvolbar = 0.0f;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD:
		fvol = 0.9f;
		fvolbar = 0.2f;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
		fvol = 0.8f;
		fvolbar = 0.2f;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case 'K':
		return 0.0f;
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong( 0, cnt - 1 )], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
	return fvolbar;
}

static cvar_t *r_decals;

void EV_DOD_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, float *vecSrc, float *vecEnd, int iBulletType )
{
	physent_t *pe;
	char chTextureType;
	int entity;
	char *pTextureName;
	char texname[64];
	char szbuffer[64];
	float flHeight;
	int idx;
	static char decalname[32];

	flHeight = EV_DOD_WaterHeight( vecSrc, vecSrc[2], vecSrc[2] + 256.0f ) - vecSrc[2];

	if( flHeight < 8.0f )
	{
		flHeight = EV_DOD_WaterHeight( pTrace->endpos, pTrace->endpos[2], pTrace->endpos[2] + 256.0f ) - pTrace->endpos[2];

		if( flHeight < 8.0f )
			flHeight = 0.0f;
		else
			flHeight = flHeight + pTrace->endpos[2] - vecSrc[2];
	}

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace );
	chTextureType = 'F';

	if( entity <= 0 || entity > gEngfuncs.GetMaxClients() )
	{
		pmtrace_t trace2;

		gEngfuncs.pEventAPI->EV_PlayerTrace( pTrace->endpos, vecEnd, 2, -1, &trace2 );

		chTextureType = 'K';
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( trace2.ent, pTrace->endpos, vecEnd );

		if( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if( *pTextureName == '-' || *pTextureName == '+' )
				pTextureName += 2;

			if( *pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ' )
				pTextureName++;

			strcpy( szbuffer, pTextureName );
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			chTextureType = PM_FindTextureType( szbuffer );

			if( flHeight != 0.0f )
			{
				vec3_t location;

				chTextureType = 'S';

				if( EV_DOD_WaterEntryPoint( pTrace, vecSrc, location ) )
					VectorCopy( location, pTrace->endpos );
			}
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if( pe->classnumber == 1 || chTextureType == 'Y' )
	{
		chTextureType = 'Y';
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else
	{
		switch( chTextureType )
		{
		case 'A':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'P':
		case 'R':
		case 'S':
			decalname[0] = 0;
			break;
		case 'B':
			sprintf( decalname, "{cement%i", gEngfuncs.pfnRandomLong( 1, 2 ) );
			break;
		case 'C':
			sprintf( decalname, "{cement%i", gEngfuncs.pfnRandomLong( 1, 2 ) );
			break;
		case 'T':
			sprintf( decalname, "{crack%i", gEngfuncs.pfnRandomLong( 1, 4 ) );
			break;
		case 'W':
			sprintf( decalname, "{wood%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
			break;
		case 'Z':
			sprintf( decalname, "{wood%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
			break;
		default:
			sprintf( decalname, "{generic%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
			break;
		}
	}

	EV_PlaySurfaceHitSound( pTrace, iBulletType, chTextureType );

	if( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP )
	{
		if( !r_decals )
			r_decals = gEngfuncs.pfnGetCvarPointer( "r_decals" );

		if( decalname[0] && r_decals && r_decals->value != 0.0f )
		{
			idx = gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace );
			gEngfuncs.pEfxAPI->R_DecalShoot( gEngfuncs.pEfxAPI->Draw_DecalIndex( gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalname ) ),
				idx, 0, pTrace->endpos, 0 );
		}

		EV_DOD_SurfaceFX( pTrace, iBulletType, chTextureType );
	}
}

TEMPENTITY *g_DeadPlayerModels[MAX_PLAYERS];

void EV_RoundReset( event_args_t *args )
{
	int maxdecals;

	if( r_decals || ( r_decals = gEngfuncs.pfnGetCvarPointer( "r_decals" ) ) )
	{
		maxdecals = (int)r_decals->value;

		if( maxdecals > 0 )
		{
			for( int har = 0; har < maxdecals; har++ )
				gEngfuncs.pEfxAPI->R_DecalRemoveAll( har );
		}
	}

	if( g_DeadPlayerModels[0] )
	{
		for( int i = 0; i < MAX_PLAYERS; i++ )
		{
			if( g_DeadPlayerModels[i] )
			{
				g_DeadPlayerModels[i]->die = 0.0f;
				g_DeadPlayerModels[i] = NULL;
			}
		}
	}
}

void RemoveBody( TEMPENTITY *te, float frametime, float current_time )
{
	float corpseTime = 0.0f;

	if( gHUD.cl_corpsestay )
		corpseTime = gHUD.cl_corpsestay->value;

	if( current_time >= corpseTime + te->entity.curstate.fuser2 )
		te->entity.origin.z = te->entity.origin.z - frametime * 5.0f;
}

void HitBody( TEMPENTITY *ent, pmtrace_t *ptr )
{
	if( ptr->plane.normal.z > 0.0f )
		ent->flags |= FTENT_BODYSTATIC;
}

void CreateCorpse( Vector vOrigin, Vector vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody )
{
	float time;
	int iModelIndex;
	TEMPENTITY *pBody;
	float corpseTime;
	bool bSuccess;

	time = gEngfuncs.GetClientTime();

	gEngfuncs.pEventAPI->EV_FindModelIndex( pModel );
	iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( pModel );

	pBody = gEngfuncs.pEfxAPI->R_TempModel( vOrigin, Vector( 0, 0, 0 ), vAngles, 100.0f, iModelIndex, 0 );

	if( pBody )
	{
		pBody->flags |= FTENT_COLLIDEWORLD | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_CORPSE;

		pBody->entity.curstate.framerate = 1.0f;
		pBody->frameMax = 255.0f;
		pBody->entity.curstate.animtime = flAnimTime;
		pBody->entity.curstate.frame = 0.0f;
		pBody->entity.curstate.renderamt = 255;
		pBody->entity.curstate.sequence = iSequence;
		pBody->entity.curstate.body = iBody;

		float flSpawnTime = gHUD.m_flTime + 1.0f;

		pBody->entity.curstate.fuser1 = flSpawnTime;
		pBody->entity.curstate.fuser2 = flSpawnTime;

		corpseTime = 0.0f;

		if( gHUD.cl_corpsestay )
			corpseTime = gHUD.cl_corpsestay->value;

		pBody->callback = RemoveBody;
		pBody->hitcallback = HitBody;
		pBody->bounceFactor = 0.0f;
		pBody->die = flSpawnTime + corpseTime + 5.0f;

		bSuccess = false;

		for( int i = 0; i < MAX_PLAYERS; i++ )
		{
			if( !g_DeadPlayerModels[i] || pBody == g_DeadPlayerModels[i] )
			{
				g_DeadPlayerModels[i] = pBody;
				return;
			}

			if( time > g_DeadPlayerModels[i]->die )
			{
				g_DeadPlayerModels[i]->die = 0.0f;
				bSuccess = true;
				g_DeadPlayerModels[i] = pBody;
			}
		}

		if( !bSuccess )
			pBody->die = pBody->entity.curstate.fuser2 + 5.0f;
	}
}

void EV_BasicPuff( pmtrace_t *pTrace, float scale )
{
	for( int i = 0; i < 4; i++ )
	{
		vec3_t vColor = Vector( 175.0f, 175.0f, 175.0f );
		vec3_t vVelocity = Vector( pTrace->plane.normal );
		vec3_t origin = Vector( pTrace->endpos ) + Vector( pTrace->plane.normal ) * (float)gEngfuncs.pfnRandomLong( 1, 3 );

		CreateDebrisWallPuff( origin, vVelocity, vColor, i );
	}
}

void EV_CreteRubble( pmtrace_t *pTrace, float fScale )
{
	vec3_t origin;

	VectorMA( pTrace->endpos, 2.0f, pTrace->plane.normal, origin );

	if( cl_numshotrubble && cl_numshotrubble->value > 0.0f )
	{
		int iAmmount = gEngfuncs.pfnRandomLong( 0, (int)cl_numshotrubble->value );

		for( int i = 0; i < iAmmount; i++ )
		{
			CreateFlyingRubble( origin, false, (float)gEngfuncs.pfnRandomLong( -300, 300 ), (float)gEngfuncs.pfnRandomLong( -300, 300 ), 75.0f, fScale, 0, Vector( pTrace->plane.normal ) );
		}
	}
}

void CreateSpark( Vector origin, Vector vNormal, const char *szSpriteName )
{
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( szSpriteName ) );

	if( pSprite )
	{
		vec3_t angles;

		VectorAngles( vNormal, angles );

		CDoDParticle *pCustom = CDoDParticle::Create( origin, Vector( 0.0f, 0.0f, 1.0f ), pSprite, 2.5f, 255.0f, "dod_particle", true );

		if( pCustom )
		{
			pCustom->m_flGravity = 0.0f;
			pCustom->m_vVelocity = Vector( 0.0f, 0.0f, 0.0f );
			pCustom->m_vAngles = angles;
			pCustom->m_iFramerate = 0;
			pCustom->m_flDieTime = 0.05f;
			pCustom->m_iRendermode = kRenderTransAdd;
			pCustom->m_flSize = 2.5f;
			pCustom->m_vAVelocity = Vector( 0.0f, 0.0f, 0.0f );
			pCustom->m_iPFlags = PFLAG_DOD_SPARK_INSTANT;
			pCustom->SetLightFlag( LIGHT_NONE );
			pCustom->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
		}
	}
	else
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", szSpriteName );
	}
}

void EV_MetalHit( pmtrace_t *pTrace )
{
	vec3_t origin = Vector( pTrace->endpos );
	vec3_t angles = Vector( pTrace->plane.normal ); // the normal of the plane
	vec3_t vEnd;
	vec3_t vNormal;
	vec3_t vDir;
	float incr = 0.0f;

	int iSparkSprite = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/metalspark.spr" );
	int iSparkNum = gEngfuncs.pfnRandomLong( 4, 6 );
	float length = gEngfuncs.pfnRandomFloat( 12.0f, 14.0f );

	if( iSparkSprite )
	{
		for( int j = 0; j < iSparkNum; j++ )
		{
			if( j )
			{
				vDir.x = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + vNormal.x;
				vDir.y = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + vNormal.y;
				vDir.z = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + vNormal.z;

				length -= incr;

				VectorNormalize( vDir );
				VectorMA( origin, length * 2.0f, vDir, vEnd );
			}
			else
			{
				vNormal.x = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + angles.x;
				vNormal.y = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + angles.y;
				vNormal.z = gEngfuncs.pfnRandomFloat( -0.2f, 0.2f ) + angles.z;

				VectorMA( origin, length, vNormal, vEnd );

				vDir = vNormal;
				length *= 0.75f;
				incr = length / (float)iSparkNum;
			}

			gEngfuncs.pEfxAPI->R_BeamPoints( origin, vEnd, iSparkSprite, 0.05f, 0.8f, 0.0f, 255.0f, 0.0f, 0, 0.0f, 250.0f, 220.0f, 192.0f );
		}

		CreateSpark( Vector( pTrace->endpos ), Vector( pTrace->plane.normal ), "sprites/smallspark.spr" );

		if( cl_dynamiclights->value > 0.0f )
		{
			dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

			VectorCopy( pTrace->endpos, dl->origin );
			dl->radius = 55.0f;
			dl->color.r = 240;
			dl->color.g = 240;
			dl->color.b = 180;
			dl->die = gHUD.m_flTime + 0.1f;
			dl->decay = 5.0f;
		}
	}
}

void EV_DirtHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pDirt3 = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_dirt3.spr" ) );

	if( pDirt3 )
	{
		// the result is not used in the original, there are always three particles
		gEngfuncs.pfnRandomLong( 2, 3 );

		for( int i = 0; i < 3; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pDirt3, fScale * 4.0f, 160.0f, "dod_particle", false );

			if( !pParticle )
				return;

			pParticle->m_flStretchX = 2.0f;
			pParticle->m_flStretchY = 6.0f;

			vec3_t dir;

			dir.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
			dir.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
			dir.z = 0.0f;
			dir = dir.Normalize();

			pParticle->m_vVelocity.x = 0.0f;
			pParticle->m_vVelocity.y = 0.0f;

			if( i == 0 )
			{
				pParticle->m_vVelocity.z = 100.0f;
				pParticle->m_flStretchY = 7.0f;
			}
			else
			{
				pParticle->m_vVelocity.z = 90.0f;

				VectorMA( pParticle->m_vVelocity, ( i == 1 ) ? 20.0f : -20.0f, dir, pParticle->m_vVelocity );
			}

			pParticle->SetLightFlag( LIGHT_NONE );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iPFlags = PFLAG_DOD_DIRT_DEBRIS;
			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->m_vColor = Vector( 83.0f, 72.0f, 55.0f );
			pParticle->m_flGravity = 0.6f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
		}
	}
}

void EV_SandHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pDirt3 = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_dirt3.spr" ) );

	if( pDirt3 )
	{
		// the result is not used in the original, there are always three particles
		gEngfuncs.pfnRandomLong( 2, 3 );

		for( int i = 0; i < 3; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pDirt3, fScale * 4.0f, 140.0f, "dod_particle", false );

			if( pParticle )
			{
				pParticle->m_flStretchX = 2.0f;
				pParticle->m_flStretchY = 6.0f;

				vec3_t dir;

				dir.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.z = 0.0f;
				dir = dir.Normalize();

				pParticle->m_vVelocity.x = 0.0f;
				pParticle->m_vVelocity.y = 0.0f;

				if( i == 0 )
				{
					pParticle->m_vVelocity.z = 100.0f;
					pParticle->m_flStretchY = 7.0f;
				}
				else
				{
					pParticle->m_vVelocity.z = 90.0f;

					VectorMA( pParticle->m_vVelocity, ( i == 1 ) ? 20.0f : -20.0f, dir, pParticle->m_vVelocity );
				}

				pParticle->SetLightFlag( LIGHT_NONE );
				pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->m_iPFlags = PFLAG_DOD_DIRT_DEBRIS;
				pParticle->m_vColor = Vector( 185.0f, 177.0f, 149.0f );
				pParticle->m_flGravity = 0.6f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			}
		}
	}
}

void EV_GrassHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_grass1.spr" ) );

	if( pSprite )
	{
		int iNum = gEngfuncs.pfnRandomLong( 4, 6 );

		for( int i = 0; i < iNum; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pSprite, fScale * 3.0f, 255.0f, "dod_particle", false );

			if( pParticle )
			{
				pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -20.0f, 20.0f );
				pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -20.0f, 20.0f );
				pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 150.0f, 180.0f );
				pParticle->m_vAVelocity.z = 1000.0f;
				pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
				pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL );
				pParticle->SetLightFlag( LIGHT_COLOR );
				pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pParticle->m_iRendermode = kRenderTransAlpha;
				pParticle->m_flGravity = 0.7f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			}
		}
	}
}

void EV_WoodChips( pmtrace_t *pTrace, float fScale )
{
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_wood1.spr" ) );
	vec3_t vNormal;
	vec3_t vEnd;

	if( pSprite )
	{
		int iNum = gEngfuncs.pfnRandomLong( 1, 2 );

		for( int i = 0; i < iNum; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pSprite, fScale * 4.0f, 255.0f, "dod_particle", false );

			if( !pParticle )
				return;

			for( int k = 0; k < 3; k++ )
				vNormal[k] = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[k];

			VectorMA( pTrace->endpos, 200.0f, vNormal, vEnd );

			pParticle->m_vVelocity = Vector( pTrace->endpos ) - vEnd;
			pParticle->m_vAVelocity.z = 1000.0f;
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD );
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->m_flGravity = 0.6f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
		}

		EV_BasicPuff( pTrace, fScale * 0.5f );
	}
}

void EV_GlassShards( pmtrace_t *pTrace, float fScale )
{
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_glass.spr" ) );
	vec3_t vNormal;
	vec3_t vEnd;

	if( pSprite )
	{
		int iNum = gEngfuncs.pfnRandomLong( 3, 4 );

		for( int i = 0; i < iNum; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pSprite, fScale + fScale, 255.0f, "dod_particle", false );

			if( !pParticle )
				return;

			for( int k = 0; k < 3; k++ )
				vNormal[k] = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[k];

			VectorMA( pTrace->endpos, 400.0f, vNormal, vEnd );

			pParticle->m_vVelocity = Vector( pTrace->endpos ) - vEnd;
			pParticle->m_vAVelocity.z = 1000.0f;
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD );
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->m_flGravity = 0.6f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
		}
	}
}

void EV_WaterHit( pmtrace_t *pTrace, float fScale )
{
	int i;
	float fTime = gEngfuncs.GetClientTime();
	model_s *pRipple = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/ripple.spr" ) );

	for( i = 0; i < 3; i++ )
	{
		vec3_t vOrg = Vector( pTrace->endpos );

		// the original shifts only x, and twice
		vOrg.x += gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );
		vOrg.x += gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );

		CDoDParticle *pParticle = CDoDParticle::Create( vOrg, Vector( 90.0f, 0.0f, 0.0f ), pRipple, 30.0f, 150.0f, "dod_particle", false );

		if( !pParticle )
			return;

		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->m_flFadeSpeed = 4.0f;
		pParticle->SetLightFlag( LIGHT_NONE );
		pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
		pParticle->m_flScaleSpeed = 2.0f;
		pParticle->m_flDieTime = fTime + 2.0f;
		pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
		pParticle->m_iPFlags = PFLAG_DOD_SPARK_INSTANT;
	}

	model_s *pSplash = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/bazookapuff.spr" ) );

	if( pSplash )
	{
		for( i = 0; i < 5; i++ )
		{
			vec3_t vStart = Vector( pTrace->endpos );

			vStart.z += i * 5.0f;

			CDoDParticle *pParticle = CDoDParticle::Create( vStart, Vector( 90.0f, 0.0f, 0.0f ), pSplash, 40.0f, 80.0f, "dod_particle", false );

			if( !pParticle )
				return;

			pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -14.0f, 14.0f );
			pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -14.0f, 14.0f );
			pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 100.0f, 110.0f );
			pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -2.0f, 2.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL | TRI_WATERTRACE );
			pParticle->SetLightFlag( LIGHT_NONE );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->m_flMass = 1.0f;
			pParticle->m_flGravity = 0.4f;
			pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
			pParticle->m_flDieTime = fTime + 1.0f;
		}

		model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/water_big.spr" ) );

		if( pSprite )
		{
			float flScale = gEngfuncs.pfnRandomFloat( 30.0f, 40.0f );
			vec3_t vStart = Vector( pTrace->endpos );

			vStart.z -= 6.0f;

			CDoDParticle *pParticle = CDoDParticle::Create( vStart, Vector( 0.0f, 0.0f, 0.0f ), pSprite, flScale, 150.0f, "dod_particle", false );

			if( pParticle )
			{
				pParticle->m_flStretchY = 1.4f;
				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->m_iPFlags = PFLAG_DOD_DIRT_DEBRIS;
				pParticle->SetLightFlag( LIGHT_NONE );
				pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
				pParticle->m_flGravity = 0.5f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pParticle->m_flFadeSpeed = 10.0f;
				pParticle->m_vVelocity.x = 0.0f;
				pParticle->m_vVelocity.y = 0.0f;
				pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 110.0f, 130.0f );
			}
		}
	}
}

void EV_LeavesHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pFoliage = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_foliage.spr" ) );
	model_s *pLeaf1 = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_leaf1.spr" ) );
	model_s *pLeaf2 = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_leaf2.spr" ) );
	vec3_t vNormal;
	vec3_t vEnd;

	if( pLeaf1 && pFoliage && pLeaf2 )
	{
		int iNum = gEngfuncs.pfnRandomLong( 2, 3 );
		int i;

		for( i = 0; i < iNum; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pFoliage, fScale * 4.0f, 255.0f, "dod_particle", false );

			if( !pParticle )
				return;

			for( int k = 0; k < 3; k++ )
				vNormal[k] = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[k];

			VectorMA( pTrace->endpos, 100.0f, vNormal, vEnd );

			pParticle->m_vVelocity = Vector( pTrace->endpos ) - vEnd;
			pParticle->m_vAVelocity.z = 1000.0f;
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_SPIRAL );
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->m_flGravity = 0.2f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			pParticle->m_flDampingTime = gEngfuncs.GetClientTime() + 0.1f;
			pParticle->AddGlobalWind();
		}

		iNum = gEngfuncs.pfnRandomLong( 0, 1 );

		for( i = 0; i < iNum; i++ )
		{
			model_s *pModel = pLeaf1;

			if( gEngfuncs.pfnRandomLong( 0, 1 ) )
				pModel = pLeaf2;

			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pModel, fScale * 8.0f, 255.0f, "dod_particle", false );

			if( !pParticle )
				return;

			for( int k = 0; k < 3; k++ )
				vNormal[k] = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[k];

			VectorMA( pTrace->endpos, 100.0f, vNormal, vEnd );

			pParticle->m_vVelocity = Vector( pTrace->endpos ) - vEnd;
			pParticle->m_vAVelocity.x = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->m_vAVelocity.y = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->m_vAngles.x = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->m_vAngles.y = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_SPIRAL );
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->m_flGravity = 0.2f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			pParticle->m_flDampingTime = gEngfuncs.GetClientTime() + 0.1f;
			pParticle->m_iPFlags = PFLAG_DOD_SPARK_INSTANT;
			pParticle->AddGlobalWind();
		}
	}
}

void EV_SnowHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pSplash = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/bazookapuff.spr" ) );

	if( pSplash )
	{
		for( int i = 0; i < 5; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 90.0f, 0.0f, 0.0f ), pSplash, fScale * 10.0f, 140.0f, "dod_particle", false );

			if( !pParticle )
				return;

			pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -10.0f, 10.0f );
			pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -10.0f, 10.0f );
			pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 40.0f, 80.0f );
			pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->m_flMass = 1.0f;
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_flGravity = 0.4f;
			pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL | TRI_WATERTRACE );
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 1.0f;
		}
	}
}

void EV_TileHit( pmtrace_t *pTrace, float fScale )
{
	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/debris_tile1.spr" ) );
	vec3_t vNormal;
	vec3_t vEnd;

	if( pSprite )
	{
		int iNum = gEngfuncs.pfnRandomLong( 3, 4 );

		for( int i = 0; i < iNum; i++ )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( Vector( pTrace->endpos ), Vector( 0.0f, 0.0f, 0.0f ), pSprite, fScale * 1.5f, 255.0f, "dod_particle", false );

			if( !pParticle )
				return;

			for( int k = 0; k < 3; k++ )
				vNormal[k] = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[k];

			VectorMA( pTrace->endpos, 400.0f, vNormal, vEnd );

			pParticle->m_vVelocity = Vector( pTrace->endpos ) - vEnd;
			pParticle->m_vAVelocity.z = 1000.0f;
			pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 90.0f );
			pParticle->SetCollisionFlags( TRI_COLLIDEWORLD );
			pParticle->SetLightFlag( LIGHT_COLOR );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->m_flGravity = 0.6f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
		}

		EV_BasicPuff( pTrace, fScale * 0.5f );
	}
}

void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	char hitsound[44];

	switch( cSurfaceType )
	{
	case 'A':
	case 'D':
		sprintf( hitsound, "weapons/hit_sand%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'E':
		sprintf( hitsound, "weapons/hit_leaves%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'G':
	case 'M':
		sprintf( hitsound, "weapons/hit_metal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'H':
		sprintf( hitsound, "weapons/hit_heavymetal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'K':
		return;
	case 'N':
		sprintf( hitsound, "weapons/hit_snow%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'P':
		sprintf( hitsound, "weapons/hit_grass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'S':
		sprintf( hitsound, "weapons/hit_water%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'W':
		sprintf( hitsound, "weapons/hit_wood%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	case 'Y':
		sprintf( hitsound, "weapons/hit_glass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		break;
	default:
		sprintf( hitsound, "weapons/ric%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );
		break;
	}

	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, hitsound, 1.0f, 0.8f, 0, 100 );
}

void EV_DOD_SurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	float fScale = 1.5f;

	// the size of the effects depends on the weapon
	switch( iBulletType )
	{
	case BULLET_PLAYER_COLT:
	case BULLET_PLAYER_LUGER:
	case BULLET_PLAYER_WEBLEY:
		fScale = 1.2f;
		break;
	case BULLET_PLAYER_GARAND:
	case BULLET_PLAYER_SCOPEDKAR:
	case BULLET_PLAYER_THOMPSON:
	case BULLET_PLAYER_SPRING:
	case BULLET_PLAYER_KAR:
	case BULLET_PLAYER_MP40:
	case BULLET_PLAYER_M1CARBINE:
	case BULLET_PLAYER_GREASEGUN:
	case BULLET_PLAYER_K43:
	case BULLET_PLAYER_ENFIELD:
	case BULLET_PLAYER_STEN:
		fScale = 1.6f;
		break;
	case BULLET_PLAYER_MP44:
	case BULLET_PLAYER_BAR:
	case BULLET_PLAYER_MG42:
	case BULLET_PLAYER_MG34:
	case BULLET_PLAYER_30CAL:
	case BULLET_PLAYER_FG42:
	case BULLET_PLAYER_BREN:
		fScale = 2.2f;
		break;
	}

	switch( cSurfaceType )
	{
	case 'A':
		EV_SandHit( pTrace, fScale );
		break;
	case 'D':
		EV_DirtHit( pTrace, fScale );
		break;
	case 'E':
		EV_LeavesHit( pTrace, fScale );
		break;
	case 'G':
	case 'H':
	case 'M':
		EV_MetalHit( pTrace );
		break;
	case 'K':
		break;
	case 'L':
		EV_CreteRubble( pTrace, fScale );
		// the rubble is made twice for this one in the original
	case 'B':
	case 'C':
	case 'R':
		EV_CreteRubble( pTrace, fScale );
		EV_BasicPuff( pTrace, fScale );
		break;
	case 'N':
		EV_SnowHit( pTrace, fScale );
		break;
	case 'P':
		EV_GrassHit( pTrace, fScale );
		break;
	case 'S':
		EV_WaterHit( pTrace, fScale );
		break;
	case 'T':
		EV_TileHit( pTrace, fScale );
		break;
	case 'W':
		EV_WoodChips( pTrace, fScale );
		break;
	case 'Y':
		EV_GlassShards( pTrace, fScale );
		break;
	default:
		EV_BasicPuff( pTrace, fScale );
		break;
	}
}

void EV_DOD_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd )
{
	physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if( pe && pe->solid == SOLID_BSP )
		EV_DOD_GunshotDecalTrace( pTrace, " ", vecSrc, vecEnd, iBulletType );
}

int EV_DOD_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount, float *tracerOrigin )
{
	int tracer = 0;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;

	if( iTracerFreq != 0 && ( (*tracerCount)++ % iTracerFreq ) == 0 )
	{
		if( iTracerFreq != 1 )		// guns that always trace also always decal
			tracer = 1;

		EV_CreateTracer( tracerOrigin, end );
	}

	return tracer;
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_DOD_FireBullets( int idx, float *forward, float *right, float *up, int cShots,
	float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType,
	int iTracerFreq, int *tracerCount )
{
	int i;
	pmtrace_t tr;
	int iShot;
	int tracer;
	cl_entity_t *thisplayer = gEngfuncs.GetLocalPlayer();
	cl_entity_t *firingPlayer = gEngfuncs.GetEntityByIndex( idx );
	vec3_t tracerOrigin;

	for( iShot = 1; iShot <= cShots; iShot++ )
	{
		vec3_t vecEnd;
		bool bIsProneAnim;
		vec3_t vecTarget, vecBullet;
		float fTheta, fDist;

		for( i = 0; i < 3; i++ )
			vecEnd[i] = ( right[i] * vecSpread[0] + vecDirShooting[i] + up[i] * vecSpread[1] ) * flDistance + vecSrc[i];

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
		gEngfuncs.pEventAPI->EV_PushPMStates();
		gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );
		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

		VectorClear( tracerOrigin );

		bIsProneAnim = ( unsigned int )( firingPlayer->curstate.gaitsequence - 15 ) <= 1;

		if( !bIsProneAnim )
		{
			for( i = 0; i < 2; i++ )
				tracerOrigin[i] = vecSrc[i] + right[i] + right[i] + forward[i] * 16.0f;

			tracerOrigin[2] = vecSrc[2] - 4.0f + right[2] + right[2] + forward[2] * 16.0f;
		}
		else
		{
			for( i = 0; i < 2; i++ )
				tracerOrigin[i] = vecSrc[i] + right[i] + right[i] + forward[i] * 16.0f + forward[i] * 34.0f + right[i] * 10.0f;

			tracerOrigin[2] = vecSrc[2] - 16.0f + right[2] + right[2] + forward[2] * 16.0f + forward[2] * 34.0f + right[2] * 10.0f;
		}

		tracer = EV_DOD_CheckTracer( idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount, tracerOrigin );

		if( firingPlayer != thisplayer )
		{
			pmtrace_t tr2;

			for( i = 0; i < 3; i++ )
			{
				vecBullet[i] = tr.endpos[i] - vecSrc[i];
				vecTarget[i] = firingPlayer->curstate.origin[i] - thisplayer->curstate.origin[i];
			}

			gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, thisplayer->curstate.origin, PM_STUDIO_BOX, -1, &tr2 );

			if( tr2.fraction > 0.9f )
			{
				vec3_t vecCross = CrossProduct( vecBullet, vecTarget );

				if( vecCross.Length() != 0.0f )
				{
					fTheta = acosf( DotProduct( vecBullet, vecTarget ) / ( vecBullet.Length() * vecTarget.Length() ) );
					fDist = sinf( fTheta ) * vecTarget.Length();

					if( fDist < 100.0f && vecBullet.Length() > vecTarget.Length() * cosf( fTheta ) )
					{
						char buf[32];

						sprintf( buf, "weapons/whizz%d.wav", gEngfuncs.pfnRandomLong( 1, 17 ) );
						gEngfuncs.pfnPlaySoundByName( buf, gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ) );
					}
				}
			}
		}

		if( tr.fraction != 1.0f )
		{
			physent_t *pe;

			EV_DOD_PlayTextureSound( idx, &tr, vecSrc, vecEnd, iBulletType );

			pe = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );

			if( pe && pe->solid == SOLID_BSP )
				EV_DOD_GunshotDecalTrace( &tr, " ", vecSrc, vecEnd, iBulletType );
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

float EV_DOD_WaterHeight( Vector position, float minz, float maxz )
{
	vec3_t midUp;
	float diff;

	midUp[0] = position[0];
	midUp[1] = position[1];
	midUp[2] = minz;

	if( gEngfuncs.PM_PointContents( midUp, NULL ) == CONTENTS_WATER )
	{
		midUp[2] = maxz;

		if( gEngfuncs.PM_PointContents( midUp, NULL ) == CONTENTS_WATER )
			return maxz;

		diff = minz;

		if( maxz - minz > 1.0f )
		{
			float flRange = maxz - minz;

			while( 1 )
			{
				midUp[2] = flRange * 0.5f + diff;

				if( gEngfuncs.PM_PointContents( midUp, NULL ) == CONTENTS_WATER )
					diff = midUp[2];
				else
					maxz = midUp[2];

				flRange = maxz - diff;

				if( flRange <= 1.0f )
					break;
			}
		}
	}

	return midUp[2];
}

int EV_DOD_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult )
{
	int endlevel, midlevel, startlevel;
	vec3_t a, b, c;
	vec3_t vecDir;
	float fTolerance;

	endlevel = gEngfuncs.PM_PointContents( pTrace->endpos, NULL );
	startlevel = gEngfuncs.PM_PointContents( vecSrc, NULL );

	if( endlevel == startlevel )
		return 0;

	VectorCopy( vecSrc, a );
	VectorCopy( pTrace->endpos, b );
	VectorSubtract( b, a, vecDir );
	fTolerance = 4.0f;

	while( Length( vecDir ) > fTolerance )
	{
		VectorScale( vecDir, 0.5f, vecDir );
		VectorAdd( a, vecDir, c );

		startlevel = gEngfuncs.PM_PointContents( a, NULL );
		midlevel = gEngfuncs.PM_PointContents( c, NULL );

		if( midlevel == startlevel )
		{
			VectorCopy( c, a );
		}
		else
		{
			VectorCopy( c, b );
		}

		VectorSubtract( b, a, vecDir );
	}

	VectorCopy( a, vecResult );

	return 1;
}

void EV_DOD_BubbleTrails( Vector from, Vector to, int count )
{
	float flHeight;
	int m_iBubbles;

	flHeight = EV_DOD_WaterHeight( from, from[2], from[2] + 256.0f ) - from[2];

	if( flHeight < 8.0f )
	{
		flHeight = EV_DOD_WaterHeight( to, to[2], to[2] + 256.0f ) - to[2];

		if( flHeight < 8.0f )
			return;

		flHeight = flHeight + to[2] - from[2];
	}

	m_iBubbles = count > 255 ? 255 : count;

	gEngfuncs.pEfxAPI->R_BubbleTrail( from, to, flHeight, gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/bubble.spr" ), m_iBubbles, 8.0f );
}

void EV_FireMelee( event_args_t *args )
{
	int iAnim;
	int bPlayWiff;
	int bPlayHit;

	iAnim = args->iparam1;
	bPlayWiff = args->bparam1;
	bPlayHit = args->bparam2;

	if( EV_IsLocal( args->entindex ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );

	if( bPlayWiff )
	{
		gEngfuncs.pfnRandomLong( 0, 1 );
		gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_WEAPON, "weapons/knife_slash1.wav", 1.0f, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( bPlayHit )
	{
		gEngfuncs.pfnRandomLong( 0, 2 );
		gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_WEAPON, "weapons/knife_hit1.wav", 1.0f, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_ResetAnimationEvents( args->entindex );
}

void EV_FireColt( event_args_t *args )
{
	int idx;
	int empty;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;
	empty = args->bparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? COLT_SHOOT : COLT_SHOOT_EMPTY, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_COLT );
	}

	EV_MuzzleFlash( idx, PISTOL );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/colt_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_COLT, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireLuger( event_args_t *args )
{
	int idx;
	int empty;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;
	empty = args->bparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? LUGER_SHOOT : LUGER_SHOOT_EMPTY, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_LUGER );
	}

	EV_MuzzleFlash( idx, PISTOL );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/luger_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_LUGER, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

int MG42_DownAnims[9] =
{
	MG42_DOWNSHOOT2,
	MG42_DOWNSHOOT2,
	MG42_DOWNSHOOT3,
	MG42_DOWNSHOOT4,
	MG42_DOWNSHOOT5,
	MG42_DOWNSHOOT6,
	MG42_DOWNSHOOT7,
	MG42_DOWNSHOOT8,
	MG42_DOWNSHOOT
};

int MG42_UpAnims[9] =
{
	MG42_UPSHOOT2,
	MG42_UPSHOOT2,
	MG42_UPSHOOT3,
	MG42_UPSHOOT4,
	MG42_UPSHOOT5,
	MG42_UPSHOOT6,
	MG42_UPSHOOT7,
	MG42_UPSHOOT8,
	MG42_UPSHOOT
};

void EV_FireMG42( event_args_t *args )
{
	int idx;
	int shell;
	bool bCallAnim;
	int iBulletsRemaining;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;
	bCallAnim = args->bparam1 == 0;
	iBulletsRemaining = args->iparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shell_mg42.mdl" );

	if( bCallAnim )
	{
		if( EV_IsLocal( idx ) )
		{
			if( iBulletsRemaining > 8 )
				iBulletsRemaining = 8;

			if( gHUD.IsInMGDeploy() )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_DownAnims[iBulletsRemaining], EV_GetWeaponBody() );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_UpAnims[iBulletsRemaining], EV_GetWeaponBody() );

			gHUD.DoRecoil( WEAPON_MG42 );
		}

		EV_MuzzleFlash( idx, MG );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 2, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_MG42, 3, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireMG34( event_args_t *args )
{
	int idx;
	int empty;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	empty = args->iparam1;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		int iAnim;

		if( gHUD.IsInMGDeploy() )
			iAnim = empty > 0 ? MG34_DOWNSHOOT : MG34_DOWNSHOOTEMPTY;
		else
			iAnim = empty > 0 ? MG34_UPSHOOT : MG34_UPSHOOTEMPTY;

		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_MG34 );
	}

	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg34_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 2, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_MG34, 3, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

int ThirtyCal_DownAnims[9] =
{
	CAL30_DOWNSHOOT2,
	CAL30_DOWNSHOOT2,
	CAL30_DOWNSHOOT3,
	CAL30_DOWNSHOOT4,
	CAL30_DOWNSHOOT5,
	CAL30_DOWNSHOOT6,
	CAL30_DOWNSHOOT7,
	CAL30_DOWNSHOOT8,
	CAL30_DOWNSHOOT
};

int ThirtyCal_UpAnims[9] =
{
	CAL30_UPSHOOT2,
	CAL30_UPSHOOT2,
	CAL30_UPSHOOT3,
	CAL30_UPSHOOT4,
	CAL30_UPSHOOT5,
	CAL30_UPSHOOT6,
	CAL30_UPSHOOT7,
	CAL30_UPSHOOT8,
	CAL30_UPSHOOT
};

void EV_Fire30CAL( event_args_t *args )
{
	int idx;
	int iBulletsRemaining;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;
	iBulletsRemaining = args->iparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( iBulletsRemaining > 8 )
			iBulletsRemaining = 8;

		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_DownAnims[iBulletsRemaining], EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_UpAnims[iBulletsRemaining], EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_CAL30 );
	}

	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/30cal_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_30CAL, 3, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireGarand( event_args_t *args )
{
	int idx;
	int empty;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;
	empty = args->bparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( args->iparam1 == 1 )
	{
		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( empty == 0 ? GARAND_SMASH : GARAND_SMASH_EMPTY, EV_GetWeaponBody() );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			int iAnim = GARAND_SHOOT_EMPTY;

			if( empty )
				iAnim = gEngfuncs.pfnRandomLong( 0, 2 ) + GARAND_SHOOT1;

			gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_GARAND );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/garand_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !empty )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/garand_reload_clipding.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

		EV_GetGunPosition( args, vecSrc, origin );
		VectorCopy( forward, vecAiming );

		vecSpread[0] = flSpread_x;
		vecSpread[1] = flSpread_y;
		vecSpread[2] = flSpread_y;

		EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_GARAND, 0, &tracerCount[idx - 1] );

		EV_ResetAnimationEvents( idx );
	}
}

void EV_FireM1Carbine( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	GetViewEntity();

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M1CARBINE_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_M1CARBINE );
	}

	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/carbine_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_M1CARBINE, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireScopedKar( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDKAR_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_SCOPEDKAR );
	}

	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/kar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_SCOPEDKAR, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireThompson( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + THOMPSON_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_THOMPSON );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/thompson_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_THOMPSON, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireMP44( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MP44_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_MP44 );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp44_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_MP44, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireGreaseGun( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_GREASEGUN );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/greasegun_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_GREASEGUN, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireFG42( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gHUD.IsInMGDeploy() ? FG42_DOWN_SHOOT : FG42_UP_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_FG42 );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/fg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_FG42, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireK43( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( args->iparam1 == 1 )
	{
		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( K43_SMASH, EV_GetWeaponBody() );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + K43_SHOOT1, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_K43 );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/k43_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );
		VectorCopy( forward, vecAiming );

		vecSpread[0] = flSpread_x;
		vecSpread[1] = flSpread_y;
		vecSpread[2] = flSpread_y;

		EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_K43, 0, &tracerCount[idx - 1] );

		EV_ResetAnimationEvents( idx );
	}
}

void EV_FireEnfield( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( args->iparam1 != 1 )
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SHOOT, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_ENFIELD );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfield_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );
		VectorCopy( forward, vecAiming );

		vecSpread[0] = flSpread_x;
		vecSpread[1] = flSpread_y;
		vecSpread[2] = flSpread_y;

		EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_ENFIELD, 0, &tracerCount[idx - 1] );

		EV_ResetAnimationEvents( idx );
		return;
	}

	switch( ( int )gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
	{
	case 1:
	case 3:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	case 0:
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	default:
		return;
	}

	if( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SWING, EV_GetWeaponBody() );
}

void EV_FireSten( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( STEN_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_STEN );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sten_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_STEN, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireBren( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gHUD.IsInMGDeploy() ? BREN_DOWN_SHOOT : BREN_UP_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_BREN );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bren_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_BREN, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireWebley( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_WEBLEY );
	}

	EV_MuzzleFlash( idx, PISTOL );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/webley_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_WEBLEY, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_PopHelmet( event_args_t *args )
{
	vec3_t origin, angles;
	int life;
	int model;
	vec3_t endpos;
	TEMPENTITY *helmetmdl;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	if( args->iparam1 > 6 )
		return;

	life = 5;
	model = gEngfuncs.pEventAPI->EV_FindModelIndex( sHelmetModels[args->iparam1] );

	if( !model )
		return;

	endpos[0] = args->fparam1;
	endpos[1] = args->fparam2;
	endpos[2] = 0.0f;

	helmetmdl = gEngfuncs.pEfxAPI->R_TempModel( origin, angles, endpos, life, model, 1 );

	if( helmetmdl )
		helmetmdl->flags |= FTENT_ROTATE | FTENT_COLLIDEWORLD | FTENT_HITSOUND;
}

int g_iSwing;

void EV_Knife( event_args_t *args )
{
	int idx;
	vec3_t origin;
	int temp;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	temp = ( int )gEngfuncs.pfnRandomFloat( 0.0f, 1.0f );

	switch( temp )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash2.wav", 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );
		break;
	}

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, EV_GetWeaponBody() );

		switch( g_iSwing++ % 2 )
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, EV_GetWeaponBody() );
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH2, EV_GetWeaponBody() );
			break;
		}
	}
}

void EV_FireMP40( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	GetViewEntity();

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MP40_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_MP40 );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp40_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_MP40, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireSpring( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + SPRING_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_SPRING );
	}

	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spring_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_SPRING, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireKar( event_args_t *args )
{
	int idx;
	int FireMode;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	FireMode = args->iparam1;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( FireMode == 1 )
	{
		switch( ( int )gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
		{
		case 0:
		case 2:
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

			if( EV_IsLocal( idx ) )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SWING, EV_GetWeaponBody() );

			break;
		case 1:
		case 3:
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

			if( EV_IsLocal( idx ) )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SLASH, EV_GetWeaponBody() );

			break;
		}

		return;
	}

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_KAR );
	}

	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/kar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_KAR, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireBAR( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gHUD.IsInMGDeploy() ? BAR_DOWN_SHOOT : BAR_UP_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_BAR );
	}

	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_BAR, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_FireBazooka( event_args_t *args )
{
	if( EV_IsLocal( args->entindex ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( BAZOOKA_FIRE, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_BAZOOKA );
	}

	EV_MuzzleFlash( args->entindex, MG );

	EV_ResetAnimationEvents( args->entindex );
}

void EV_FirePschreck( event_args_t *args )
{
	if( EV_IsLocal( args->entindex ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( PSCHRECK_FIRE, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_PSCHRECK );
	}

	EV_MuzzleFlash( args->entindex, MG );

	EV_ResetAnimationEvents( args->entindex );
}

void EV_FirePIAT( event_args_t *args )
{
	if( EV_IsLocal( args->entindex ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( PIAT_FIRE, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_PIAT );
	}

	EV_MuzzleFlash( args->entindex, MG );

	EV_ResetAnimationEvents( args->entindex );
}

void EV_FireMortar( event_args_t *args )
{
	if( EV_IsLocal( args->entindex ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MORTAR_DEPLOYED_FIRE, EV_GetWeaponBody() );

	gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_WEAPON, "weapons/mortar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_MuzzleFlash( args->entindex, MG );

	EV_ResetAnimationEvents( args->entindex );
}

void EV_FireScopedEnfield( event_args_t *args )
{
	int idx;
	float flSpread_x, flSpread_y;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	idx = args->entindex;
	flSpread_x = args->fparam1;
	flSpread_y = args->fparam2;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SCOPEDENFIELD_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_ENFIELD );
	}

	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfieldsniper_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	vecSpread[0] = flSpread_x;
	vecSpread[1] = flSpread_y;
	vecSpread[2] = flSpread_y;

	EV_DOD_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f, BULLET_PLAYER_ENFIELD, 0, &tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

void EV_PlayWhizz( event_args_t *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
		gEngfuncs.pfnPlaySoundByName( "player/whizz.wav", 1.0f );
}

void EV_DoDCamera( event_args_t *args )
{
	int iparam1 = args->iparam1;

	if( EV_IsLocal( args->entindex ) )
	{
		if( iparam1 == 1 )
		{
			gHUD.m_Scope.SetScope( WEAPON_BINOC );
		}
		else if( iparam1 == 2 )
		{
			gHUD.m_Scope.SetScope( WEAPON_SPRING );
		}
		else
		{
			gHUD.m_Scope.SetScope( 0 );

			// the original also shows (args->bparam1) or hides the widescreen bars of the VGUI2 viewport here
		}
	}
}

void EV_SparkShower( event_args_t *args )
{
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_SparkShower( origin );
}

void P_ExplosionFlash( event_args_t *args )
{
	static const char *sprname = "sprites/grenade_flash.spr";

	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( sprname ) );

	if( pSprite )
	{
		vec3_t org = Vector( args->origin ) + Vector( 0, 0, 16 );

		CDoDParticle *pParticle = CDoDParticle::Create( org, Vector( 0, 0, 0 ), pSprite, 100.0f, 234.0f, "dod_particle", false );

		if( pParticle )
		{
			pParticle->SetCollisionFlags( TRI_ANIMATEDIE );
			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->SetLightFlag( LIGHT_NONE );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_flScaleSpeed = 2.0f;
			pParticle->m_iFrame = 0;
			pParticle->m_iFramerate = 60;
			pParticle->m_iPFlags = 0;
		}
	}
	else
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
	}
}

void PlayExplosionSound( float *origin )
{
	switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/explode3.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/explode4.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	default:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/explode5.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	}
}

void PlayMortarExplosionSound( float *origin )
{
	switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/mortar_hit1.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/mortar_hit2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	default:
		gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, "weapons/mortar_hit3.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		break;
	}
}

void EV_BaseExplosion( event_args_t *args )
{
	if( args->bparam2 )
		PlayMortarExplosionSound( args->origin );
	else
		PlayExplosionSound( args->origin );

	P_ExplosionFlash( args );

	if( cl_dynamiclights->value > 0.0f )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		VectorCopy( args->origin, dl->origin );
		dl->radius = 400.0f;
		dl->decay = 275.0f;
		dl->color.r = 248;
		dl->color.g = 255;
		dl->color.b = 120;
		dl->die = gHUD.m_flTime + 3.0f;
	}
}

void EV_WaterExplosion( event_args_t *args )
{
	vec3_t vStart = Vector( args->origin );
	vec3_t vNormal = Vector( 0, 0, 1 );
	vec3_t vRight;
	vec3_t vUp;
	vec3_t vAngles;

	VectorAngles( vNormal, vAngles );
	AngleVectors( vAngles, NULL, vRight, vUp );

	vRight = vRight.Normalize();
	vUp = vUp.Normalize();

	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/dirt_puff.spr" ) );

	float flHeight = EV_DOD_WaterHeight( vStart, vStart.z, vStart.z + 512.0f );

	for( int i = 0; i < 350; i += 10 )
	{
		float flScale = gEngfuncs.pfnRandomFloat( 100.0f, 150.0f );
		vec3_t vOrigin = Vector( vStart.x, vStart.y, flHeight + i );

		vOrigin.x += gEngfuncs.pfnRandomFloat( -35.0f, 35.0f );
		vOrigin.y += gEngfuncs.pfnRandomFloat( -35.0f, 35.0f );

		if( pSprite )
		{
			CDoDDirtExploDust *pSmoke = CDoDDirtExploDust::Create( vOrigin, Vector( 0, 0, 1 ), pSprite, flScale, 255.0f, "dod_dirtexplo" );

			if( pSmoke )
			{
				pSmoke->m_iRendermode = kRenderTransAlpha;
				pSmoke->m_flGravity = 0.2f;
				pSmoke->m_flSize = flScale;
				pSmoke->SetLightFlag( LIGHT_NONE );
				pSmoke->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.5f, 1.5f ) );
				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_iFrame = (int)gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );
				pSmoke->m_vColor = Vector( 200.0f, 200.0f, 225.0f );
				pSmoke->m_flScaleSpeed = gEngfuncs.pfnRandomFloat( 1.5f, 2.8f );

				float flUp = gEngfuncs.pfnRandomFloat( 25.0f, 50.0f );

				flUp *= gEngfuncs.pfnRandomLong( -1, 1 );

				float flRight = gEngfuncs.pfnRandomFloat( 25.0f, 50.0f );

				flRight *= gEngfuncs.pfnRandomLong( -1, 1 );

				float flForward = gEngfuncs.pfnRandomFloat( 300.0f, 450.0f );

				pSmoke->m_vVelocity = vNormal * flForward + vRight * flRight + vUp * flUp;
				pSmoke->m_bFire = false;
				pSmoke->m_flFadeSpeed = -1.0f;
				pSmoke->m_flActivateTime = gEngfuncs.pfnRandomFloat( 0.95f, 0.96f );
			}
		}
	}

	pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/water_big.spr" ) );

	float flScale = gEngfuncs.pfnRandomFloat( 275.0f, 300.0f );
	vec3_t vOrigin = Vector( vStart.x, vStart.y, 0.5f * flScale - 10.0f + flHeight );

	CDoDParticle *pParticle = CDoDParticle::Create( vOrigin, Vector( 90, 0, 0 ), pSprite, flScale, 255.0f, "dod_particle", false );

	if( pParticle )
	{
		pParticle->m_iRendermode = kRenderTransAdd;
		pParticle->SetLightFlag( LIGHT_NONE );
		pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
		pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
		pParticle->m_flGravity = 0.5f;
		pParticle->m_flFadeSpeed = 10.0f;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
		pParticle->m_vVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( 100.0f, 125.0f ) );
	}
}

void EV_DirtTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
	if( currenttime >= ent->entity.baseline.fuser2 )
	{
		ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime();

		model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" ) );
		vec3_t vOrigin = Vector( ent->entity.origin );
		float flScale = ent->entity.baseline.fuser1;

		ent->entity.baseline.fuser1 -= 7.0f;

		if( ent->entity.baseline.fuser1 < 10.0f )
			ent->entity.baseline.fuser1 = 10.0f;

		vOrigin.x += gEngfuncs.pfnRandomFloat( -10.0f, 10.0f );
		vOrigin.y += gEngfuncs.pfnRandomFloat( -10.0f, 10.0f );
		vOrigin.z += gEngfuncs.pfnRandomFloat( -10.0f, 10.0f );

		vec3_t vVelocity = Vector( ent->entity.baseline.origin[0], ent->entity.baseline.origin[1], ent->entity.baseline.origin[2] );
		float flSpeed = vVelocity.Length() * 0.9f;

		vVelocity = vVelocity.Normalize() * flSpeed;

		ent->entity.baseline.origin[0] = vVelocity.x;
		ent->entity.baseline.origin[1] = vVelocity.y;
		ent->entity.baseline.origin[2] = vVelocity.z;

		if( pSprite )
		{
			CDoDRocketTrail *pSmoke = CDoDRocketTrail::Create( vOrigin, Vector( 0, 0, 1 ), pSprite, flScale, 135.0f, "dod_trailsmoke" );

			if( pSmoke )
			{
				pSmoke->m_flGravity = gEngfuncs.pfnRandomFloat( 0.05f, 0.1f );
				pSmoke->m_iRendermode = kRenderTransAlpha;
				pSmoke->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL );
				pSmoke->m_flSize = flScale;
				pSmoke->SetLightFlag( LIGHT_NONE );
				pSmoke->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.5f, 1.5f ) );
				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 8.0f;
				pSmoke->m_iFrame = (int)gEngfuncs.pfnRandomFloat( 10.0f, 15.0f );
				pSmoke->m_vColor = Vector( 155.0f, 155.0f, 140.0f );
				pSmoke->m_bRocketTrail = false;
			}
		}
	}
}

void EV_DirtRubble( event_args_t *args )
{
	TEMPENTITY *pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

	if( pTrailSpawner )
	{
		pTrailSpawner->flags |= FTENT_GRAVITY | FTENT_PERSIST | FTENT_COLLIDEKILL | FTENT_NOMODEL | FTENT_CLIENTCUSTOM;
		pTrailSpawner->callback = EV_DirtTrailCallback;
		pTrailSpawner->die = gEngfuncs.GetClientTime() + 2.0f;
		pTrailSpawner->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.pfnRandomFloat( 175.0f, 200.0f );
		pTrailSpawner->entity.baseline.origin[0] = gEngfuncs.pfnRandomLong( 0, 1 ) ? 500.0f : -500.0f;
		pTrailSpawner->entity.baseline.origin[1] = gEngfuncs.pfnRandomLong( 0, 1 ) ? 500.0f : -500.0f;
		pTrailSpawner->entity.baseline.origin[2] = gEngfuncs.pfnRandomFloat( 500.0f, 1000.0f );
	}
}

void EV_DirtExplosion( event_args_t *args )
{
	vec3_t vStart = Vector( args->origin );
	vec3_t vEnd = vStart + Vector( 0.0f, 0.0f, -64.0f );
	int ignoreent = -1;
	pmtrace_t tr = *gEngfuncs.PM_TraceLine( vStart, vEnd, PM_WORLD_ONLY, 2, ignoreent );

	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/dirt_puff.spr" ) );
	vec3_t vRight;
	vec3_t vUp;
	vec3_t vAngles;

	VectorAngles( tr.plane.normal, vAngles );
	AngleVectors( vAngles, NULL, vRight, vUp );

	vRight = vRight.Normalize();
	vUp = vUp.Normalize();

	for( int i = 0; i < 350; i += 10 )
	{
		float flScale = gEngfuncs.pfnRandomFloat( 150.0f, 225.0f );
		vec3_t vOrigin = Vector( tr.endpos[0], tr.endpos[1], tr.endpos[2] + i );

		vOrigin.x += gEngfuncs.pfnRandomFloat( -15.0f, 15.0f );
		vOrigin.y += gEngfuncs.pfnRandomFloat( -15.0f, 15.0f );

		if( pSprite )
		{
			CDoDDirtExploDust *pSmoke = CDoDDirtExploDust::Create( vOrigin, Vector( 0, 0, 1 ), pSprite, flScale, 225.0f, "dod_dirtexplo" );

			if( pSmoke )
			{
				pSmoke->m_iRendermode = kRenderTransAlpha;
				pSmoke->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEKILL );
				pSmoke->m_flSize = flScale;
				pSmoke->SetLightFlag( LIGHT_NONE );
				pSmoke->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pSmoke->m_flGravity = 0.25f;
				pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.5f, 1.5f ) );
				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
				pSmoke->m_iFrame = 0;
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_flScaleSpeed = gEngfuncs.pfnRandomFloat( 0.0f, 1.7f );
				pSmoke->m_vColor = Vector( 155.0f, 155.0f, 140.0f );

				float flUp = gEngfuncs.pfnRandomFloat( 25.0f, 50.0f );

				flUp *= gEngfuncs.pfnRandomLong( -1, 1 );

				float flRight = gEngfuncs.pfnRandomFloat( 25.0f, 50.0f );

				flRight *= gEngfuncs.pfnRandomLong( -1, 1 );

				float flForward = gEngfuncs.pfnRandomFloat( 300.0f, 450.0f );

				pSmoke->m_vVelocity = Vector( tr.plane.normal ) * flForward + vRight * flRight + vUp * flUp;
				pSmoke->m_flActivateTime = gEngfuncs.pfnRandomFloat( 0.9f, 0.99f );
				pSmoke->m_bFire = false;
				pSmoke->m_flFadeSpeed = -1.0f;
			}
		}
	}

	pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/explosion_flash.spr" ) );

	float flScale = gEngfuncs.pfnRandomFloat( 100.0f, 125.0f );

	CDoDParticle *pParticle = CDoDParticle::Create( vStart, Vector( 90, 0, 0 ), pSprite, flScale, 255.0f, "dod_particle", false );

	if( pParticle )
	{
		pParticle->m_iRendermode = kRenderTransAlpha;
		pParticle->SetLightFlag( LIGHT_NONE );
		pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
		pParticle->m_vColor = Vector( 255.0f, 255.0f, 200.0f );
		pParticle->m_flGravity = 0.0f;
		pParticle->m_flFadeSpeed = 36.0f;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.1f;
		pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

		P_Rubble( args );

		int iRandom = gEngfuncs.pfnRandomLong( 4, 6 );

		for( int i = 0; i < iRandom; i++ )
			EV_DirtRubble( args );
	}
}

void EV_SandExplosion( event_args_t *args )
{
	EV_DirtExplosion( args );
}

void EV_Explosion( event_args_t *args )
{
	vec3_t end;
	vec3_t down;
	char chTextureType;

	if( args->bparam1 )
	{
		EV_WaterExplosion( args );

		gEngfuncs.pEventAPI->EV_PlaySound( 0, args->origin, CHAN_WEAPON, "weapons/explodewater.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		return;
	}

	down = Vector( 0, 0, -1 );
	VectorMA( args->origin, 256.0f, down, end );

	chTextureType = EV_FindHitTexture( args->origin, end );

	if( chTextureType == 'D' )
	{
		EV_DirtExplosion( args );
		P_ExplosionSmoke( args );
	}
	else if( chTextureType == 'A' )
	{
		EV_SandExplosion( args );
	}
	else if( chTextureType != 'L' && chTextureType != 'C' && chTextureType != 'R' && chTextureType != 'B' )
	{
		P_ExplosionSmoke( args );
	}
	else
	{
		P_Rubble( args );
		P_ExplosionSmoke( args );
	}

	EV_BaseExplosion( args );
}

void EV_Bubbles( event_args_t *args )
{
	float height;
	float speed;
	int count;
	int model;
	vec3_t mins;
	vec3_t maxs;

	VectorCopy( args->origin, mins );
	VectorCopy( args->angles, maxs );

	height = args->fparam1;
	model = args->iparam1;
	count = args->iparam2;
	speed = args->fparam2;

	gEngfuncs.pEfxAPI->R_Bubbles( mins, maxs, height, model, count, speed );
}

void EV_BubbleTrails( event_args_t *args )
{
	int idx;
	float height;
	float speed;
	int count;
	int model;
	vec3_t origin;
	vec3_t angles;

	idx = args->entindex;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	height = args->fparam1;
	model = args->iparam1;
	count = args->iparam2;
	speed = args->fparam2;

	gEngfuncs.pEfxAPI->R_BubbleTrail( origin, angles, height, model, count, speed );
}

void EV_BulletTracers( event_args_t *args )
{
	vec3_t origin;
	vec3_t end;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, end );

	gEngfuncs.pEfxAPI->R_TracerEffect( origin, end );
}

void EV_BloodStream( event_args_t *args )
{
	vec3_t origin;
	vec3_t velocity( 0.0f, 0.0f, 0.0f );	// not initialized in the original
	int modelindex2;
	TEMPENTITY *pTemp;

	VectorCopy( args->angles, origin );

	modelindex2 = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" );

	velocity.z += 4.0f;
	origin.z += 2.0f;

	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, (float)gEngfuncs.pfnRandomLong( 2, 4 ), modelindex2, kRenderTransAlpha, kRenderFxNone, 0.35f, 1.0f, FTENT_FADEOUT | FTENT_SPRANIMATE );

	pTemp->fadeSpeed = 4.0f;
	pTemp->entity.curstate.renderamt = 800;
	pTemp->entity.curstate.framerate = 15.0f;
	pTemp->entity.curstate.rendercolor.r = 180;
	pTemp->entity.curstate.rendercolor.g = 176;
	pTemp->entity.curstate.rendercolor.b = 148;

	modelindex2 = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" );

	velocity.z += 4.0f;
	origin.z += 2.0f;

	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, (float)gEngfuncs.pfnRandomLong( 0, 0 ), modelindex2, kRenderTransAlpha, kRenderFxNone, 0.35f, 1.0f, FTENT_FADEOUT | FTENT_SPRANIMATE );

	pTemp->fadeSpeed = 4.0f;
	pTemp->entity.curstate.framerate = 15.0f;
	pTemp->entity.curstate.renderamt = 800;
	pTemp->entity.curstate.rendercolor.r = 75;
	pTemp->entity.curstate.rendercolor.g = 0;
	pTemp->entity.curstate.rendercolor.b = 0;
}

void EV_BloodSprite( event_args_t *args )
{
	int idx;
	float model2;
	float size;
	int model;
	int color;
	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	model2 = args->fparam1;
	size = args->fparam2;
	model = args->iparam1;
	color = args->iparam2;

	gEngfuncs.pEfxAPI->R_BloodSprite( origin, color, model, (int)model2, size );
}

void EV_Smoke( event_args_t *args )
{
	P_ExplosionSmoke( args );
}

void EV_Pain( event_args_t *args )
{
	int idx;
	vec3_t origin;
	char szSound[32];

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	sprintf( szSound, "player/damage%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, szSound, 1.0f, ATTN_IDLE, 0, PITCH_NORM );
}

extern char *s_USVoiceFiles[];
extern char *s_BRITVoiceFiles[];
extern char *s_GERVoiceFiles[];
extern char *s_VoiceCommands[][4];

void EV_USVoice( event_args_t *args )
{
	int idx = args->entindex;
	vec3_t origin = Vector( args->origin );
	int voicenum = args->iparam1;
	int i_isBrit = args->iparam2;

	if( !GetClientVoiceMgr() || !GetClientVoiceMgr()->IsPlayerBlocked( idx ) )
	{
		if( i_isBrit )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, s_BRITVoiceFiles[voicenum], gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, s_USVoiceFiles[voicenum], gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( idx );
		cl_entity_t *thisplayer = gEngfuncs.GetLocalPlayer();

		gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

		if( ent->curstate.team == g_iTeamNumber && !g_iUser1 && ent->curstate.team == thisplayer->curstate.team )
		{
			vec3_t v_playerDistance = origin - thisplayer->curstate.origin;

			if( v_playerDistance.Length() <= 1100.0f )
			{
				char pattern[256];
				char *subtitle;

				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[idx].name, ": %s2" );

				subtitle = s_VoiceCommands[voicenum][3];

				if( !subtitle || !*subtitle || !gHUD.m_bBritish )
					subtitle = s_VoiceCommands[voicenum][1];

				gHUD.m_SayText.SayTextPrint( pattern, sizeof( pattern ), idx, (char *)"#VOICE", subtitle, NULL, NULL );

				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				gHUD.m_Spectator.AddVoiceIconToPlayerEnt( idx );
			}
		}
	}
}

void EV_GERVoice( event_args_t *args )
{
	int idx = args->entindex;
	vec3_t origin = Vector( args->origin );
	int voicenum = args->iparam1;

	if( !GetClientVoiceMgr() || !GetClientVoiceMgr()->IsPlayerBlocked( idx ) )
	{
		// the allies are the british, there is another voice for the "spotted" call
		if( voicenum == 27 && gHUD.m_bBritish )
			voicenum = 28;

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, s_GERVoiceFiles[voicenum], gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( idx );
		cl_entity_t *thisplayer = gEngfuncs.GetLocalPlayer();

		if( ent->curstate.team == g_iTeamNumber && !g_iUser1 && ent->curstate.team == thisplayer->curstate.team )
		{
			vec3_t v_playerDistance = origin - thisplayer->curstate.origin;

			if( v_playerDistance.Length() <= 1100.0f )
			{
				char pattern[256];
				char *subtitle;

				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[idx].name, ": %s2" );

				if( *s_VoiceCommands[voicenum][2] )
					subtitle = gHUD.m_TextMessage.BufferedLocaliseTextString( s_VoiceCommands[voicenum][2] );
				else
					subtitle = gHUD.m_TextMessage.BufferedLocaliseTextString( s_VoiceCommands[voicenum][1] );

				gHUD.m_SayText.SayTextPrint( pattern, sizeof( pattern ), idx, (char *)"#VOICE", subtitle, NULL, NULL );

				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				gHUD.m_Spectator.AddVoiceIconToPlayerEnt( idx );
			}
		}
	}
}

void EV_BodyDamage( event_args_t *args )
{
	int idx;
	vec3_t origin;
	int bodyarea;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	bodyarea = args->iparam1;

	if( bodyarea == 0 )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, "player/helmet_hit.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, PITCH_NORM );
}

void EV_RoundReleaseSound( event_args_t *args )
{
	int idx;
	vec3_t origin;
	char *pszString;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) && !g_iVuser1z )
	{
		int teamwav = args->iparam1;
		int randomgen = args->iparam2;
		int isbritish = args->bparam1;

		pszString = NULL;

		if( teamwav == 1 && isbritish )
		{
			switch( randomgen )
			{
			case 0:
			case 2:
				pszString = "player/britstartround.wav";
				break;
			case 1:
			case 3:
				pszString = "player/britstartround2.wav";
				break;
			default:
				return;
			}
		}
		else if( teamwav == 1 && !isbritish )
		{
			switch( randomgen )
			{
			case 0:
			case 2:
				pszString = "player/usstartround.wav";
				break;
			case 1:
			case 3:
				pszString = "player/usstartround2.wav";
				break;
			default:
				return;
			}
		}
		else if( teamwav == 2 )
		{
			switch( randomgen )
			{
			case 0:
			case 2:
				pszString = "player/gerstartround.wav";
				break;
			case 1:
			case 3:
				pszString = "player/gerstartround2.wav";
				break;
			}
		}

		if( pszString )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, pszString, 1.0f, ATTN_NORM, 0, PITCH_NORM );
	}
}

void EV_TrainPitchAdjust( event_args_t *args )
{
	int idx;
	vec3_t origin;
	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;
	char sz[256];

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	us_params = (unsigned short)args->iparam1;
	stop = args->bparam1;

	m_flVolume = (float)( us_params & 0x003f ) / 40.0f;
	noise = (int)( ( ( us_params ) >> 12 ) & 0x0007 );
	pitch = (int)( 10.0 * (float)( ( us_params >> 6 ) & 0x003f ) );

	switch( noise )
	{
	case 1: strcpy( sz, "plats/ttrain1.wav" ); break;
	case 2: strcpy( sz, "plats/ttrain2.wav" ); break;
	case 3: strcpy( sz, "plats/ttrain3.wav" ); break;
	case 4: strcpy( sz, "plats/ttrain4.wav" ); break;
	case 5: strcpy( sz, "plats/ttrain6.wav" ); break;
	case 6: strcpy( sz, "plats/ttrain7.wav" ); break;
	default:
		// no sound
		return;
	}

	if( stop )
		gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, sz );
	else
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch );
}

int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 )
{
	return iTeam1 == iTeam2;
}

void P_Rubble( event_args_t *args )
{
	int i;
	int ammount;

	ammount = gEngfuncs.pfnRandomLong( 32, 42 );

	for( i = 0; i <= ammount; i++ )
	{
		float vVelocityz = gEngfuncs.pfnRandomFloat( 350.0f, 450.0f );
		float vVelocityx = gEngfuncs.pfnRandomFloat( -500.0f, 500.0f );
		float vVelocityy = gEngfuncs.pfnRandomFloat( -500.0f, 500.0f );

		CreateFlyingRubble( Vector( args->origin ), false, vVelocityx, vVelocityy, vVelocityz, 2.5f, 0, Vector( 0, 0, 0 ) );
	}

	ammount = gEngfuncs.pfnRandomLong( 8, 16 );

	for( i = 0; i <= ammount; i++ )
	{
		float vVelocityz = gEngfuncs.pfnRandomFloat( 200.0f, 350.0f );
		float vVelocityx = gEngfuncs.pfnRandomFloat( -250.0f, 250.0f );
		float vVelocityy = gEngfuncs.pfnRandomFloat( -250.0f, 250.0f );

		CreateFlyingRubble( Vector( args->origin ), true, vVelocityx, vVelocityy, vVelocityz, 4.0f, 0, Vector( 0, 0, 0 ) );
	}
}

void P_ExplosionSmoke( event_args_t *args )
{
	vec3_t vAngles = Vector( 0, 270, 0 );
	vec3_t vforward;
	vec3_t vright;
	vec3_t vup;

	AngleVectors( vAngles, vforward, vright, vup );

	CreateExplosionSmokeInside( Vector( args->origin ) );
}

char EV_FindHitTexture( float *start, float *end )
{
	int ignoreent = -1;
	pmtrace_t *tr = gEngfuncs.PM_TraceLine( start, end, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent );
	char *pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( tr->ent, start, end );

	return EV_TexNameToType( pTextureName );
}

char EV_TexNameToType( char *pTextureName )
{
	char type = CHAR_TEX_CONCRETE;
	char texname[64];
	char szbuffer[64];

	if( pTextureName )
	{
		strcpy( texname, pTextureName );
		pTextureName = texname;

		// strip leading '-0' or '+0~' or '{' or '!'
		if( *pTextureName == '-' || *pTextureName == '+' )
			pTextureName += 2;

		if( *pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ' )
			pTextureName++;

		strcpy( szbuffer, pTextureName );
		szbuffer[CBTEXTURENAMEMAX - 1] = 0;

		type = PM_FindTextureType( szbuffer );
	}

	return type;
}

// the queue is checked by the caller too, it saves the work when the rubble is not wanted
void CreateFlyingRubble( Vector origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, Vector vNormal )
{
	if( g_pParticleMan && !g_RubbleQueue.Full() )
	{
		const char *sprname = "sprites/rock_small.spr";

		if( bLargeRubble )
			sprname = "sprites/rock_big.spr";

		if( iTextureType )
			sprname = "sprites/dirt.spr";

		model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( sprname ) );

		if( pSprite )
		{
			CDoDParticle *pParticle = CDoDParticle::Create( origin, vNormal, pSprite, fSize, 190.0f, "dod_particle", true );

			if( pParticle )
			{
				g_RubbleQueue.Add( gHUD.m_flTime );

				pParticle->m_vVelocity.x = vVelocityx;
				pParticle->m_vVelocity.y = vVelocityy;
				pParticle->m_vVelocity.z = vVelocityz;
				pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
				pParticle->SetCollisionFlags( TRI_COLLIDEWORLD | TRI_COLLIDEALL | TRI_WATERTRACE );
				pParticle->m_iRendermode = kRenderTransTexture;
				pParticle->SetLightFlag( LIGHT_INTENSITY );
				pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
				pParticle->m_flScaleSpeed = 0.0f;
				pParticle->m_flDampingTime = 0.0f;
				pParticle->m_iFrame = 0;
				pParticle->m_flMass = 1.0f;
				pParticle->m_flGravity = 0.5f;
				pParticle->m_iPFlags = PFLAG_DOD_SHRINK_DIE | PFLAG_DOD_WATER_RIPPLE;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 7.0f;
			}
		}
		else
		{
			gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
		}
	}
}

void EV_RocketTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
	if( currenttime >= ent->entity.baseline.fuser2 )
	{
		ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.02f;

		model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" ) );
		vec3_t vOrigin = Vector( ent->entity.origin );
		float flScale = gEngfuncs.pfnRandomFloat( 40.0f, 60.0f );

		vOrigin.x += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
		vOrigin.y += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
		vOrigin.z += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );

		vec3_t vVelocity = Vector( ent->entity.baseline.vuser1[0], ent->entity.baseline.vuser1[1], ent->entity.baseline.vuser1[2] );

		if( ent->entity.baseline.iuser4 )
		{
			ent->entity.baseline.iuser4--;
		}
		else
		{
			if( gEngfuncs.pfnRandomLong( 0, 3 ) <= 1 )
			{
				ent->entity.baseline.vuser1[0] = 0.0f;
				ent->entity.baseline.vuser1[1] = 0.0f;
				ent->entity.baseline.vuser1[2] = 0.0f;
			}
			else
			{
				int z = (int)gEngfuncs.pfnRandomFloat( -25.0f, 25.0f );
				int y = (int)gEngfuncs.pfnRandomFloat( -25.0f, 25.0f );
				int x = gEngfuncs.pfnRandomLong( -25, 25 );

				ent->entity.baseline.vuser1[0] = (float)x;
				ent->entity.baseline.vuser1[1] = (float)y;
				ent->entity.baseline.vuser1[2] = (float)z;
			}

			ent->entity.baseline.iuser4 = gEngfuncs.pfnRandomLong( 5, 15 );
		}

		if( ent->entity.baseline.iuser4 < 0 )
			ent->entity.baseline.iuser4 = 0;

		if( pSprite )
		{
			CDoDRocketTrail *pSmoke = CDoDRocketTrail::Create( vOrigin, Vector( 0, 0, 1 ), pSprite, flScale, 200.0f, "dod_trailsmoke" );

			if( !pSmoke )
				return;

			pSmoke->m_flGravity = -0.01f;
			pSmoke->m_iRendermode = kRenderTransAlpha;
			pSmoke->SetCollisionFlags( TRI_COLLIDEWORLD );
			pSmoke->m_flSize = flScale;
			pSmoke->SetLightFlag( LIGHT_NONE );
			pSmoke->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pSmoke->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.5f, 1.5f ) );
			pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
			pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
			pSmoke->m_iFrame = (int)gEngfuncs.pfnRandomFloat( 10.0f, 15.0f );
			pSmoke->m_flScaleSpeed = 2.0f;
			pSmoke->m_vColor = Vector( 25.0f, 25.0f, 25.0f );
			pSmoke->m_bRocketTrail = true;
			pSmoke->m_flFadeSpeed = -1.0f;
			pSmoke->m_vVelocity = vVelocity;
		}
	}

	if( ent->entity.baseline.fuser1 > currenttime )
		return;

	if( ent->entity.origin == ent->entity.attachment[0] )
		ent->die = gEngfuncs.GetClientTime();
	else
		ent->entity.attachment[0] = ent->entity.origin;

	if( cl_dynamiclights->value > 0.0f )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		VectorCopy( ent->entity.origin, dl->origin );
		dl->radius = 160.0f;
		dl->dark = 1;
		dl->die = gEngfuncs.GetClientTime() + 0.001f;
		dl->color.r = 255;
		dl->color.g = 255;
		dl->color.b = 255;
	}
}

void EV_RocketTrail( event_args_t *args )
{
	int iEntIndex = args->entindex;
	TEMPENTITY *pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

	if( pTrailSpawner )
	{
		pTrailSpawner->flags |= FTENT_PERSIST | FTENT_COLLIDEKILL | FTENT_PLYRATTACHMENT | FTENT_NOMODEL | FTENT_CLIENTCUSTOM;
		pTrailSpawner->callback = EV_RocketTrailCallback;
		pTrailSpawner->clientIndex = iEntIndex;
		pTrailSpawner->die = gEngfuncs.GetClientTime() + 10.0f;
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.5f;
		pTrailSpawner->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
	}
}

void EV_MortarShellCallback( tempent_s *ent, float frametime, float currenttime )
{
	vec3_t temp;

	VectorScale( ent->entity.baseline.origin, -1.0f, temp );

	if( ent->entity.baseline.iuser1 <= 0 && ent->entity.baseline.origin[2] < 0.0f )
	{
		gEngfuncs.pEventAPI->EV_PlaySound( 0, ent->entity.origin, CHAN_VOICE, "weapons/mortar_incoming.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, gEngfuncs.pfnRandomLong( 0, 10 ) + 95 );

		ent->entity.baseline.iuser1 = 1;
	}

	VectorAngles( temp, ent->entity.angles );
}

void EV_MortarShell( event_args_t *args )
{
	int modelindex;
	TEMPENTITY *pTemp;

	pTemp = gEngfuncs.pEfxAPI->CL_TempEntAlloc( args->origin, gEngfuncs.CL_LoadModel( "models/w_piat_rocket.mdl", &modelindex ) );

	if( pTemp )
	{
		pTemp->flags = FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_SCALE | FTENT_SCALEDGRAVITY;
		pTemp->callback = EV_MortarShellCallback;
		pTemp->die = gEngfuncs.GetClientTime() + 15.0f;

		VectorScale( args->angles, 1000.0f, pTemp->entity.baseline.origin );

		pTemp->entity.baseline.iuser1 = 0;
		pTemp->entity.baseline.gravity = args->fparam1;
	}
}

void EV_OverheatCallback( tempent_s *ent, float frametime, float currenttime )
{
	if( currenttime < ent->entity.baseline.fuser2 )
		return;

	ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.2f;

	model_s *pSprite = (model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( "sprites/puff.spr" ) );
	vec3_t A;
	vec3_t B;
	vec3_t vBarrelOffset;
	vec3_t vOrigin;
	vec3_t vVelocity;

	if( EV_IsLocal( ent->clientIndex ) )
	{
		if( g_flWeaponHeat < 94.0f )
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}

		if( Length( gHUD.m_vecVelocity ) > 20.0f )
			return;

		cl_entity_t *viewent = GetViewEntity();

		if( !viewent )
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}

		A = viewent->attachment[0];
		B = viewent->attachment[1];
	}
	else
	{
		cl_entity_t *playerEnt = gEngfuncs.GetEntityByIndex( ent->clientIndex );

		if( !playerEnt || !playerEnt->player )
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}

		vec3_t forward, right, up;

		AngleVectors( playerEnt->curstate.angles, forward, right, up );

		if( ( unsigned int )( playerEnt->curstate.gaitsequence - 15 ) <= 1 )
		{
			// prone
			VectorMA( playerEnt->origin, 15.0f, forward, A );
			VectorMA( playerEnt->origin, 60.0f, forward, B );
			VectorMA( A, -6.0f, up, A );
			VectorMA( B, -6.0f, up, B );
		}
		else
		{
			VectorMA( playerEnt->origin, 5.0f, forward, A );
			VectorMA( playerEnt->origin, 30.0f, forward, B );
			VectorMA( A, 13.0f, up, A );
			VectorMA( B, 13.0f, up, B );
		}
	}

	vBarrelOffset = A - B;

	float scale = gEngfuncs.pfnRandomFloat( 0.2f, 0.8f );

	VectorMA( B, scale, vBarrelOffset, vOrigin );

	float flScale = gEngfuncs.pfnRandomFloat( 3.0f, 3.0f );

	vVelocity.x = gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
	vVelocity.y = gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
	vVelocity.z = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );

	vOrigin.z += gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );

	if( pSprite )
	{
		CDoDParticle *pParticle = CDoDParticle::Create( vOrigin, Vector( 90, 0, 0 ), pSprite, flScale, 180.0f, "dod_particle", false );

		if( pParticle )
		{
			pParticle->m_iRendermode = kRenderTransAlpha;
			pParticle->SetLightFlag( LIGHT_NONE );
			pParticle->SetCullFlag( CULL_PVS | CULL_FRUSTUM_SPHERE );
			pParticle->m_vColor = Vector( 255.0f, 255.0f, 255.0f );
			pParticle->m_flGravity = -0.015f;
			pParticle->m_flFadeSpeed = 2.0f;
			pParticle->m_flScaleSpeed = 1.0f;
			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
			pParticle->m_vAVelocity = Vector( 0.0f, 0.0f, gEngfuncs.pfnRandomFloat( -1.5f, 1.5f ) );
			pParticle->m_vVelocity = vVelocity;
		}
	}
}

float lastOverheatTime;

extern cvar_t *cl_particlefx;

void EV_Overheat( event_args_t *args )
{
	gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_ITEM, "weapons/mgoverheat.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	float lastTime = gEngfuncs.GetClientTime() - lastOverheatTime;

	lastOverheatTime = gEngfuncs.GetClientTime();

	if( ( lastTime < 0.0f || lastTime >= 1.0f ) && cl_particlefx->value >= 2.0f )
	{
		TEMPENTITY *pBarrelSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

		if( pBarrelSpawner )
		{
			pBarrelSpawner->flags |= FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_NOMODEL | FTENT_CLIENTCUSTOM;
			pBarrelSpawner->callback = EV_OverheatCallback;
			pBarrelSpawner->clientIndex = args->entindex;
			pBarrelSpawner->die = gEngfuncs.GetClientTime() + 2.5f;
			pBarrelSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.5f;
			pBarrelSpawner->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
		}
	}
}
