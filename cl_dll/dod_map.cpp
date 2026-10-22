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
// dod_map.cpp
//
// implementation of the CHudDoDMap class: the map (big and small) of the overview
//

#include <math.h>

#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "com_model.h"
#include "demo_api.h"
#include "entity_types.h"
#include "pm_shared.h"

extern vec3_t v_sim_org;
extern vec3_t v_cl_angles;

extern void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles );

extern cvar_t *cl_dmsmallmap;

extern int g_iTeamNumber;

// The team of the icons that the map shows. A spectator has no team of his own, so the original shows the icon of the
// Allies and no players at all while he watches somebody; the fix follows the team of the watched player.
static int GetMapTeam( void )
{
#if MINIMAP_SPECTATOR_FIX
	if( g_iUser1 && g_iUser2 > 0 )
	{
		cl_entity_t *pTarget = gEngfuncs.GetEntityByIndex( g_iUser2 );

		if( pTarget && pTarget->player )
			return pTarget->curstate.team;
	}
#endif

	return g_iTeamNumber;
}

bool b_turnedoffmap;

int flZoomLevels[3] = { 1, 3, 8 };

int CHudDoDMap::Draw( float flTime )
{
	return 1;
}

void CHudDoDMap::SetMapState( int mapstate )
{
	gHUD.SetMinimapState( mapstate );
}

int CHudDoDMap::VidInit( void )
{
	int zoomlevel;

	b_turnedoffmap = false;

	zoomlevel = gHUD.GetMinimapZoomLevel();

	m_flIdealMapScale = (float)flZoomLevels[zoomlevel];
	m_flPreviousMapScale = m_flIdealMapScale;

	memset( gHUD.m_Spectator.m_OverviewEntities, 0, sizeof( gHUD.m_Spectator.m_OverviewEntities ) );
	memset( m_ExtraOverviewEntities, 0, sizeof( m_ExtraOverviewEntities ) );

	gHUD.m_Spectator.m_MapSprite = NULL;
	gHUD.m_Spectator.LoadMapSprites();

	gHUD.InitMapBounds();

	return 1;
}

int CHudDoDMap::Init( void )
{
	int zoomlevel;

	gHUD.AddHudElem( this );

	m_iFlags |= HUD_ACTIVE;

	zoomlevel = gHUD.GetMinimapZoomLevel();

	m_flSmallMapScale = (float)flZoomLevels[zoomlevel];
	m_flIdealMapScale = m_flSmallMapScale;
	m_flPreviousMapScale = m_flSmallMapScale;

	return 1;
}

void CHudDoDMap::InitHUDData( void )
{
	/* Nothing. */
}

void CHudDoDMap::DrawOverview( void )
{
	float time;

	if( gHUD.m_iIntermission )
		return;

	// the zoom of the small map is animated
	time = gEngfuncs.GetClientTime() - m_flZoomTime;

	if( time < 1.0f )
	{
		float fraction = PM_SplineFraction( time, 1.0f );

		m_flSmallMapScale = fraction * ( m_flIdealMapScale - m_flPreviousMapScale ) + m_flPreviousMapScale;
	}

	gHUD.m_Spectator.m_mapZoom = gHUD.m_Spectator.m_OverviewData.zoom;
	gHUD.m_Spectator.m_mapOrigin = gHUD.m_Spectator.m_OverviewData.origin;

	DrawOverviewLayer();
	DrawOverviewEntities();
	CheckOverviewEntities();
}

