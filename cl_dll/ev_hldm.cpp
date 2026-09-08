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
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include "dod_shared.h"

extern "C"
{
	#include "pm_shared.h"
}

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

#include "voice_status.h"

#ifdef USE_PMAN
#include "particleman.h"

extern IParticleMan *g_pParticleman;
#endif

extern engine_studio_api_t IEngineStudio;
static float fl_timeMusicLeft;
static int g_tracerCount[32];
float total_time;
extern char *sHelmetModels[];
extern cvar_t *cl_dynamiclights;
cvar_t *cl_numshotrubble;
extern float g_flWeaponHeat;

enum GUNTYPE_e
{
	PISTOL = 1,
	RIFLE,
	SUBMACHINE,
	MG
};

void V_PunchAxis( int axis, float punch );
void VectorAngles( const float *forward, float *angles );

extern cvar_t *cl_lw;

extern "C"
{
	void EV_TrainPitchAdjust( struct event_args_s *args );
	void EV_FireColt( struct event_args_s *args );
	void EV_FireLuger( struct event_args_s *args );
	void EV_FireM1Carbine( struct event_args_s *args );
	void EV_FireGarand( struct event_args_s *args );
	void EV_FireScopedKar( struct event_args_s *args );
	void EV_FireThompson( struct event_args_s *args );
	void EV_FireMP44( struct event_args_s *args );
	void EV_FireSpring( struct event_args_s *args );
	void EV_FireKar( struct event_args_s *args );
	void EV_FireMP40( struct event_args_s *args );
	void EV_Knife( struct event_args_s *args );
	void EV_FireBAR( struct event_args_s *args );
	void EV_FireMG42( struct event_args_s *args );
	void EV_FireMG34( struct event_args_s *args );
	void EV_Fire30CAL( struct event_args_s *args );
	void EV_FireGreaseGun( struct event_args_s *args );
	void EV_FireFG42( struct event_args_s *args );
	void EV_FireK43( struct event_args_s *args );
	void EV_FireEnfield( struct event_args_s *args );
	void EV_FireSten( struct event_args_s *args );
	void EV_FireBren( struct event_args_s *args );
	void EV_FireWebley( struct event_args_s *args );
	void EV_FireScopedEnfield( struct event_args_s *args );
	void EV_FireBazooka( struct event_args_s *args );
	void EV_FirePschreck( struct event_args_s *args );
	void EV_FirePIAT( struct event_args_s *args );
	void EV_FireMortar( struct event_args_s *args );
	void EV_FireMelee( struct event_args_s *args );
	void EV_Pain( struct event_args_s *args );
	void EV_Smoke( struct event_args_s *args );
	void EV_BloodSprite( struct event_args_s *args );
	void EV_BloodStream( struct event_args_s *args );
	void EV_BulletTracers( struct event_args_s *args );
	void EV_BubbleTrails( struct event_args_s *args );
	void EV_Bubbles( struct event_args_s *args );
	void EV_Explosion( struct event_args_s *args );
	void EV_SparkShower( struct event_args_s *args );
	void EV_PlayWhizz( struct event_args_s *args );
	void EV_USVoice( struct event_args_s *args );
	void EV_GERVoice( struct event_args_s *args );
	void EV_BodyDamage( struct event_args_s *args );
	void EV_RoundReleaseSound( struct event_args_s *args );
	void EV_DoDCamera( struct event_args_s *args );
	void EV_PopHelmet( struct event_args_s *args );
	void EV_RoundReset( struct event_args_s *args );
	void EV_Overheat( struct event_args_s *args );
	void EV_RocketTrail( struct event_args_s *args );
	void EV_MortarShell( struct event_args_s *args );
}

#define VECTOR_CONE_1DEGREES Vector( 0.00873f, 0.00873f, 0.00873f )
#define VECTOR_CONE_2DEGREES Vector( 0.01745f, 0.01745f, 0.01745f )
#define VECTOR_CONE_3DEGREES Vector( 0.02618f, 0.02618f, 0.02618f )
#define VECTOR_CONE_4DEGREES Vector( 0.03490f, 0.03490f, 0.03490f )
#define VECTOR_CONE_5DEGREES Vector( 0.04362f, 0.04362f, 0.04362f )
#define VECTOR_CONE_6DEGREES Vector( 0.05234f, 0.05234f, 0.05234f )
#define VECTOR_CONE_7DEGREES Vector( 0.06105f, 0.06105f, 0.06105f )	
#define VECTOR_CONE_8DEGREES Vector( 0.06976f, 0.06976f, 0.06976f )
#define VECTOR_CONE_9DEGREES Vector( 0.07846f, 0.07846f, 0.07846f )
#define VECTOR_CONE_10DEGREES Vector( 0.08716f, 0.08716f, 0.08716f )
#define VECTOR_CONE_15DEGREES Vector( 0.13053f, 0.13053f, 0.13053f )
#define VECTOR_CONE_20DEGREES Vector( 0.17365f, 0.17365f, 0.17365f )

int EV_GetWeaponBody( void )
{
	int team = g_iTeamNumber;
	cl_entity_t *target = gEngfuncs.GetEntityByIndex( g_iUser2 );

	if( g_iUser1 == OBS_IN_EYE )
		team = target->curstate.team;

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
float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	const char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	cl_entity_t *ent;
	char *pTextureName;
	char texname[64];
	char szbuffer[64];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	// FIXME check if playtexture sounds movevar is set
	//
	chTextureType = 0;

	// Player
	if( ( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	    || ( ( ent = gEngfuncs.GetEntityByIndex( entity )) && ( ent->curstate.eflags & EFLAG_FLESH_SOUND )))
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );

		if ( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if( *pTextureName == '-' || *pTextureName == '+' )
			{
				pTextureName += 2;
			}

			if( *pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ' )
			{
				pTextureName++;
			}

			// '}}'
			strcpy( szbuffer, pTextureName );
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );
		}
	}
	
	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE:
		fvol = 0.9;
		fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL:
		fvol = 0.9;
		fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:
		fvol = 0.9;
		fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:
		fvol = 0.5;
		fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE:
		fvol = 0.9;
		fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:
		fvol = 0.8;
		fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH:
		fvol = 0.9;
		fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD:
		fvol = 0.9;
		fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8;
		fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		fvol = 1.0;
		fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong( 0, cnt - 1 )], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
	return fvolbar;
}

