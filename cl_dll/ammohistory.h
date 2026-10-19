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

// this is the max number of items in each bucket
#define MAX_WEAPON_POSITIONS		12

class WeaponsResource
{
private:
	// Information about weapons & ammo
	WEAPON		rgWeapons[MAX_WEAPONS];	// Weapons Array

	// counts of weapons * ammo
	WEAPON*		rgSlots[MAX_WEAPON_SLOTS + 1][MAX_WEAPON_POSITIONS + 1];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	int			riAmmo[MAX_AMMO_TYPES];					// count of each ammo type

public:
	WEAPON scoped_fg42;
	WEAPON folding_carbine;
	WEAPON gravity_knife;
	WEAPON scoped_enfield;
	WEAPON brit_knife;
	WEAPON brit_grenade;
	WEAPON ger_binoculars;

	void Init( void )
	{
		memset( rgWeapons, 0, sizeof rgWeapons );
		memset( riAmmo, 0, sizeof riAmmo );
		Reset();
	}

	void Reset( void )
	{
		iOldWeaponBits = 0;
		iOldWeaponBits2 = 0;
		memset( rgSlots, 0, sizeof rgSlots );
	}

///// WEAPON /////
	int			iOldWeaponBits;
	int			iOldWeaponBits2;

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

	void DropAllWeapons( void )
	{
		// the original clears only the first 60 pointers of rgSlots
		WEAPON **ppSlots = &rgSlots[0][0];

		for( int i = 0; i < MAX_WEAPON_SLOTS * MAX_WEAPON_POSITIONS; i++ )
			ppSlots[i] = NULL;
	}

	WEAPON* GetWeaponSlot( int slot, int pos ) { return rgSlots[slot][pos]; }

	void LoadWeaponSprites( WEAPON* wp );
	void LoadAllWeaponSprites( void );
	WEAPON* GetFirstPos( int iSlot );
	void SelectSlot( int iSlot, int fAdvance, int iDirection );
	WEAPON* GetNextActivePos( int iSlot, int iSlotPos );

	int HasAmmo( WEAPON *p );

///// AMMO /////
	AMMO GetAmmo( int iId ) { return iId; }

	void SetAmmo( int iId, int iCount ) { riAmmo[ iId ] = iCount;	}

	int CountAmmo( int iId );

	HSPRITE* GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect );
};

extern WeaponsResource gWR;

#endif // AMMOHISTORY_H
