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

#include "tri.h"

extern IParticleMan *g_pParticleman;
extern Queue g_RubbleQueue;
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

	if( g_iUser1 == OBS_IN_EYE )
	{
		cl_entity_t *target = gEngfuncs.GetEntityByIndex( g_iUser2 );

		if( target )
			team = target->curstate.team;
	}

	if( team == 1 )
		return gHUD.m_bBritish;

	return 2;
}

void EV_ResetAnimationEvents( int index )
{
	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( index );

	if( !ent )
		return;

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

static cvar_t *r_decals;

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, float *vecSrc, float *vecEnd, int iBulletType )
{
	physent_t *pe;
	char chTextureType = 'F';
	int idx;
	static char decalname[32];

	float flHeight = EV_HLDM_WaterHeight( vecSrc, 0.0f, 0.0f ) - vecSrc[2];

	if( flHeight < 8.0f )
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

	if( entity > gEngfuncs.GetMaxClients() || entity <= 0 )
	{
		chTextureType = 'K';

		gEngfuncs.pEventAPI->EV_PlayerTrace( pTrace->endpos, vecEnd, 2, -1, &tr2 );

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
			sprintf( decalname, "{cement%i", gEngfuncs.pfnRandomLong( 1, 2 ) );
			break;
		case 'C':
			chTextureType = 'C';
			sprintf( decalname, "{cement%i", gEngfuncs.pfnRandomLong( 1, 2 ) );
			break;
		case 'T':
			chTextureType = 'T';
			sprintf( decalname, "{crack%i", gEngfuncs.pfnRandomLong( 1, 4 ) );
			break;
		case 'W':
			chTextureType = 'W';
			sprintf( decalname, "{wood%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
			break;
		case 'Z':
			chTextureType = 'Z';
			sprintf( decalname, "{wood%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
			break;
		default:
			sprintf( decalname, "{generic%i", gEngfuncs.pfnRandomLong( 1, 3 ) );
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

		EV_HLDM_DoDSurfaceFX( pTrace, iBulletType, chTextureType );
	}
}


TEMPENTITY *g_DeadPlayerModels[64];

void EV_RoundReset( event_args_t *args )
{
	if( !r_decals )
	{
		r_decals = gEngfuncs.pfnGetCvarPointer( "r_decals" );
	}

	if( r_decals )
	{
		int maxdecals = ( int ) r_decals->value;

		if( maxdecals > 0 )
		{
			for( int i = 0; i < maxdecals; i++ )
			{
				gEngfuncs.pEfxAPI->R_DecalRemoveAll( i );
			}
		}
	}

	for( int j = 0; j < 64; j++ )
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
		ent->flags |= FTENT_BODYSTATIC;
}

void CreateCorpse( vec3_t vOrigin, vec3_t vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody )
{
	float vecVelocity[3];
	gEngfuncs.pEventAPI->EV_FindModelIndex( pModel );
	memset( vecVelocity, 0, sizeof( vecVelocity ) );
	int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( pModel );
	TEMPENTITY *pBody = gEngfuncs.pEfxAPI->R_TempModel( &vOrigin.x, vecVelocity, &vAngles.x, 100.0f, iModelIndex, 0 );

	if( pBody )
	{
		pBody->flags |= ( FTENT_COLLIDEWORLD | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_CORPSE );

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

		pBody->callback = RemoveBody;
		pBody->hitcallback = HitBody;
		pBody->bounceFactor = 0.0f;

		float flCorpseStayValue = 0.0f;

		if( gHUD.cl_corpsestay )
			flCorpseStayValue = gHUD.cl_corpsestay->value;

		pBody->die = flSpawnTime + flCorpseStayValue + 5.0f;

		char j = '\0';

		for( int i = 0; i < 64; i++ )
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
				return;
			}
		}

		if( !j )
			pBody->die = pBody->entity.curstate.fuser2 + 5.0f;
	}
}


void EV_BasicPuff( pmtrace_t *pTrace, float scale )
{
	vec3_t origin, vColor, vVelocity;

	vColor = { 175.0f, 175.0f, 175.0f };
	vVelocity = pTrace->plane.normal;

	for( int i = 0; i < 4; i++ )
	{
		float flRandomOffset = gEngfuncs.pfnRandomLong( 1, 3 );

		VectorMA( pTrace->endpos, flRandomOffset, pTrace->plane.normal, origin );
		CreateDebrisWallPuff( origin, vVelocity, vColor, i );
	}
}

void EV_CreteRubble( pmtrace_t *pTrace, float fScale )
{
	vec3_t origin, vVelocity;
	int iAmmount;
	int i;
	bool bLargeRubble;

	int iTextureType = 0;
	const char *pTextureName;

	VectorMA( pTrace->endpos, 2.0f, pTrace->plane.normal, origin );

	if( !cl_numshotrubble || cl_numshotrubble->value <= 0.0f )
		return;

	iAmmount = gEngfuncs.pfnRandomLong( 0, ( int ) cl_numshotrubble->value );

	if( iAmmount <= 0 )
		return;

	if( !g_pParticleMan )
		return;

	pTextureName = gEngfuncs.pEventAPI->EV_TraceTexture( pTrace->ent, pTrace->endpos, origin );

	if( pTextureName )
		iTextureType = ( int ) pTextureName;

	bLargeRubble = ( fScale > 1.0f );

	for( i = 0; i < iAmmount; ++i )
	{
		if( g_RubbleQueue.Full() )
			break;

		vVelocity[0] = gEngfuncs.pfnRandomFloat( -300.0f, 300.0f );
		vVelocity[1] = gEngfuncs.pfnRandomFloat( -300.0f, 300.0f );
		vVelocity[2] = 75.0f;

		CreateFlyingRubble( origin, bLargeRubble, vVelocity[0], vVelocity[1], vVelocity[2], fScale,
			iTextureType, pTrace->plane.normal);
	}
}

void CreateSpark( vec3_t origin, vec3_t vNormal, const char *szSpriteName )
{
	model_s *pSprite;
	float scale;
	vec3_t angles;
	CDoDParticle *pCustom;

	vec3_t normal = { 0.0f, 0.0f, 1.0f };

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( szSpriteName );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", szSpriteName );
		return;
	}

	VectorAngles( &vNormal.x, &angles.x );

	scale = 2.5f;

	pCustom = pCustom->Create( &origin, &normal, pSprite, scale, 255.0f, "dod_particle", 1 );

	if( pCustom )
	{
		pCustom->m_vAngles = angles;
		pCustom->m_flGravity = 0.0f;
		pCustom->m_vVelocity = Vector( 0, 0, 0 );
		pCustom->m_vAVelocity = Vector( 0, 0, 0 );
		pCustom->m_iFramerate = 0;
		pCustom->m_flSize = scale;
		pCustom->m_flDieTime = 0.05f;
		pCustom->m_iRendermode = kRenderTransAdd;
		pCustom->m_iPFlags = 64; 
		pCustom->SetLightFlag( 0 );
		pCustom->SetCullFlag( 1 );
	}
}

void EV_MetalHit( pmtrace_t *pTrace )
{
	vec3_t origin, angles, vEnd, vNormal, vDir;
	float ofs, incr;
	int iSparkSprite, iSparkNum;
	float length, scale;
	dlight_t *dl;

	origin = pTrace->endpos;

	iSparkSprite = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/metalspark.spr" );

	if( iSparkSprite )
	{
		iSparkNum = gEngfuncs.pfnRandomLong( 4, 6 );
		length = gEngfuncs.pfnRandomFloat( 12.0f, 16.0f );

		if( iSparkNum > 0 )
		{
			for( int j = 0; j != iSparkNum; ++j )
			{
				if( j )
				{
					ofs = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f );
					vDir.x = ofs + vNormal.x;
					ofs = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f );
					vDir.y = ofs + vNormal.y;
					ofs = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f );
					vDir.z = ofs + vNormal.z;

					length -= incr;
					VectorNormalize( vDir );
					scale = length + length;
					VectorMA( origin, scale, vDir, vEnd );
				}
				else
				{
					vNormal.x = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -0.18f, 0.12f ) + pTrace->plane.normal[2];

					VectorMA( origin, length, vNormal, vEnd );
					vDir = vNormal;
					length *= 0.75f;
					incr = length / ( float ) iSparkNum;
				}

				gEngfuncs.pEfxAPI->R_BeamPoints( origin, vEnd, iSparkSprite, 0.05f, 0.6f, 0, 200.0f, 0, 0, 0,
					190.0f, 160.0f, 130.0f );
			}
		}

		CreateSpark( origin, pTrace->plane.normal, "sprites/smallspark.spr" );

		if( cl_dynamiclights && cl_dynamiclights->value > 0.0f )
		{
			dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

			if( dl )
			{
				dl->origin = pTrace->endpos;
				dl->radius = 55.0f;
				dl->die = gHUD.m_flTime + 0.1f;
				dl->decay = 5.0f;
				dl->color.r = 240;
				dl->color.g = 240;
				dl->color.b = 180;
			}
		}
	}
}

void EV_DirtHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pDirt3;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_dirt3.spr" );
	pDirt3 = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pDirt3 )
	{
		iNum = gEngfuncs.pfnRandomLong( 2, 3 );

		vec3_t p_org = pTrace->endpos;
		vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
		float size = fScale * 4.0f;

		for( int i = 0; i < iNum; i++ )
		{
			pParticle = pParticle->Create( &p_org, &p_normal, pDirt3, size, 160.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_flStretchX = 2.0f;
				pParticle->m_flStretchY = 6.0f;

				vec3_t dir;
				dir.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.z = 0.0f;
				dir.Normalize();

				if( i == 0 )
				{
					pParticle->m_vVelocity.x = 0.0f;
					pParticle->m_vVelocity.y = 0.0f;
					pParticle->m_vVelocity.z = 100.0f;
					pParticle->m_flStretchY = 7.0f;
				}
				else
				{
					pParticle->m_vVelocity.x = 0.0f;
					pParticle->m_vVelocity.y = 0.0f;
					pParticle->m_vVelocity.z = 90.0f;

					float flSpreadMultiplier = ( i == 1 ) ? 20.0f : -20.0f;
					VectorMA( pParticle->m_vVelocity, flSpreadMultiplier, dir, pParticle->m_vVelocity );
				}

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;

				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags | LIGHT_NONE );

				pParticle->m_iPFlags = 128;
				pParticle->m_iRendermode = kRenderTransAdd;

				pParticle->m_vColor.x = 83.0f;
				pParticle->m_vColor.y = 72.0f;
				pParticle->m_vColor.z = 55.0f;

				pParticle->m_flGravity = 0.6f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			}
		}
	}
}