char *EV_HLDM_DamageDecal( physent_t *pe )
{
	static char decalname[32];
	int idx;

	if( pe->classnumber == 1 )
	{
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else if( pe->rendermode != kRenderNormal )
	{
		strcpy( decalname, "{bproof1" );
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong( 0, 4 );
		sprintf( decalname, "{shot%i", idx + 1 );
	}
	return decalname;
}

static cvar_t *r_decals;

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, float *vecSrc, float *vecEnd, int iBulletType )
{
	physent_t *pe;
	char chTextureType = 'F';
	int idx;
	static char decalname[32];

	float flHeight = EV_HLDM_WaterHeight( vecSrc, 0.0f, 0.0f ) - vecSrc[2];

	if( flHeight < 0.8f )
	{
		if( EV_HLDM_WaterHeight( pTrace->endpos, 0.0f, 0.0f ) - pTrace->endpos[2] < 8.0f )
			flHeight = 0.0f;
		else
			flHeight = EV_HLDM_WaterHeight( pTrace->endpos, 0.0f, 0.0f ) - pTrace->endpos[2] 
				+ pTrace->endpos[2] - vecSrc[2];
	}

	int entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace );
	char *pTextureName;
	pmtrace_t tr2;
	char texname[64], szbuffer[64];
	vec3_t location;

	if( entity > gEngfuncs.GetMaxClients() )
	{
		chTextureType = 'K';
		pTextureName = ( char * ) gEngfuncs.pEventAPI->EV_TraceTexture( tr2.ent, pTrace->endpos, vecEnd );

		if( pTextureName )
		{
			strcpy( texname, pTextureName );

			char *pCleanName = texname;

			if( texname[0] == '+' || texname[0] == '-' )
			{
				pCleanName = &texname[2];
			}

			if( pCleanName[0] == '!' || pCleanName[0] == '{' || pCleanName[0] == '~' || pCleanName[0] == ' ' )
			{
				pCleanName++;
			}

			strcpy( szbuffer, pCleanName );
			szbuffer[12] = '\0';

			chTextureType = PM_FindTextureType( szbuffer );

			if( flHeight != 0.0f )
			{
				chTextureType = 'S';
				if( EV_HLDM_WaterEntryPoint( pTrace, vecSrc, location ) )
				{
					VectorCopy( location, pTrace->endpos );
				}
			}
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if( pe && ( pe->classnumber == 1 || chTextureType == 'Y' ) )
	{
		chTextureType = 'Y';
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else
	{
		switch( chTextureType )
		{
		case 'A': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'K': case 'L': case 'M': case 'N':
		case 'P': case 'R': case 'S':
			decalname[0] = '\0';
			break;
		case 'B':
			chTextureType = 'B';
			idx = gEngfuncs.pfnRandomLong( 1, 2 );
			sprintf( decalname, "{cement%i", idx );
			break;
		case 'C':
			chTextureType = 'C';
			idx = gEngfuncs.pfnRandomLong( 1, 2 );
			sprintf( decalname, "{cement%i", idx );
			break;
		case 'T':
			chTextureType = 'T';
			idx = gEngfuncs.pfnRandomLong( 1, 4 );
			sprintf( decalname, "{crack%i", idx );
			break;
		case 'W':
			chTextureType = 'W';
			idx = gEngfuncs.pfnRandomLong( 1, 3 );
			sprintf( decalname, "{wood%i", idx );
			break;
		case 'Z':
			chTextureType = 'Z';
			idx = gEngfuncs.pfnRandomLong( 1, 3 );
			sprintf( decalname, "{wood%i", idx );
			break;
		default:
			idx = gEngfuncs.pfnRandomLong( 1, 3 );
			sprintf( decalname, "{generic%i", idx );
			break;
		}
	}

	EV_PlaySurfaceHitSound( pTrace, iBulletType, chTextureType );

	if( pe && ( pe->solid == 4 || pe->movetype == 13 ) )
	{
		if( !r_decals )
		{
			r_decals = gEngfuncs.pfnGetCvarPointer( "r_decals" );
		}

		if( decalname[0] != '\0' && r_decals && r_decals->value != 0.0f )
		{
			int iHitEntity = gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace );
			int iDecalIndex = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalname );
			int iFinalDecal = gEngfuncs.pEfxAPI->Draw_DecalIndex( iDecalIndex );

			gEngfuncs.pEfxAPI->R_DecalShoot( iFinalDecal, iHitEntity, 0, pTrace->endpos, 0 );
		}

#ifdef USE_PMAN
		EV_HLDM_DoDSurfaceFX( pTrace, iBulletType, chTextureType );
#endif
	}
}

TEMPENTITY *g_DeadPlayerModels[64];

