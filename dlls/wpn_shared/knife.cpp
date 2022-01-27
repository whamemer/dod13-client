#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "dod_shared.h"

typedef float float_precision;

LINK_ENTITY_TO_CLASS( weapon_melee, CMeleeWeapon );

void CMeleeWeapon::Spawn( void )
{
    CMeleeWeapon::Precache();

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

    m_usKnifeFireEvent = PRECACHE_EVENT( 1, "events/weapons/melee.sc" );
}

int CMeleeWeapon::AddToplayer( CBasePlayer *pPlayer )
{
    return FALSE;
}

BOOL CMeleeWeapon::Deploy( void )
{
    return DefaultDeploy( 0, 0, KNIFE_DRAW, "knife", 1 ); //TODO
}

void CMeleeWeapon::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int i, j, k;
	float distance;
	float *minmaxs[2] = { mins, maxs };
	TraceResult tmpTrace;
	Vector vecHullEnd = tr.vecEndPos;
	Vector vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);

	if (tmpTrace.flFraction < 1.0f)
	{
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; ++i)
	{
		for (j = 0; j < 2; ++j)
		{
			for (k = 0; k < 2; ++k)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);

				if (tmpTrace.flFraction < 1.0f)
				{
					float_precision thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();

					if (thisDistance < distance)
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
    CMeleeWeapon::Swing( TRUE );
}

void CMeleeWeapon::Smack( void )
{
    DecalGunshot( &m_trHit, BULLET_NONE );
}

void CMeleeWeapon::SwingAgain( void )
{
    CMeleeWeapon::Swing( FALSE );
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
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
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
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
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
    float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0f, 1.0f );

    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( flRand > 0.0f )
        {
            iAnim = KNIFE_IDLE;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
        }
    }
}

LINK_ENTITY_TO_CLASS( weapon_amerkinfe, CAmerKnife );

void CAmerKnife::Spawn( void )
{
    CAmerKnife::Precache();
    m_iId = WEAPON_AMER_KNIFE;
    SET_MODEL( ENT( pev ), "models/w_amerk.mdl" );

    FallInit();
}

void CAmerKnife::Precache( void )
{
    PRECACHE_MODEL( "models/v_amerk.mdl" );
    PRECACHE_MODEL( "models/w_amerk.mdl" );
    PRECACHE_MODEL( "models/p_amerk.mdl" );

    CMeleeWeapon::Precache();
}

int CAmerKnife::GetItemInfo( ItemInfo *p )
{
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AMERKNIFE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_AMER_KNIFE;
	p->iWeight = AMERKNIFE_WEIGHT;

	return 1;
}

int CAmerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
    return KNIFE_SLASH1;
}

int CAmerKnife::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CAmerKnife::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}

LINK_ENTITY_TO_CLASS( weapon_spade, CSpade );

void CSpade::Spawn( void )
{
    CSpade::Precache();
    m_iId = WEAPON_SPADE;
    SET_MODEL( ENT( pev ), "models/w_spade.mdl" );

    FallInit();
}

void CSpade::Precache( void )
{
    PRECACHE_MODEL( "models/v_spade.mdl" );
    PRECACHE_MODEL( "models/w_spade.mdl" );
    PRECACHE_MODEL( "models/p_spade.mdl" );

    CMeleeWeapon::Precache();
}

int CSpade::GetItemInfo( ItemInfo *p )
{
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SPADE_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_SPADE;
	p->iWeight = SPADE_WEIGHT;

	return 1;
}

int CSpade::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
    return KNIFE_SLASH1;
}

int CSpade::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CSpade::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}

LINK_ENTITY_TO_CLASS( weapon_amerkinfe, CGerKnife );

void CGerKnife::Spawn( void )
{
    CGerKnife::Precache();
    m_iId = WEAPON_AMER_KNIFE;
    SET_MODEL( ENT( pev ), "models/w_fairbairn.mdl" );

    FallInit();
}

void CGerKnife::Precache( void )
{
    PRECACHE_MODEL( "models/v_fairbairn.mdl" );
    PRECACHE_MODEL( "models/w_fairbairn.mdl" );
    PRECACHE_MODEL( "models/p_fairbairn.mdl" );

    CGerKnife::Precache();
}

int CGerKnife::GetItemInfo( ItemInfo *p )
{
    p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
    p->pszName = STRING(pev->classname);
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GERKNIFE_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_GER_KNIFE;
	p->iWeight = GERKNIFE_WEIGHT;

	return 1;
}

int CGerKnife::GetSlashAnim( int m_iSwing )
{
    if( m_iSwing % KNIFE_SLASH2 )
    {
        if( m_iSwing % KNIFE_SLASH2 == KNIFE_SLASH1 )
            return KNIFE_SLASH2;
    }
    return KNIFE_SLASH1;
}

int CGerKnife::GetDrawAnim( void )
{
    return KNIFE_DRAW;
}

int CGerKnife::GetIdleAnim( void )
{
    return KNIFE_IDLE;
}