void EV_SandHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pDirt3;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_dirt3.spr" );
	pDirt3 = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pDirt3 )
	{
		iNum = gEngfuncs.pfnRandomLong( 2, 3 );

		vec3_t p_org = pTrace->endpos;
		vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
		float size = fScale * 4.0f;

		for( int i = 0; i < iNum; i++ )
		{
			pParticle = pParticle->Create( &p_org, &p_normal, pDirt3, size, 140.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_flStretchX = 2.0f;
				pParticle->m_flStretchY = 6.0f;

				vec3_t dir;
				dir.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f );
				dir.z = 0.0f;
				dir.Normalize();

				if( i == 0 )
				{
					pParticle->m_vVelocity.x = 0.0f;
					pParticle->m_vVelocity.y = 0.0f;
					pParticle->m_vVelocity.z = 100.0f;
					pParticle->m_flStretchY = 7.0f;
				}
				else
				{
					pParticle->m_vVelocity.x = 0.0f;
					pParticle->m_vVelocity.y = 0.0f;
					pParticle->m_vVelocity.z = 90.0f;

					float flSpreadMultiplier = ( i == 1 ) ? 20.0f : -20.0f;
					VectorMA( pParticle->m_vVelocity, flSpreadMultiplier, dir, pParticle->m_vVelocity );
				}

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;

				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags | LIGHT_NONE );

				pParticle->m_iPFlags = 128;
				pParticle->m_iRendermode = kRenderTransAdd;

				pParticle->m_vColor.x = 185.0f;
				pParticle->m_vColor.y = 177.0f;
				pParticle->m_vColor.z = 149.0f;

				pParticle->m_flGravity = 0.6f;
				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
			}
		}
	}
}

void EV_GrassHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pSprite;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_grass1.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSprite )
	{
		iNum = gEngfuncs.pfnRandomLong( 4, 6 );

		if( iNum > 0 )
		{
			vec3_t p_org = pTrace->endpos;
			vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
			float size = fScale * 3.0f;

			for( int i = 0; i != iNum; ++i )
			{
				pParticle = pParticle->Create( &p_org, &p_normal, pSprite, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -80.0f, 80.0f );
					pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -80.0f, 80.0f );
					pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 150.0f, 180.0f );

					pParticle->m_vAVelocity.z = 1000.0f;
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE | TRI_COLLIDEDAMP );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;

					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 128;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.7f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}
	}
}

void EV_WoodChips( pmtrace_t *pTrace, float fScale )
{
	model_t *pSprite;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_wood1.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSprite )
	{
		iNum = gEngfuncs.pfnRandomLong( 1, 2 );

		if( iNum > 0 )
		{
			vec3_t p_org = pTrace->endpos;
			vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
			float size = fScale * 4.0f;

			for( int i = 0; i < iNum; i++ )
			{
				pParticle = pParticle->Create( &p_org, &p_normal, pSprite, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					vec3_t vNormal, vEnd;

					vNormal.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[2];

					VectorMA( pTrace->endpos, 200.0f, vNormal, vEnd );
					VectorSubtract( pTrace->endpos, vEnd, pParticle->m_vVelocity );

					pParticle->m_vAVelocity.z = 1000.0f;
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;

					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 128;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.6f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}

		float scale = fScale * 0.5f;
		EV_BasicPuff( pTrace, scale );
	}
}

void EV_GlassShards( pmtrace_t *pTrace, float fScale )
{
	model_t *pSprite;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_glass.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSprite )
	{
		iNum = gEngfuncs.pfnRandomLong( 3, 4 );

		if( iNum > 0 )
		{
			vec3_t p_org = pTrace->endpos;
			vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
			float size = fScale * 2.0f;

			for( int i = 0; i < iNum; i++ )
			{
				pParticle = pParticle->Create( &p_org, &p_normal, pSprite, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					vec3_t vNormal, vEnd;

					vNormal.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[2];

					VectorMA( pTrace->endpos, 400.0f, vNormal, vEnd );
					VectorSubtract( pTrace->endpos, vEnd, pParticle->m_vVelocity );

					pParticle->m_vAVelocity.z = 1000.0f;
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;

					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 128;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.6f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}
	}
}

void EV_WaterHit( pmtrace_t *pTrace, float fScale )
{
	float fTime = gEngfuncs.GetClientTime();
	model_t *pRipple;
	model_t *pSplash;
	model_t *pSprite;
	CDoDParticle *pParticle;

	HSPRITE hRippleSprite = gEngfuncs.pfnSPR_Load( "sprites/ripple.spr" );
	pRipple = ( model_s * ) gEngfuncs.GetSpritePointer( hRippleSprite );

	if( pRipple )
	{
		vec3_t p_normal = { 90.0f, 0.0f, 0.0f };

		for( int i = 0; i < 3; i++ )
		{
			vec3_t vOrg;

			vOrg.x = pTrace->endpos[0] + gEngfuncs.pfnRandomFloat( -20.0f, 30.0f );
			vOrg.y = pTrace->endpos[1] + gEngfuncs.pfnRandomFloat( -20.0f, 30.0f );
			vOrg.z = pTrace->endpos[2];

			pParticle = pParticle->Create( &vOrg, &p_normal, pRipple, 30.0f, 150.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_iPFlags = 64;
				pParticle->m_iRendermode = kRenderTransAdd;

				pParticle->m_flScaleSpeed = 2.0f;
				pParticle->m_flFadeSpeed = 4.0f;
				pParticle->m_flDieTime = fTime + 2.0f;

				pParticle->m_vColor.x = 255.0f;
				pParticle->m_vColor.y = 255.0f;
				pParticle->m_vColor.z = 255.0f;

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags );
			}
		}
	}

	HSPRITE hSplashSprite = gEngfuncs.pfnSPR_Load( "sprites/bazookapuff.spr" );
	pSplash = ( model_s * ) gEngfuncs.GetSpritePointer( hSplashSprite );

	if( pSplash )
	{
		vec3_t p_normal = { 90.0f, 0.0f, 0.0f };

		for( int j = 0; j < 3; j++ )
		{
			vec3_t vStart;

			vStart.x = pTrace->endpos[0];
			vStart.y = pTrace->endpos[1];
			vStart.z = pTrace->endpos[2] + ( ( float ) j * 5.0f );

			pParticle = pParticle->Create( &vStart, &p_normal, pSplash, 40.0f, 80.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -50.0f, 70.0f );
				pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -50.0f, 70.0f );
				pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 100.0f, 140.0f );

				pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -2.0f, 2.0f );

				pParticle->SetCollisionFlags( TRI_WATERTRACE | TRI_ANIMATEDIE | TRI_COLLIDEDAMP | TRI_COLLIDESLIDE 
					| TRI_COLLIDEBREAK );

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags );

				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->m_flMass = 1.0f;
				pParticle->m_flGravity = 0.4f;

				pParticle->m_vColor.x = 255.0f;
				pParticle->m_vColor.y = 255.0f;
				pParticle->m_vColor.z = 255.0f;

				pParticle->m_flDieTime = fTime + 1.0f;
			}
		}
	}

	HSPRITE hBubbleSprite = gEngfuncs.pfnSPR_Load( "sprites/bubble.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hBubbleSprite );

	if( pSprite )
	{
		vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
		float flScale = 20.0f;

		for( int k = 0; k < 3; k++ )
		{
			vec3_t vStart = pTrace->endpos;

			pParticle = pParticle->Create( &vStart, &p_normal, pSprite, flScale, 100.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -20.0f, 20.0f );
				pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -20.0f, 20.0f );
				pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 30.0f, 60.0f );

				pParticle->SetCollisionFlags( TRI_WATERTRACE );

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_ANIMATEDIE;
				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags );

				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->m_flGravity = -0.1f;

				pParticle->m_vColor.x = 255.0f;
				pParticle->m_vColor.y = 255.0f;
				pParticle->m_vColor.z = 255.0f;

				pParticle->m_flDieTime = fTime + 1.5f;
			}
		}
	}
}

void EV_LeavesHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pFoliage;
	model_t *pLeaf1;
	model_t *pLeaf2;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hFoliage = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_foliage.spr" );
	pFoliage = ( model_s * ) gEngfuncs.GetSpritePointer( hFoliage );

	HSPRITE hLeaf1 = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_leaf1.spr" );
	pLeaf1 = ( model_s * ) gEngfuncs.GetSpritePointer( hLeaf1 );

	HSPRITE hLeaf2 = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_leaf2.spr" );
	pLeaf2 = ( model_s * ) gEngfuncs.GetSpritePointer( hLeaf2 );

	if( pLeaf1 && pFoliage && pLeaf2 )
	{
		vec3_t p_org = pTrace->endpos;
		vec3_t p_normal = { 0.0f, 0.0f, 0.0f };

		iNum = gEngfuncs.pfnRandomLong( 2, 3 );

		if( iNum > 0 )
		{
			float size = fScale * 4.0f;

			for( int i = 0; i < iNum; i++ )
			{
				pParticle = pParticle->Create( &p_org, &p_normal, pFoliage, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					vec3_t vNormal, vEnd;

					vNormal.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[2];

					VectorMA( pTrace->endpos, 100.0f, vNormal, vEnd );
					VectorSubtract( pTrace->endpos, vEnd, pParticle->m_vVelocity );

					pParticle->m_vAVelocity.z = 1000.0f;
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE | TRI_WIND );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;
					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 64;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.2f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
					pParticle->m_flDampingTime = gEngfuncs.GetClientTime() + 0.1f;

					pParticle->AddGlobalWind();
				}
			}
		}

		iNum = gEngfuncs.pfnRandomLong( 0, 1 );

		if( iNum > 0 )
		{
			float size = fScale * 8.0f;

			for( int j = 0; j < iNum; j++ )
			{
				model_t *pModel = ( gEngfuncs.pfnRandomLong( 0, 1 ) == 0 ) ? pLeaf1 : pLeaf2;

				pParticle = pParticle->Create( &p_org, &p_normal, pModel, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					vec3_t vNormal, vEnd;

					vNormal.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[2];

					VectorMA( pTrace->endpos, 100.0f, vNormal, vEnd );
					VectorSubtract( pTrace->endpos, vEnd, pParticle->m_vVelocity );

					pParticle->m_vAVelocity.x = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
					pParticle->m_vAVelocity.y = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
					pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->m_vAngles.x = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
					pParticle->m_vAngles.y = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE | TRI_WIND );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;
					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 64;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.2f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
					pParticle->m_flDampingTime = gEngfuncs.GetClientTime() + 0.1f;

					pParticle->AddGlobalWind();
				}
			}
		}
	}
}

