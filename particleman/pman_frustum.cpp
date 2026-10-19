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
#include "pman_frustum.h"

#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_MODELVIEW_MATRIX               0x0BA6

CFrustum g_cFrustum;

void CFrustum::NormalizeFrustumPlane( float* frustum, int side )
{
	float magnitude;

	magnitude = sqrt(frustum[side * 4 + A] * frustum[side * 4 + A] + frustum[side * 4 + B] * frustum[side * 4 + B] + frustum[side * 4 + C] * frustum[side * 4 + C]);

	frustum[side * 4 + A] *= 1.0 / magnitude;
	frustum[side * 4 + B] *= 1.0 / magnitude;
	frustum[side * 4 + C] *= 1.0 / magnitude;
	frustum[side * 4 + D] *= 1.0 / magnitude;
}

void CFrustum::CalculateFrustum()
{
	float proj[16];
	float modl[16];
	float clip[16];

	gEngfuncs.pTriAPI->GetMatrix(GL_PROJECTION_MATRIX, proj);
	gEngfuncs.pTriAPI->GetMatrix(GL_MODELVIEW_MATRIX, modl);

	clip[0] = proj[12] * modl[3] + proj[8] * modl[2] + proj[4] * modl[1] + proj[0] * modl[0];
	clip[1] = proj[13] * modl[3] + proj[9] * modl[2] + proj[5] * modl[1] + proj[1] * modl[0];
	clip[2] = proj[14] * modl[3] + proj[10] * modl[2] + proj[6] * modl[1] + proj[2] * modl[0];
	clip[3] = proj[15] * modl[3] + proj[11] * modl[2] + proj[7] * modl[1] + proj[3] * modl[0];

	clip[4] = modl[7] * proj[12] + modl[6] * proj[8] + modl[5] * proj[4] + modl[4] * proj[0];
	clip[5] = modl[7] * proj[13] + modl[6] * proj[9] + modl[5] * proj[5] + modl[4] * proj[1];
	clip[6] = modl[7] * proj[14] + modl[6] * proj[10] + modl[5] * proj[6] + modl[4] * proj[2];
	clip[7] = modl[7] * proj[15] + modl[6] * proj[11] + modl[5] * proj[7] + modl[4] * proj[3];

	clip[8] = modl[11] * proj[12] + modl[10] * proj[8] + modl[9] * proj[4] + modl[8] * proj[0];
	clip[9] = modl[11] * proj[13] + modl[10] * proj[9] + modl[9] * proj[5] + modl[8] * proj[1];
	clip[10] = modl[11] * proj[14] + modl[10] * proj[10] + modl[9] * proj[6] + modl[8] * proj[2];
	clip[11] = modl[11] * proj[15] + modl[10] * proj[11] + modl[9] * proj[7] + modl[8] * proj[3];

	clip[12] = modl[15] * proj[12] + modl[14] * proj[8] + modl[13] * proj[4] + modl[12] * proj[0];
	clip[13] = modl[15] * proj[13] + modl[14] * proj[9] + modl[13] * proj[5] + modl[12] * proj[1];
	clip[14] = modl[15] * proj[14] + modl[14] * proj[10] + modl[13] * proj[6] + modl[12] * proj[2];
	clip[15] = modl[15] * proj[15] + modl[14] * proj[11] + modl[13] * proj[7] + modl[12] * proj[3];

	g_flFrustum[0][0] = clip[3] - clip[0];
	g_flFrustum[0][1] = clip[7] - clip[4];
	g_flFrustum[0][2] = clip[11] - clip[8];
	g_flFrustum[0][3] = clip[15] - clip[12];
	NormalizeFrustumPlane((float*)g_flFrustum, RIGHT);

	g_flFrustum[1][0] = clip[3] + clip[0];
	g_flFrustum[1][1] = clip[7] + clip[4];
	g_flFrustum[1][2] = clip[11] + clip[8];
	g_flFrustum[1][3] = clip[15] + clip[12];
	NormalizeFrustumPlane((float*)g_flFrustum, LEFT);

	g_flFrustum[2][0] = clip[3] + clip[1];
	g_flFrustum[2][1] = clip[7] + clip[5];
	g_flFrustum[2][2] = clip[11] + clip[9];
	g_flFrustum[2][3] = clip[15] + clip[13];
	NormalizeFrustumPlane((float*)g_flFrustum, BOTTOM);

	g_flFrustum[3][0] = clip[3] - clip[1];
	g_flFrustum[3][1] = clip[7] - clip[5];
	g_flFrustum[3][2] = clip[11] - clip[9];
	g_flFrustum[3][3] = clip[15] - clip[13];
	NormalizeFrustumPlane((float*)g_flFrustum, TOP);

	g_flFrustum[4][0] = clip[3] - clip[2];
	g_flFrustum[4][1] = clip[7] - clip[6];
	g_flFrustum[4][2] = clip[11] - clip[10];
	g_flFrustum[4][3] = clip[15] - clip[14];
	NormalizeFrustumPlane((float*)g_flFrustum, BACK);

	g_flFrustum[5][0] = clip[3] + clip[2];
	g_flFrustum[5][1] = clip[7] + clip[6];
	g_flFrustum[5][2] = clip[11] + clip[10];
	g_flFrustum[5][3] = clip[15] + clip[14];
	NormalizeFrustumPlane((float*)g_flFrustum, FRONT);
}

bool CFrustum::PointInsideFrustum( float x, float y, float z )
{
	for (int i = 0; i < 6; i++)
	{
		if (((g_flFrustum[i][A] * x) + (g_flFrustum[i][B] * y) + (g_flFrustum[i][C] * z) + g_flFrustum[i][D]) <= 0)
		{
			return false;
		}
	}

	return true;
}

bool CFrustum::SphereInsideFrustum( float x, float y, float z, float radius )
{
	for (int i = 0; i < 6; i++)
	{
		if (((g_flFrustum[i][A] * x) + (g_flFrustum[i][B] * y) + (g_flFrustum[i][C] * z) + g_flFrustum[i][D]) <= -radius)
		{
			return false;
		}
	}

	return true;
}

bool CFrustum::PlaneInsideFrustum( float x, float y, float z, float size )
{
	for (int i = 0; i < 6; i++)
	{
		// Test every permutation of plane sizes.

		if (((x - size) * g_flFrustum[i][A]) +
			((y - size) * g_flFrustum[i][B]) +
			((z - size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x + size) * g_flFrustum[i][A]) +
			((y - size) * g_flFrustum[i][B]) +
			((z - size) * g_flFrustum[i][C]) + g_flFrustum[i][3] > 0)
			continue;

		if (((x - size) * g_flFrustum[i][A]) +
			((y + size) * g_flFrustum[i][B]) +
			((z - size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x + size) * g_flFrustum[i][A]) +
			((y + size) * g_flFrustum[i][B]) +
			((z - size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x - size) * g_flFrustum[i][A]) +
			((y - size) * g_flFrustum[i][B]) +
			((z + size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x + size) * g_flFrustum[i][A]) +
			((y - size) * g_flFrustum[i][B]) +
			((z + size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x - size) * g_flFrustum[i][A]) +
			((y + size) * g_flFrustum[i][B]) +
			((z + size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		if (((x + size) * g_flFrustum[i][A]) +
			((y + size) * g_flFrustum[i][B]) +
			((z + size) * g_flFrustum[i][C]) + g_flFrustum[i][D] > 0)
			continue;

		return false;
	}

	return true;
}