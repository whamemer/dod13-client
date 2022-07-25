#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "dod_shared.h"

void CMeleeWeapon::Spawn( int weapon_id )
{	
	Precache();
	m_iId = weapon_id;

	FallInit();
}

void CMeleeWeapon::Precache( void )
{
    PRECACHE_SOUND( "weapons/knife_deploy1.wav" );
    PRECACHE_SOUND( "weapons/knife_hit1.wav" );
    PRECACHE_SOUND( "weapons/knife_hit2.wav" );
    PRECACHE_SOUND( "weapons/knife_hit3.wav" );
    PRECACHE_SOUND( "weapons/knife_hit4.wav" );
    PRECACHE_SOUND( "weapons/knife_hitwall1.wav" );
    PRECACHE_SOUND( "weapons/knife_slash1.wav" );
    PRECACHE_SOUND( "weapons/knife_slash2.wav" );
    PRECACHE_SOUND( "weapons/knifeswing.wav" );
    PRECACHE_SOUND( "weapons/knifeswing2.wav" );

    m_iFireEvent = PRECACHE_EVENT( 1, "events/weapons/melee.sc" );
}

int CMeleeWeapon::AddToplayer( CBasePlayer *pPlayer )
{
    return FALSE;
}

void CMeleeWeapon::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void FindHullIntersectionDOD( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult	tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2.0f );
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if( tmpTrace.flFraction < 1.0f )
	{
		tr = tmpTrace;
		return;
	}

	for( i = 0; i < 2; i++ )
	{
		for( j = 0; j < 2; j++ )
		{
			for( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if( tmpTrace.flFraction < 1.0f )
				{
					float thisDistance = ( tmpTrace.vecEndPos - vecSrc ).Length();
					if( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CMeleeWeapon::PrimaryAttack( void )
{
    CMeleeWeapon::Swing( 1 );
}

void CMeleeWeapon::Smack( void )
{
    DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}

void CMeleeWeapon::SwingAgain( void )
{
    CMeleeWeapon::Swing( 0 );
}

int CMeleeWeapon::Swing( int fFirst )
{
	int fDidHit = FALSE;
	TraceResult tr;
	Vector vecSrc, vecEnd;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	vecSrc = m_pPlayer->GetGunPosition();
	vecEnd = vecSrc + gpGlobals->v_forward * 48.0f;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr );

	if ( tr.flFraction >= 1.0f )
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);

		if ( tr.flFraction < 1.0f )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if ( !pHit || pHit->IsBSPModel() )
			{
				FindHullIntersectionDOD( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			}

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if ( tr.flFraction >= 1.0f )
	{
		if ( fFirst )
		{
			switch ( ( m_iSwing++ ) % 2)
			{
			case 0: SendWeaponAnim( KNIFE_SLASH1, UseDecrement() != FALSE ); break;
			case 1: SendWeaponAnim( KNIFE_SLASH2, UseDecrement() != FALSE ); break;
			}

			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.35);
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;

			// play wiff or swish sound
			if ( RANDOM_LONG( 0, 1 ) )
				EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94 );
			else
				EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94 );

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		// hit
		fDidHit = TRUE;

		switch ( ( m_iSwing++ ) % 2)
		{
		case 0: SendWeaponAnim( KNIFE_SLASH1, UseDecrement() != FALSE ); break;
		case 1: SendWeaponAnim( KNIFE_SLASH2, UseDecrement() != FALSE ); break;
		}

			m_flNextPrimaryAttack = GetNextAttackDelay( 0.4 );
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;

		// play thwack, smack, or dong sound
		float flVol = 1.0f;
		int fHitWorld = TRUE;

		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		ClearMultiDamage();

		if ( m_flNextPrimaryAttack + 0.4f < UTIL_WeaponTimeBase() )
			pEntity->TraceAttack( m_pPlayer->pev, 20, gpGlobals->v_forward, &tr, ( DMG_NEVERGIB | DMG_BULLET ) );
		else
			pEntity->TraceAttack( m_pPlayer->pev, 15, gpGlobals->v_forward, &tr, ( DMG_NEVERGIB | DMG_BULLET ) );

		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		if ( pEntity != NULL )	// -V595
		{
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// play thwack or smack sound
				switch ( RANDOM_LONG( 0, 3 ) )
				{
				case 0: EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM); break;
				case 1: EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM); break;
				case 2: EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM); break;
				case 3: EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM); break;
				}

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return TRUE;
				else
					flVol = 0.1f;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			//fvolbar = 1.0;

			if (RANDOM_LONG(0, 1) > 1)
			{
				fHitWorld = FALSE;
			}
		}

		if (!fHitWorld)
		{
			// delay the decal a bit
			m_trHit = tr;
			SetThink( &CMeleeWeapon::Smack );

			pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;
			m_pPlayer->m_iWeaponVolume = int( flVol * KNIFE_WALLHIT_VOLUME );
		}
		else
		{
			// also play knife strike
			EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG( 0, 3 ) + 98 );
		}
	return fDidHit;
}