void EV_SnowHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pSplash;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/bazookapuff.spr" );
	pSplash = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSplash )
	{
		vec3_t p_org = pTrace->endpos;
		vec3_t p_normal = { 90.0f, 0.0f, 0.0f };
		float size = 10.0f * fScale;

		for( int i = 0; i < 5; i++ )
		{
			pParticle = pParticle->Create( &p_org, &p_normal, pSplash, size, 140.0f, "dod_particle", 0 );

			if( pParticle )
			{
				pParticle->m_vVelocity.x = gEngfuncs.pfnRandomFloat( -30.0f, 50.0f );
				pParticle->m_vVelocity.y = gEngfuncs.pfnRandomFloat( -30.0f, 50.0f );
				pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 40.0f, 70.0f );

				pParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -6.0f, 5.0f );

				pParticle->SetCollisionFlags( TRI_WATERTRACE | TRI_ANIMATEDIE | TRI_COLLIDEDAMP | TRI_COLLIDESLIDE | TRI_COLLIDEBREAK );

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;
				pParticle->SetCullFlag( iParticleFlags );
				pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

				pParticle->m_iPFlags = 128;
				pParticle->m_iRendermode = kRenderTransAdd;
				pParticle->m_flMass = 1.0f;
				pParticle->m_flGravity = 0.4f;

				pParticle->m_vColor.x = 255.0f;
				pParticle->m_vColor.y = 255.0f;
				pParticle->m_vColor.z = 255.0f;

				pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 1.0f;
			}
		}
	}
}

void EV_TileHit( pmtrace_t *pTrace, float fScale )
{
	model_t *pSprite;
	int iNum;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/debris_tile1.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSprite )
	{
		iNum = gEngfuncs.pfnRandomLong( 3, 4 );

		if( iNum > 0 )
		{
			vec3_t p_org = pTrace->endpos;
			vec3_t p_normal = { 0.0f, 0.0f, 0.0f };
			float size = fScale * 1.5f;

			for( int i = 0; i < iNum; i++ )
			{
				pParticle = pParticle->Create( &p_org, &p_normal, pSprite, size, 255.0f, "dod_particle", 0 );

				if( pParticle )
				{
					vec3_t vNormal, vEnd;

					vNormal.x = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[0];
					vNormal.y = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[1];
					vNormal.z = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) + pTrace->plane.normal[2];

					VectorMA( pTrace->endpos, 400.0f, vNormal, vEnd );
					VectorSubtract( pTrace->endpos, vEnd, pParticle->m_vVelocity );

					pParticle->m_vAVelocity.z = 1000.0f;
					pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

					pParticle->SetCollisionFlags( TRI_WATERTRACE );

					int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_WATERTRACE;

					pParticle->SetCullFlag( iParticleFlags );
					pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

					pParticle->m_iPFlags = 128;
					pParticle->m_iRendermode = kRenderTransTexture;

					pParticle->m_flGravity = 0.6f;
					pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
				}
			}
		}

		float scale = fScale * 0.5f;
		EV_BasicPuff( pTrace, scale );
	}
}

void EV_PlaySurfaceHitSound( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	char hitsound;

	switch( cSurfaceType )
	{
	default:
		sprintf( &hitsound, "weapons/ric%d.wav", gEngfuncs.pfnRandomLong( 1, 11 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'A':
	case 'D':
		sprintf( &hitsound, "weapons/hit_sand%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'E':
		sprintf( &hitsound, "weapons/hit_leaves%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'G':
	case 'M':
		sprintf( &hitsound, "weapons/hit_metal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'H':
		sprintf( &hitsound, "weapons/hit_heavymetal%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'K':
		return;
	case 'N':
		sprintf( &hitsound, "weapons/hit_snow%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'P':
		sprintf( &hitsound, "weapons/hit_grass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'S':
		sprintf( &hitsound, "weapons/hit_water%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'W':
		sprintf( &hitsound, "weapons/hit_wood%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	case 'Y':
		sprintf( &hitsound, "weapons/hit_glass%d.wav", gEngfuncs.pfnRandomLong( 1, 2 ) );
		gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, &hitsound, 1.0, 0.8, 0, 100 );
		return;
	}
}

void EV_HLDM_DoDSurfaceFX( pmtrace_t *pTrace, int iBulletType, char cSurfaceType )
{
	float fScale = 1.5f;

	switch( iBulletType )
	{
	case BULLET_PLAYER_COLT:
	case BULLET_PLAYER_LUGER:
	case BULLET_PLAYER_THOMPSON:
	case BULLET_PLAYER_MP40:
	case BULLET_PLAYER_GREASEGUN:
	case BULLET_PLAYER_STEN:
	case BULLET_PLAYER_WEBLEY:
		fScale = 1.0f;
		break;
	case BULLET_PLAYER_M1CARBINE:
	case BULLET_PLAYER_MP44:
	case BULLET_PLAYER_FG42:
		fScale = 1.4f;
		break;
	case BULLET_PLAYER_GARAND:
	case BULLET_PLAYER_SCOPEDKAR:
	case BULLET_PLAYER_SPRING:
	case BULLET_PLAYER_KAR:
	case BULLET_PLAYER_BAR:
	case BULLET_PLAYER_K43:
	case BULLET_PLAYER_ENFIELD:
	case BULLET_PLAYER_BREN:
		fScale = 1.7f;
		break;
	case BULLET_PLAYER_MG42:
	case BULLET_PLAYER_MG34:
	case BULLET_PLAYER_30CAL:
		fScale = 2.2f;
		break;
	default:
		fScale = 1.5f;
		break;
	}

	switch( cSurfaceType )
	{
	case 'A':
		EV_SandHit( pTrace, fScale );
		return;
	case 'B':
	case 'C':
	case 'R':
	case 'L':
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
	default:
		EV_BasicPuff( pTrace, fScale );
		return;
	}
}

void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd )
{
	physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if( pe && pe->solid == SOLID_BSP )
		EV_HLDM_GunshotDecalTrace( pTrace, " ", vecSrc, vecEnd, iBulletType );
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
				EV_HLDM_GunshotDecalTrace( &tr, " ", vecSrc, vecEnd, iBulletType);
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
	vec3_t b;
	int startlevel = gEngfuncs.PM_PointContents( vecSrc, 0 );
	int iEndContents = gEngfuncs.PM_PointContents( pTrace->endpos, 0 );

	if( iEndContents == startlevel )
		return 0;

	vec3_t vecCurrent;
	VectorCopy( vecSrc, vecCurrent );

	vec3_t vecDir;

	VectorSubtract( pTrace->endpos, vecSrc, vecDir );

	while( vecDir.Length() > 4.0f )
	{
		VectorScale( vecDir, 0.5f, vecDir );
		VectorAdd( vecCurrent, vecDir, b );

		if( gEngfuncs.PM_PointContents( b, 0 ) == startlevel )
		{
			VectorCopy( b, vecCurrent );
		}
		else
		{
			VectorSubtract( b, vecCurrent, vecDir );
		}
	}

	VectorCopy( vecCurrent, vecResult );

	return 1;
}

void EV_HLDM_BubbleTrails( vec3_t *from, vec3_t *to, int count )
{
	float flHeight;
	int m_iBubbles;
	float flHeightFrom = EV_HLDM_WaterHeight( *from, from->z + 256.0f, 0.0f ) - from->z;

	if( flHeightFrom < 8.0f )
	{
		float flHeightTo = EV_HLDM_WaterHeight( *to, to->z + 256.0f, 0.0f );
		float flHeightDiff = flHeightTo - to->z;

		if( flHeightDiff < 8.0f )
			return;

		flHeight = flHeightDiff + to->z - from->z;
	}
	else
	{
		flHeight = flHeightFrom;
	}

	int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/bubble.spr" );

	m_iBubbles = ( count > 255 ) ? 255 : count;

	gEngfuncs.pEfxAPI->R_BubbleTrail( &from->x, &to->x, flHeight, iModelIndex, m_iBubbles, 8.0f );
}

//======================
//	    MELEE START
//======================
void EV_FireMelee( event_args_s *args )
{
	vec3_t origin;

	int iAnim = args->iparam1;
	int bPlayWiff = args->bparam1;
	int bPlayHit = args->bparam2;
	int idx = args->entindex;

	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, EV_GetWeaponBody() );
	}

	if( bPlayWiff )
	{
		gEngfuncs.pfnRandomLong( 0, 1 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", 
			gEngfuncs.pfnRandomFloat( ATTN_NORM, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	if( bPlayHit )
	{
		gEngfuncs.pfnRandomLong( 0, 2 );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_hit1.wav", 
			gEngfuncs.pfnRandomFloat( ATTN_NORM, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_ResetAnimationEvents( idx );
}
//======================
//	   MELEE END
//======================

//======================
//	    COLT START
//======================
void EV_FireColt( event_args_t *args )
{
	int idx = args->entindex;
	int empty = args->bparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	// are not used
	//vec3_t velocity;
	//vec3_t ShellVelocity;
	//vec3_t ShellOrigin;

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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/colt_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_COLT, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	   COLT END
//======================

//======================
//	    LUGER START
//======================
void EV_FireLuger( event_args_t *args )
{
	int idx = args->entindex;
	int empty = args->bparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/luger_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_LUGER, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    LUGER END
//======================

//======================
//	    MG42 START
//======================
void EV_FireMG42( event_args_t *args )
{
	int idx = args->entindex;
	int iBulletsRemaining = args->iparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;
	bool bCallAnim = ( args->bparam1 == 0 );

	int shell;
	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	static const int MG42_DownAnims[9] = { MG42_DOWNSHOOT, MG42_DOWNSHOOT8, MG42_DOWNSHOOT7, MG42_DOWNSHOOT6, MG42_DOWNSHOOT5, MG42_DOWNSHOOT4, MG42_DOWNSHOOT3, MG42_DOWNSHOOT2, MG42_DOWNSHOOT1 };
	static const int MG42_UpAnims[9] = { MG42_UPSHOOT, MG42_UPSHOOT8, MG42_UPSHOOT7, MG42_UPSHOOT6, MG42_UPSHOOT5, MG42_UPSHOOT4, MG42_UPSHOOT3, MG42_UPSHOOT2, MG42_UPSHOOT1 };

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shell_mg42.mdl" );

	if( bCallAnim )
	{
		if( EV_IsLocal( idx ) )
		{
			int iAnimIndex = ( iBulletsRemaining <= 8 ) ? iBulletsRemaining : 8;

			if( gHUD.IsInMGDeploy() )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_DownAnims[iAnimIndex], EV_GetWeaponBody() );
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation( MG42_UpAnims[iAnimIndex], EV_GetWeaponBody() );

			gHUD.DoRecoil( WEAPON_MG42 );
		}

		EV_MuzzleFlash( idx, MG );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg42_shoot.wav", 
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 2, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_MG42, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    MG42 END
//======================

//======================
//	    MG34 START
//======================
void EV_FireMG34( event_args_t *args )
{
	int idx = args->entindex;
	int iAnim = args->iparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		int iAnimIndex;

		if( gHUD.IsInMGDeploy() )
			iAnimIndex = MG34_DOWNSHOOTEMPTY - ( iAnim > 0 ? 1 : 0 );
		else
			iAnimIndex = MG34_UPSHOOTEMPTY - ( iAnim > 0 ? 1 : 0 );

		gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnimIndex, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_MG34 );
	}

	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mg34_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 2, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_MG34, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    MG34 END
//======================

//======================
//	    30CAL START
//======================
void EV_Fire30CAL( event_args_t *args )
{
	int idx = args->entindex;
	int iBulletsRemaining = args->iparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	static const int ThirtyCal_DownAnims[9] = { CAL30_DOWNSHOOT, CAL30_DOWNSHOOT8, CAL30_DOWNSHOOT7, CAL30_DOWNSHOOT6, CAL30_DOWNSHOOT5, CAL30_DOWNSHOOT4, CAL30_DOWNSHOOT3, CAL30_DOWNSHOOT2, CAL30_DOWNSHOOT1 };
	static const int ThirtyCal_UpAnims[9] = { CAL30_UPSHOOT, CAL30_UPSHOOT8, CAL30_UPSHOOT7, CAL30_UPSHOOT6, CAL30_UPSHOOT5, CAL30_UPSHOOT4, CAL30_UPSHOOT3, CAL30_UPSHOOT2, CAL30_UPSHOOT1 };

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		int iAnimIndex = ( iBulletsRemaining <= 8 ) ? iBulletsRemaining : 8;

		if( gHUD.IsInMGDeploy() )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_DownAnims[iAnimIndex], EV_GetWeaponBody() );
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ThirtyCal_UpAnims[iAnimIndex], EV_GetWeaponBody() );

		gHUD.DoRecoil( WEAPON_CAL30 );
	}

	EV_MuzzleFlash( idx, MG );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/30cal_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_30CAL, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    30CAL END
//======================

//======================
//	   GARAND START
//======================
void EV_FireGarand( event_args_t *args )
{
	// test link
	gEngfuncs.Con_Printf( ">>> READY idx: %d, iparam1: %d\n", args->entindex, args->iparam1 );
	gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_STATIC, "common/wpn_hudon.wav", 1.0f, ATTN_NORM, 0, 100 );
	// end

	int idx = args->entindex;
	int empty = args->bparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( args->iparam1 == 1 )
	{
		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( GARAND_SMASH_EMPTY - ( empty == 0 ? 1 : 0 ), EV_GetWeaponBody() );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			int iAnimIndex = GARAND_SHOOT_EMPTY;

			if( !empty )
				iAnimIndex = gEngfuncs.pfnRandomLong( 0, 2 ) + GARAND_SHOOT1;

			gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnimIndex, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_GARAND );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/garand_shoot.wav", 
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		if( empty )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/garand_reload_clipding.wav", 
				gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
			BULLET_PLAYER_GARAND, 0, &g_tracerCount[idx - 1] );
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
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/carbine_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_M1CARBINE, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	 M1CARBINE END
//======================

//======================
//	 SCOPEDKAR START
//======================
void EV_FireScopedKar( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// int scoped;
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/kar_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_SCOPEDKAR, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	 SCOPEDKAR END
//======================

//======================
//	 THOMPSON START
//======================
void EV_FireThompson( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/thompson_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_THOMPSON, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	  THOMPSON END
//======================

//======================
//	   MP44 START
//======================
void EV_FireMP44( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp44_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_MP44, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    MP44 END
//======================

//======================
//	  GREASEGUN START
//======================
void EV_FireGreaseGun( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/greasegun_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_GREASEGUN, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	  GREASEGUN END
//======================

//======================
//	    FG42 START
//======================
void EV_FireFG42( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// int scoped;
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_FG42, 0, &g_tracerCount[idx - 1] ); 

	EV_ResetAnimationEvents( idx );
}

//======================
//	    FG42 END
//======================

//======================
//	    K43 START
//======================
void EV_FireK43( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( args->iparam1 == 1 )
	{
		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( K43_SMASH, EV_GetWeaponBody() );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knifeswing.wav", 
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( gEngfuncs.pfnRandomLong( 0, 1 ) + K43_SHOOT1, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_K43 );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/k43_shoot.wav", 
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
			BULLET_PLAYER_K43, 0, &g_tracerCount[idx - 1] );
	}

	EV_ResetAnimationEvents( idx );
}

//======================
//	     K43 END
//======================

//======================
//	   ENFIELD START
//======================
void EV_FireEnfield( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( args->iparam1 == 1 )
	{
		float flSound = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );

		if( flSound == 1.0f || flSound == 3.0f )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/knifeswing2.wav",
				gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), 0.8f, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
		else if( flSound == 2.0f )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/knifeswing.wav",
				gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), 0.8f, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
		else
		{
			return;
		}

		if( EV_IsLocal( idx ) )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SWING, EV_GetWeaponBody() );
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( ENFIELD_SHOOT, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_ENFIELD );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/enfield_shoot.wav",
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), 0.8f, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
			BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1] );
	}

	EV_ResetAnimationEvents( idx );
}

//======================
//	   ENFIELD END
//======================

//======================
//	   STEN START
//======================
void EV_FireSten( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;
	// vec3_t vecSpread;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( STEN_SHOOT, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_STEN );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sten_shoot.wav", gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_STEN, 0, &g_tracerCount[idx - 1] ); 

	EV_ResetAnimationEvents( idx );
}

//======================
//	    STEN END
//======================

//======================
//	   BREN START
//======================
void EV_FireBren( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bren_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_BREN, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
//	    BREN END
//======================

//======================
//	   WEBLEY START
//======================
void EV_FireWebley( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/webley_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_WEBLEY, 0, &g_tracerCount[idx - 1] );

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
	int idx = args->iparam1;
	vec3_t origin, angles, endpos;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	endpos[0] = args->fparam1;
	endpos[1] = args->fparam2;
	endpos[2] = 0.0f;

	if( idx <= 6 )
	{
		int model = gEngfuncs.pEventAPI->EV_FindModelIndex( sHelmetModels[idx] );

		if( model )
		{
			TEMPENTITY *helmetmdl = gEngfuncs.pEfxAPI->R_TempModel( &origin[0], &angles[0], &endpos[0], 5.0f, model, 1 );

			if( helmetmdl )
				helmetmdl->flags |= ( FTENT_ROTATE | FTENT_COLLIDEWORLD | FTENT_HITSOUNDPHYSICS );
		}
	}
}

//======================
//	  POPHELMET END
//======================

//======================
//	   KNIFE START
//======================
int g_iSwing = 0;

void EV_Knife( event_args_s *args )
{
	int idx = args->entindex;
	int temp = gEngfuncs.pfnRandomLong( 0, 1 );

	vec3_t origin;
	VectorCopy( args->origin, origin );

	float flRandomPitch = 94.0f + gEngfuncs.pfnRandomFloat( 0.0f, 15.0f );

	if( temp == 1 )
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash2.wav", 
			1.0f, ATTN_NORM, 0, ( int ) flRandomPitch );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife_slash1.wav", 
			1.0f, ATTN_NORM, 0, ( int ) flRandomPitch );
	}

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( 1, EV_GetWeaponBody() );

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
void EV_FireMP40( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	GetViewEntity();

	if( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MP40_SHOOT1, EV_GetWeaponBody() );
		gHUD.DoRecoil( WEAPON_MP40 );
	}

	EV_MuzzleFlash( idx, SUBMACHINE );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp40_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_MP40, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
// 	    MP40 END
//======================

//======================
// 	   SPRING START
//======================
void EV_FireSpring( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spring_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_SPRING, 0, &g_tracerCount[idx - 1] );

	EV_ResetAnimationEvents( idx );
}

//======================
// 	   SPRING END
//======================

//======================
// 	   KAR START
//======================
void EV_FireKar( event_args_t *args )
{
	int idx = args->entindex;
	int FireMode = args->iparam1;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, right, up );

	if( FireMode == 1 )
	{
		float flSound = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );

		if( flSound == 1.0f || flSound == 3.0f )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/knifeswing2.wav",
				gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

			if( EV_IsLocal( idx ) )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SLASH, EV_GetWeaponBody() );
		}
		else if( flSound == 2.0f )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/knifeswing.wav",
				gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

			if( EV_IsLocal( idx ) )
				gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SWING, EV_GetWeaponBody() );
		}
		else
			return;
	}
	else
	{
		if( EV_IsLocal( idx ) )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation( KAR_SHOOT, EV_GetWeaponBody() );
			gHUD.DoRecoil( WEAPON_KAR );
		}

		EV_MuzzleFlash( idx, RIFLE );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, args->origin, CHAN_WEAPON, "weapons/kar_shoot.wav",
			gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

		EV_GetGunPosition( args, vecSrc, origin );

		VectorCopy( forward, vecAiming );

		vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
			BULLET_PLAYER_KAR, 0, &g_tracerCount[idx - 1] );
	}

	EV_ResetAnimationEvents( idx );
}

