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

#ifndef PMAN_FORCE_H
#define PMAN_FORCE_H

//---------------------------------------------------------------------------
// Simulated force on CPU.
class ForceMember
{
public:
	int m_iIndex;			// index into the main list
	ForceMember *m_pNext;	// a linked list entry
	ForceMember *m_pPrevious;
	Vector m_vOrigin;		// position
	Vector m_vDirection;
	float m_flRadius;		// force radius, i.e. impact scale
	float m_flStrength;
	float m_flDieTime;		// # amount of seconds till it'll be removed

	ForceMember()
	{
		m_iIndex = 0;
		m_pNext = NULL;
		m_pPrevious = NULL;
	}
};

#define MAX_FORCE_MEMBERS	128

//---------------------------------------------------------------------------
// The list of simualted forces.
class ForceList
{
public:
	int m_iElements;
	ForceMember *pFirst;	// Front
	ForceMember *pCurrent;	// Middle
	ForceMember *pLast;		// Back

	ForceList()
	{
		m_iElements = 0;
		pFirst = NULL;
		pCurrent = NULL;
		pLast = NULL;
	}
	// Iterate over the whole list and free all forces.
	ForceMember *AddForceMember( void )
	{
		if( m_iElements >= MAX_FORCE_MEMBERS )
			return NULL;

		m_iElements++;
		ForceMember *p = new ForceMember;
		if( p )
		{
			p->m_pNext = NULL;
			p->m_pPrevious = NULL;
			p->m_flRadius = 0;
			p->m_flStrength = 0;
			p->m_flDieTime = 0;
			p->m_iIndex = m_iElements;
		}

		if( pFirst )
		{
			p->m_pPrevious = pCurrent;
			pCurrent->m_pNext = p;
			pLast = pCurrent = p;
		}
		else
		{
			pLast = pCurrent = pFirst = p;
		}
		return p;
	}

	void ClearForceMembers( void )
	{
		ForceMember *pNext;

		if( !pFirst )
			return;

		ForceMember *pTemp = pFirst->m_pNext;
		while( pTemp )
		{
			pNext = pTemp->m_pNext;
			if( pCurrent == pTemp )
				pCurrent = NULL;

			delete pTemp;
			pTemp = pNext;
			m_iElements--;
		}

		if( pFirst )
		{
			delete pFirst;
			pFirst = NULL;
			m_iElements--;
		}
	}

	void UpdateForce( float m_flTime )
	{
		ForceMember *pTemp;
		ForceMember *pTemp2;

		pTemp = pFirst;
		while( pTemp )
		{
			if( pTemp->m_flDieTime == 0.0 || pTemp->m_flDieTime >= m_flTime )
			{
				pTemp = pTemp->m_pNext;
			}
			else
			{
				// the original sets this to NULL, so the removed member was never freed
				pTemp2 = pTemp;
				// If the element is a head of the list, relink it to the next one
				if( pTemp == pFirst )
				{
					// The last one, relink to the previous one
					if( pTemp == pLast )
					{
						pLast = NULL;
						pCurrent = NULL;
						pFirst = NULL;
						pTemp = NULL;
						delete pTemp2;
						m_iElements--;
					}
					else
					{
						pTemp = pTemp->m_pNext;
						pTemp->m_pPrevious = NULL;
						pFirst = pTemp;
						delete pTemp2;

						if( pTemp == pLast )
						{
							pLast = pTemp;
							pTemp->m_pNext = NULL;
							pCurrent = pLast;
						}
						m_iElements--;
					}
				}
				else
				{
					// The last one, relink to the previous one
					if( pTemp == pLast )
					{
						pTemp = pLast->m_pPrevious;
						pTemp->m_pNext = NULL;
						pLast = pTemp;
						pCurrent = pTemp;
						delete pTemp2;
					}
					else
					{
						ForceMember *pNext = pTemp->m_pNext;

						pNext->m_pPrevious = pTemp->m_pPrevious;
						pTemp->m_pPrevious->m_pNext = pTemp->m_pNext;
						delete pTemp2;
						pTemp = pNext;
					}
					m_iElements--;
				}
			}
		}

		for( pTemp = pFirst; pTemp; pTemp = pTemp->m_pNext )
		{
			CMiniMem::ApplyForce( pTemp->m_vOrigin, pTemp->m_vDirection, pTemp->m_flRadius, pTemp->m_flStrength );
		}
	}

	~ForceList()
	{
		ClearForceMembers();
	}
};

#endif //PMAN_FORCE_H