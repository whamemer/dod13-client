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

#include "wrect.h"
#include "cl_dll.h"
#include "cl_entity.h"
#include "particleman.h"
#include "pman_misc.h"
#include "pman_particlemem.h"

MemoryBlock** CMiniMem::m_pMemoryPool = NULL;
long CMiniMem::m_lMemoryBlockSize = 0;
long CMiniMem::m_lMaxBlocks = 0;
long CMiniMem::m_lMemoryPoolSize = 0;
int g_lMaxParticleClassSize = 0;
CMiniMem* CMiniMem::_instance = NULL;

static MemList m_FreeMem;
static MemList m_ActiveMem;

#define MAX_PMAN_MEMORY		30000

float g_flOldTime = 0.0f;


long MaxParticleClassSize(unsigned long lSize)
{
	unsigned long lMaxSize = g_lMaxParticleClassSize;
	if (lMaxSize < sizeof(CBaseParticle))
		lMaxSize = sizeof(CBaseParticle);

	if (lSize && lSize > (unsigned long)g_lMaxParticleClassSize)
	{
		lMaxSize = lSize;
	}

	g_lMaxParticleClassSize = lMaxSize;
	return lMaxSize;
}

CMiniMem::CMiniMem(long lMemoryPoolSize, long lMaxBlockSize)
{
	m_lMemoryPoolSize = lMemoryPoolSize;
	m_lMemoryBlockSize = lMaxBlockSize;
	m_lMaxBlocks = MAX_PMAN_BLOCKS;
	m_pMemoryPool = new MemoryBlock*[m_lMaxBlocks];

	// Allocate all blocks
	for (long i = 0; i < m_lMaxBlocks; i++)
	{
		MemoryBlock* pNewBlock = new MemoryBlock(m_lMemoryBlockSize);
		m_pMemoryPool[i] = pNewBlock;
		m_FreeMem.Push(pNewBlock);
	}

	m_pVisibleParticles = new visibleparticles_t[m_lMaxBlocks];
}

CMiniMem::~CMiniMem()
{
	MemoryBlock* pFreeBlock = m_FreeMem.Front();
	while (pFreeBlock)
	{
		MemoryBlock* pTemp = pFreeBlock->next;
		m_FreeMem.Delete(pFreeBlock);

		delete pFreeBlock;
		pFreeBlock = pTemp;
	}

	delete[] m_pMemoryPool;
	m_pMemoryPool = NULL;

	delete[] m_pVisibleParticles;
	m_pVisibleParticles = NULL;
}

void CMiniMem::Shutdown( void )
{
	delete CMiniMem::_instance;

	// the original leaves the pointer dangling
	CMiniMem::_instance = NULL;
}

// Find a free block and mark it as "in use".  Return NULL
//  if no free blocks found.
char* CMiniMem::AllocateFreeBlock(void)
{
	MemoryBlock* pNewBlock = m_FreeMem.Pop();
	if (!pNewBlock)
		return NULL;

	m_ActiveMem.Push(pNewBlock);

	return pNewBlock->Memory();
}

bool CMiniMem::CheckSize( int iSize )
{
	return iSize <= m_lMemoryBlockSize;
}

// Return a pointer to usable block of memory.
char* CMiniMem::newBlock(void)
{
	char* p = AllocateFreeBlock();
	if (!p)
		gEngfuncs.Con_DPrintf("Particleman is out of memory, too many particles");

	return p;
}

// Mark a target memory item as no longer "in use".
void CMiniMem::deleteBlock( MemoryBlock* p )
{
	m_ActiveMem.Delete(p);
	m_FreeMem.Push(p);
}

// Return the remaining capacity of the memory pool as a percent.
long CMiniMem::PercentUsed(void)
{
	long iFreeNum = 0;

	MemoryBlock* pFreeBlock = m_FreeMem.Front();
	while (pFreeBlock)
	{
		iFreeNum++;
		pFreeBlock = pFreeBlock->next;
	}

	return 100 * iFreeNum / m_lMaxBlocks;
}

long CMiniMem::MaxBlockSize(void)
{
	return m_lMemoryBlockSize;
}

CMiniMem* CMiniMem::Instance(void)
{
	if (!_instance)
		_instance = new CMiniMem(MAX_PMAN_MEMORY, g_lMaxParticleClassSize);

	return _instance;
}

static int ParticleSortFn( const void* elem1, const void* elem2 )
{
	CCoreTriangleEffect* pParticle1 = ((visibleparticles_t*)elem1)->pVisibleParticle;
	CCoreTriangleEffect* pParticle2 = ((visibleparticles_t*)elem2)->pVisibleParticle;

	if (pParticle1->GetPlayerDistance() > pParticle2->GetPlayerDistance())
		return -1;
	if (pParticle1->GetPlayerDistance() == pParticle2->GetPlayerDistance())
		return 0;

	return 1;
}

