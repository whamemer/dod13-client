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
#include "parsemsg.h"
#include "shake.h"
#include "screenfade.h"
#include "r_studioint.h"
#include "triangleapi.h"
#include "com_model.h"
#include "demo_api.h"
#include "view.h"
#include "pm_shared.h"

#include "dod_shared.h"

extern engine_studio_api_t IEngineStudio;
extern float in_fov;
extern vec3_t v_origin;
extern vec3_t v_angles;

float CalcFov( float fov_x, float width, float height );

#if SCOPE_WIDESCREEN_FIX
// The FOV that the engine renders with. On a widescreen it keeps the vertical FOV of 4:3 and widens the horizontal one
// (V_AdjustFov of the engine), the scope of the original is sized with the FOV of the screen itself, so its frame
// is smaller than the screen and the world shows at the borders. The code follows V_AdjustFov.
static void AdjustScopeFov( float &fov_x, float &fov_y )
{
	cvar_t *pAdjustFov = gEngfuncs.pfnGetCvarPointer( "r_adjust_fov" );
	float x;
	float y;

	// the engine does not adjust 4:3 and 5:4, and the engines without the cvar do not adjust at all
	if( !pAdjustFov || !pAdjustFov->value || ScreenWidth * 3 == ScreenHeight * 4 || ScreenWidth * 4 == ScreenHeight * 5 )
		return;

	y = CalcFov( fov_x, 640.0f, 480.0f );
	x = CalcFov( y, (float)ScreenHeight, (float)ScreenWidth );

	if( x >= fov_x )
	{
		fov_x = x;
		fov_y = y;
	}
}
#endif

DECLARE_MESSAGE( m_Scope, Scope )

int CHudScope::Init( void )
{
	HOOK_MESSAGE( Scope );

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem( this );

	m_iWeaponId = 0;
	m_bWeaponChanged = false;
	m_nLastWpnId = 0;

	return 1;
}