int CMeleeWeapon::Stab( int fFirst )
{
	int fDidHit = FALSE;
	TraceResult tr;
	Vector vecSrc, vecEnd;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	vecSrc = m_pPlayer->GetGunPosition();
	vecEnd = vecSrc + gpGlobals->v_forward * 32.0f;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr );

	if ( tr.flFraction >= 1.0f )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr );

		if ( tr.flFraction < 1.0f )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			if ( !pHit || pHit->IsBSPModel() )
			{
				FindHullIntersectionDOD( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			}

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if ( tr.flFraction >= 1.0f )
	{
		if ( fFirst )
		{
			SendWeaponAnim( KNIFE_SLASH1, UseDecrement() != FALSE );
            SendWeaponAnim( KNIFE_SLASH2, UseDecrement() != FALSE );

			m_flNextPrimaryAttack = GetNextAttackDelay( 1.0 );
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;

			// play wiff or swish sound
			if ( RANDOM_LONG( 0, 1 ) )
				EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94 );
			else
				EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94 );

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		// hit
		fDidHit = TRUE;

		SendWeaponAnim( KNIFE_SLASH1, UseDecrement() != FALSE );
        SendWeaponAnim( KNIFE_SLASH2, UseDecrement() != FALSE );

		m_flNextPrimaryAttack = GetNextAttackDelay( 1.1 );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;

		// play thwack, smack, or dong sound
		float flVol = 1.0f;
		int fHitWorld = TRUE;

		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		float flDamage = 65.0f;

		if ( pEntity != NULL && pEntity->IsPlayer() )
		{
			Vector2D vec2LOS;
			float flDot;
			Vector vMyForward = gpGlobals->v_forward;

			UTIL_MakeVectors( pEntity->pev->angles );

			vec2LOS = vMyForward.Make2D();
			vec2LOS = vec2LOS.Normalize();

			flDot = DotProduct( vec2LOS, gpGlobals->v_forward.Make2D() );

			//Triple the damage if we are stabbing them in the back.
			if ( flDot > 0.80f )
			{
				flDamage *= 3.0f;
			}
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		ClearMultiDamage();

		pEntity->TraceAttack( m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, (DMG_NEVERGIB | DMG_BULLET) );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		if ( pEntity != NULL )	// -V595
		{
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				EMIT_SOUND( m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_stab.wav", VOL_NORM, ATTN_NORM );
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if ( !pEntity->IsAlive() )
					return TRUE;
				else
					flVol = 0.1f;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if ( fHitWorld )
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR );
			//fvolbar = 1.0;

			if ( RANDOM_LONG(0, 1) > 1 )
			{
				fHitWorld = FALSE;
			}
		}

		if ( !fHitWorld )
		{
			// delay the decal a bit
			m_trHit = tr;
			m_pPlayer->m_iWeaponVolume = int( flVol * KNIFE_WALLHIT_VOLUME );

			SetThink(&CMeleeWeapon::Smack);
			pev->nextthink = UTIL_WeaponTimeBase() + 0.2f;
		}
		else
		{
			// also play knife strike
			EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG( 0, 3 ) + 98 );
		}
	}

	return fDidHit;
}

void CMeleeWeapon::WeaponIdle( void )
{
    int iAnim;

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() )
        {
			if( m_flNextSecondaryAttack <= UTIL_WeaponTimeBase() )
			{
				if( GetIdleAnim() >= 0 )
					iAnim = KNIFE_IDLE;
				
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
			}
        }
    }
}