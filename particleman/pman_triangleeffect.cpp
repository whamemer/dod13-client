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
#include "triangleapi.h"
#include "event_api.h"
#include "eiface.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "pman_misc.h"
#include "pman_frustum.h"
#include "pman_triangleffect.h"

// ------------ Triangles.
// The triangle effect.  Created by TriAPI.
void CCoreTriangleEffect::InitializeSprite( Vector org, Vector normal, model_s* sprite, float size, float brightness )
{
	m_flSize = m_flOriginalSize = 10;

	m_flScaleSpeed = 0.0;
	m_flContractSpeed = 0.0;

	m_flStretchX = 1.0;
	m_flStretchY = 1.0;

	SetNextPVSCheck(gEngfuncs.GetClientTime());

	m_flBrightness = m_flOriginalBrightness = 255;

	SetParticlePVS(true);

	m_flFadeSpeed = -1.0;
	m_flDieTime = 0.0;
	m_iFramerate = 0;
	m_iNumFrames = 0;
	m_iFrame = 0;
	m_flDampingTime = 0.0;
	m_pTexture = NULL;

	m_flGravity = 0.0; // No gravity by default.
	m_iCollisionFlags = 0;
	m_flNextCollisionTime = 0.0;

	strcpy(m_szClassname, "particle");

	m_flBounceFactor = 1.0;

	// Global Zero Vector ( 0, 0, 0 ).
	Vector zero = Vector(g_vecZero);

	// Static.
	m_vVelocity = zero;

	m_iRendermode = kRenderNormal;
	m_iRenderFlags = 0;
	
	m_bInWater = false;
	m_bAffectedByForce = false;

	m_vOriginalAngles = zero;
	m_vAngles = zero;
	m_vAVelocity = zero;

	m_vColor = Vector(255.0f, 255.0f, 255.0f);

	m_flTimeCreated = gEngfuncs.GetClientTime();

	m_vOrigin = Vector(org.x, org.y, org.z);
	m_vPrevOrigin = Vector(org.x, org.y, org.z);
	m_vAngles = Vector(normal.x, normal.y, normal.z);

	m_pTexture = sprite;
	
	m_flSize = m_flOriginalSize = size;
	m_flBrightness = m_flOriginalBrightness = brightness;

	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(normal, forward, right, up);

	Vector lowLeft, lowRight, topLeft, topRight;

	lowLeft = org - (right * m_flSize) * 0.5f - (up * m_flSize) * 0.5f;
	lowRight = lowLeft + (right * m_flSize) * 2;
	topLeft = lowLeft + (up * m_flSize) * 2;
	topRight = lowRight + (up * m_flSize) * 2;

	m_vOriginalAngles = Vector(m_vAngles.x, m_vAngles.y, m_vAngles.z);

	m_vLowLeft = lowLeft;
	m_vLowRight = lowRight;
	m_vTopLeft = topLeft;
}

bool CCoreTriangleEffect::CheckVisibility()
{
	float radius = m_flSize / 5.0;

	if (gEngfuncs.GetClientTime() >= m_flNextPVSCheck)
	{
		Vector mins, maxs;
		Vector r = Vector(radius, radius, radius); // The radius vector.

		mins = m_vOrigin - r;
		maxs = m_vOrigin + r;

		SetParticlePVS(gEngfuncs.pTriAPI->BoxInPVS(mins, maxs) != 0);

		SetNextPVSCheck(gEngfuncs.GetClientTime() + 0.1);
	}

	if (m_iRenderFlags & CULL_FRUSTUM_SPHERE)
	{
		if (!g_cFrustum.SphereInsideFrustum(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, radius))
			return false;
	}
	else if (m_iRenderFlags & CULL_FRUSTUM_PLANE)
	{
		if (!g_cFrustum.PlaneInsideFrustum(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, radius))
			return false;
	}
	else if (m_iRenderFlags & CULL_FRUSTUM_POINT)
	{
		if (!g_cFrustum.PointInsideFrustum(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z))
			return false;
	}

	if (!m_bInPVS)
	{
		return (m_iRenderFlags & CULL_PVS) == 0;
	}

	return true;
}

