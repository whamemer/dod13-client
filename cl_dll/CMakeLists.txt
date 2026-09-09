#
# Copyright (c) 2016 Alibek Omarov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.10)
project (CLDLL)

set (CLDLL_LIBRARY client)

add_definitions(-DCLIENT_DLL)

if(NOT MSVC)
	add_compile_options(-fno-exceptions) # GCC/Clang flag
	add_compile_options(-fno-rtti) # GCC/Clang flag
	add_compile_options(-fvisibility=hidden) # GCC/Clang flag
	add_definitions(-D_LINUX -DLINUX) # It seems enough for all non-Win32 systems
	add_definitions(-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp)
	if(NOT MINGW)
		add_definitions(-D_snprintf=snprintf -D_vsnprintf=vsnprintf)
	endif()
	if(CMAKE_BUILD_TYPE MATCHES "Release"
		OR (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo"
		AND CMAKE_SYSTEM_NAME STREQUAL "Android"))
		add_compile_options(-fno-unwind-tables -fno-asynchronous-unwind-tables) # GCC/Clang flag
		add_compile_options(-fomit-frame-pointer) # GCC/Clang flag
		add_compile_options(-ftree-vectorize) # GCC/Clang flag
		add_compile_options(-funsafe-math-optimizations) # GCC/Clang flag
	endif()
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if (GOLDSOURCE_SUPPORT)
	add_definitions(-DGOLDSOURCE_SUPPORT)
endif()

if (USE_PMAN)
	add_definitions(-DUSE_PMAN)
endif()

set (CLDLL_SOURCES
	../dlls/wpn_shared/amer_knife.cpp
	../dlls/wpn_shared/bar.cpp
	../dlls/wpn_shared/bazooka.cpp
	../dlls/wpn_shared/bipod.cpp
	../dlls/wpn_shared/bren.cpp
	../dlls/wpn_shared/colt.cpp
	../dlls/wpn_shared/enfield.cpp
	../dlls/wpn_shared/fg42.cpp
	../dlls/wpn_shared/garand.cpp
	../dlls/wpn_shared/ger_knife.cpp
	../dlls/wpn_shared/greasegun.cpp
	../dlls/wpn_shared/handgrenade.cpp
	../dlls/wpn_shared/handgrenade_ex.cpp
	../dlls/wpn_shared/kar.cpp
	../dlls/wpn_shared/knife.cpp
	../dlls/wpn_shared/k43.cpp
	../dlls/wpn_shared/luger.cpp
	../dlls/wpn_shared/mg34.cpp
	../dlls/wpn_shared/mg42.cpp
	../dlls/wpn_shared/mp40.cpp
	../dlls/wpn_shared/mp44.cpp
	../dlls/wpn_shared/m1carbine.cpp
	../dlls/wpn_shared/piat.cpp
	../dlls/wpn_shared/pistol.cpp
	../dlls/wpn_shared/pschreck.cpp
	../dlls/wpn_shared/scoped98k.cpp
	../dlls/wpn_shared/spade.cpp
	../dlls/wpn_shared/spring.cpp
	../dlls/wpn_shared/sten.cpp
	../dlls/wpn_shared/stickgrenade.cpp
	../dlls/wpn_shared/stickgrenade_ex.cpp
	../dlls/wpn_shared/thompson.cpp
	../dlls/wpn_shared/webley.cpp
	../dlls/wpn_shared/30cal.cpp
	../dlls/dod_shared.cpp
	ev_hldm.cpp
	hl/hl_baseentity.cpp
	hl/hl_events.cpp
	hl/hl_objects.cpp
	hl/hl_weapons.cpp
	ammo.cpp
	ammo_secondary.cpp
	ammohistory.cpp
	CClientEnvModel.cpp
	cdll_int.cpp
	com_weapons.cpp
	death.cpp
	demo.cpp
	DMC_BSPFile.cpp
	DOD_camera.cpp
	dod_common.cpp
	dod_crosshair.cpp
	dod_fog.cpp
	dod_icons.cpp
	dod_map.cpp
	dod_objectives.cpp
	entity.cpp
	ev_common.cpp
	events.cpp
	GameStudioModelRenderer.cpp
	hud.cpp
	hud_msg.cpp
	hud_redraw.cpp
	hud_spectator.cpp
	hud_update.cpp
	hud_vgui2print.cpp
	in_camera.cpp
	input.cpp
	input_goldsource.cpp
	input_mouse.cpp
	input_xash3d.cpp
	interpolation.cpp
	menu.cpp
	message.cpp
	mortarhud.cpp
	parsemsg.cpp
	ParticleShooter.cpp
	../pm_shared/pm_debug.c
	../pm_shared/pm_math.c
	../pm_shared/pm_shared.c
	scope.cpp
	saytext.cpp
	status_icons.cpp
	statusbar.cpp
	studio_util.cpp
	StudioModelRenderer.cpp
	text_message.cpp
	train.cpp
	tri.cpp
	util.cpp
	view.cpp
	../public/safe_snprintf.c
)