void EV_RoundReset( event_args_t *args )
{
	if( r_decals || gEngfuncs.pfnGetCvarPointer( "r_decals" ) != 0 )
	{
		int maxdecals = r_decals->value;

		if( maxdecals > 0 )
		{
			for( int i = 0; i != maxdecals; i++ )
				gEngfuncs.pEfxAPI->R_DecalRemoveAll( i );
		}
	}

	if( g_DeadPlayerModels[NULL] )
		for( int j = 0; j != 64; j++ )
		{
			if( g_DeadPlayerModels[j] )
			{
				g_DeadPlayerModels[j]->die = 0.0f;
				g_DeadPlayerModels[j] = NULL;
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
		ent->flags |= FTENT_PLYRATTACHMENT;
}

void CreateCorpse( vec3_t vOrigin, vec3_t vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody )
{
	float time[6];

	gEngfuncs.pEventAPI->EV_FindModelIndex( pModel );

	memset( time, 0, 12 );

	TEMPENTITY *pBody;

	pBody = gEngfuncs.pEfxAPI->R_TempModel( &vOrigin.x, time, &vAngles.x, 100.0f, gEngfuncs.pEventAPI->EV_FindModelIndex( pModel ), TE_BOUNCE_NULL );

	if( pBody )
	{
		pBody->flags |= FTENT_CLIENTCUSTOM;
		pBody->entity.curstate.framerate = 1.0f;
		pBody->frameMax = 255.0f;
		pBody->entity.curstate.animtime = flAnimTime;
		pBody->entity.curstate.frame = 0.0f;
		pBody->entity.curstate.renderamt = 255;
		pBody->entity.curstate.sequence = iSequence;
		pBody->entity.curstate.fuser1 = gHUD.m_flTime + 1.0f;
		pBody->entity.curstate.body = iBody;
		pBody->entity.curstate.fuser2 = gHUD.m_flTime + 1.0f;
		pBody->callback = RemoveBody;
		pBody->hitcallback = HitBody;
		pBody->bounceFactor = 0.0f;
		pBody->die = gHUD.m_flTime + 1.0f + gHUD.cl_corpsestay->value + 5.0f;

		char j = '\0';

		for( int i = 0; i != 64; i++ )
		{
			if( !g_DeadPlayerModels[i] || pBody == g_DeadPlayerModels[i] )
			{
				g_DeadPlayerModels[i] = pBody;
				return;
			}

			if( gEngfuncs.GetClientTime() > g_DeadPlayerModels[i]->die )
			{
				g_DeadPlayerModels[i]->die = 0.0f;
				j = '\x01';
				g_DeadPlayerModels[i] = pBody;
			}
		}

		if( !j )
			pBody->die = pBody->entity.curstate.fuser2 + 5.0f;
	}
}

#ifdef USE_PMAN
void EV_BasicPuff( pmtrace_t *pTrace, float scale )
{
	vec3_t origin, vColor, vVelocity;

	vColor = Vector( 175.0f, 175.0f, 175.0f );
	vVelocity = pTrace->plane.normal;
	origin = gEngfuncs.pfnRandomLong( 1, 3 ) * pTrace->plane.normal + pTrace->endpos;

	CreateDebrisWallPuff( origin, vVelocity, vColor, 0 );
	CreateDebrisWallPuff( origin, vVelocity, vColor, 1 );
	CreateDebrisWallPuff( origin, vVelocity, vColor, 2 );
	CreateDebrisWallPuff( origin, vVelocity, vColor, 3 );
}

void EV_CreteRubble( pmtrace_t *pTrace, float fScale )
{

}

void CreateSpark( struct Vector origin, struct Vector vNormal, const char *szSpriteName )
{

}

void EV_MetalHit( pmtrace_t *pTrace )
{

}

void EV_DirtHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_SandHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_GrassHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_WoodChips( pmtrace_t *pTrace, float fScale )
{

}

void EV_GlassShards( pmtrace_t *pTrace, float fScale )
{

}

void EV_WaterHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_LeavesHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_SnowHit( pmtrace_t *pTrace, float fScale )
{

}

void EV_TileHit( pmtrace_t *pTrace, float fScale )
{

}
#endif // USE_PMAN

void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	char hitsound;

	switch( cSurfaceType )
	{
	default:
		sprintf( &hitsound, "weapons/ric%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'A':
	case 'D':
		sprintf( &hitsound, "weapons/hit_sand%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'E':
		sprintf( &hitsound, "weapons/hit_leaves%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'G':
	case 'M':
		sprintf( &hitsound, "weapons/hit_metal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'H':
		sprintf( &hitsound, "weapons/hit_heavymetal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'K':
		return;
	case 'N':
		sprintf( &hitsound, "weapons/hit_snow%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'P':
		sprintf( &hitsound, "weapons/hit_grass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'S':
		sprintf( &hitsound, "weapons/hit_water%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'W':
		sprintf( &hitsound, "weapons/hit_wood%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	case 'Y':
		sprintf( &hitsound, "weapons/hit_glass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.80000001, 0, 100 );
		return;
	}
}

#ifdef USE_PMAN
void EV_HLDM_DoDSurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	float fScale = 1.5f;

	if( ( iBulletType - 6 ) <= 20 )
		fScale = iBulletType - 6;

	switch( cSurfaceType )
	{
	default:
		EV_BasicPuff( pTrace, fScale );
		return;
	case 'A':
		EV_SandHit( pTrace, fScale );
		return;
	case 'B':
	case 'C':
	case 'R':
		EV_CreteRubble( pTrace, fScale );
		EV_BasicPuff( pTrace, fScale );
		return;
	case 'D':
		EV_DirtHit( pTrace, fScale );
		return;
	case 'E':
		EV_LeavesHit( pTrace, fScale );
		return;
	case 'G':
	case 'H':
	case 'M':
		EV_MetalHit( pTrace );
		return;
	case 'K':
		return;
	case 'L':
		EV_CreteRubble( pTrace, fScale );
		EV_CreteRubble( pTrace, fScale );
		EV_BasicPuff( pTrace, fScale );
		return;
	case 'N':
		EV_SnowHit( pTrace, fScale );
		return;
	case 'P':
		EV_GrassHit( pTrace, fScale );
		return;
	case 'S':
		EV_WaterHit( pTrace, fScale );
		return;
	case 'T':
		EV_TileHit( pTrace, fScale );
		return;
	case 'W':
		EV_WoodChips( pTrace, fScale );
		return;
	case 'Y':
		EV_GlassShards( pTrace, fScale );
		return;
	}
}
#endif // USE_PMAN

void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd )
{
	physent_t *pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if( pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{
		EV_HLDM_GunshotDecalTrace( pTrace, "", vecSrc, vecEnd, iBulletType );
	}
}

int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount, float *tracerOrigin )
{
	int tracer = 0;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;

	if( iTracerFreq != 0 && ( (*tracerCount)++ % iTracerFreq ) == 0 )
	{
		vec3_t vecTracerSrc;

		if( player )
		{
			vec3_t offset( 0, 0, -4 );

			// adjust tracer position for player
			for( i = 0; i < 3; i++ )
			{
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
			}
		}
		else
		{
			VectorCopy( vecSrc, vecTracerSrc );
		}

		if( iTracerFreq != 1 )		// guns that always trace also always decal
			tracer = 1;

		EV_CreateTracer( vecTracerSrc, end );
	}

	return tracer;
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, 
	float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, 
	int iTracerFreq, int *tracerCount )
{
	int i;
	pmtrace_t tr;
	int iShot;
	vec3_t tracerOrigin;
	vec3_t vecDir, vecEnd;

	cl_entity_t *thisplayer = gEngfuncs.GetLocalPlayer();
	cl_entity_t *firingPlayer = gEngfuncs.GetEntityByIndex( idx );

	if( cShots <= 0 )
		return;

	for( iShot = 1; iShot <= cShots; iShot++ )
	{
		vec3_t vecDir, vecEnd;

		for( i = 0; i < 3; i++ )
		{
			vecEnd[i] = ( right[i] * vecSpread[0] + vecDirShooting[i] + up[i] * vecSpread[1] ) * flDistance + vecSrc[i];
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
		gEngfuncs.pEventAPI->EV_PushPMStates();
		gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );
		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, 2, -1, &tr );

		tracerOrigin = { 0.0f, 0.0f, 0.0f };

		if( firingPlayer->curstate.gaitsequence - 15 > 1 )
		{
			for( i = 0; i < 2; i++ )
			{
				tracerOrigin[i] = vecSrc[i] + right[i] + right[i] + forward[i] * 16.0f;
			}

			tracerOrigin[2] = vecSrc[2] - 4.0f + right[2] + right[2] + forward[2] * 16.0f;
		}
		else
		{
			for( i = 0; i < 2; i++ )
			{
				tracerOrigin[i] = vecSrc[i] + right[i] + right[i] + forward[i] * 16.0f 
					+ forward[i] * 34.0f + right[i] * 10.0f;
			}

			tracerOrigin[2] = vecSrc[2] - 16.0f + right[2] + right[2] + forward[2] * 16.0f 
				+ forward[2] * 34.0f + right[2] * 10.0f;
		}

		EV_HLDM_CheckTracer( idx, vecSrc, vecEnd, forward, right, iBulletType, iTracerFreq, tracerCount, tracerOrigin );

		pmtrace_t *tr2;
		vec3_t vecTarget, vecBullet;
		vec3_t vecCross;
		float projectionLen;

		if( firingPlayer != thisplayer )
		{
			for( i = 0; i < 3; i++ )
			{
				vecBullet[i] = tr.endpos[i] - vecSrc[i];
			}

			vecTarget = firingPlayer->curstate.origin - thisplayer->curstate.origin;

			gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, thisplayer->curstate.origin, PM_WORLD_ONLY, -1, &tr );

			if( tr2->fraction > 0.9f )
			{
				vecCross = CrossProduct( vecBullet, vecTarget );
				projectionLen = vecCross.Length();

				if( projectionLen != 0.0f )
				{
					float fl = DotProduct( vecBullet, vecTarget ) / ( vecBullet.Length() * vecTarget.Length() );

					if( fl > 1.0f )
						fl = 1.0f;
					else if( fl < -1.0f )
						fl = -1.0f;

					float fTheta = acosf( fl );
					float flDist = sinf( fTheta ) * vecTarget.Length();

					if( flDist < 100.0f )
					{
						if( vecBullet.Length() > vecTarget.Length() * cosf( fTheta ) )
						{
							char buf[32];

							sprintf( buf, "weapons/whizz%d.wav", gEngfuncs.pfnRandomLong( 1, 17 ) );
							gEngfuncs.pfnPlaySoundByName( buf, gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ) );
						}
					}
				}
			}
		}

		if( tr.fraction != 1.0f )
		{
			EV_HLDM_PlayTextureSound( idx, &tr, vecSrc, &vecEnd.x, iBulletType );

			physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );

			if( pe && pe->solid == SOLID_BBOX )
				EV_HLDM_GunshotDecalTrace( &tr, vecSrc, vecEnd, iBulletType );
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

float EV_HLDM_WaterHeight( vec3_t position, float minz, float maxz )
{
	vec3_t midUp;
	float diff;

	midUp[0] = position.x;
	midUp[1] = position.y;
	midUp[2] = minz;

	if( gEngfuncs.PM_PointContents( midUp, 0 ) == CONTENTS_WATER )
	{
		midUp[2] = maxz;

		if( gEngfuncs.PM_PointContents( midUp, 0 ) == CONTENTS_WATER )
		{
			return maxz;
		}
		else
		{
			diff = minz;
			float flRange = maxz - minz;

			if( flRange > 1.0f )
			{
				while( 1 )
				{
					midUp[2] = flRange * 0.5f + diff;

					if( gEngfuncs.PM_PointContents( midUp, 0 ) == CONTENTS_WATER )
						diff = midUp[2];
					else
						maxz = midUp[2];

					float flNextRange = maxz - diff;

					if( flNextRange <= 1.0f )
						break;

					flRange = flNextRange;
				}
			}
			else
			{
				return midUp[2];
			}
		}
	}

	return midUp[2];
}

int EV_HLDM_WaterEntryPoint( pmtrace_t *pTrace, float *vecSrc, float *vecResult )
{
	return 0;
}

void EV_HLDM_BubbleTrails( vec3_t from, vec3_t to, int count )
{

}

//======================
//	    MELEE START
//======================
void EV_FireMelee( event_args_s *args )
{
	int pitch, empty, empty2, idx;

	vec3_t origin;

	pitch = args->iparam1;
	empty = args->bparam1;
	empty2 = args->bparam2;
	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( pitch, EV_GetWeaponBody() );
	}

	if( empty )
	{
		gEngfuncs.pfnRandomLong( 0, 1 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( empty2 )
	{
		gEngfuncs.pfnRandomLong( 0, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_hit1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_ResetAnimationEvents( idx );
}
//======================
//	   MELEE END
//======================

//======================
//	    COLT START
//======================
void EV_FireColt( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_COLT, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	   COLT END
//======================

//======================
//	    LUGER START
//======================
void EV_FireLuger( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( LUGER_SHOOT_EMPTY - ( empty == 0 ), EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_LUGER );
	}
	EV_MuzzleFlash( idx, PISTOL );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/luger_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_LUGER, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    LUGER END
//======================

//======================
//	    MG42 START
//======================
void EV_FireMG42( event_args_s *args )
{
	int idx, pitch, empty;
	int shell;
	bool bCallAnim;

	int MG42_DownAnims[9] = { MG42_DOWNSHOOT, MG42_DOWNSHOOT8, MG42_DOWNSHOOT7, MG42_DOWNSHOOT6, MG42_DOWNSHOOT5, MG42_DOWNSHOOT4, MG42_DOWNSHOOT3,
	MG42_DOWNSHOOT2, MG42_DOWNSHOOT1 };

	int MG42_UpAnims[9] = { MG42_UPSHOOT, MG42_UPSHOOT8, MG42_UPSHOOT7, MG42_UPSHOOT6, MG42_UPSHOOT5, MG42_UPSHOOT4, MG42_UPSHOOT3,
	MG42_UPSHOOT2, MG42_UPSHOOT1 };

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;
	empty = args->bparam1;

	bCallAnim = empty == 0;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shell_mg42.dll" );

	if( !EV_IsLocal( idx ) )
	{
		if( !bCallAnim )
		{
			EV_GetGunPosition( args, vecSrc, origin );

			VectorCopy( forward, vecAiming );

			EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MG42, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

			EV_ResetAnimationEvents( idx );
		}
		EV_MuzzleFlash( idx, MG );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( bCallAnim )
	{
		if( pitch <= MG42_UPIDLE1 )
			pitch = MG42_UPIDLE1;

		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_DownAnims[pitch], EV_GetWeaponBody() );
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_UpAnims[pitch], EV_GetWeaponBody() );
		}
		gHUD.DoRecoil( WEAPON_MG42 );

		EV_MuzzleFlash( idx, MG );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MG42, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    MG42 END
//======================

//======================
//	    MG34 START
//======================
void EV_FireMG34( event_args_s *args )
{
	int idx, pitch;
	int iAnim;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
			iAnim = MG34_DOWNSHOOTEMPTY - ( pitch <= 0 == 0 );
		else
			iAnim = MG34_UPSHOOTEMPTY - ( pitch <= 0 == 0 );

		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_MG34 );
	}
	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg34_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MG34, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    MG34 END
//======================

//======================
//	    30CAL START
//======================
void EV_Fire30CAL( event_args_s *args )
{
	int idx, pitch;

	int ThirtyCal_DownAnims[9] = { CAL30_DOWNSHOOT, CAL30_DOWNSHOOT8, CAL30_DOWNSHOOT7, CAL30_DOWNSHOOT6, CAL30_DOWNSHOOT5, CAL30_DOWNSHOOT4, CAL30_DOWNSHOOT3,
	CAL30_DOWNSHOOT2, CAL30_DOWNSHOOT1 };

	int ThirtyCal_UpAnims[9] = { CAL30_UPSHOOT, CAL30_UPSHOOT8, CAL30_UPSHOOT7, CAL30_UPSHOOT6, CAL30_UPSHOOT5, CAL30_UPSHOOT4, CAL30_UPSHOOT3,
	CAL30_UPSHOOT2, CAL30_UPSHOOT1 };

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( pitch <= CAL30_UPIDLE1 )
			pitch = CAL30_UPIDLE1;

		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_DownAnims[pitch], EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_UpAnims[pitch], EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_CAL30 );
	}
	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/30cal_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_30CAL, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    30CAL END
//======================

//======================
//	   GARAND START
//======================
void EV_FireGarand( event_args_s *args )
{
	int idx, empty;
	int iAnim;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( args->iparam1 )
	{
		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( GARAND_SMASH_EMPTY - ( empty == 0 ), EV_GetWeaponBody() );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			iAnim = GARAND_SHOOT_EMPTY;

			if( empty )
			{
				iAnim = gEngfuncs.pfnRandomLong( 0, 2 ) + GARAND_SHOOT1;
			}

			gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );

			gHUD.DoRecoil( WEAPON_GARAND );
		}
		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/garand_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !empty )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/garand_reload_clipding.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_GARAND, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );
	}

	EV_ResetAnimationEvents( idx );
}
//======================
//	   GARAND END
//======================

//======================
//	 M1CARBINE START
//======================
void EV_FireM1Carbine( struct event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M1CARBINE, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	 M1CARBINE END
//======================

//======================
//	 SCOPEDKAR START
//======================
void EV_FireScopedKar( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SCOPEDKAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	 SCOPEDKAR END
//======================

//======================
//	 THOMPSON START
//======================
void EV_FireThompson( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_THOMPSON, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	  THOMPSON END
//======================

//======================
//	   MP44 START
//======================
void EV_FireMP44( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP44_SHOOT1, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_MP44 );
	}
	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp44_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP44, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    MP44 END
//======================

//======================
//	  GREASEGUN START
//======================
void EV_FireGreaseGun( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_GREASEGUN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	  GREASEGUN END
//======================

//======================
//	    FG42 START
//======================
void EV_FireFG42( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_DOWN_SHOOT, EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( FG42_UP_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_FG42 );
	}
	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/fg42_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_FG42, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    FG42 END
//======================

//======================
//	    K43 START
//======================
void EV_FireK43( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( pitch == 1 )
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( K43_SMASH, EV_GetWeaponBody() );

			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
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

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_K43, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

		EV_ResetAnimationEvents( idx );
	}
}
//======================
//	     K43 END
//======================

//======================
//	   ENFIELD START
//======================
void EV_FireEnfield( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( pitch == 1 )
	{
		if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 1.0f )
		{
			if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) <= 1.0f )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
					return;
			}
			else if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 2.0f )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 3.0f )
					return;

				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

				if( !EV_IsLocal( idx ) )
					return;

				gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SWING, EV_GetWeaponBody() );
				return;
			}
		}
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !EV_IsLocal( idx ) )
			return;
	}
	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_ENFIELD );
	}
	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfield_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	   ENFIELD END