int CHudScope::MsgFunc_Scope( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int weaponId = READ_BYTE();

	if( weaponId != m_iWeaponId )
		m_bWeaponChanged = true;

	m_iFlags |= HUD_ACTIVE;
	m_iWeaponId = weaponId;

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

	spring_sprite = gHUD.GetSprite( spring );

	if( IEngineStudio.IsHardware() )
		spring_model = (struct model_s *)gEngfuncs.GetSpritePointer( spring_sprite );
	else
		spring_model = (struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_spring_soft.spr" ) );

	k43_sprite = gHUD.GetSprite( k43 );

	if( IEngineStudio.IsHardware() )
		k43_model = (struct model_s *)gEngfuncs.GetSpritePointer( k43_sprite );
	else
		k43_model = (struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_k43_soft.spr" ) );

	binoc_sprite = gHUD.GetSprite( binoc );
	binoc_model = (struct model_s *)gEngfuncs.GetSpritePointer( binoc_sprite );

	enfield_sprite = gHUD.GetSprite( enfield );

	if( IEngineStudio.IsHardware() )
		enfield_model = (struct model_s *)gEngfuncs.GetSpritePointer( enfield_sprite );
	else
		enfield_model = (struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load( "sprites/scope_enfield_soft.spr" ) );

	Reset();

	return 1;
}

void CHudScope::Reset( void )
{
	if( m_iWeaponId )
		m_bWeaponChanged = true;

	m_iWeaponId = 0;
}

int CHudScope::Draw( float flTime )
{
	return 1;
}

void CHudScope::Think( void )
{
	if( m_bWeaponChanged )
	{
		screenfade_t sf;

		if( m_iWeaponId == WEAPON_SCOPEDKAR || m_iWeaponId == WEAPON_SPRING || m_iWeaponId == WEAPON_BINOC
			|| ( ( m_iWeaponId == WEAPON_ENFIELD || m_iWeaponId == WEAPON_FG42 ) && ( g_iWeaponFlags & WPNSTATE_SCOPED ) ) )
		{
			gEngfuncs.pfnGetScreenFade( &sf );

			sf.fader = 0;
			sf.fadeg = 0;
			sf.fadeb = 0;
			sf.fadealpha = 255;
			sf.fadeSpeed = 90.0f;
			sf.fadeEnd = gEngfuncs.GetClientTime();
			sf.fadeReset = gEngfuncs.GetClientTime() + 3.0f;
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
			sf.fadeFlags = FFADE_OUT | FFADE_STAYOUT;

			gEngfuncs.pfnSetScreenFade( &sf );
		}

		m_bWeaponChanged = false;
	}
}

// the scope overlay is drawn as four quads (the four frames of the sprite) in front of the eyes
void CHudScope::DrawTriApiScope( void )
{
	bool bDrawScope;
	int weaponId;
	int checkFOV;
	int fov;
	vec3_t view;

	checkFOV = (int)in_fov;
	fov = gHUD.m_iFOV;
	weaponId = m_iWeaponId;

	gEngfuncs.GetViewAngles( view );

	if( g_iUser1 == OBS_IN_EYE )
	{
		m_iWeaponId = WEAPON_SPRING;
		view = v_angles;
	}

	bDrawScope = (unsigned int)( fov - 1 ) <= 88 && !gEngfuncs.pDemoAPI->IsPlayingback() && IEngineStudio.IsHardware();

	if( checkFOV == fov && bDrawScope )
	{
		model_t *pModel;
		vec3_t org;
		vec3_t spriteorg;
		vec3_t temporg;
		vec3_t forward, right, up;
		float dist;
		float fov_x;
		float fov_y;
		float width;
		float height;

		fov = 90;

		if( checkFOV )
			fov = checkFOV;

		if( !weaponId )
			weaponId = gHUD.GetCurrentWeaponId();

		switch( weaponId )
		{
		case WEAPON_SCOPEDKAR:
			pModel = k43_model;
			break;
		case WEAPON_SPRING:
			pModel = spring_model;
			break;
		case WEAPON_FG42:
			if( !( g_iWeaponFlags & WPNSTATE_SCOPED ) )
				return;

			pModel = k43_model;
			break;
		case WEAPON_ENFIELD:
			if( !( g_iWeaponFlags & WPNSTATE_SCOPED ) )
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

		dist = 5.0f;

		fov_x = (float)fov;
		fov_y = CalcFov( fov_x, (float)ScreenWidth, (float)ScreenHeight );

#if SCOPE_WIDESCREEN_FIX
		AdjustScopeFov( fov_x, fov_y );
#endif

		width = (float)( tan( fov_x / 360.0 * M_PI ) * dist * 1.1 );
		height = (float)( tan( fov_y / 360.0 * M_PI ) * dist * 1.1 );

		if( gHUD.m_fPlayerDead )
			view.z = 80.0f;

		AngleVectors( view, forward, right, up );

		org = v_origin + forward * dist;

		gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
		gEngfuncs.pTriAPI->CullFace( TRI_FRONT );

		for( int i = 0; i < 4; i++ )
		{
			if( !gEngfuncs.pTriAPI->SpriteTexture( pModel, i ) )
				return;

			gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
			gEngfuncs.pTriAPI->Begin( TRI_QUADS );

			gEngfuncs.pTriAPI->Brightness( 1.0f );
			gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.99f );

			switch( i )
			{
			case 0:
				// upper left
				temporg = org + right * -width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg = org + up * height + right * -width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg = org + up * height;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg = org;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );
				break;
			case 1:
				// upper right
				temporg = org;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg = org + up * height;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg = org + up * height + right * width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg = org + right * width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );
				break;
			case 2:
				// lower right
				temporg = org + up * -height;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg = org;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg = org + right * width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg = org + up * -height + right * width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );
				break;
			case 3:
				// lower left
				temporg = org + up * -height + right * -width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.01f, 0.01f );
				temporg = org + right * -width;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.01f );
				temporg = org;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );

				gEngfuncs.pTriAPI->Brightness( 1.0f );
				gEngfuncs.pTriAPI->TexCoord2f( 0.99f, 0.99f );
				temporg = org + up * -height;
				gEngfuncs.pTriAPI->Vertex3fv( temporg );
				break;
			}

			gEngfuncs.pTriAPI->End();
		}
	}
}