//======================
// 	   KAR START
//======================

//======================
// 	   BAR START
//======================
void EV_FireBAR( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/bar_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_BAR, 0, &g_tracerCount[idx - 1] );

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
	int idx = args->entindex;

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
	int idx = args->entindex;

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
	int idx = args->entindex;

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
	int idx = args->entindex;

	vec3_t origin;
	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MORTAR_DEPLOYED_FIRE, EV_GetWeaponBody() );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mortar_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_MuzzleFlash( idx, MG );

	EV_ResetAnimationEvents( idx );
}
//======================
// 	   MORTAR END
//======================

//======================
// 	SCOPEDENFIELD START
//======================
void EV_FireScopedEnfield( event_args_t *args )
{
	int idx = args->entindex;
	float flSpread_x = args->fparam1;
	float flSpread_y = args->fparam2;

	// are not used
	// int scoped;
	// vec3_t velocity;
	// vec3_t ShellVelocity;
	// vec3_t ShellOrigin;

	vec3_t origin, angles;
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

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/enfieldsniper_shoot.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );

	vec3_t vecSpread = { flSpread_x, flSpread_y, 0.0f };

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, vecSpread, 8192.0f,
		BULLET_PLAYER_ENFIELD, 0, &g_tracerCount[idx - 1] );

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

void EV_DoDCamera( event_args_t *args )
{
	int idx = args->entindex;
	int scopeBins = args->iparam1;
	// int zoomLevel = args->bparam1;
	// int wideScreen;

	if( EV_IsLocal( idx ) )
	{
		if( scopeBins == 1 )
			gHUD.m_Scope.SetScope( WEAPON_BINOC );
		else if( scopeBins == 2 )
			gHUD.m_Scope.SetScope( WEAPON_SPRING );
		else
			gHUD.m_Scope.SetScope( WEAPON_NONE );
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
	model_s *pSprite;
	vec3_t org;
	CDoDParticle *pParticle;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/grenade_flash.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( pSprite )
	{
		gEngfuncs.GetLocalPlayer();

		VectorCopy( args->origin, org );

		org.z += 16.0f;

		vec3_t p_normal = { 0.0f, 0.0f, 0.0f };

		pParticle = pParticle->Create( &org, &p_normal, pSprite, 100.0f, 234.0f, "dod_particle", 0 );

		if( pParticle )
		{
			pParticle->SetCollisionFlags( ( 1 << 13 ) | TRI_COLLIDEDAMP | TRI_COLLIDESLIDE );

			int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
			pParticle->SetCullFlag( iParticleFlags );
			pParticle->SetLightFlag( iParticleFlags );

			pParticle->m_iPFlags = 0;
			pParticle->m_iRendermode = kRenderTransAdd;
			pParticle->m_iFrame = 0;
			pParticle->m_iFramerate = 60;
			pParticle->m_flScaleSpeed = 2.0f;

			pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 2.0f;
		}
	}
	else
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", "sprites/grenade_flash.spr" );
	}
}