if (USE_PMAN)
	list(APPEND CLDLL_SOURCES
		../3rdparty/particleman/particleman/particlefx_source/pman_main.cpp
		../3rdparty/particleman/particleman/particlefx_source/pman_frustum.cpp
		../3rdparty/particleman/particleman/particlefx_source/pman_math.cpp
		../3rdparty/particleman/particleman/particlefx_source/pman_particlemem.cpp
		../3rdparty/particleman/particleman/particlefx_source/pman_triangleeffect.cpp
		)
endif()

if (HAVE_STRLCPY)
	add_definitions(-DHAVE_STRLCPY)
else()
	list(APPEND CLDLL_SOURCES
		../external/openbsd/strlcpy.c)
endif()

if (HAVE_STRLCAT)
	add_definitions(-DHAVE_STRLCAT)
else()
	list(APPEND CLDLL_SOURCES
		../external/openbsd/strlcat.c)
endif()

include_directories (. hl/ ../dlls ../dlls/wpn_shared ../common ../engine ../pm_shared ../game_shared ../public)

if (WIN32)
	include_directories (../external/)
endif()

if(USE_VOICEMGR)
	#set(CLDLL_SOURCES
	#	${CLDLL_SOURCES}
	#	../game_shared/voice_banmgr.cpp
	#	../game_shared/voice_status.cpp)
endif()

add_library (${CLDLL_LIBRARY} SHARED ${CLDLL_SOURCES})
target_link_libraries(${CLDLL_LIBRARY} vcs_info)

if (GOLDSOURCE_SUPPORT)
	target_link_libraries( ${CLDLL_LIBRARY} ${CMAKE_DL_LIBS} )
endif()

if(WIN32)
	target_link_libraries( ${CLDLL_LIBRARY} user32.lib )
	if (GOLDSOURCE_SUPPORT)
		target_link_libraries( ${CLDLL_LIBRARY} winmm.lib )
	endif()
endif()

set_target_properties (${CLDLL_LIBRARY} PROPERTIES
	POSITION_INDEPENDENT_CODE 1)

set_target_postfix(${CLDLL_LIBRARY})

set_property(TARGET ${CLDLL_LIBRARY} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(HAVE_LTO)
	set_property(TARGET ${CLDLL_LIBRARY} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${CLDLL_LIBRARY}
	RUNTIME DESTINATION "${GAMEDIR}/${CLIENT_INSTALL_DIR}/"
	LIBRARY DESTINATION "${GAMEDIR}/${CLIENT_INSTALL_DIR}/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE)

if(CMAKE_BUILD_TYPE MATCHES "Release" AND NOT WIN32)
	add_custom_command(TARGET ${CLDLL_LIBRARY}
			POST_BUILD DEPENDS ${CLDLL_LIBRARY}
			COMMAND ${CMAKE_STRIP} -s $<TARGET_FILE:${CLDLL_LIBRARY}>)
endif()