void CHudDoDMap::DrawOverviewLayer( void )
{
	float screenaspect, xs, ys, xStep, yStep, x, y, z;
	int ix, iy, i, xTiles, yTiles, frame;

	qboolean hasMapImage = gHUD.m_Spectator.m_MapSprite ? TRUE : FALSE;
	model_t *dummySprite = (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_Spectator.m_hsprUnkownMap );
	float alpha;
	float scale;
	float points[32][32][3];

	if( hasMapImage )
	{
		i = gHUD.m_Spectator.m_MapSprite->numframes / ( 4 * 3 );
		i = (int)sqrt( (double)i );
		xTiles = i * 4;
		yTiles = i * 3;
	}
	else
	{
		xTiles = 8;
		yTiles = 6;
	}

	screenaspect = 4.0f / 3.0f;

	xs = gHUD.m_Spectator.m_OverviewData.origin[0];
	ys = gHUD.m_Spectator.m_OverviewData.origin[1];
	z = 0.0f;

	if( gHUD.GetMinimapState() == 2 )
	{
		// the small map is centered at the player
		GetSmallMapOffset( xs, ys, z );

		alpha = 0.6f;
		scale = m_flSmallMapScale;
	}
	else
	{
		alpha = 1.0f;
		scale = 1.0f;
	}

	gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
	gEngfuncs.pTriAPI->CullFace( TRI_FRONT );
	gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, alpha );

	xStep = -( 2 * 4096.0f / gHUD.m_Spectator.m_OverviewData.zoom ) / xTiles;
	yStep = -( 2 * 4096.0f / ( gHUD.m_Spectator.m_OverviewData.zoom * screenaspect ) ) / yTiles;

	x = xs + ( 4096.0f / ( gHUD.m_Spectator.m_OverviewData.zoom * screenaspect ) );

	// the corners of the tiles
	for( i = 0; i <= yTiles && i < 32; i++ )
	{
		y = ys + ( 4096.0f / gHUD.m_Spectator.m_OverviewData.zoom );

		for( ix = 0; ix <= xTiles && ix < 32; ix++ )
		{
			points[i][ix][0] = x * scale;
			points[i][ix][1] = y * scale;
			points[i][ix][2] = z * scale;

			y += yStep;
		}

		x += xStep;
	}

	frame = 0;

	for( iy = 1; iy <= yTiles; iy++ )
	{
		for( ix = 0; ix < xTiles; ix++ )
		{
			if( hasMapImage )
				gEngfuncs.pTriAPI->SpriteTexture( gHUD.m_Spectator.m_MapSprite, frame );
			else
				gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

			frame++;

			gEngfuncs.pTriAPI->Begin( TRI_QUADS );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				gEngfuncs.pTriAPI->Vertex3f( points[iy - 1][ix][0], points[iy - 1][ix][1], points[iy - 1][ix][2] );

				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				gEngfuncs.pTriAPI->Vertex3f( points[iy - 1][ix + 1][0], points[iy - 1][ix + 1][1], points[iy - 1][ix + 1][2] );

				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				gEngfuncs.pTriAPI->Vertex3f( points[iy][ix + 1][0], points[iy][ix + 1][1], points[iy][ix + 1][2] );

				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
				gEngfuncs.pTriAPI->Vertex3f( points[iy][ix][0], points[iy][ix][1], points[iy][ix][2] );
			gEngfuncs.pTriAPI->End();
		}
	}
}

bool CHudDoDMap::AddMapEntityToMap( HSPRITE sprite, double lifeTime, Vector origin )
{
	for( int i = 0; i < MAX_EXTRA_MAP_ICONS; i++ )
	{
		// find empty entity slot
		if( !m_ExtraOverviewEntities[i].hSprite )
		{
			m_ExtraOverviewEntities[i].hSprite = sprite;
			m_ExtraOverviewEntities[i].killTime = gEngfuncs.GetClientTime() + lifeTime;
			m_ExtraOverviewEntities[i].origin = origin;
			return true;
		}
	}

	return false;
}

void CHudDoDMap::DrawOverviewIcon( Vector origin, HSPRITE hIcon, int iconScale, Vector angles )
{
	vec3_t right, up;
	model_t *pModel;
	vec3_t point;

	AngleVectors( angles, right, up, NULL );

	gEngfuncs.pTriAPI->CullFace( TRI_FRONT );

	pModel = (struct model_s *)gEngfuncs.GetSpritePointer( hIcon );

	right = right * (float)iconScale * 16.0f;
	up = up * (float)iconScale * 16.0f;

	gEngfuncs.pTriAPI->SpriteTexture( pModel, 0 );

	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
		point = origin + up + right;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
		point = origin + up - right;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
		point = origin - up - right;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
		point = origin - up + right;
		gEngfuncs.pTriAPI->Vertex3fv( point );
	gEngfuncs.pTriAPI->End();
}