void PlayExplosionSound( float *origin )
{
	int iSound = gEngfuncs.pfnRandomLong( 0, 2 );

	const char *pSoundName;

	switch( iSound )
	{
	case 0:
		pSoundName = "weapons/explode3.wav";
		break;
	case 1:
		pSoundName = "weapons/explode4.wav";
		break;
	case 2:
		pSoundName = "weapons/explode5.wav";
		break;
	default:
		pSoundName = "weapons/explode3.wav";
		break;
	}

	float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );
	int iPitch = 98 + gEngfuncs.pfnRandomLong( 0, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, pSoundName, flVolume, ATTN_NORM, 0, iPitch );
}

void PlayMortarExplosionSound( float *origin )
{
	int iSound = gEngfuncs.pfnRandomLong( 0, 2 );

	const char *pSoundName;

	switch( iSound )
	{
	case 0:
		pSoundName = "weapons/mortar_hit1.wav";
		break;
	case 1:
		pSoundName = "weapons/mortar_hit2.wav";
		break;
	case 2:
		pSoundName = "weapons/mortar_hit3.wav";
		break;
	default:
		pSoundName = "weapons/mortar_hit1.wav";
		break;
	}

	float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );
	int iPitch = 98 + gEngfuncs.pfnRandomLong( 0, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( 0, origin, CHAN_WEAPON, pSoundName, flVolume, ATTN_NORM, 0, iPitch );
}

void EV_BaseExplosion( event_args_t *args )
{
	float *origin = args->origin;

	if( args->bparam2 )
		PlayMortarExplosionSound( origin );
	else
		PlayExplosionSound( origin );

	P_ExplosionFlash( args );

	if( cl_dynamiclights && cl_dynamiclights->value > 0.0f )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		if( dl )
		{
			VectorCopy( args->origin, dl->origin );

			dl->radius = 400.0f;
			dl->decay = 275.0f;
			dl->color.r = 248;
			dl->color.g = 255;
			dl->color.b = 120;
			dl->die = gHUD.m_flTime + 3.0f;
		}
	}
}

void EV_WaterExplosion( event_args_t *args )
{
	model_s *pSprite;
	float flHeight;
	vec3_t vOrigin;
	vec3_t vStart;
	float flScale;
	CDoDDirtExploDust *pSmoke;
	CDoDParticle *pParticle;

	vec3_t vNormal = { 0.0f, 0.0f, 1.0f };
	vec3_t vAngles, vForward, vRight, vUp;

	VectorCopy( args->origin, vOrigin );

	VectorAngles( vNormal, vAngles );
	AngleVectors( vAngles, vForward, vRight, vUp );

	vRight.Normalize();
	vUp.Normalize();

	HSPRITE hDirtPuff = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/dirt_puff.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hDirtPuff );


	float flWaterHeightArgs[3] = { vOrigin.x, vOrigin.y, vOrigin.z };
	float flWaterSurfaceZ = EV_HLDM_WaterHeight( vOrigin, vOrigin.z, vOrigin.z + 512.0f ) - vOrigin.z + vOrigin.z;

	for( flHeight = 0.0f; flHeight < 350.0f; flHeight += 10.0f )
	{
		flScale = gEngfuncs.pfnRandomFloat( 40.0f, 64.0f );

		float flRandX = gEngfuncs.pfnRandomFloat( -30.0f, 50.0f );
		float flRandY = gEngfuncs.pfnRandomFloat( -30.0f, 50.0f );

		if( pSprite )
		{
			vec3_t p_normal = { 0.0f, 0.0f, 1.0f };

			vStart.x = flRandX + vOrigin.x;
			vStart.y = flRandY + vOrigin.y;
			vStart.z = flHeight + flWaterSurfaceZ;

			pSmoke = pSmoke->Create( &vStart, &p_normal, pSprite, flScale, 255.0f, "dod_dirtexplo" );

			if( pSmoke )
			{
				pSmoke->m_iRendermode = kRenderTransTexture;
				pSmoke->m_flGravity = 0.2f;
				pSmoke->m_flSize = flScale;

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pSmoke->SetCullFlag( iParticleFlags );
				pSmoke->SetLightFlag( iParticleFlags );

				pSmoke->m_vAVelocity.x = 0.0f;
				pSmoke->m_vAVelocity.y = 0.0f;
				pSmoke->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );

				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_iFrame = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );
				pSmoke->m_flScaleSpeed = gEngfuncs.pfnRandomFloat( 2.5f, 3.5f );

				pSmoke->m_vColor.x = 200.0f;
				pSmoke->m_vColor.y = 200.0f;
				pSmoke->m_vColor.z = 225.0f;

				float flSpreadUp = gEngfuncs.pfnRandomFloat( 25.0f, 45.0f );
				float flSpeedUp = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * flSpreadUp;

				float flSpreadRight = gEngfuncs.pfnRandomFloat( 25.0f, 45.0f );
				float flSpeedRight = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * flSpreadRight;

				float flVerticalImpulse = gEngfuncs.pfnRandomFloat( 450.0f, 750.0f );

				pSmoke->m_vVelocity.x = ( vNormal.x * flVerticalImpulse ) + flSpeedRight * vRight.x + flSpeedUp * vUp.x;
				pSmoke->m_vVelocity.y = ( vNormal.y * flVerticalImpulse ) + flSpeedRight * vRight.y + flSpeedUp * vUp.y;
				pSmoke->m_vVelocity.z = ( vNormal.z * flVerticalImpulse ) + flSpeedRight * vRight.z + flSpeedUp * vUp.z;

				pSmoke->m_bFire = false;
				pSmoke->m_flFadeSpeed = -1.0f;
				pSmoke->m_flActivateTime = gEngfuncs.pfnRandomFloat( 0.07f, 0.11f );
			}
		}
	}

	HSPRITE hWaterBig = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/water_big.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hWaterBig );

	flScale = gEngfuncs.pfnRandomFloat( 400.0f, 550.0f );

	vec3_t p_normal = { 90.0f, 0.0f, 0.0f };

	vStart.x = vOrigin.x;
	vStart.y = vOrigin.y;
	vStart.z = ( 0.5f * flScale ) - 10.0f + flWaterSurfaceZ;

	pParticle = pParticle->Create( &vStart, &p_normal, pSprite, flScale, 255.0f, "dod_particle", 0 );

	if( pParticle )
	{
		pParticle->m_iRendermode = kRenderTransAdd;

		int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
		pParticle->SetCullFlag( iParticleFlags );
		pParticle->SetLightFlag( iParticleFlags | LIGHT_COLOR );

		pParticle->m_vColor.x = 255.0f;
		pParticle->m_vColor.y = 255.0f;
		pParticle->m_vColor.z = 255.0f;

		pParticle->m_flGravity = 0.5f;
		pParticle->m_flFadeSpeed = 10.0f;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;

		pParticle->m_vVelocity.x = 0.0f;
		pParticle->m_vVelocity.y = 0.0f;
		pParticle->m_vVelocity.z = gEngfuncs.pfnRandomFloat( 400.0f, 500.0f );
	}
}

void EV_DirtTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
	model_s *pSprite;
	vec3_t vOrigin;
	float flScale;
	float flSpeed;
	CDoDRocketTrail *pSmoke;

	if( currenttime >= ent->entity.baseline.fuser2 )
	{
		ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime();

		HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" );
		pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

		VectorCopy( ent->entity.origin, vOrigin );

		flScale = ent->entity.baseline.fuser1 - 7.0f;
		ent->entity.baseline.fuser1 = flScale;

		if( flScale < 10.0f )
		{
			ent->entity.baseline.fuser1 = 10.0f;
			flScale = 10.0f;
		}

		float flRandX = gEngfuncs.pfnRandomFloat( -60.0f, 60.0f );
		float flRandY = gEngfuncs.pfnRandomFloat( -60.0f, 60.0f );
		float flRandZ = gEngfuncs.pfnRandomFloat( -60.0f, 60.0f );

		vec3_t vecBaseOrigin = { ent->entity.baseline.origin[0], ent->entity.baseline.origin[1], ent->entity.baseline.origin[2] };
		flSpeed = vecBaseOrigin.Length() * 0.9f;

		vecBaseOrigin.Normalize();
		ent->entity.baseline.origin[0] = vecBaseOrigin.x * flSpeed;
		ent->entity.baseline.origin[1] = vecBaseOrigin.y * flSpeed;
		ent->entity.baseline.origin[2] = vecBaseOrigin.z * flSpeed;

		if( pSprite )
		{
			vec3_t p_normal = { 0.0f, 0.0f, 1.0f };
			vec3_t p_org;

			p_org.x = vOrigin.x + flRandX;
			p_org.y = vOrigin.y + flRandY;
			p_org.z = vOrigin.z + flRandZ;

			pSmoke = pSmoke->Create( &p_org, &p_normal, pSprite, flScale, 135.0f, "dod_trailsmoke" );

			if( pSmoke )
			{
				pSmoke->m_flGravity = gEngfuncs.pfnRandomFloat( 0.15f, 0.25f );
				pSmoke->m_flSize = flScale;

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pSmoke->SetCullFlag( iParticleFlags );
				pSmoke->SetLightFlag( iParticleFlags );

				pSmoke->SetCollisionFlags( TRI_WATERTRACE | TRI_COLLIDEDAMP );

				pSmoke->m_iRendermode = kRenderTransTexture;
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 8.0f;
				pSmoke->m_iFrame = gEngfuncs.pfnRandomFloat( 4.0f, 8.0f );
				pSmoke->m_bRocketTrail = false;

				pSmoke->m_vColor.x = 155.0f;
				pSmoke->m_vColor.y = 155.0f;
				pSmoke->m_vColor.z = 140.0f;

				pSmoke->m_vAVelocity.x = 0.0f;
				pSmoke->m_vAVelocity.y = 0.0f;
				pSmoke->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );

				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 3.0f );
			}
		}
	}
}

void EV_DirtRubble( event_args_t *args )
{
	TEMPENTITY *pTrailSpawner;

	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

	if( pTrailSpawner )
	{
		pTrailSpawner->flags |= ( FTENT_GRAVITY | FTENT_COLLIDEWORLD | FTENT_CLIENTCUSTOM | FTENT_HITSOUNDPHYSICS
			| FTENT_PERSISTPHYSICS );

		pTrailSpawner->callback = EV_DirtTrailCallback;
		pTrailSpawner->die = gEngfuncs.GetClientTime() + 2.0f;
		pTrailSpawner->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.pfnRandomFloat( 175.0f, 200.0f );

		float flSpeedX = ( gEngfuncs.pfnRandomLong( 0, 1 ) == 0 ) ? -400.0f : 450.0f;
		pTrailSpawner->entity.baseline.origin[0] = flSpeedX;

		float flSpeedY = ( gEngfuncs.pfnRandomLong( 0, 1 ) == 0 ) ? -400.0f : 450.0f;
		pTrailSpawner->entity.baseline.origin[1] = flSpeedY;

		pTrailSpawner->entity.baseline.origin[2] = gEngfuncs.pfnRandomFloat( 500.0f, 1000.0f );
	}
}

