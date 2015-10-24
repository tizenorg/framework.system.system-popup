
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(system-servant C)

IF(SYSTEM_SERVANT STREQUAL off)
	RETURN()
ENDIF(SYSTEM_SERVANT STREQUAL off)

SET(SRCS ${CMAKE_SOURCE_DIR}/system-servant/system-servant.c)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(servant_pkgs REQUIRED
		appcore-efl
		ecore
		glib-2.0
		vconf
		dlog
		tts
		feedback
		capi-media-sound-manager
		capi-media-wav-player
)

FOREACH(flag ${servant_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g ")
SET(CMAKE_C_FLAGS_RELEASE "-O2")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_DEFINITIONS("-DAPPNAME=\"${PROJECT}\"")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${servant_pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)