void CHudDoDMap::DrawOverviewEntities( void )
{
	int i;
	float origin[3];
	vec3_t angles;
	float sizeScale;
	cl_entity_t *ent;
	double time;
	bool bSmallMapVisible;
	float temp;
	int iconscale;
	model_t *pVoiceIcon;
	HSPRITE hIcon;
	int iMapTeam;

	time = gEngfuncs.GetClientTime();

	bSmallMapVisible = gHUD.GetMinimapState() == 2;

	sizeScale = 12.0f;

	gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );

	// the markers that the players have set on the map
	for( i = 0; i < MAX_EXTRA_MAP_ICONS; i++ )
	{
		if( !m_ExtraOverviewEntities[i].hSprite || time > m_ExtraOverviewEntities[i].killTime )
			continue;

		origin[0] = m_ExtraOverviewEntities[i].origin.x;
		origin[1] = m_ExtraOverviewEntities[i].origin.y;
		origin[2] = 0.0f;

		angles = Vector( 0.0f, 90.0f, 0.0f );

		if( gHUD.m_Spectator.m_OverviewData.rotated )
		{
			temp = gHUD.m_Spectator.m_OverviewData.origin[0] + ( origin[1] - gHUD.m_Spectator.m_OverviewData.origin[1] );
			origin[1] = gHUD.m_Spectator.m_OverviewData.origin[1] - ( origin[0] - gHUD.m_Spectator.m_OverviewData.origin[0] );
			origin[0] = temp;
		}

		if( bSmallMapVisible )
		{
			GetSmallMapOffset( origin[0], origin[1], origin[2] );

			angles.x = 180.0f;

			sizeScale = m_flSmallMapScale * 10.0f;

			if( sizeScale > 40.0f )
				sizeScale = 40.0f;

			origin[0] *= m_flSmallMapScale;
			origin[1] *= m_flSmallMapScale;
		}
		else
		{
			sizeScale = 16.0f;
		}

		DrawOverviewIcon( Vector( origin[0], origin[1], 0.0f ), m_ExtraOverviewEntities[i].hSprite, (int)sizeScale, angles );
	}

	iconscale = bSmallMapVisible ? 25 : 10;
	iMapTeam = GetMapTeam();

	// the entities of the overview: the players of the team, the objectives ...
	for( i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if( !gHUD.m_Spectator.m_OverviewEntities[i].hSprite )
			continue;

		if( gHUD.m_Spectator.m_OverviewEntities[i].killTime - 10.0 > time )
			continue;

		ent = gHUD.m_Spectator.m_OverviewEntities[i].entity;

		if( ent->player )
		{
			if( ent->curstate.team != iMapTeam )
				continue;

			angles = ent->curstate.angles;
		}
		else if( ent->baseline.iuser1 == 1 )
		{
			angles = Vector( 0.0f, 90.0f, 0.0f );
		}
		else
		{
			angles = ent->curstate.angles;
		}

		if( gHUD.m_Spectator.m_OverviewEntities[i].b_dodMapTag )
		{
			origin[0] = gHUD.m_Spectator.m_OverviewEntities[i].origin.x;
			origin[1] = gHUD.m_Spectator.m_OverviewEntities[i].origin.y;
		}
		else
		{
			origin[0] = ent->origin[0];
			origin[1] = ent->origin[1];
		}

		origin[2] = 0.0f;

		if( ent->player )
		{
			// only the yaw of the players is used
			angles.x = 0.0f;
			angles.z = 0.0f;
		}

		if( gHUD.m_Spectator.m_OverviewData.rotated )
		{
			temp = gHUD.m_Spectator.m_OverviewData.origin[0] + ( origin[1] - gHUD.m_Spectator.m_OverviewData.origin[1] );
			origin[1] = gHUD.m_Spectator.m_OverviewData.origin[1] - ( origin[0] - gHUD.m_Spectator.m_OverviewData.origin[0] );
			origin[0] = temp;

			if( ent->player )
				angles.y += 90.0f;
		}
		else if( ent->player )
		{
			angles.y += 180.0f;
		}

		if( bSmallMapVisible )
		{
			GetSmallMapOffset( origin[0], origin[1], origin[2] );

			if( ent->player )
			{
				sizeScale = 40.0f;
			}
			else if( ent->curstate.entityType == ENTITY_BEAM )
			{
				sizeScale = m_flSmallMapScale * 10.0f;

				if( sizeScale > 40.0f )
					sizeScale = 40.0f;
			}
			else
			{
				sizeScale = m_flSmallMapScale * 15.0f;

				if( sizeScale > 45.0f )
					sizeScale = 45.0f;
			}

			origin[0] *= m_flSmallMapScale;
			origin[1] *= m_flSmallMapScale;
		}

		DrawOverviewIcon( Vector( origin[0], origin[1], 0.0f ), gHUD.m_Spectator.m_OverviewEntities[i].hSprite, (int)sizeScale, angles );

		if( gHUD.m_Spectator.ShouldSetVoiceIcon( ent->index ) )
			ent->baseline.fuser4 = (float)( time + 1.6 );

		// the speaker icon over the players that talk
		if( ( ent->player && gHUD.m_StatusBar.IsEntityTalking( ent->index ) ) || ent->baseline.fuser4 > time )
		{
			pVoiceIcon = (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_Spectator.m_hsprSpeakerIcon );

			DrawOverviewIcon( Vector( origin[0], origin[1], 0.0f ), gHUD.m_Spectator.m_hsprSpeakerIcon, iconscale, Vector( 0.0f, 90.0f, 0.0f ) );
		}
	}

	// the camera icon of the own position
	origin[0] = v_sim_org.x;
	origin[1] = v_sim_org.y;
	origin[2] = 0.0f;

	if( g_iUser1 == OBS_CHASE_LOCKED )
	{
		float chaseOrigin[3];

		V_GetChasePos( g_iUser2, NULL, chaseOrigin, angles );
	}
	else
	{
		angles.y = v_cl_angles.y;
	}

	angles.x = 0.0f;
	angles.z = 0.0f;

	if( gHUD.m_Spectator.m_OverviewData.rotated )
	{
		temp = gHUD.m_Spectator.m_OverviewData.origin[0] + ( origin[1] - gHUD.m_Spectator.m_OverviewData.origin[1] );
		origin[1] = gHUD.m_Spectator.m_OverviewData.origin[1] - ( origin[0] - gHUD.m_Spectator.m_OverviewData.origin[0] );
		origin[0] = temp;

		angles.y += 90.0f;
	}
	else
	{
		angles.y += 180.0f;
	}

	if( gEngfuncs.IsSpectateOnly() )
	{
		hIcon = gHUD.m_Spectator.m_hsprCameraSpec;
	}
	else if( iMapTeam == 2 )
	{
		hIcon = gHUD.m_Spectator.m_hsprCameraAxis;
	}
	else if( gHUD.m_bBritish )
	{
		hIcon = gHUD.m_Spectator.m_hsprCameraBrit;
	}
	else
	{
		hIcon = gHUD.m_Spectator.m_hsprCameraAllies;
	}

	if( hIcon )
	{
		if( bSmallMapVisible )
		{
			GetSmallMapOffset( origin[0], origin[1], origin[2] );

			DrawOverviewIcon( Vector( origin[0], origin[1], 0.0f ), hIcon, 50, angles );
		}
		else
		{
			DrawOverviewIcon( Vector( origin[0], origin[1], 0.0f ), hIcon, 16, angles );
		}
	}
}