//======================

//======================
//	   STEN START
//======================

void EV_FireSten( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + GREASEGUN_SHOOT1, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_STEN );
	}
	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sten_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_STEN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    STEN END
//======================

//======================
//	   BREN START
//======================
void EV_FireBren( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_DOWN_SHOOT, EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BREN_UP_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_BREN );
	}
	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/breen_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_BREN, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    BREN END
//======================

//======================
//	   WEBLEY START
//======================
void EV_FireWebley( event_args_s *args )
{
	int idx, empty;

	idx = args->entindex;
	empty = args->bparam1;

	vec3_t origin;
	vec3_t angles;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( WEBLEY_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_WEBLEY );
	}
	EV_MuzzleFlash( idx, PISTOL );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/webley_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_WEBLEY, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
//	    WEBLEY END
//======================

//======================
//	  POPHELMET START
//======================
void EV_PopHelmet( struct event_args_s *args )
{
	int pitch;

	vec3_t origin, angles, endpos;

	pitch = args->iparam1;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	VectorClear( endpos );

	endpos[1] = args->fparam1;

	if( pitch <= 6 )
	{
		int model = gEngfuncs.pEventAPI->EV_FindModelIndex( sHelmetModels[pitch] );

		if( model )
		{
			TEMPENTITY *helmetmdl = gEngfuncs.pEfxAPI->R_TempModel( origin, angles, endpos, 5.0f, model, TE_BOUNCE_SHELL );

			if( helmetmdl )
			{
				helmetmdl->flags |= FTENT_HITSOUND;
			}
		}
	}
}
//======================
//	  POPHELMET END
//======================

