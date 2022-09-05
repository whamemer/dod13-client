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

#define MAX_WEAPON_POSITIONS		MAX_WEAPON_SLOTS

class WeaponsResource
{
private:
	WEAPON		rgWeapons[MAX_WEAPONS];

	WEAPON*		rgSlots[MAX_WEAPON_SLOTS + 1][MAX_WEAPON_POSITIONS + 8];
	int			riAmmo[MAX_AMMO_TYPES];

    WEAPON      scoped_fg42;
    WEAPON      folding_carbine;
    WEAPON      gravity_knife;
    WEAPON      scoped_enfield;
    WEAPON      brit_knife;
    WEAPON      brit_grenade;
    WEAPON      ger_binoculars;

    int iOldWeaponBits;
    int iOldWeaponBits2;

public:
	void Init( void )
	{
		memset( rgWeapons, 0, sizeof rgWeapons );
		Reset();
	}

	void Reset( void )
	{
		iOldWeaponBits = 0;
		memset( rgSlots, 0, sizeof rgSlots );
		memset( riAmmo, 0, sizeof riAmmo );
	}

///// WEAPON /////
	int			iOldWeaponBits;

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
