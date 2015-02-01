
########################### crash popup ###########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(VENDOR "tizen")
SET(PACKAGE "crash-popup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(TABLE_PATH ${CMAKE_SOURCE_DIR}/edcs/table)

SET(SRCS
		${CMAKE_SOURCE_DIR}/crash-popup/micro-src/crash.c
		${CMAKE_SOURCE_DIR}/share/micro-common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)
pkg_check_modules(crash_pkgs REQUIRED
		appcore-efl
		elementary
		syspopup
		ecore
		deviced
		pkgmgr-info
		dlog
		efl-assist
		feedback
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/crash-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

FOREACH(flag ${crash_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${crash_pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.crash-popup/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.crash-popup.xml DESTINATION /usr/share/packages/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PACKAGE}/micro-src/org.tizen.crash-popup.efl DESTINATION /etc/smack/accesses.d)

INSTALL(FILES ${TABLE_PATH}/system-font.xml  DESTINATION ${RESDIR}/table)
INSTALL(FILES ${TABLE_PATH}/system-color.xml DESTINATION ${RESDIR}/table)

################################# End ##############################################

