cmake_minimum_required(VERSION 3.10)
project(client)

set(CLIENT_LIB client)

file(GLOB DOD_CLIENT_SRC "*.cpp")
file(GLOB DOD_WPNSH_SRC "../dlls/wpn_shared/*.cpp")
file(GLOB DOD_SHARED_SRC "../dlls/dod_shared.cpp")
file(GLOB DOD_PM_SRC "../pm_shared/*.cpp")
file(GLOB DOD_GAMSH_SRC "../game_shared/*.cpp")

list(APPEND DOD_CLIENT_SRC ${DOD_WPNSH_SRC})
list(APPEND DOD_CLIENT_SRC ${DOD_SHARED_SRC})
list(APPEND DOD_CLIENT_SRC ${DOD_PM_SRC})
list(APPEND DOD_CLIENT_SRC ${DOD_GAMSH_SRC})

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

include_directories(
	include
	../cl_dll
	../common
	../engine
	../pm_shared
	../dlls
	../game_shared
	../external/GL
	../public)

add_definitions(-DCLIENT_WEAPONS -DCLIENT_DLL -DSTDINT_H=<cstdint>)

if(NOT MSVC)
	add_definitions(-DLINUX -D_LINUX)
	add_definitions(-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -fms-extensions)
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

add_library(${CLIENT_LIB} SHARED ${DOD_CLIENT_SRC})

set_target_postfix(${CLIENT_LIB})

install(TARGETS ${CLIENT_LIB}
	DESTINATION "${GAME_DIR}/${CLIENT_INSTALL_DIR}/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	    GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE)

# Install PDB file on Windows
if(MSVC)
	install(FILES $<TARGET_PDB_FILE:${CLIENT_LIB}>
		DESTINATION "${GAME_DIR}/${CLIENT_INSTALL_DIR}/" OPTIONAL)
endif()