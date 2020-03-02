cmake_minimum_required(VERSION 3.5)
project(window-follower)

include(${CMAKE_SOURCE_DIR}/external/FindLibObs.cmake)
find_package(LibObs REQUIRED)
set(window-follower_SOURCES
	getHWND.c
	setup.c
	tick.c
	properties.c
	)
	
set(window-follower_HEADERS
	capture-internals.h
	getHWND.h
	window-follower.h
	)	
	
include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	"${LIBOBS_INCLUDE_DIR}/../deps/ipc-util")	
	
add_library(window-follower MODULE
	${window-follower_SOURCES}
	${window-follower_HEADERS})
	
target_link_libraries(window-follower
	libobs)

if(UNIX)
	message(FATAL_ERROR "Not designed for non windows systems")

	if(ARCH EQUAL 64)
		set(ARCH_NAME "x86_64")
	else()
		set(ARCH_NAME "i686")
	endif()

	set_target_properties(window-follower PROPERTIES PREFIX "")

	install(TARGETS window-follower
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/window-follower/
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/window-follower/")
endif()

	
if(WIN32)
	set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
	if(OBS_FRONTEND_LIB EQUAL "OBS_FRONTEND_LIB-NOTFOUND")
		message(FATAL_ERROR "OBS_FRONTEND_LIB NOTFOUND")
	endif()
	
		target_link_libraries(window-follower
		"${OBS_FRONTEND_LIB}")
				
endif()