//======================
//	   KNIFE START
//======================
void EV_Knife( event_args_s *args )
{
	int idx;
	int g_iSwing = 0;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );


	if( gEngfuncs.pfnRandomFloat( 0.0f, 1.0f ) == 1.0f )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash2.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );
	else
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", gEngfuncs.pfnRandomFloat( 0.8f, 1.0f ), ATTN_NORM, 0, 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f ) );

	if( EV_IsLocal( idx ) )
	{
		switch( ( g_iSwing++ ) % 2 )
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
//======================
//	   KNIFE END
//======================

//======================
// 	    MP40 START
//======================
void EV_FireMP40( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + MP40_SHOOT1, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_MP40 );
	}
	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp40_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP40, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	    MP40 END
//======================

//======================
// 	   SPRING START
//======================
void EV_FireSpring( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + SPRING_SHOOT1, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_SPRING );
	}
	EV_MuzzleFlash( idx, RIFLE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spring_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SPRING, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   SPRING END
//======================

//======================
// 	   KAR START
//======================
void EV_FireKar( event_args_s *args )
{
	int idx, pitch;

	vec3_t origin, angles;

	idx = args->entindex;
	pitch = args->iparam1;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( pitch == 1 )
	{
		if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 1.0f )
		{
			if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) <= 1.0f )
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) )
					return;
			}
			else if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 2.0f)
			{
				if( gEngfuncs.pfnRandomFloat( 0.0f, 3.0f ) != 3.0f )
					return;

				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing2.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

				if( !EV_IsLocal( idx ) )
					return;
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SLASH, EV_GetWeaponBody() );
				return;
			}
		}
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( !EV_IsLocal( idx ) )
			return;

		gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SWING, EV_GetWeaponBody() );
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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_KAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   KAR START
//======================