void EV_DirtExplosion( event_args_t *args )
{
	model_s *pSprite;
	vec3_t vOrigin;
	float flScale;
	CDoDDirtExploDust *pSmoke;
	CDoDParticle *pParticle;
	int iRandom;

	vec3_t vStart, vEnd;
	VectorCopy( args->origin, vStart );
	VectorCopy( vStart, vEnd );
	vEnd.z -= 64.0f;

	pmtrace_t *pTrace = gEngfuncs.PM_TraceLine( vStart, vEnd, 8, 2, -1 );
	VectorCopy( pTrace->endpos, vOrigin );

	HSPRITE hDirtPuff = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/dirt_puff.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hDirtPuff );

	vec3_t vAngles, vForward, vRight, vUp;
	VectorAngles( pTrace->plane.normal, vAngles );
	AngleVectors( vAngles, vForward, vRight, vUp );

	vRight.Normalize();
	vUp.Normalize();

	for( float flHeight = 0.0f; flHeight < 350.0f; flHeight += 10.0f )
	{
		flScale = gEngfuncs.pfnRandomFloat( 60.0f, 90.0f );

		float flRandX = gEngfuncs.pfnRandomFloat( -48.0f, 48.0f );
		float flRandY = gEngfuncs.pfnRandomFloat( -48.0f, 48.0f );

		if( pSprite )
		{
			vec3_t p_normal = { 0.0f, 0.0f, 1.0f };
			vec3_t p_org;

			p_org.x = flRandX + vOrigin.x;
			p_org.y = flRandX + vOrigin.y;
			p_org.z = flHeight + vOrigin.z;

			pSmoke = pSmoke->Create( &p_org, &p_normal, pSprite, flScale, 225.0f, "dod_dirtexplo" );

			if( pSmoke )
			{
				pSmoke->m_iRendermode = kRenderTransTexture;
				pSmoke->m_flGravity = 0.25f;
				pSmoke->m_flSize = flScale;

				pSmoke->SetCollisionFlags( TRI_WATERTRACE | TRI_COLLIDEDAMP );

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pSmoke->SetCullFlag( iParticleFlags );
				pSmoke->SetLightFlag( iParticleFlags );

				pSmoke->m_vAVelocity.x = 0.0f;
				pSmoke->m_vAVelocity.y = 0.0f;
				pSmoke->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 4.0f );

				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 4.0f );
				pSmoke->m_iFrame = 0;
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_flScaleSpeed = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );

				pSmoke->m_vColor.x = 155.0f;
				pSmoke->m_vColor.y = 155.0f;
				pSmoke->m_vColor.z = 140.0f;

				float flSpreadUp = gEngfuncs.pfnRandomFloat( 25.0f, 45.0f );
				float flSpeedUp = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) * flSpreadUp;

				float flSpreadRight = gEngfuncs.pfnRandomFloat( 25.0f, 45.0f );
				float flSpeedRight = gEngfuncs.pfnRandomFloat( -1.0f, 1.0f ) *flSpreadRight;
				float flVerticalImpulse = gEngfuncs.pfnRandomFloat( 450.0f, 750.0f );

				pSmoke->m_vVelocity.x = ( pTrace->plane.normal[0] * flVerticalImpulse ) + ( vRight.x * flSpeedRight ) + ( vUp.x * flSpeedUp );
				pSmoke->m_vVelocity.y = ( pTrace->plane.normal[1] * flVerticalImpulse ) + ( vRight.y * flSpeedRight ) + ( vUp.y * flSpeedUp );
				pSmoke->m_vVelocity.z = ( pTrace->plane.normal[2] * flVerticalImpulse ) + ( vRight.z * flSpeedRight ) + ( vUp.z * flSpeedUp );

				pSmoke->m_bFire = false;
				pSmoke->m_flFadeSpeed = -1.0f;
				pSmoke->m_flActivateTime = gEngfuncs.pfnRandomFloat( 0.06f, 0.12f );
			}
		}
	}

	HSPRITE hExploFlash = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/explosion_flash.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hExploFlash );

	flScale = gEngfuncs.pfnRandomFloat( 100.0f, 150.0f );

	vec3_t p_normal = { 90.0f, 0.0f, 0.0f };

	pParticle = pParticle->Create( &vStart, &p_normal, pSprite, flScale, 255.0f, "dod_particle", 0 );

	if( pParticle )
	{
		pParticle->m_iRendermode = kRenderTransTexture;

		int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
		pParticle->SetCullFlag( iParticleFlags );
		pParticle->SetLightFlag( iParticleFlags );

		pParticle->m_vColor.x = 255.0f;
		pParticle->m_vColor.y = 255.0f;
		pParticle->m_vColor.z = 200.0f;

		pParticle->m_flGravity = 0.0f;
		pParticle->m_flFadeSpeed = 36.0f;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 0.1f;
		pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 500.0f );

		P_Rubble( args );

		iRandom = gEngfuncs.pfnRandomLong( 4, 6 );

		if( iRandom > 0 )
		{
			for( int j = 0; j < iRandom; ++j )
			{
				EV_DirtRubble( args );
			}
		}
	}
}

void EV_SandExplosion( event_args_t *args )
{
	EV_DirtExplosion( args );
}

void EV_Explosion( event_args_t *args )
{
	vec3_t down;
	char chTextureType;

	if( args->bparam1 )
	{
		EV_WaterExplosion( args );

		float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );
		int iPitch = 98 + gEngfuncs.pfnRandomLong( 0, 3 );

		gEngfuncs.pEventAPI->EV_PlaySound( 0, args->origin, CHAN_WEAPON, "weapons/explodewater.wav", flVolume, ATTN_NORM, 0, iPitch );
		return;
	}

	down.x = 0.0f;
	down.y = 0.0f;
	down.z = -1.0f;

	vec3_t end;
	VectorMA( args->origin, 256.0f, down, end );

	pmtrace_t *pTrace = gEngfuncs.PM_TraceLine( args->origin, end, 0, 2, -1 );
	const char *pTextureName = gEngfuncs.pEventAPI->EV_TraceTexture( pTrace->ent, args->origin, end );

	if( pTextureName )
	{
		char szTextureName[64];
		char szCleanName[64];

		strcpy( szTextureName, pTextureName );
		char *pCurrentChar = szTextureName;

		if( szTextureName[0] == '+' || szTextureName[0] == '-' )
			pCurrentChar = &szTextureName[1];

		if( *pCurrentChar == '{' || *pCurrentChar == '}' || *pCurrentChar == '~' || *pCurrentChar == '!' || *pCurrentChar == ' ' )
			pCurrentChar++;

		strcpy( szCleanName, pCurrentChar );

		chTextureType = PM_FindTextureType( szCleanName );

		if( chTextureType == 'D' || chTextureType == 'A' )
		{
			EV_DirtExplosion( args );
		}
		else if( chTextureType == 'L' || chTextureType == 'C' || chTextureType == 'R' || chTextureType == 'B' )
		{
			vec3_t p_origin;
			VectorCopy( args->origin, p_origin );
			CreateExplosionSmokeInside( p_origin );
		}
		else
		{
			vec3_t p_origin;
			VectorCopy( args->origin, p_origin );
			CreateExplosionSmokeInside( p_origin );
		}
	}
	else
	{
		P_Rubble( args );

		vec3_t p_origin;
		VectorCopy( args->origin, p_origin );
		CreateExplosionSmokeInside( p_origin );
	}

	if( args->bparam2 )
		PlayMortarExplosionSound( args->origin );
	else
		PlayExplosionSound( args->origin );

	P_ExplosionFlash( args );

	if( cl_dynamiclights && cl_dynamiclights->value > 0.0f )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		if( dl )
		{
			VectorCopy( args->origin, dl->origin );

			dl->radius = 400.0f;
			dl->decay = 275.0f;
			dl->color.r = 248;
			dl->color.g = 255;
			dl->color.b = 120;
			dl->die = gHUD.m_flTime + 3.0f;
		}
	}
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
	float height = args->fparam1;
	float speed = args->fparam2;
	int count = args->iparam2;
	int model = args->iparam1;

	vec3_t vecFrom, vecTo;
	VectorCopy( args->origin, vecFrom );
	VectorCopy( args->angles, vecTo );

	gEngfuncs.pEfxAPI->R_BubbleTrail( vecFrom, vecTo, height, model, count, speed );
}

void EV_BulletTracers( event_args_t *args )
{
	vec3_t vecFrom, vecTo;
	VectorCopy( args->origin, vecFrom );
	VectorCopy( args->angles, vecTo );

	gEngfuncs.pEfxAPI->R_TracerEffect( vecFrom, vecTo );
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
	int idx = args->entindex;
	int color = args->iparam2;
	int model = args->iparam1;
	int model2 = (int)args->fparam2;
	float size = args->fparam1;

	vec3_t origin;
	VectorCopy( args->origin, origin );

	gEngfuncs.pEfxAPI->R_BloodSprite( origin, color, model, model2, size );
}

void EV_Smoke( event_args_t *args )
{
	vec3_t vOrigin;
	vec3_t vPOrigin;

	vec3_t vAnglesDummy = { 0.0f, 90.0f, 0.0f };
	vec3_t vForwardDummy, vRightDummy, vUpDummy;

	AngleVectors( vAnglesDummy, vForwardDummy, vRightDummy, vUpDummy );

	VectorCopy( args->origin, vOrigin );
	VectorCopy( vOrigin, vPOrigin );

	CreateExplosionSmokeInside( vPOrigin );
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
	int idx = args->entindex;
	int voicenum = args->iparam1;
	int i_isBrit = args->iparam2;
	vec3_t v_playerDistance;
	cl_entity_t *ent;
	cl_entity_t *thisplayer;
	char *subtitle;

	vec3_t origin;
	VectorCopy( args->origin, origin );

	if( !GetClientVoice() || !GetClientVoice()->IsPlayerBlocked( idx ) )
	{
		const char *pVoiceFile;

		if( i_isBrit )
		{
			pVoiceFile = s_BRITVoiceFiles[voicenum];
		}
		else
		{
			pVoiceFile = s_USVoiceFiles[voicenum];
		}

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_VOICE, pVoiceFile, gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), 
			ATTN_NORM, 0, 100 );

		GetPlayerInfo( idx, &g_PlayerInfoList[idx] );

		ent = gEngfuncs.GetEntityByIndex( idx );
		thisplayer = gEngfuncs.GetLocalPlayer();

		int team = ent->curstate.team;

		if( team == g_iTeamNumber && !g_iUser1 && team == thisplayer->curstate.team )
		{
			VectorSubtract( origin, thisplayer->curstate.origin, v_playerDistance );
			float len = v_playerDistance.Length();

			char pattern[256];

			if( len <= 1100.0f )
			{
				GetPlayerInfo( idx, &g_PlayerInfoList[idx] );
				sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[idx].name, ": %s2" );

				subtitle = s_VoiceCommands[voicenum][3];

				if( !subtitle || !*subtitle || !gHUD.m_bBritish )
				{
					subtitle = s_VoiceCommands[voicenum][1];
				}

				gHUD.m_SayText.SayTextPrint( pattern, 256, idx, "#VOICE", subtitle, 0, 0 );

				GetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				gHUD.m_Spectator.AddVoiceIconToPlayerEnt( idx );
			}
		}
	}
}