// Processes all.
void CMiniMem::ProcessAll( void )
{
	float time = gEngfuncs.GetClientTime();

	MemoryBlock* currentBlock = m_ActiveMem.Front();

	// Clear list of visible particles
	m_iTotalParticles = 0;
	m_iParticlesDrawn = 0;

	memset(m_pVisibleParticles, 0, sizeof(m_pVisibleParticles[0]) * m_lMaxBlocks);

	while (currentBlock)
	{
		CCoreTriangleEffect* pEffect = (CCoreTriangleEffect*)currentBlock->Memory();
		if (pEffect)
		{
			m_iTotalParticles++;

			if (pEffect->CheckVisibility())
			{
				Vector vPlayerOrigin = gEngfuncs.GetLocalPlayer()->origin;
				float flDistance = 0.0;

				float flPOrigin;

				for (int j = 0; j < 3; j++)
				{
					flPOrigin = vPlayerOrigin[j] - pEffect->m_vOrigin[j];
					flDistance += flPOrigin * flPOrigin;
				}

				pEffect->SetPlayerDistance(flDistance);

				m_pVisibleParticles[m_iParticlesDrawn].pVisibleParticle = pEffect;
				Instance()->IncreaseParticlesDrawn();
			}

			if (!IsGamePaused())
				pEffect->Think(time);

			if (pEffect->m_flDieTime != 0.0 && pEffect->m_flDieTime <= time)
			{
				pEffect->Die();

				MemoryBlock* temp = currentBlock->next;

				deleteBlock(currentBlock);
				currentBlock = temp;
				continue;
			}
		}

		currentBlock = currentBlock->next;
	}

	// Sort it by distance between particle and player
	qsort(m_pVisibleParticles, m_iParticlesDrawn, sizeof(visibleparticles_t*), ParticleSortFn);

	for (int i = 0; i < m_iParticlesDrawn; i++)
	{
		m_pVisibleParticles[i].pVisibleParticle->Draw();
	}

	g_flOldTime = time;
}

int CMiniMem::ApplyForce( Vector vOrigin, Vector vDirection, float flRadius, float flStrength )
{
	MemoryBlock* currentBlock = m_ActiveMem.Front();

	float flAdjustedStrength, falloff;
	float flPOrigin, flSQRDistance;

	float flRadiusSquared = flRadius * flRadius;

	for (; currentBlock; currentBlock = currentBlock->next)
	{
		CCoreTriangleEffect* pEffect = (CCoreTriangleEffect*)currentBlock->Memory();
		if (!pEffect)
			continue;

		if (!pEffect->m_bAffectedByForce)
			continue;

		float size = pEffect->m_flSize * 0.2;
		Vector mins, maxs;

		mins = pEffect->m_vOrigin - Vector(size, size, size);
		maxs = pEffect->m_vOrigin + Vector(size, size, size);

		flSQRDistance = 0.0;
		for (int j = 0; j < 3 && flSQRDistance <= flRadiusSquared; j++)
		{
			if (vOrigin[j] < mins[j])
			{
				flPOrigin = mins[j];
			}
			else if (vOrigin[j] > maxs[j])
			{
				flPOrigin = maxs[j];
			}
			else
			{
				continue;
			}

			flSQRDistance += (vOrigin[j] - flPOrigin) * (vOrigin[j] - flPOrigin);
		}

		if (flSQRDistance > flRadiusSquared)
			continue;

		falloff = flStrength / (flRadiusSquared * 0.5);
		flAdjustedStrength = flStrength - (vOrigin - pEffect->m_vOrigin).Length() * falloff;
		if (flAdjustedStrength < 0)
			flAdjustedStrength = 0;

		if (vDirection == g_vecZero)
		{
			float flAccel = -(flAdjustedStrength / pEffect->m_flMass);
			Vector vDir = (vOrigin - pEffect->m_vOrigin).Normalize();
			Vector vVel = pEffect->m_vVelocity.Normalize();

			pEffect->m_vVelocity = flAccel * (vDir + vVel);
		}
		else
		{
			float flAccel = flAdjustedStrength / pEffect->m_flMass;
			Vector vDir = vDirection.Normalize();
			Vector vVel = pEffect->m_vVelocity.Normalize();

			pEffect->m_vVelocity = flAccel * (vDir + vVel);
		}

		pEffect->Force();
	}

	return 0;
}

// Clears memory, setting all particles to not used.
void CMiniMem::Reset()
{
	m_ActiveMem.Reset();
	m_FreeMem.Reset();
	
	for (int i = 0; i < m_lMaxBlocks; i++)
	{
		m_FreeMem.Push(m_pMemoryPool[i]);
	}
}

void UpdateParticles()
{
	CMiniMem::Instance()->ProcessAll();
}