void CCoreTriangleEffect::Draw()
{
	float radius = m_flSize;
	float brightness = m_flBrightness;

	if (m_flDieTime == gEngfuncs.GetClientTime())
		return;

	// the original leaves the draw color uninitialized when there is no LIGHT_* flag
	Vector vColor, vDrawColor = m_vColor;
	float flColor = 0.0f;

	if (!(m_iRenderFlags & LIGHT_NONE))
	{
		gEngfuncs.pTriAPI->LightAtPoint(m_vOrigin, vColor);
		flColor = (vColor.x + vColor.y + vColor.z) / 3.0f;
	}

	if ((m_iRenderFlags & RENDER_FACEPLAYER) || (m_iRenderFlags & RENDER_FACEPLAYER_ROTATEZ))
	{
		m_vAngles.x = g_vViewAngles.x;
		m_vAngles.y = g_vViewAngles.y;

		if (!(m_iRenderFlags & RENDER_FACEPLAYER_ROTATEZ))
			m_vAngles.z = g_vViewAngles.z; // If we shouldn't rotate it.
	}

	// Check the light.
	if (m_iRenderFlags & LIGHT_NONE)
	{
		vDrawColor = m_vColor;
	}
	else if (m_iRenderFlags & LIGHT_COLOR)
	{
		vDrawColor.x = vColor.x / m_vColor.x * 255.0;
		vDrawColor.y = vColor.y / m_vColor.y * 255.0;
		vDrawColor.z = vColor.z / m_vColor.z * 255.0;
	}
	else if (m_iRenderFlags & LIGHT_INTENSITY)
	{
		vDrawColor.x = flColor / m_vColor.x * 255.0;
		vDrawColor.y = flColor / m_vColor.y * 255.0;
		vDrawColor.z = flColor / m_vColor.z * 255.0;
	}

	// Clamp it.
	if (vDrawColor.x > 255.0) vDrawColor.x = 255.0;
	if (vDrawColor.y > 255.0) vDrawColor.y = 255.0;
	if (vDrawColor.z > 255.0) vDrawColor.z = 255.0;
	if (vDrawColor.x < 0.0) vDrawColor.x = 0.0;
	if (vDrawColor.y < 0.0) vDrawColor.y = 0.0;
	if (vDrawColor.z < 0.0) vDrawColor.z = 0.0;

	Vector lowLeft, lowRight, topLeft, topRight, forward, right, up;
	gEngfuncs.pfnAngleVectors(m_vAngles, forward, right, up);

	lowLeft = m_vOrigin - ((radius * right * m_flStretchX) * 0.5) - (radius * up * 0.5);
	lowRight = lowLeft + (radius * right * m_flStretchX);
	topLeft = lowLeft + (radius * up * m_flStretchY);
	topRight = lowRight + (radius * up * m_flStretchY);

	gEngfuncs.pTriAPI->SpriteTexture(m_pTexture, m_iFrame);
	gEngfuncs.pTriAPI->RenderMode(m_iRendermode);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);

	gEngfuncs.pTriAPI->Color4f(vDrawColor.x / 255.0, vDrawColor.y / 255.0, vDrawColor.z / 255.0, brightness / 255.0);

	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3fv(topLeft);

	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3fv(lowLeft);

	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3fv(lowRight);

	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3fv(topRight);

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	gEngfuncs.pTriAPI->CullFace(TRI_FRONT);
}

void CCoreTriangleEffect::Animate( float time )
{
	if (m_iFramerate == 0 || m_iNumFrames == 0)
		return;

	float frame = (time - m_flTimeCreated) * m_iFramerate;
	// Clamp the frame number.
	m_iFrame = (int)frame % m_iNumFrames;
}

void CCoreTriangleEffect::AnimateAndDie( float time )
{
	if (m_iFramerate == 0 || m_iNumFrames == 0)
		return;

	float frame = (time - m_flTimeCreated) * m_iFramerate;

	if (frame > m_iNumFrames)
		m_flDieTime = time;
	else
		m_iFrame = frame;

	if (m_iFrame > m_iNumFrames)
		m_iFrame = m_iNumFrames - 1;
}

void CCoreTriangleEffect::Expand( float time )
{
	if (!m_flScaleSpeed)
		return;

	m_flSize = m_flOriginalSize + ((time - m_flTimeCreated) * m_flScaleSpeed * 30.0);
	if (m_flSize < 0.0001)
		m_flDieTime = time;
}

void CCoreTriangleEffect::Contract( float time )
{
	if (!m_flContractSpeed)
		return;

	m_flSize = m_flOriginalSize - ((time - m_flTimeCreated) * m_flContractSpeed * 30.0);
	if (m_flSize < 0.0001)
		m_flDieTime = time;
}

void CCoreTriangleEffect::Fade( float time )
{
	if (m_flFadeSpeed < -0.5)
		return;

	if (m_flFadeSpeed == 0.0)
		m_flBrightness = (1.0 - (time - m_flTimeCreated) / (m_flDieTime - m_flTimeCreated)) * m_flOriginalBrightness;
	else
		m_flBrightness = m_flOriginalBrightness - m_flFadeSpeed * 30.0 * (time - m_flTimeCreated);

	if (m_flBrightness < 1.0)
		m_flDieTime = time;
}

void CCoreTriangleEffect::Spin( float time )
{
	if (m_vAVelocity == g_vecZero)
		return;

	m_vAngles = m_vAngles + m_vAVelocity;

	Vector point = m_vAVelocity;

	float scale = VectorNormalize(point) * 30.0 * (time - m_flTimeCreated);

	m_vAngles = m_vOriginalAngles + m_vAVelocity * scale;
}

