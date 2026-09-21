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
//  scope.cpp - implementation of the CHudScope class
//

#include "hud.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "dod_shared.h"
#include "screenfade.h"
#include "shake.h"
#include "demo_api.h"
#include "triangleapi.h"
#include "com_model.h"
#include "view.h"
#include "in_defs.h"

extern "C"
{
#include "pm_shared.h"
}

extern engine_studio_api_t IEngineStudio;
extern vec3_t v_origin, v_angles;
extern float in_fov;
extern int g_iWeaponFlags;

DECLARE_MESSAGE( m_Scope, Scope )

int CHudScope::Init( void )
{
	HOOK_MESSAGE( Scope );
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_ACTIVE;
	m_iWeaponId = WEAPON_NONE;
	m_bWeaponChanged = false;
	m_nLastWpnId = 0;
	return 1;
}

int CHudScope::MsgFunc_Scope( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iWeaponId = READ_BYTE();

	if( iWeaponId != m_iWeaponId )
	{
		m_bWeaponChanged = true;
	}

	m_iFlags |= HUD_ACTIVE;
	m_iWeaponId = iWeaponId;

	return 1;
}

void CHudScope::SetScope( int weaponId )
{
	if( m_iWeaponId != weaponId )
		m_bWeaponChanged = true;

	m_iWeaponId = weaponId;
}

int CHudScope::VidInit( void )
{
	int spring;
	int k43;
	int binoc;
	int enfield;

	spring = gHUD.GetSpriteIndex( "scope_spring" );
	k43 = gHUD.GetSpriteIndex( "scope_k43" );
	binoc = gHUD.GetSpriteIndex( "scope_binoc" );
	enfield = gHUD.GetSpriteIndex( "scope_enfield" );

	if( spring >= 0 )
		spring_sprite = gHUD.m_rghSprites[spring];
	else
		spring_sprite = 0;

	if( IEngineStudio.IsHardware() )
	{
		spring_model = (model_s *)gEngfuncs.GetSpritePointer( spring_sprite );
	}
	else
	{
		HSPRITE hSoftSpr = gEngfuncs.pfnSPR_Load( "sprites/scope_spring_soft.spr" );
		spring_model = (model_s *)gEngfuncs.GetSpritePointer( hSoftSpr );
	}

	if( k43 >= 0 )
		k43_sprite = gHUD.m_rghSprites[k43];
	else
		k43_sprite = 0;

	if( IEngineStudio.IsHardware() )
	{
		k43_model = (model_s *)gEngfuncs.GetSpritePointer( k43_sprite );
	}
	else
	{
		HSPRITE hSoftSpr = gEngfuncs.pfnSPR_Load( "sprites/scope_k43_soft.spr" );
		k43_model = (model_s *)gEngfuncs.GetSpritePointer( hSoftSpr );
	}

	if( binoc >= 0 )
		binoc_sprite = gHUD.m_rghSprites[binoc];
	else
		binoc_sprite = 0;

	binoc_model = (model_s *)gEngfuncs.GetSpritePointer( binoc_sprite );

	if( enfield >= 0 )
		enfield_sprite = gHUD.m_rghSprites[enfield];
	else
		enfield_sprite = 0;

	if( IEngineStudio.IsHardware() )
	{
		enfield_model = (model_s *)gEngfuncs.GetSpritePointer( enfield_sprite );
	}
	else
	{
		HSPRITE hSoftSpr = gEngfuncs.pfnSPR_Load( "sprites/scope_enfield_soft.spr" );
		enfield_model = (model_s *)gEngfuncs.GetSpritePointer( hSoftSpr );
	}

	Reset();
	return 1;
}

void CHudScope::Reset( void )
{
	SetScope( WEAPON_NONE );
}

int CHudScope::Draw( float flTime )
{
	return 1;
}

void CHudScope::Think( void )
{
	screenfade_t sf;

	if( m_bWeaponChanged )
	{
		if( m_iWeaponId == WEAPON_SCOPEDKAR || m_iWeaponId == WEAPON_SPRING || m_iWeaponId == WEAPON_BINOC ||
			( ( m_iWeaponId == WEAPON_ENFIELD || m_iWeaponId == WEAPON_FG42 ) && ( g_iWeaponFlags & 1 ) != 0 ) )
		{
			gEngfuncs.pfnGetScreenFade( &sf );

			sf.fader = 0;
			sf.fadeg = 0;
			sf.fadeb = 0;
			sf.fadealpha = 255;

			sf.fadeSpeed = 900.0f;
			sf.fadeEnd = gEngfuncs.GetClientTime();
			sf.fadeTotalEnd = gEngfuncs.GetClientTime() + 3.0f;
			sf.fadeReset = 1.0f;

			sf.fadeFlags = FFADE_OUT;

			gEngfuncs.pfnSetScreenFade( &sf );
		}
		else
		{
			gEngfuncs.pfnGetScreenFade( &sf );

			sf.fader = 0;
			sf.fadeg = 0;
			sf.fadeb = 0;
			sf.fadealpha = 0;

			sf.fadeSpeed = 0.0f;
			sf.fadeEnd = 0.0f;
			sf.fadeTotalEnd = 0.0f;
			sf.fadeReset = 0.0f;

			sf.fadeFlags = FFADE_OUT | FFADE_STAYOUT;

			gEngfuncs.pfnSetScreenFade( &sf );
		}

		m_bWeaponChanged = false;
	}
}

