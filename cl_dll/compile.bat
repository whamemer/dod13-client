@echo off
echo Setting environment for minimal Visual C++ 6
set INCLUDE=%MSVCDir%\VC98\Include
set LIB=%MSVCDir%\VC98\Lib
set PATH=%MSVCDir%\VC98\Bin;%MSVCDir%\Common\MSDev98\Bin\;%PATH%

echo -- Compiler is MSVC6

set XASH3DSRC=..\..\Xash3D_original
set INCLUDES=-I../common -I../engine -I../pm_shared -I../game_shared -I../public -I../external -I../dlls -I../utils/fake_vgui/include
set SOURCES=../dlls/wpn_shared/30cal.cpp ^
    ../dlls/wpn_shared/amer_knife.cpp ^
    ../dlls/wpn_shared/bar.cpp ^
    ../dlls/wpn_shared/bazooka.cpp ^
    ../dlls/wpn_shared/bipod.cpp ^
    ../dlls/wpn_shared/bren.cpp ^
    ../dlls/wpn_shared/colt.cpp ^
    ../dlls/wpn_shared/enfield.cpp ^
    ../dlls/wpn_shared/fg42.cpp ^
    ../dlls/wpn_shared/garand.cpp ^
    ../dlls/wpn_shared/ger_knife.cpp ^
    ../dlls/wpn_shared/greasegun.cpp ^
    ../dlls/wpn_shared/handgrenade_ex.cpp ^
    ../dlls/wpn_shared/handgrenade.cpp ^
    ../dlls/wpn_shared/k43.cpp ^
    ../dlls/wpn_shared/kar.cpp ^
    ../dlls/wpn_shared/knife.cpp ^
    ../dlls/wpn_shared/luger.cpp ^
    ../dlls/wpn_shared/m1carbine.cpp ^
    ../dlls/wpn_shared/mg34.cpp ^
    ../dlls/wpn_shared/mg42.cpp ^
    ../dlls/wpn_shared/mp40.cpp ^
    ../dlls/wpn_shared/mp44.cpp ^
    ../dlls/wpn_shared/piat.cpp ^
    ../dlls/wpn_shared/pistol.cpp ^
    ../dlls/wpn_shared/pschreck.cpp ^
    ../dlls/wpn_shared/scoped98k.cpp ^
    ../dlls/wpn_shared/spade.cpp ^
    ../dlls/wpn_shared/spring.cpp ^
    ../dlls/wpn_shared/sten.cpp ^
    ../dlls/wpn_shared/stickgrenade_ex.cpp ^
    ../dlls/wpn_shared/stickgrenade.cpp ^
    ../dlls/wpn_shared/thompson.cpp ^
    ../dlls/wpn_shared/webley.cpp ^
	ev_hldm.cpp ^
    ev_common.cpp ^
    events.cpp ^
	hl/hl_baseentity.cpp ^
	hl/hl_events.cpp ^
	hl/hl_objects.cpp ^
	hl/hl_weapons.cpp ^
	ammo.cpp ^
	CClientEncModel.cpp ^
    cdll_int.cpp ^
    com_weapons.cpp ^
    death.cpp ^
    demo.cpp ^
    DOD_Camera.cpp ^
    dod_common.cpp ^
    dod_crosshair.cpp ^
    dod_fog.cpp ^
    dod_icons.cpp ^
    dod_map.cpp ^
    dod_objectives.cpp ^
    ../dlls/dod_shared.cpp ^
    entity.cpp ^
    GameStudioModelRenderer.cpp ^
    hud_msg.cpp ^
    hud_redraw.cpp ^
    hud_spectator.cpp ^
    hud_update.cpp ^
    hud_vgui2pring.cpp ^
    hud.cpp ^
    mortarhud.cpp ^
    scope.cpp ^
    in_camera.cpp, ^
    input.cpp ^
    inputw32.cpp ^
    menu.cpp ^
    message.cpp ^
    parsemsg.cpp ^
    ../pm_shared/pm_debug.c ^
	../pm_shared/pm_math.c ^
	../pm_shared/pm_shared.c ^
    ParticleShooter.cpp ^
    saytext.cpp ^
    status_icons.cpp ^
    statusbar.cpp ^
    studio_util.cpp ^
    StudioModelRenderer.cpp ^
    text_message.cpp ^
    train.cpp ^
    tri.cpp ^
    view.cpp ^
    weather.cpp
set DEFINES=/DCLIENT_DLL /DCLIENT_WEAPONS /Dsnprintf=_snprintf /DNO_VOICEGAMEMGR /DGOLDSOURCE_SUPPORT /DNDEBUG
set LIBS=user32.lib Winmm.lib
set OUTNAME=client.dll

cl %DEFINES% %LIBS% %SOURCES% %INCLUDES% -o %OUTNAME% /link /dll /out:%OUTNAME% /release

echo -- Compile done. Cleaning...

del *.obj *.exp *.lib *.ilk
echo -- Done.
