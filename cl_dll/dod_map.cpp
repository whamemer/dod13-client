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
//  dod_map.cpp - implementation of the CHudDoDMap class
//

#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "com_model.h"
#include "dod_shared.h"

extern vec3_t v_sim_org, v_cl_angles;
extern cvar_t *cl_dmsmallmap;
extern int g_iTeamNumber;
bool b_turnedoffmap;
int flZoomLevels[3];

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
	b_turnedoffmap = false;

	int zoomlevel = gHUD.GetMinimapZoomLevel();
	float flCurrentZoom = flZoomLevels[zoomlevel];

	m_flIdealMapScale = flCurrentZoom;
	m_flPreviousMapScale = flCurrentZoom;

	memset( gHUD.m_Spectator.m_OverviewEntities, 0, sizeof( gHUD.m_Spectator.m_OverviewEntities ) );
	memset( m_ExtraOverviewEntities, 0, sizeof( m_ExtraOverviewEntities ) );

	gHUD.m_Spectator.m_MapSprite = NULL;
	gHUD.m_Spectator.LoadMapSprites();

	gHUD.InitMapBounds();

	return 1;
}

int CHudDoDMap::Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_ACTIVE;

	int zoomlevel = gHUD.GetMinimapZoomLevel();
	float flCurrentZoom = flZoomLevels[zoomlevel];

	m_flSmallMapScale = flCurrentZoom;
	m_flIdealMapScale = flCurrentZoom;
	m_flPreviousMapScale = flCurrentZoom;

	return 1;
}

extern "C" float PM_SplineFraction(float value, float scale);

void CHudDoDMap::InitHUDData( void )
{
	// Nothing.
}

void CHudDoDMap::DrawOverview( void )
{
	if( gHUD.m_iIntermission )
		return;

	float time = gEngfuncs.GetClientTime();
	float flTimeDelta = time - m_flZoomTime;

	if( flTimeDelta < 1.0f )
	{
		float fraction = PM_SplineFraction( flTimeDelta, 1.0f );

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
	HSPRITE hsprUnkownMap = gHUD.m_Spectator.m_hsprUnkownMap;
	model_s *pMapSprite = gHUD.m_Spectator.m_MapSprite;
	model_s *dummySprite = NULL;

	int xTiles = 8;
	int yTiles = 6;
	qboolean hasMapImage = false;

	if( pMapSprite )
	{
		dummySprite = (model_s *)gEngfuncs.GetSpritePointer( hsprUnkownMap );

		int multiplier = sqrt( pMapSprite->numframes / 12 );
		xTiles = 4 * multiplier;
		yTiles = 3 * multiplier;
		hasMapImage = true;
	}
	else
	{
		dummySprite = (model_s *)gEngfuncs.GetSpritePointer( hsprUnkownMap );
	}

	float x = gHUD.m_Spectator.m_OverviewData.origin.x;
	float y = gHUD.m_Spectator.m_OverviewData.origin.y;
	float z = 0.0f;

	float alpha = 1.0f;
	float scale = 1.0f;

	if( gHUD.GetMinimapState() == 2 )
	{
		float playerX = v_sim_org.x;
		float playerY = v_sim_org.y;

		if( gHUD.m_Spectator.m_OverviewData.rotated )
		{
			float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
			float cy = gHUD.m_Spectator.m_OverviewData.origin.y;

			playerX = cx + v_sim_org.y - cy;
			playerY = cy - ( v_sim_org.x - cx );
		}

		alpha = 0.6f;
		x -= playerX;
		y -= playerY;
		scale = m_flSmallMapScale;
	}
	else
	{
		GetSmallMapOffset( x, y, z );
	}

	gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
	gEngfuncs.pTriAPI->CullFace( TRI_FRONT );
	gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, alpha );

	float zoom = gHUD.m_Spectator.m_OverviewData.zoom;
	float screenaspect = 1.33f;

	float xStep = -( 8192.0f / zoom ) / ( float ) xTiles;
	float yStep = -( 8192.0f / ( screenaspect * zoom ) ) / ( float ) yTiles;

	float xs = ( 4096.0f / ( screenaspect * zoom ) ) + x;
	float ys = ( 4096.0f / zoom ) + y;

	float points[32][32][3];
	float currentX = xs;

	for( int ix = 0; ix <= yTiles && ix < 32; ix++ )
	{
		float currentY = ys;
		for( int iy = 0; iy <= xTiles && iy < 32; iy++ )
		{
			points[ix][iy][0] = currentX * scale;
			points[ix][iy][1] = currentY * scale;
			points[ix][iy][2] = z * scale;

			currentY += yStep;
		}
		currentX += xStep;
	}

	int frame = 0;

	for( int ix = 1; ix <= yTiles && ix < 32; ix++ )
	{
		for( int iy = 0; iy < xTiles && iy < 31; iy++ )
		{
			if( hasMapImage )
				gEngfuncs.pTriAPI->SpriteTexture( pMapSprite, frame );
			else
				gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

			frame++;

			gEngfuncs.pTriAPI->Begin( TRI_QUADS );

			gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
			gEngfuncs.pTriAPI->Vertex3f( points[ix - 1][iy][0], points[ix - 1][iy][1], points[ix - 1][iy][2] );

			gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
			gEngfuncs.pTriAPI->Vertex3f( points[ix - 1][iy + 1][0], points[ix - 1][iy + 1][1], points[ix - 1][iy + 1][2] );

			gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
			gEngfuncs.pTriAPI->Vertex3f( points[ix][iy + 1][0], points[ix][iy + 1][1], points[ix][iy + 1][2] );

			gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
			gEngfuncs.pTriAPI->Vertex3f( points[ix][iy][0], points[ix][iy][1], points[ix][iy][2] );

			gEngfuncs.pTriAPI->End();
		}
	}
}

