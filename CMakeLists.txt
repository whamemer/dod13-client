cmake_minimum_required(VERSION 3.10)
project(dod13-client)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(LibraryNaming)

option(BUILD_CLIENT "Build client library." ON)

set(GAME_DIR "dod" CACHE STRING "Game directory name.")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Client library location.")

if(NOT WIN32)
	add_link_options(-Wl,--no-undefined)
endif()

if(BUILD_CLIENT)
	add_subdirectory(cl_dll)
endif()