//======================
// 	   BAR START
//======================
void EV_FireBAR( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_DOWN_SHOOT, EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( BAR_UP_SHOOT, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_BAR );
	}
	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_BAR, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	     BAR END
//======================

//======================
// 	   BAZOOKA START
//======================
void EV_FireBazooka( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( BAZOOKA_FIRE, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_BAZOOKA );
	}
	EV_MuzzleFlash( idx, MG );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   BAZOOKA END
//======================

//======================
// 	   PSCHRECK START
//======================
void EV_FirePschreck( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( PSCHRECK_FIRE, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_PSCHRECK );
	}
	EV_MuzzleFlash( idx, MG );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   PSCHRECK END
//======================

//======================
// 	   PIAT START
//======================
void EV_FirePIAT( event_args_s *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( PIAT_FIRE, EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_PIAT );
	}
	EV_MuzzleFlash( idx, MG );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   PIAT END
//======================

//======================
// 	  MORTAR START
//======================
void EV_FireMortar( event_args_s *args )
{
	int idx;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MORTAR_DEPLOYED_FIRE, EV_GetWeaponBody() );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mortar_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_MuzzleFlash( idx, MG );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   MORTAR END
//======================

//======================
// 	SCOPEDENFIELD START
//======================
void EV_FireScopedEnfield( event_args_s *args )
{
	int idx;

	vec3_t origin, angles;

	idx = args->entindex;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1], args->fparam1, args->fparam2 );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	SCOPEDENFIELD END
//======================