extern char *s_GERVoiceFiles[];

void EV_GERVoice( event_args_t *args )
{
	int idx = args->entindex;
	int voicenum = args->iparam1;
	vec3_t v_playerDistance;
	cl_entity_t *ent;
	cl_entity_t *thisplayer;
	char *subtitle;

	vec3_t origin;
	VectorCopy( args->origin, origin );

	if( !GetClientVoice() || !GetClientVoice()->IsPlayerBlocked( idx ) )
	{
		if( voicenum == 27 && gHUD.m_bBritish )
		{
			voicenum = 28;
		}

		float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );

		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_BODY, s_GERVoiceFiles[voicenum], flVolume, ATTN_NORM, 0, 100 );

		ent = gEngfuncs.GetEntityByIndex( idx );
		thisplayer = gEngfuncs.GetLocalPlayer();

		int team = ent->curstate.team;

		if( team == g_iTeamNumber && !g_iUser1 && team == thisplayer->curstate.team )
		{
			VectorSubtract( origin, thisplayer->curstate.origin, v_playerDistance );
			float len = v_playerDistance.Length();

			char pattern[256];

			if( len <= 1100.0f )
			{
				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );
				sprintf( pattern, "%c%s%s%s\n", 2, "(%s1) ", g_PlayerInfoList[idx].name, ": %s2" );

				if( s_VoiceCommands[voicenum][2] && *s_VoiceCommands[voicenum][2] )
					subtitle = CHudTextMessage::BufferedLocaliseTextString( s_VoiceCommands[voicenum][2] );
				else
					subtitle = CHudTextMessage::BufferedLocaliseTextString( s_VoiceCommands[voicenum][1] );

				gHUD.m_SayText.SayTextPrint( pattern, 256, idx, "#VOICE", subtitle );

				gEngfuncs.pfnGetPlayerInfo( idx, &g_PlayerInfoList[idx] );

				gHUD.m_Spectator.AddVoiceIconToPlayerEnt( idx );
			}
		}
	}
}

void EV_BodyDamage( event_args_t *args )
{
	int idx, bodyarea;

	vec3_t origin;

	idx = args->entindex;
	bodyarea = args->iparam1;

	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, "player/helmet_hit.wav", 
		gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0, 100 );
}

void EV_RoundReleaseSound( event_args_t *args )
{
	int idx = args->entindex;
	int teamwav = args->iparam1;
	int randomgen = args->iparam2;
	int isbritish = args->bparam1;
	const char *pszString;

	vec3_t origin;
	VectorCopy( args->origin, origin );

	if( EV_IsLocal( idx ) && !g_iVuser1z )
	{
		if( isbritish && teamwav == 1 )
		{
			if( randomgen > 3 )
				return;

			switch( randomgen )
			{
			case 0: 
				pszString = "player/britstartround.wav"; 
				break;
			case 1: 
				pszString = "player/britstartround2.wav"; 
				break;
			default: 
				pszString = '\0';
				break;
			}
		}

		if( !isbritish && teamwav == 1 )
		{
			if( randomgen > 3 )
				return;

			switch( randomgen )
			{
			case 0: 
				pszString = "player/usstartround.wav"; 
				break;
			case 1: 
				pszString = "player/usstartround2.wav"; 
				break;
			default: 
				pszString = '\0';
				break;
			}
		}

		if( teamwav == 2 && randomgen <= 3 )
		{
			switch( randomgen )
			{
			case 0: 
				pszString = "player/gerstartround.wav"; 
				break;
			case 1: 
				pszString = "player/gerstartround2.wav"; 
				break;
			default: 
				pszString = '\0';
				break;
			}
		}

		if( pszString )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, &origin.x, CHAN_STATIC, pszString, 1.0f, ATTN_NORM, 0, 100 );
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

void P_Rubble( event_args_t *args )
{
	int ammount;
	vec3_t vOrigin;
	float fSize;
	int iTextureType = 0;

	VectorCopy( args->origin, vOrigin );

	vec3_t vNormal = { 0.0f, 0.0f, 1.0f };
	ammount = gEngfuncs.pfnRandomLong( 32, 42 );

	if( ammount >= 0 )
	{
		for( int i = 0; i <= ammount; ++i )
		{
			if( g_pParticleMan && !g_RubbleQueue.Full() )
			{
				vec3_t vecVelocity;

				vecVelocity.x = gEngfuncs.pfnRandomFloat( -400.0f, 450.0f );
				vecVelocity.y = gEngfuncs.pfnRandomFloat( -400.0f, 450.0f );
				vecVelocity.z = gEngfuncs.pfnRandomFloat( 450.0f, 750.0f );

				fSize = vecVelocity.x;

				CreateFlyingRubble( vOrigin, true, vecVelocity.x, vecVelocity.y, vecVelocity.z, fSize, iTextureType, vNormal );
			}
		}
	}

	ammount = gEngfuncs.pfnRandomLong( 8, 16 );

	if( ammount >= 0 )
	{
		for( int j = 0; j <= ammount; ++j )
		{
			if( g_pParticleMan && !g_RubbleQueue.Full() )
			{
				vec3_t vecVelocity;

				vecVelocity.x = gEngfuncs.pfnRandomFloat( -300.0f, 350.0f );
				vecVelocity.y = gEngfuncs.pfnRandomFloat( -300.0f, 350.0f );
				vecVelocity.z = gEngfuncs.pfnRandomFloat( 250.0f, 450.0f );

				fSize = vecVelocity.x;

				CreateFlyingRubble( vOrigin, false, vecVelocity.x, vecVelocity.y, vecVelocity.z, fSize, iTextureType, vNormal );
			}
		}
	}
}

void P_ExplosionSmoke( event_args_t *args )
{
	vec3_t vPOrigin;

	vec3_t vAngles = { 0.0f, 270.0f, 0.0f };
	vec3_t vForward, vRight, vUp;

	gEngfuncs.pfnAngleVectors( vAngles, vForward, vRight, vUp );

	VectorCopy( args->origin, vPOrigin );

	CreateExplosionSmokeInside( vPOrigin );
}

char EV_FindHitTexture( float *start, float *end )
{
	pmtrace_t tr;
	const char *pTextureName;

	gEngfuncs.PM_TraceLine( start, end, 0, 2, -1 );

	tr = *gEngfuncs.PM_TraceLine( start, end, 0, 2, -1 );

	pTextureName = gEngfuncs.pEventAPI->EV_TraceTexture( tr.ent, start, end );

	if( pTextureName )
		return EV_TexNameToType( ( char * ) pTextureName );

	return 'C';
}

char EV_TexNameToType( char *pTextureName )
{
	char type;
	char texname[64];
	char szbuffer[64];

	type = 'C';

	if( pTextureName )
	{
		strcpy( texname, pTextureName );
		char *pCurrentChar = texname;

		if( texname[0] == '+' || texname[0] == '-' )
			pCurrentChar = &texname[2];

		if( *pCurrentChar == '!' || *pCurrentChar == '{' || *pCurrentChar == '}' || *pCurrentChar == '~' || *pCurrentChar == ' ' )

			pCurrentChar++;

		strcpy( szbuffer, pCurrentChar );

		szbuffer[12] = '\0';

		return PM_FindTextureType( szbuffer );
	}

	return type;
}

void CreateFlyingRubble( vec3_t origin, bool bLargeRubble, float vVelocityx, float vVelocityy, float vVelocityz, float fSize, int iTextureType, vec3_t vNormal )
{
	char *sprname;
	model_s *pSprite;
	CDoDParticle *pParticle;

	if( !g_pParticleMan || g_RubbleQueue.Full() )
		return;

	sprname = "sprites/rock_small.spr";

	if( bLargeRubble )
		sprname = "sprites/rock_big.spr";

	if( iTextureType != 0 )
		sprname = "sprites/dirt.spr";

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( sprname );
	pSprite = ( model_s * )gEngfuncs.GetSpritePointer( hSprite );

	if( !pSprite )
	{
		gEngfuncs.Con_DPrintf( "Couldn't load Sprite: %s\n", sprname );
		return;
	}

	pParticle = pParticle->Create( &origin, &vNormal, pSprite, fSize, 190.0f, "dod_particle", 1 );

	if( pParticle )
	{
		g_RubbleQueue.Add( gHUD.m_flTime );

		pParticle->m_vVelocity.x = vVelocityx;
		pParticle->m_vVelocity.y = vVelocityy;
		pParticle->m_vVelocity.z = vVelocityz;

		pParticle->m_vAngles.z = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );

		pParticle->SetCollisionFlags( 0x33020u );
		pParticle->SetLightFlag( 0 );
		pParticle->SetCullFlag( 1 );

		pParticle->m_iRendermode = kRenderTransTexture;
		pParticle->m_flScaleSpeed = 0.0f;
		pParticle->m_flDampingTime = 0.0f;
		pParticle->m_iFrame = 0;
		pParticle->m_flMass = 1.0f;
		pParticle->m_flGravity = 0.5f;
		pParticle->m_iPFlags = 5120;
		pParticle->m_flDieTime = gEngfuncs.GetClientTime() + 7.0f;
	}
}

