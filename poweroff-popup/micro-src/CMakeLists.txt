
#################### Poweroff #########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(poweroff-popup C)

SET(SRCS ${SRCS}
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/device-options.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/power-off.c
#		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/flightmode.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/double-data.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/double-data-flightmode.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/double-data-mobiledata.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/restart.c
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/sound_outdoor.c
		${CMAKE_SOURCE_DIR}/share/micro-common.c
)

IF("$ENV{CFLAGS}" MATCHES "-DSYSTEM_APPS_MICRO_3")
IF(${SIMULATOR} STREQUAL "no")
SET(SRCS ${SRCS}
		${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src/power-saving.c
)
ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/poweroff-popup/micro-src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/popup-launcher/micro-src)

SET(PKG_MODULES ${PKG_MODULES}
		appcore-efl
		elementary
		ecore-x
		utilX
		dlog
		vconf
		edbus
		deviced
		feedback
		efl-assist
		appsvc
		capi-media-sound-manager
)

INCLUDE(FindPkgConfig)
pkg_check_modules(poweroff_pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${poweroff_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

IF("$ENV{CFLAGS}" MATCHES "-DSYSTEM_APPS_MICRO_3")
ADD_DEFINITIONS("-DSYSTEM_APPS_MICRO_3")
ENDIF()

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${poweroff_pkgs_LDFLAGS} "-pie")
ADD_DEFINITIONS("-fpie")

SET(POWEROFF_EDC "micro-poweroff.edc")

ADD_CUSTOM_TARGET(poweroff.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/${POWEROFF_EDC} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/poweroff.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/${POWEROFF_EDC}
)
ADD_DEPENDENCIES(${PROJECT_NAME} poweroff.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.poweroff-syspopup/bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/poweroff.edj DESTINATION /usr/apps/org.tizen.poweroff-syspopup/res/edje/poweroff)

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.poweroff-syspopup.xml DESTINATION /usr/share/packages/)

# instatll rule file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.poweroff-syspopup.rule DESTINATION /etc/smack/accesses2.d)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.system.popup.poweroff.service DESTINATION /usr/share/dbus-1/services)