bool CHudDoDMap::AddMapEntityToMap( HSPRITE sprite, double lifeTime, Vector *p_origin )
{
	double flClientTime = gEngfuncs.GetClientTime();

	for( int i = 0; i < MAX_EXTRA_MAP_ICONS; i++ )
	{
		if( !m_ExtraOverviewEntities[i].hSprite )
		{
			m_ExtraOverviewEntities[i].hSprite = sprite;
			m_ExtraOverviewEntities[i].origin = *p_origin;
			m_ExtraOverviewEntities[i].killtime = flClientTime + lifeTime;

			return true;
		}
	}

	return false;
}

void CHudDoDMap::DrawOverviewIcon( vec3_t origin, HSPRITE hIcon, int iconScale, vec3_t angles )
{
	vec3_t right, up, point;

	AngleVectors( angles, right, up, NULL );

	gEngfuncs.pTriAPI->CullFace( TRI_FRONT );

	float scaleFactor = ( float ) iconScale * 16.0f;
	right = right * scaleFactor;
	up = up * scaleFactor;

	class model_s *pModel = ( class model_s * ) gEngfuncs.GetSpritePointer( hIcon );
	gEngfuncs.pTriAPI->SpriteTexture( pModel, 0 );

	gEngfuncs.pTriAPI->Begin( TRI_QUADS );

	gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
	point = origin + up + right;
	gEngfuncs.pTriAPI->Vertex3fv( &point.x );

	gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
	point = origin + up - right;
	gEngfuncs.pTriAPI->Vertex3fv( &point.x );

	gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
	point = origin - up - right;
	gEngfuncs.pTriAPI->Vertex3fv( &point.x );

	gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
	point = origin - up + right;
	gEngfuncs.pTriAPI->Vertex3fv( &point.x );

	gEngfuncs.pTriAPI->End();
}

