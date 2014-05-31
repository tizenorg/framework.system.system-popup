
########################### low memory ###########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PACKAGE "crash-popup")

SET(SRCS
		${WEARABLE_DIR}/crash-popup/micro-src/crash.c
		${WEARABLE_DIR}/share/micro-common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
		appcore-efl
		elementary
		syspopup
		ecore
		deviced
		dlog
)

INCLUDE_DIRECTORIES(${WEARABLE_DIR})
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/crash-popup)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/share)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.crash-popup/bin)
INSTALL(FILES ${WEARABLE_DIR}/data/org.tizen.crash-popup.xml DESTINATION /usr/share/packages/)
INSTALL(FILES ${WEARABLE_DIR}/${PACKAGE}/micro-src/org.tizen.crash-popup.rule DESTINATION /etc/smack/accesses2.d)

################################# End ##############################################

