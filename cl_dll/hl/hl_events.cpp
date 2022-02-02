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

#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// HLDM
void EV_FireGlock1( struct event_args_s *args  );
void EV_FireGlock2( struct event_args_s *args  );
void EV_FireShotGunSingle( struct event_args_s *args  );
void EV_FireShotGunDouble( struct event_args_s *args  );
void EV_FireMP5( struct event_args_s *args  );
void EV_FireMP52( struct event_args_s *args  );
void EV_FirePython( struct event_args_s *args  );
void EV_FireGauss( struct event_args_s *args  );
void EV_SpinGauss( struct event_args_s *args  );
void EV_Crowbar( struct event_args_s *args );
void EV_FireCrossbow( struct event_args_s *args );
void EV_FireCrossbow2( struct event_args_s *args );
void EV_FireRpg( struct event_args_s *args );
void EV_EgonFire( struct event_args_s *args );
void EV_EgonStop( struct event_args_s *args );
void EV_HornetGunFire( struct event_args_s *args );
void EV_TripmineFire( struct event_args_s *args );
void EV_SnarkFire( struct event_args_s *args );

void EV_TrainPitchAdjust( struct event_args_s *args ); // DOD too

// DOD
void EV_FireColt( struct event_args_s *args );
void EV_FireLuger( struct event_args_s *args );
void EV_FireM1Carbine( struct event_args_s *args );
void EV_FireGarand( struct event_args_s *args );
void EV_FireScopedKar( struct event_args_s *args );
void EV_FireThompson( struct event_args_s *args );
void EV_FireMP44( struct event_args_s *args );
void EV_FireSpring( struct event_args_s *args );
void EV_FireKar( struct event_args_s *args );
void EV_FireMP40( struct event_args_s *args );
void EV_Knife( struct event_args_s *args );
void EV_FireBAR( struct event_args_s *args );
void EV_FireMG42( struct event_args_s *args );
void EV_FireMG34( struct event_args_s *args );
void EV_Fire30CAL( struct event_args_s *args );
void EV_FireGreaseGun( struct event_args_s *args );
void EV_FireFG42( struct event_args_s *args );
void EV_FireK43( struct event_args_s *args );
void EV_FireEnfield( struct event_args_s *args );
void EV_FireSten( struct event_args_s *args );
void EV_FireBren( struct event_args_s *args );
void EV_FireWebley( struct event_args_s *args );
void EV_FireScopedEnfield( struct event_args_s *args );
void EV_FireBazooka( struct event_args_s *args );
void EV_FirePschreck( struct event_args_s *args );
void EV_FirePIAT( struct event_args_s *args );
void EV_FireMortar( struct event_args_s *args );
void EV_FireMelee( struct event_args_s *args );
void EV_Pain( struct event_args_s *args );
void EV_Smoke( struct event_args_s *args );
void EV_BloodSprite( struct event_args_s *args );
void EV_BloodStream( struct event_args_s *args );
void EV_BulletTracers( struct event_args_s *args );
void EV_BubbleTrails( struct event_args_s *args );
void EV_Bubbles( struct event_args_s *args );
void EV_Explosion( struct event_args_s *args );
void EV_SparkShower( struct event_args_s *args );
void EV_PlayWhizz( struct event_args_s *args );
void EV_USVoice( struct event_args_s *args );
void EV_GERVoice( struct event_args_s *args );
void EV_BodyDamage( struct event_args_s *args );
void EV_RoundReleaseSound( struct event_args_s *args );
void EV_DoDCamera( struct event_args_s *args );
void EV_PopHelmet( struct event_args_s *args );
void EV_RoundReset( struct event_args_s *args );
void EV_Overheat( struct event_args_s *args );
void EV_RocketTrail( struct event_args_s *args );
void EV_MortarShell( struct event_args_s *args );
}