void CHudDoDMap::DrawOverviewEntities( void )
{
	double time = gEngfuncs.GetClientTime();
	bool bSmallMapVisible = ( gHUD.GetMinimapState() == 2 );

	gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );

	for( int i = 0; i < MAX_EXTRA_MAP_ICONS; i++ )
	{
		map_icon_t *pExtraIcon = &m_ExtraOverviewEntities[i];

		if( pExtraIcon->hSprite && time <= pExtraIcon->killtime )
		{
			float x = pExtraIcon->origin.x;
			float y = pExtraIcon->origin.y;
			float z = pExtraIcon->origin.z;

			vec3_t org( x, y, z );
			vec3_t angles( 0.0f, 90.0f, 0.0f );

			if( gHUD.m_Spectator.m_OverviewData.rotated )
			{
				float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
				float cy = gHUD.m_Spectator.m_OverviewData.origin.y;

				org.x = cx + ( y - cy );
				org.y = cy - ( x - cx );
			}

			if( bSmallMapVisible )
			{
				float centerX = v_sim_org.x;
				float centerY = v_sim_org.y;

				if( gHUD.m_Spectator.m_OverviewData.rotated )
				{
					float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
					float cy = gHUD.m_Spectator.m_OverviewData.origin.y;
					centerX = cx + v_sim_org.y - cy;
					centerY = cy - ( v_sim_org.x - cx );
				}

				org.x = ( org.x - centerX ) * m_flSmallMapScale;
				org.y = ( org.y - centerY ) * m_flSmallMapScale;
				angles.x = 180.0f;
			}
			else
			{
				GetSmallMapOffset( org.x, org.y, org.z );
			}
			HSPRITE hIcon = pExtraIcon->hSprite;

			DrawOverviewIcon( org, hIcon, 1, angles );
		}
	}

	for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		overviewEntity_t *pOverviewEnt = &gHUD.m_Spectator.m_OverviewEntities[i];

		if( !pOverviewEnt->hSprite || ( pOverviewEnt->killTime - 10.0f ) > time )
			continue;

		cl_entity_t *ent = pOverviewEnt->entity;

		if( !ent )
			continue;

		if( !ent->player && ent->baseline.iuser1 != 1 )
			continue;

		if( ent->player && ent->curstate.team != g_iTeamNumber )
			continue;

		vec3_t angles( 0.0f, 0.0f, 0.0f );

		if( ent->player || ent->baseline.iuser1 == 1 )
			angles = ent->curstate.angles;

		if( !ent->player && ent->baseline.iuser1 == 1 )
		{
			angles.x = 0.0f;
			angles.y = 90.0f;
			angles.z = 0.0f;
		}

		vec3_t org;

		if( pOverviewEnt->b_dodMapTag )
			org = pOverviewEnt->origin;
		else
			org = ent->origin;

		bool rotated = gHUD.m_Spectator.m_OverviewData.rotated;

		if( rotated )
		{
			float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
			float cy = gHUD.m_Spectator.m_OverviewData.origin.y;
			float tx = org.x;
			float ty = org.y;

			org.x = cx + ty - cy;
			org.y = cy - ( tx - cx );

			if( ent->player )
				angles.y += 90.0f;
		}
		else
		{
			if( ent->player )
				angles.y += 180.0f;
		}

		if( bSmallMapVisible )
		{
			float centerX = v_sim_org.x;
			float centerY = v_sim_org.y;

			if( rotated )
			{
				float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
				float cy = gHUD.m_Spectator.m_OverviewData.origin.y;
				centerX = cx + v_sim_org.y - cy;
				centerY = cy - ( v_sim_org.x - cx );
			}

			org.x = ( org.x - centerX ) * m_flSmallMapScale;
			org.y = ( org.y - centerY ) * m_flSmallMapScale;
		}
		else
		{
			GetSmallMapOffset( org.x, org.y, org.z );
		}

		HSPRITE hIcon = pOverviewEnt->hSprite;
		this->DrawOverviewIcon( org, hIcon, 1, angles );

		if( gHUD.m_Spectator.ShouldSetVoiceIcon(  ent->index ) )
			ent->baseline.fuser4 = time + 1.6f;

		bool bIsTalking = gHUD.m_StatusBar.IsEntityTalking( ent->index );

		if( bIsTalking || ent->baseline.fuser4 > time )
		{
			vec3_t voiceAngles( 0.0f, 1127481344.0f, 0.0f );

			if( bSmallMapVisible )
				voiceAngles.x = 180.0f;
			else
				GetSmallMapOffset( org.x, org.y, org.z );

			HSPRITE hSpeaker = gHUD.m_Spectator.m_hsprSpeakerIcon;

			DrawOverviewIcon( org, hSpeaker, 1, voiceAngles );
		}
	}
}

void CHudDoDMap::CheckOverviewEntities( void )
{
	double time = gEngfuncs.GetClientTime();

	for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		overviewEntity_t *pOverviewEnt = &gHUD.m_Spectator.m_OverviewEntities[i];

		if( pOverviewEnt->entity && time > pOverviewEnt->killTime )
			memset( pOverviewEnt, 0, sizeof( overviewEntity_t ) );
	}

	for( int j = 0; j < MAX_EXTRA_MAP_ICONS; j++ )
	{
		map_icon_t *pExtraIcon = &m_ExtraOverviewEntities[j];

		if( pExtraIcon->hSprite && time > pExtraIcon->killtime )
			memset( pExtraIcon, 0, sizeof( map_icon_t ) );
	}
}

void CHudDoDMap::HandleMapButton( void )
{
	if( gHUD.m_iFOV > 89 )
	{
		int MinimapState = gHUD.GetMinimapState();

		if( MinimapState != 0 )
		{
			if( MinimapState == 1 && cl_dmsmallmap->value >= 1.0f )
			{
				gHUD.SetMinimapState( 2 );
			}
			else
			{
				gHUD.SetMinimapState( 0 );
			}
		}
		else
		{
			gHUD.SetMinimapState( 1 );
		}
	}
}

void CHudDoDMap::HandleMapZoomButton( void )
{
	if( gHUD.m_iFOV > 89 && gHUD.GetMinimapState() == 2 )
	{
		int newlevel = gHUD.ZoomMinimap();

		m_flPreviousMapScale = m_flIdealMapScale;
		m_flIdealMapScale = flZoomLevels[newlevel];
		m_flZoomTime = gEngfuncs.GetClientTime();
	}
}

void CHudDoDMap::GetSmallMapOffset( float &x, float &y, float &z )
{
	vec3_t origin = v_sim_org;

	if( gHUD.m_Spectator.m_OverviewData.rotated )
	{
		vec3_t newOrigin;

		float cx = gHUD.m_Spectator.m_OverviewData.origin.x;
		float cy = gHUD.m_Spectator.m_OverviewData.origin.y;

		newOrigin.x = cx + v_sim_org.y - cy;
		newOrigin.y = cy - ( v_sim_org.x - cx );
		newOrigin.z = v_sim_org.z;

		origin = newOrigin;
	}

	x -= origin.x;
	y -= origin.y;
}