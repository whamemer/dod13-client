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
// ammohistory.h
//
#pragma once
#if !defined(AMMOHISTORY_H)
#define AMMOHISTORY_H

#define DOD_MAX_SLOTS         6
#define DOD_MAX_POSITIONS     13

class WeaponsResource
{
private:
	WEAPON		  rgWeapons[MAX_WEAPONS];
	WEAPON		  *rgSlots[DOD_MAX_SLOTS][DOD_MAX_POSITIONS];
	int           riAmmo[MAX_AMMO_TYPES];

public:
	WEAPON scoped_fg42;
	WEAPON folding_carbine;
	WEAPON gravity_knife;
	WEAPON scoped_enfield;
	WEAPON brit_knife;
	WEAPON brit_grenade;
	WEAPON ger_binoculars;

	int           iOldWeaponBits;
	int           iOldWeaponBits2;

	inline void Init( void )
	{
		memset( rgWeapons, 0, sizeof rgWeapons );
		Reset();
	}

	inline void Reset( void )
	{
		iOldWeaponBits = 0;
		iOldWeaponBits2 = 0;
		memset( rgSlots, 0, sizeof rgSlots );
		memset( riAmmo, 0, sizeof riAmmo );
	}

	WEAPON *GetWeapon( int iId ) { return &rgWeapons[iId]; }

	void AddWeapon( WEAPON *wp )
	{
		rgWeapons[wp->iId] = *wp;
		LoadWeaponSprites( &rgWeapons[wp->iId] );
	}

	void PickupWeapon( WEAPON *wp )
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = wp;
	}

	void DropWeapon( WEAPON *wp )
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = NULL;
	}

	void DropAllWeapons( void )
	{
		for( int i = 0; i < MAX_WEAPONS; i++ )
		{
			if( rgWeapons[i].iId )
				DropWeapon( &rgWeapons[i] );
		}
	}

	WEAPON *GetWeaponSlot( int slot, int pos ) { return rgSlots[slot][pos]; }

	void LoadWeaponSprites( WEAPON *wp );
	void LoadAllWeaponSprites( void );
	WEAPON *GetFirstPos( int iSlot );
	void SelectSlot( int iSlot, int fAdvance, int iDirection );
	WEAPON *GetNextActivePos( int iSlot, int iSlotPos );

	int HasAmmo( WEAPON *p );

	AMMO GetAmmo( int iId ) { return riAmmo[iId]; }
	void SetAmmo( int iId, int iCount ) { riAmmo[iId] = iCount; }
	int CountAmmo( int iId );

	HSPRITE *GetAmmoPicFromWeapon( int iAmmoId, wrect_t &rect );
};

extern WeaponsResource gWR;

#endif // AMMOHISTORY_H