void EV_PlayWhizz( event_args_t *args )
{
	int idx;

	idx = args->entindex;

	if( EV_IsLocal( idx ) )
		gEngfuncs.pfnPlaySoundByName( "player/whizz.wav", 1.0f );
}

// vgui2
void EV_DoDCamera( event_args_t *args )
{

}

void EV_SparkShower( event_args_t *args )
{
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_SparkShower( origin );
}

#ifdef USE_PMAN
void P_ExplosionFlash( event_args_t *args )
{

}
#endif // USE_PMAN

void PlayExplosionSound( float *origin )
{

}

void PlayMortarExplosionSound( float *origin )
{

}

void EV_BaseExplosion( event_args_t *args )
{

}

void EV_WaterExplosion( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_DirtTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_DirtRubble( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_DirtExplosion( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_Explosion( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_Bubbles( event_args_t *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	gEngfuncs.pEfxAPI->R_Bubbles( origin, angles, args->fparam1, args->iparam1, args->iparam2, args->fparam2 );
}

void EV_BubbleTrails( event_args_t *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	gEngfuncs.pEfxAPI->R_BubbleTrail( origin, angles, args->fparam1, args->iparam1, args->iparam2, args->fparam2 );
}

void EV_BulletTracers( event_args_t *args )
{
	vec3_t origin, angles;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	gEngfuncs.pEfxAPI->R_TracerEffect( origin, angles );
}

void EV_BloodStream( event_args_t *args )
{
	vec3_t origin, angles, velocity;

	TEMPENTITY *pShotDust, *pBloodNarrow;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	pShotDust = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, gEngfuncs.pfnRandomFloat( 2.0f, 4.0f ), gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/shot-dust.spr" ), kRenderTransAlpha, kRenderFxNone, 0.34f, 1.0f, FTENT_FADEOUT );

	pShotDust->fadeSpeed = 4.0f;
	pShotDust->entity.curstate.renderamt = 800;
	pShotDust->entity.curstate.framerate = 15.0f;
	pShotDust->entity.curstate.rendercolor.r = -76;
	pShotDust->entity.curstate.rendercolor.g = -80;
	pShotDust->entity.curstate.rendercolor.b = -108;

	pBloodNarrow = gEngfuncs.pEfxAPI->R_TempSprite( origin, velocity, 0.0f, gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/blood-narrow.spr" ), kRenderTransAlpha, kRenderFxNone, 0.34f, 1.0f, FTENT_FADEOUT );

	pBloodNarrow->fadeSpeed = 4.0f;
	pBloodNarrow->entity.curstate.renderamt = 800;
	pBloodNarrow->entity.curstate.framerate = 15.0f;
	pBloodNarrow->entity.curstate.rendercolor.r = 75;
	pBloodNarrow->entity.curstate.rendercolor.g = 0;
	pBloodNarrow->entity.curstate.rendercolor.b = 0;
}

void EV_BloodSprite( event_args_t *args )
{
	vec3_t origin;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_BloodSprite( origin, args->iparam2, args->iparam1, args->fparam1, args->fparam2 );
}

void EV_Smoke( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_Pain( event_args_t *args )
{
	int idx;

	char szSound[32];

	idx = args->entindex;

	vec3_t origin;

	VectorCopy( args->origin, origin );

	sprintf( szSound, "player/damage%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, szSound, 1.0f, 2.0f, 0, 100 );
}

extern char *s_USVoiceFiles[];
extern char *s_BRITVoiceFiles[];
extern char *s_VoiceCommands[][4];

void EV_USVoice( event_args_t *args )
{

	int iPlayer, pitch1, pitch2;

	vec3_t origin;

	iPlayer = args->entindex;
	pitch1 = args->iparam1;
	pitch2 = args->iparam2;

	VectorCopy( args->origin, origin );

	char *vcFiles, *vcCommands;

	if( pitch2 )
		vcFiles = s_BRITVoiceFiles[pitch1];
	else
		vcFiles = s_USVoiceFiles[pitch1];

	gEngfuncs.pEventAPI->EV_PlaySound( iPlayer, origin, CHAN_VOICE, vcFiles, gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
	GetPlayerInfo( iPlayer, &g_PlayerInfoList[iPlayer] );

	int team = gEngfuncs.GetEntityByIndex( iPlayer )->curstate.team;

	if( team == g_iTeamNumber && !g_iUser1 && team == gEngfuncs.GetLocalPlayer()->curstate.team )
	{
		Vector vecTeam = origin - gEngfuncs.GetLocalPlayer()->curstate.origin;
		float len = VectorNormalize( vecTeam );

		char pattern[256];

		if( len <= 1100.0f )
		{
			GetPlayerInfo( iPlayer, &g_PlayerInfoList[iPlayer] );
			sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[iPlayer].name, ": %s2" );

			vcCommands = s_VoiceCommands[pitch1][3];

			if( !vcCommands || !*vcCommands || !gHUD.m_bBritish )
				vcCommands = s_VoiceCommands[pitch1][1];

			gHUD.m_SayText.SayTextPrint( pattern, 256, iPlayer, "#VOICE", vcCommands, 0, 0 );

			GetPlayerInfo( iPlayer, &g_PlayerInfoList[iPlayer] );

			gHUD.m_Spectator.AddVoiceIconToPlayerEnt( iPlayer );
		}
	}
}

extern char *s_GERVoiceFiles[];

void EV_GERVoice( event_args_t *args )
{
int iPlayer, pitch1;

	vec3_t origin;

	iPlayer = args->entindex;
	pitch1 = args->iparam1;

	VectorCopy( args->origin, origin );

	char *vcFiles = s_GERVoiceFiles[pitch1];
	const char *vcCommands;

	if( pitch1 == 27 && gHUD.m_bBritish )
		pitch1 = 28;

	gEngfuncs.pEventAPI->EV_PlaySound( iPlayer, origin, CHAN_VOICE, vcFiles, gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

	int team = gEngfuncs.GetEntityByIndex( iPlayer )->curstate.team;

	if( team == g_iTeamNumber && !g_iUser1 && team == gEngfuncs.GetLocalPlayer()->curstate.team )
	{
		Vector vecTeam = origin - gEngfuncs.GetLocalPlayer()->curstate.origin;
		float len = VectorNormalize( vecTeam );

		char pattern[256];

		if( len <= 1100.0f )
		{
			GetPlayerInfo( iPlayer, &g_PlayerInfoList[iPlayer] );
			sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[iPlayer].name, ": %s2" );

			vcCommands = *s_VoiceCommands[pitch1];

			char *text;

			if( vcCommands[2] )
				text = CHudTextMessage::BufferedLocaliseTextString( &vcCommands[2] );
			else
				text = CHudTextMessage::BufferedLocaliseTextString( &vcCommands[1] );

			gHUD.m_SayText.SayTextPrint( pattern, 256, iPlayer, "#VOICE", text, 0, 0 );

			GetPlayerInfo( iPlayer, &g_PlayerInfoList[iPlayer] );

			gHUD.m_Spectator.AddVoiceIconToPlayerEnt( iPlayer );
		}
	}
}

void EV_BodyDamage( event_args_t *args )
{
	int idx, pitch;

	vec3_t origin;

	idx = args->entindex;
	pitch = args->iparam1;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, "player/helmet_hit.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
}

void EV_RoundReleaseSound( event_args_t *args )
{
	int idx, empty, pitch1, pitch2;

	vec3_t origin;

	idx = args->entindex;
	empty = args->bparam1;
	pitch1 = args->iparam1;
	pitch2 = args->iparam2;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) && !g_iVuser1z )
	{
		char *m_cStartRoundSound;

		if( empty && pitch1 == 1 )
		{
			if( pitch2 > 3 )
				return;

			switch( gEngfuncs.pfnRandomLong( 1, 2 ) )
			{
			case 1:
				m_cStartRoundSound = "player/britstartround.wav";
				break;
			case 2:
				m_cStartRoundSound = "player/britstartround2.wav";
				break;
			}

			if( m_cStartRoundSound )
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, m_cStartRoundSound, 1.0f, ATTN_NORM, 0, 100 );
		}

		if( !empty && pitch1 == 1 )
		{
			if( pitch2 > 3 )
				return;

			switch( gEngfuncs.pfnRandomLong( 1, 2 ) )
			{
			case 1:
				m_cStartRoundSound = "player/usstartround.wav";
				break;
			case 2:
				m_cStartRoundSound = "player/usstartround2.wav";
				break;
			}

			if( m_cStartRoundSound )
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, m_cStartRoundSound, 1.0f, ATTN_NORM, 0, 100 );
		}

		if( pitch1 == 2 && pitch2 <= 3 )
		{
			switch( gEngfuncs.pfnRandomLong( 1, 2 ) )
			{
			case 1:
				m_cStartRoundSound = "player/gerstartround.wav";
				break;
			case 2:
				m_cStartRoundSound = "player/gerstartround2.wav";
				break;
			}

			if( m_cStartRoundSound )
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, m_cStartRoundSound, 1.0f, ATTN_NORM, 0, 100 );
		}
	}
}

#define SND_STOP			(1 << 5)
#define SND_CHANGE_PITCH	(1 << 7)

void EV_TrainPitchAdjust( event_args_t *args )
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;

	const char *pszSound;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	us_params = (unsigned short)args->iparam1;
	stop = args->bparam1;

	m_flVolume = (float)( us_params & 0x003f ) / 40.0f;
	noise = (int)( ( ( us_params ) >> 12 ) & 0x0007 );
	pitch = (int)( 10.0f * (float)( ( us_params >> 6 ) & 0x003f ) );

	switch( noise )
	{
	case 1:
		pszSound = "plats/ttrain1.wav";
		break;
	case 2:
		pszSound = "plats/ttrain2.wav";
		break;
	case 3:
		pszSound = "plats/ttrain3.wav";
		break; 
	case 4:
		pszSound = "plats/ttrain4.wav";
		break;
	case 5:
		pszSound = "plats/ttrain6.wav";
		break;
	case 6:
		pszSound = "plats/ttrain7.wav";
		break;
	default:
		// no sound
		return;
	}

	if( stop )
	{
		gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, pszSound );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, pszSound, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch );
	}
}