void CHudScope::DrawTriApiScope( void )
{
	int fov = gHUD.m_iFOV;
	int weaponId = m_iWeaponId;

	vec3_t view;
	gEngfuncs.GetViewAngles( view );

	if( g_iUser1 == OBS_IN_EYE )
	{
		m_iWeaponId = WEAPON_SPRING;
		weaponId = WEAPON_SPRING;
		VectorCopy( v_angles, view );
	}

	bool checkFOV = ( fov <= 89 );
	bool bDrawScope = IEngineStudio.IsHardware() && checkFOV && ( gEngfuncs.pDemoAPI->IsPlayingback() == 0 );

	if( in_fov == fov && bDrawScope )
	{
		const model_t *pModel = NULL;

		if( !weaponId )
			weaponId = gHUD.GetCurrentWeaponId();

		switch( weaponId )
		{
		case WEAPON_SCOPEDKAR:
		case WEAPON_KAR:
			pModel = k43_model;
			break;
		case WEAPON_SPRING:
			pModel = spring_model;
			break;
		case WEAPON_FG42:
			if( ( g_iWeaponFlags & 1 ) == 0 ) 
				return;
			pModel = k43_model;
			break;
		case WEAPON_ENFIELD:
			if( ( g_iWeaponFlags & 1 ) == 0 ) 
				return;
			pModel = enfield_model;
			break;
		case WEAPON_BINOC:
			pModel = binoc_model;
			break;
		default:
			return;
		}

		if( !pModel )
			return;

		float fov_x = ( in_fov != 0 ) ? ( float ) in_fov : 90.0f;
		float fov_y = CalcFov( fov_x, ( float ) gHUD.m_scrinfo.iWidth, ( float ) gHUD.m_scrinfo.iHeight );
		float dist = 5.0f;

		float width = tan( fov_x / 360.0f * M_PI ) * dist * 1.1f;
		float height = tan( fov_y / 360.0f * M_PI ) * dist * 1.1f;

		if( gHUD.m_fPlayerDead )
		{
			view[ROLL] = 80.0f;
		}

		vec3_t forward, right, up;
		AngleVectors( view, forward, right, up );

		vec3_t org;
		org.x = forward.x * dist + v_origin.x;
		org.y = forward.y * dist + v_origin.y;
		org.z = forward.z * dist + v_origin.z;

		gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
		gEngfuncs.pTriAPI->CullFace( TRI_FRONT );

		float neg_height = -height;
		float neg_width = -width;

		for( int i = 1; i <= 4; i++ )
		{
			if( !gEngfuncs.pTriAPI->SpriteTexture( (model_s *)pModel, i - 1 ))
				return;

			gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
			gEngfuncs.pTriAPI->Begin( 2 );

			vec3_t temporg;

			if( i == 1 )
			{
				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg.x = neg_width * right.x + org.x;
				temporg.y = neg_width * right.y + org.y;
				temporg.z = neg_width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
				temporg.x = height * up.x + neg_width * right.x + org.x;
				temporg.y = height * up.y + neg_width * right.y + org.y;
				temporg.z = height * up.z + neg_width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg.x = height * up.x + org.x;
				temporg.y = height * up.y + org.y;
				temporg.z = height * up.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				VectorCopy( org, temporg );
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );
			}
			else if( i == 2 )
			{
				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				VectorCopy( org, temporg );
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
				temporg.x = height * up.x + org.x;
				temporg.y = height * up.y + org.y;
				temporg.z = height * up.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg.x = height * up.x + width * right.x + org.x;
				temporg.y = height * up.y + width * right.y + org.y;
				temporg.z = height * up.z + width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg.x = width * right.x + org.x;
				temporg.y = width * right.y + org.y;
				temporg.z = width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );
			}
			else if( i == 3 )
			{
				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg.x = neg_height * up.x + org.x;
				temporg.y = neg_height * up.y + org.y;
				temporg.z = neg_height * up.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
				VectorCopy( org, temporg );
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg.x = width * right.x + org.x;
				temporg.y = width * right.y + org.y;
				temporg.z = width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg.x = neg_height * up.x + width * right.x + org.x;
				temporg.y = neg_height * up.y + width * right.y + org.y;
				temporg.z = neg_height * up.z + width * right.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );
			}
			else if( i == 4 )
			{
				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg.x = neg_height * up.x + right.x * neg_width + org.x;
				temporg.y = neg_height * up.y + right.y * neg_width + org.y;
				temporg.z = neg_height * up.z + right.z * neg_width + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );
				temporg.x = neg_width * right.x + org.x;
				temporg.y = right.y * neg_width + org.y;
				temporg.z = right.z * neg_width + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				VectorCopy( org, temporg );
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg.x = neg_height * up.x + org.x;
				temporg.y = neg_height * up.y + org.y;
				temporg.z = neg_height * up.z + org.z;
				gEngfuncs.pTriAPI->Vertex3fv( &temporg.x );
			}

			gEngfuncs.pTriAPI->End();
		}
	}
}