/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	gEngfuncs.pfnHookEvent( "events/glock1.sc", EV_FireGlock1 );
	gEngfuncs.pfnHookEvent( "events/glock2.sc", EV_FireGlock2 );
	gEngfuncs.pfnHookEvent( "events/shotgun1.sc", EV_FireShotGunSingle );
	gEngfuncs.pfnHookEvent( "events/shotgun2.sc", EV_FireShotGunDouble );
	gEngfuncs.pfnHookEvent( "events/mp5.sc", EV_FireMP5 );
	gEngfuncs.pfnHookEvent( "events/mp52.sc", EV_FireMP52 );
	gEngfuncs.pfnHookEvent( "events/python.sc", EV_FirePython );
	gEngfuncs.pfnHookEvent( "events/gauss.sc", EV_FireGauss );
	gEngfuncs.pfnHookEvent( "events/gaussspin.sc", EV_SpinGauss );
	gEngfuncs.pfnHookEvent( "events/train.sc", EV_TrainPitchAdjust );
	gEngfuncs.pfnHookEvent( "events/crowbar.sc", EV_Crowbar );
	gEngfuncs.pfnHookEvent( "events/crossbow1.sc", EV_FireCrossbow );
	gEngfuncs.pfnHookEvent( "events/crossbow2.sc", EV_FireCrossbow2 );
	gEngfuncs.pfnHookEvent( "events/rpg.sc", EV_FireRpg );
	gEngfuncs.pfnHookEvent( "events/egon_fire.sc", EV_EgonFire );
	gEngfuncs.pfnHookEvent( "events/egon_stop.sc", EV_EgonStop );
	gEngfuncs.pfnHookEvent( "events/firehornet.sc", EV_HornetGunFire );
	gEngfuncs.pfnHookEvent( "events/tripfire.sc", EV_TripmineFire );
	gEngfuncs.pfnHookEvent( "events/snarkfire.sc", EV_SnarkFire );

	// DOD events
	gEngfuncs.pfnHookEvent( "events/weapons/colt.sc", EV_FireColt );
	gEngfuncs.pfnHookEvent( "events/weapons/luger.sc", EV_FireLuger );
	gEngfuncs.pfnHookEvent( "events/weapons/m1carbine.sc", EV_FireM1Carbine );
	gEngfuncs.pfnHookEvent( "events/weapons/garand.sc", EV_FireGarand );
	gEngfuncs.pfnHookEvent( "events/weapons/scopedkar.sc", EV_FireScopedKar );
	gEngfuncs.pfnHookEvent( "events/weapons/thompson.sc", EV_FireThompson );
	gEngfuncs.pfnHookEvent( "events/weapons/mp44.sc", EV_FireMP44 );
	gEngfuncs.pfnHookEvent( "events/weapons/spring.sc", EV_FireSpring );
	gEngfuncs.pfnHookEvent( "events/weapons/kar.sc", EV_FireKar );
	gEngfuncs.pfnHookEvent( "events/weapons/mp40.sc", EV_FireMP40 );
	gEngfuncs.pfnHookEvent( "events/weapons/knife.sc", EV_Knife );
	gEngfuncs.pfnHookEvent( "events/weapons/bar.sc", EV_FireBAR );
	gEngfuncs.pfnHookEvent( "events/weapons/mg42.sc", EV_FireMG42 );
	gEngfuncs.pfnHookEvent( "events/weapons/mg34.sc", EV_FireMG34 );
	gEngfuncs.pfnHookEvent( "events/weapons/30cal.sc", EV_Fire30CAL );
	gEngfuncs.pfnHookEvent( "events/weapons/greasegun.sc", EV_FireGreaseGun );
	gEngfuncs.pfnHookEvent( "events/weapons/fg42.sc", EV_FireFG42 );
	gEngfuncs.pfnHookEvent( "events/weapons/k43.sc", EV_FireK43 );
	gEngfuncs.pfnHookEvent( "events/weapons/enfield.sc", EV_FireEnfield );
	gEngfuncs.pfnHookEvent( "events/weapons/sten.sc", EV_FireSten );
	gEngfuncs.pfnHookEvent( "events/weapons/bren.sc", EV_FireBren );
	gEngfuncs.pfnHookEvent( "events/weapons/webley.sc", EV_FireWebley );
	gEngfuncs.pfnHookEvent( "events/weapons/scopedenfield.sc", EV_FireScopedEnfield );
	gEngfuncs.pfnHookEvent( "events/weapons/bazooka.sc", EV_FireBazooka );
	gEngfuncs.pfnHookEvent( "events/weapons/pschreck.sc", EV_FirePschreck );
	gEngfuncs.pfnHookEvent( "events/weapons/piat.sc", EV_FirePIAT );
	gEngfuncs.pfnHookEvent( "events/weapons/mortar.sc", EV_FireMortar );
	gEngfuncs.pfnHookEvent( "events/weapons/melee.sc", EV_FireMelee );
	gEngfuncs.pfnHookEvent( "events/misc/pain.sc", EV_Pain );
	gEngfuncs.pfnHookEvent( "events/effects/smoke.sc", EV_Smoke );
	gEngfuncs.pfnHookEvent( "events/effects/blood.sc", EV_BloodSprite );
	gEngfuncs.pfnHookEvent( "events/effects/bloodstream.sc", EV_BloodStream );
	gEngfuncs.pfnHookEvent( "events/effects/tracers.sc", EV_BulletTracers );
	gEngfuncs.pfnHookEvent( "events/effects/bubbletrails.sc", EV_BubbleTrails );
	gEngfuncs.pfnHookEvent( "events/effects/bubbles.sc", EV_Bubbles );
	gEngfuncs.pfnHookEvent( "events/effects/explosion.sc", EV_Explosion );
	gEngfuncs.pfnHookEvent( "events/effects/sparkshower.sc", EV_SparkShower );
	gEngfuncs.pfnHookEvent( "events/misc/whizz.sc", EV_PlayWhizz );
	gEngfuncs.pfnHookEvent( "events/misc/usvoice.sc", EV_USVoice );
	gEngfuncs.pfnHookEvent( "events/misc/gervoice.sc", EV_GERVoice );
	gEngfuncs.pfnHookEvent( "events/effects/bodydamage.sc", EV_BodyDamage );
	gEngfuncs.pfnHookEvent( "events/misc/roundrestart.sc", EV_RoundReleaseSound );
	gEngfuncs.pfnHookEvent( "events/misc/camera.sc", EV_DoDCamera );
	gEngfuncs.pfnHookEvent( "events/effects/helmet.sc", EV_PopHelmet );
	gEngfuncs.pfnHookEvent( "events/misc/roundreset.sc", EV_RoundReset );
	gEngfuncs.pfnHookEvent( "events/effects/overheat.sc", EV_Overheat );
	gEngfuncs.pfnHookEvent( "events/effects/rockettrail.sc", EV_RocketTrail );
	gEngfuncs.pfnHookEvent( "events/effects/mortarshell.sc", EV_MortarShell );
}