void CHudDoDMap::CheckOverviewEntities( void )
{
	double time = gEngfuncs.GetClientTime();
	int i;

	for( i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if( time > gHUD.m_Spectator.m_OverviewEntities[i].killTime )
			memset( &gHUD.m_Spectator.m_OverviewEntities[i], 0, sizeof( overviewEntity_t ) );
	}

	for( i = 0; i < MAX_EXTRA_MAP_ICONS; i++ )
	{
		if( m_ExtraOverviewEntities[i].hSprite && time > m_ExtraOverviewEntities[i].killTime )
			memset( &m_ExtraOverviewEntities[i], 0, sizeof( map_icon_t ) );
	}
}

void CHudDoDMap::HandleMapButton( void )
{
	// no map while the view is zoomed
	if( gHUD.m_iFOV <= 89 )
		return;

	if( gHUD.GetMinimapState() )
	{
		if( gHUD.GetMinimapState() == 1 && cl_dmsmallmap->value >= 1.0f )
			gHUD.SetMinimapState( 2 );
		else
			gHUD.SetMinimapState( 0 );
	}
	else
	{
		gHUD.SetMinimapState( 1 );
	}
}

void CHudDoDMap::HandleMapZoomButton( void )
{
	if( gHUD.m_iFOV > 89 && gHUD.GetMinimapState() == 2 )
	{
		int newlevel = gHUD.ZoomMinimap();

		m_flPreviousMapScale = m_flIdealMapScale;
		m_flIdealMapScale = (float)flZoomLevels[newlevel];
		m_flZoomTime = gEngfuncs.GetClientTime();
	}
}

void CHudDoDMap::GetSmallMapOffset( float &x, float &y, float &z )
{
	vec3_t origin;

	if( gHUD.m_Spectator.m_OverviewData.rotated )
	{
		vec3_t newOrigin;

		newOrigin.x = gHUD.m_Spectator.m_OverviewData.origin[0] + ( v_sim_org.y - gHUD.m_Spectator.m_OverviewData.origin[1] );
		newOrigin.y = gHUD.m_Spectator.m_OverviewData.origin[1] - ( v_sim_org.x - gHUD.m_Spectator.m_OverviewData.origin[0] );
		newOrigin.z = v_sim_org.z;

		origin = newOrigin;
	}
	else
	{
		origin = v_sim_org;
	}

	x = x - origin.x;
	y = y - origin.y;
}