void CCoreTriangleEffect::CalculateVelocity( float time )
{
	float frametime = time - g_flOldTime;
	float gravity = -(g_flGravity * frametime) * m_flGravity;

	if (m_vVelocity == g_vecZero && gravity == 0.0)
		return;

	if (m_iCollisionFlags & TRI_SPIRAL)
	{
		m_vOrigin = m_vOrigin + m_vVelocity * frametime;

		float fastFreq = (float)((size_t)this);

		m_vOrigin.x += sin(time * 5.0 + fastFreq) * 2;
		m_vOrigin.y += sin(time * 7.5 + fastFreq);	
	}
	else
	{
		m_vOrigin = m_vOrigin + m_vVelocity * frametime;		
	}

	m_vVelocity.z += gravity;
}

void CCoreTriangleEffect::CheckCollision( float time )
{
	if (m_iCollisionFlags == 0)
		return;

	m_flNextCollisionTime = time;

	if (!(m_iCollisionFlags & (TRI_WATERTRACE | TRI_COLLIDEALL | TRI_COLLIDEWORLD)))
		return;

	Vector traceNormal;
	float traceFraction = 1.0;
	Vector endpos;
	int entIndex = 0;

	if (m_iCollisionFlags & TRI_COLLIDEALL)
	{
		pmtrace_t pmtrace;
		physent_t* pe;

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(m_vPrevOrigin, m_vOrigin, PM_STUDIO_BOX, -1, &pmtrace);

		// Make sure it didn't bump into itself...
		if (pmtrace.fraction != 1.0)
		{
			pe = gEngfuncs.pEventAPI->EV_GetPhysent(pmtrace.ent);

			// Particle collided with the world or owner of this particle.
			if (!pmtrace.ent)
			{
				traceFraction = pmtrace.fraction;
				traceNormal = pmtrace.plane.normal;
			}

			endpos = pmtrace.endpos;
			entIndex = pmtrace.ent;
		}
	}
	else if (m_iCollisionFlags & TRI_COLLIDEWORLD)
	{
		pmtrace_t pmtrace;
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(m_vPrevOrigin, m_vOrigin, PM_STUDIO_BOX | PM_WORLD_ONLY, -1, &pmtrace);

		if (pmtrace.fraction != 1.0)
		{
			traceFraction = pmtrace.fraction;
			traceNormal = pmtrace.plane.normal;

			VectorScale(m_vVelocity, 0.6, m_vVelocity);

			if (m_vVelocity.Length() < 10.0)
			{
				m_iCollisionFlags = 0;
				m_vVelocity = g_vecZero;
				m_vAVelocity = g_vecZero;
				m_vOrigin = pmtrace.endpos;
			}

			endpos = pmtrace.endpos;
			entIndex = pmtrace.ent;
		}
	}

	// Decent collision now, and damping works.
	if (traceFraction != 1.0)
	{
		// Damp velocity.
		float proj, damp;
		float frametime = time - g_flOldTime;
		float gravity = -(g_flGravity * frametime) * m_flGravity;

		// Place it at the contact point.
		VectorMA(m_vPrevOrigin, frametime * traceFraction, m_vVelocity, m_vOrigin);

		// Damp velocity.
		damp = m_flBounceFactor * 0.5;
		if (traceNormal.z > 0.9)		// Hit floor?
		{
			if (m_vVelocity.z <= 0.0 && m_vVelocity.z >= (gravity * 3))
			{
				// No velocity to bounce back up so stop it.
				damp = 0;
				m_vVelocity = g_vecZero;
				m_vAVelocity = g_vecZero;
				m_iCollisionFlags = 0;
			}
		}

		if (m_iCollisionFlags & TRI_COLLIDEKILL)
		{
			// Die on impact.
			m_flDieTime = gEngfuncs.GetClientTime();
		}
		else
		{
			// Reflect velocity.
			if (damp != 0.0)
			{
				proj = DotProduct(traceNormal, m_vVelocity) * -2;
				m_vVelocity = traceNormal * proj + m_vVelocity;

				// Reflect rotation (fake).
				m_vAngles.y = -m_vAngles.y;
			}

			if (damp != 1.0)
			{
				m_vVelocity = m_vVelocity * damp;
				m_vAngles = m_vAngles * 0.9;
			}
		}

		Touch(endpos, traceNormal, entIndex);
	}
	else if (m_iCollisionFlags & TRI_WATERTRACE)
	{
		int contents = gEngfuncs.PM_PointContents(m_vOrigin, NULL);
		if (contents == CONTENTS_WATER && !m_bInWater)
		{
			entIndex = 0;

			Touch(m_vOrigin, Vector(0, 0, 1), entIndex);

			m_bInWater = true;

			if (m_iCollisionFlags & TRI_COLLIDEKILL)
			{
				m_flDieTime = gEngfuncs.GetClientTime();
			}
		}
	}

	m_vPrevOrigin = m_vOrigin;
}

void CCoreTriangleEffect::Touch( Vector pos, Vector normal, int index )
{

}

void CCoreTriangleEffect::Die()
{

}

void CCoreTriangleEffect::Force()
{

}

void CCoreTriangleEffect::Think( float time )
{
	if (m_iCollisionFlags & TRI_ANIMATEDIE)
		AnimateAndDie(time);
	else
		Animate(time);

	Contract(time);
	Expand(time);
	Fade(time);
	Spin(time);
	CalculateVelocity(time);
	CheckCollision(time);
}