void EV_RocketTrailCallback( tempent_s *ent, float frametime, float currenttime )
{
	model_s *pSprite;
	vec3_t vOrigin;
	float flScale;
	vec3_t vVelocity;
	CDoDRocketTrail *pSmoke;
	dlight_t *dl;

	if( currenttime >= ent->entity.baseline.fuser2 )
	{
		ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.02f;

		HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/effects/adrian/bazooka_smoke.spr" );
		pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

		VectorCopy( ent->entity.origin, vOrigin );

		flScale = gEngfuncs.pfnRandomFloat( 30.0f, 45.0f );

		float flRandX = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );
		float flRandY = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );
		float flRandZ = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );

		vVelocity.x = ent->entity.baseline.vuser1[0];
		vVelocity.y = ent->entity.baseline.vuser1[1];
		vVelocity.z = ent->entity.baseline.vuser1[2];

		if( ent->entity.baseline.iuser4 > 0 )
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
				ent->entity.baseline.vuser1[1] = gEngfuncs.pfnRandomFloat( -90.0f, 90.0f );
				ent->entity.baseline.vuser1[2] = gEngfuncs.pfnRandomFloat( -90.0f, 90.0f );
				ent->entity.baseline.vuser1[0] = ( float ) gEngfuncs.pfnRandomLong( -25, 25 );
			}

			ent->entity.baseline.iuser4 = gEngfuncs.pfnRandomLong( 5, 15 );
		}

		if( ent->entity.baseline.iuser4 < 0 )
			ent->entity.baseline.iuser4 = 0;

		if( pSprite )
		{
			vec3_t p_normal = { 0.0f, 0.0f, 1.0f };
			vec3_t p_org;

			p_org.x = vOrigin.x + flRandX;
			p_org.y = vOrigin.y + flRandY;
			p_org.z = vOrigin.z + flRandZ;

			pSmoke = pSmoke->Create( &p_org, &p_normal, pSprite, flScale, 200.0f, "dod_trailsmoke" );

			if( pSmoke )
			{
				pSmoke->m_flGravity = -0.01f;
				pSmoke->m_iRendermode = kRenderTransTexture;
				pSmoke->m_flSize = flScale;

				pSmoke->SetCollisionFlags( TRI_WATERTRACE );

				int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
				pSmoke->SetCullFlag( iParticleFlags );
				pSmoke->SetLightFlag( iParticleFlags );

				pSmoke->m_vAVelocity.x = 0.0f;
				pSmoke->m_vAVelocity.y = 0.0f;
				pSmoke->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 4.0f );

				pSmoke->m_flMass = gEngfuncs.pfnRandomFloat( 2.0f, 4.0f );
				pSmoke->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;
				pSmoke->m_flScaleSpeed = 2.0f;

				pSmoke->m_vColor.x = 25.0f;
				pSmoke->m_vColor.y = 25.0f;
				pSmoke->m_vColor.z = 25.0f;

				pSmoke->m_iFrame = gEngfuncs.pfnRandomFloat( 10.0f, 20.0f );
				pSmoke->m_bRocketTrail = true;
				pSmoke->m_flFadeSpeed = -1.0f;

				VectorCopy( vVelocity, pSmoke->m_vVelocity );
			}
		}
	}

	if( ent->entity.baseline.fuser1 > currenttime )
		return;

	if( VectorCompare2( ent->entity.origin, ent->entity.attachment[0] ) )
		ent->die = gEngfuncs.GetClientTime();
	else
		VectorCopy( ent->entity.origin, ent->entity.attachment[0] );

	if( cl_dynamiclights && cl_dynamiclights->value > 0.0f )
	{
		dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

		if( dl )
		{
			VectorCopy( ent->entity.origin, dl->origin );

			dl->radius = 160.0f;
			dl->dark = 1;
			dl->die = gEngfuncs.GetClientTime() + 0.001f;
			dl->color.r = 255;
			dl->color.g = 255;
			dl->color.b = 255;
		}
	}
}

void EV_RocketTrail( event_args_t *args )
{
	int iEntIndex = args->entindex;
	TEMPENTITY *pTrailSpawner;

	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

	if( pTrailSpawner )
	{
		pTrailSpawner->flags |= ( FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD | TRI_WIND | FTENT_HITSOUNDPHYSICS 
			| FTENT_PERSISTPHYSICS );
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
		int iPitch = 95 + gEngfuncs.pfnRandomLong( 0, 10 );
		float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );

		gEngfuncs.pEventAPI->EV_PlaySound( 0, &ent->entity.origin.x, CHAN_BODY, "weapons/mortar_incoming.wav", 
			flVolume, ATTN_NORM, 0, iPitch );

		ent->entity.baseline.iuser1 = 1;
	}

	VectorAngles( temp, ent->entity.angles );
}

void EV_MortarShell( event_args_t *args )
{
	model_s *pModel;
	TEMPENTITY *pMortarShell;
	int modelindex[4];

	pModel = ( model_s * ) gEngfuncs.CL_LoadModel( "models/w_piat_rocket.mdl", modelindex );

	pMortarShell = gEngfuncs.pEfxAPI->CL_TempEntAlloc( args->origin, pModel );

	if( pMortarShell )
	{
		pMortarShell->flags = ( FTENT_CLIENTCUSTOM | FTENT_PERSISTPHYSICS | FTENT_ARC_BALLISTICS );
		pMortarShell->callback = EV_MortarShellCallback;
		pMortarShell->die = gEngfuncs.GetClientTime() + 15.0f;

		VectorScale( args->angles, 1000.0f, pMortarShell->entity.baseline.origin );

		pMortarShell->entity.baseline.iuser1 = 0;
		pMortarShell->entity.baseline.gravity = args->fparam1;
	}
}


void EV_OverheatCallback( tempent_s *ent, float frametime, float currenttime )
{
	model_s *pSprite;
	vec3_t B;
	vec3_t vBarrelOffset;
	vec3_t vOrigin;
	vec3_t vVelocity;
	cl_entity_t *viewent;
	cl_entity_t *playerEnt;
	vec3_t up;
	float flScale;
	CBaseDoDParticle *pParticle;

	if( currenttime < ent->entity.baseline.fuser2 )
		return;

	ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.2f;

	HSPRITE hSprite = gEngfuncs.pfnSPR_Load( "sprites/puff.spr" );
	pSprite = ( model_s * ) gEngfuncs.GetSpritePointer( hSprite );

	if( !EV_IsLocal( ent->clientIndex ) )
	{
		playerEnt = gEngfuncs.GetEntityByIndex( ent->clientIndex );

		if( playerEnt && playerEnt->player )
		{
			vec3_t vForward, vRight;
			AngleVectors( playerEnt->curstate.angles, vForward, vRight, up );

			if( playerEnt->curstate.iuser4 == WEAPON_MG34 || playerEnt->curstate.iuser4 == WEAPON_MG42 )
			{
				VectorMA( playerEnt->curstate.origin, 15.0f, vForward, vOrigin );
				VectorMA( playerEnt->curstate.origin, 60.0f, vForward, vBarrelOffset );
				VectorMA( vOrigin, -6.0f, up, vOrigin );
				VectorMA( vBarrelOffset, -6.0f, up, vBarrelOffset );
			}
			else
			{
				VectorMA( playerEnt->curstate.origin, 5.0f, vForward, vOrigin );
				VectorMA( playerEnt->curstate.origin, 30.0f, vForward, vBarrelOffset );
				VectorMA( vOrigin, 13.0f, up, vOrigin );
				VectorMA( vBarrelOffset, 13.0f, up, vBarrelOffset );
			}
		}
		else
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}
	}
	else
	{
		if( g_flWeaponHeat < 94.0f )
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}

		if( gHUD.m_vecVelocity.Length() > 20.0f )
			return;

		viewent = gEngfuncs.GetViewModel();

		if( !viewent )
		{
			ent->die = gEngfuncs.GetClientTime();
			return;
		}

		VectorCopy( viewent->attachment[0], vOrigin );
		VectorCopy( viewent->attachment[1], vBarrelOffset );
	}

	VectorSubtract( vOrigin, vBarrelOffset, B );

	float flInterpolateWeight = gEngfuncs.pfnRandomFloat( 0.05f, 0.8f );
	vec3_t p_org;

	VectorMA( vBarrelOffset, flInterpolateWeight, B, p_org );

	flScale = gEngfuncs.pfnRandomFloat( 3.0f, 3.0f );

	vVelocity.x = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );
	vVelocity.y = gEngfuncs.pfnRandomFloat( -4.0f, 5.0f );
	vVelocity.z = gEngfuncs.pfnRandomFloat( 0.0f, 3.0f );

	if( pSprite )
	{
		vec3_t p_normal = { 90.0f, 0.0f, 0.0f };

		CDoDParticle *pFactory = NULL;
		pParticle = pFactory->Create( &p_org, &p_normal, pSprite, flScale, 180.0f, "dod_particle", 0 );

		if( pParticle )
		{
			CDoDParticle *pDoDParticle = ( CDoDParticle * ) pParticle;

			pDoDParticle->m_iRendermode = kRenderTransTexture;

			int iParticleFlags = TRI_COLLIDEBRUSHENTS | TRI_SPIRAL | TRI_ANIMATEDIE;
			pDoDParticle->SetCullFlag( iParticleFlags );
			pDoDParticle->SetLightFlag( iParticleFlags );

			pDoDParticle->m_flGravity = -0.015f;
			pDoDParticle->m_flFadeSpeed = 2.0f;
			pDoDParticle->m_flScaleSpeed = 1.0f;
			pDoDParticle->m_flDieTime = gEngfuncs.GetClientTime() + 10.0f;

			pDoDParticle->m_vColor.x = 255.0f;
			pDoDParticle->m_vColor.y = 255.0f;
			pDoDParticle->m_vColor.z = 255.0f;

			pDoDParticle->m_vAVelocity.x = 0.0f;
			pDoDParticle->m_vAVelocity.y = 0.0f;
			pDoDParticle->m_vAVelocity.z = gEngfuncs.pfnRandomFloat( -4.0f, 4.0f );

			pDoDParticle->m_iFrame = gEngfuncs.pfnRandomFloat( 2.0f, 4.0f );
			pDoDParticle->m_flFadeSpeed = -1.0f;

			VectorCopy( vVelocity, pDoDParticle->m_vVelocity );

			CDoDRocketTrail *pRocketTrail = ( CDoDRocketTrail * ) pParticle;
			pRocketTrail->m_bRocketTrail = true;
		}
	}
}

float lastOverheatTime;
extern cvar_t *cl_particlefx;

void EV_Overheat( event_args_t *args )
{
	float lastTime;
	TEMPENTITY *pBarrelSpawner;

	float flVolume = gEngfuncs.pfnRandomFloat( 0.92f, 1.0f );
	int iPitch = 98 + gEngfuncs.pfnRandomLong( 0, 3 );

	gEngfuncs.pEventAPI->EV_PlaySound( args->entindex, args->origin, CHAN_ITEM, "weapons/mgoverheat.wav", 
		flVolume, ATTN_NORM, 0, iPitch );

	lastTime = gEngfuncs.GetClientTime() - lastOverheatTime;
	lastOverheatTime = gEngfuncs.GetClientTime();

	if( ( lastTime < 0.0f || lastTime >= 1.0f ) && cl_particlefx && cl_particlefx->value >= 2.0f )
	{
		pBarrelSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( args->origin );

		if( pBarrelSpawner )
		{
			pBarrelSpawner->flags |= ( FTENT_CLIENTCUSTOM | TRI_WIND | FTENT_HITSOUNDPHYSICS | FTENT_PERSISTPHYSICS );
			pBarrelSpawner->callback = EV_OverheatCallback;
			pBarrelSpawner->clientIndex = args->entindex;
			pBarrelSpawner->die = gEngfuncs.GetClientTime() + 2.5f;
			pBarrelSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.5f;
			pBarrelSpawner->entity.baseline.fuser2 = gEngfuncs.GetClientTime();
		}
	}
}