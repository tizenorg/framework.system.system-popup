#################### Poweroff #########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(poweroff-popup C)

SET(PACKAGE ${PROJECT_NAME})

SET(SRCS
		${CMAKE_SOURCE_DIR}/poweroff-popup/src/poweroff.c
		${CMAKE_SOURCE_DIR}/share/common.c
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/poweroff-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

SET(PKG_MODULES ${PKG_MODULES}
		appcore-efl
		elementary
		dlog
		deviced
		utilX
		evas
		ecore
		ecore-x
		vconf
		syspopup
		syspopup-caller
		feedback
		appsvc
)

INCLUDE(FindPkgConfig)
pkg_check_modules(poweroff_pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${poweroff_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${poweroff_pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.poweroff-syspopup/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.poweroff-syspopup.xml DESTINATION /usr/share/packages/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src/org.tizen.poweroff-syspopup.efl DESTINATION /etc/smack/accesses.d)