int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 )
{
	return iTeam1 == iTeam2;
}

#ifdef USE_PMAN
void P_Rubble( event_args_t *args )
{

}

void P_ExplosionSmoke( event_args_t *args )
{

}
#endif // USE_PMAN

char EV_FindHitTexture( float *start, float *end )
{
	return 0;
}

char EV_TexNameToType( char *pTextureName )
{
	return 0;
}

void CreateFlyingRubble( vec3_t origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, vec3_t vNormal )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_RocketTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_RocketTrail( event_args_t *args )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_MortarShellCallback( tempent_s *ent, float frametime, float currenttime )
{

}

void EV_MortarShell( event_args_t *args )
{

}

void EV_OverheatCallback( tempent_s *ent, float frametime, float currenttime )
{
#ifdef USE_PMAN

#endif // USE_PMAN
}

void EV_Overheat( event_args_t *args )
{
#ifdef USE_PMAN
	int idx;

	float lastOverheatTime;

	vec3_t origin;

	idx = args->entindex;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/mgoverheat.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	float fl = gEngfuncs.GetClientTime() - lastOverheatTime;

	lastOverheatTime = gEngfuncs.GetClientTime();

	if( ( fl < 0.0f || fl >= 1.0f ) && cl_particlefx->value >= 2.0 )
	{
		TEMPENTITY *p = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( origin );

		if( gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( origin ) )
		{
			p->flags |=
				p->callback = EV_OverheatCallback;
			p->clientIndex = idx;
			p->die = gEngfuncs.GetClientTime() + 2.5f;
			p->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.5f;
			p->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
		}
	}
#endif // USE_PMAN
}