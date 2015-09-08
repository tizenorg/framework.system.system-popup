####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(TABLE_PATH ${CMAKE_SOURCE_DIR}/edcs/table)

SET(SRCS common/core.c
		 common/popup-common.c
		 common/popup-ui.c
)

IF("$ENV{CFLAGS}" MATCHES "-DSYSTEM_APPS_MICRO")
	SET(SRCS ${SRCS} common/popup-ui-micro.c)
	SET(SRCS ${SRCS} flightmode/flightmode.c)
	SET(SRCS ${SRCS} mobiledata/mobiledata.c)
	SET(SRCS ${SRCS} battery/lowbattery.c)
#	SET(SRCS ${SRCS} cooldown/cooldown.c)
	SET(SRCS ${SRCS} watchdog/watchdog.c)
	SET(SRCS ${SRCS} datausage/datausage.c)

	IF("$ENV{CFLAGS}" MATCHES "-DSYSTEM_APPS_CIRCLE")
		SET(SRCS ${SRCS} common/popup-ui-micro-circle.c)
	ELSE()
		IF("$ENV{CFLAGS}" MATCHES "-DSYSTEM_APPS_MICRO_3")
			SET(SRCS ${SRCS} common/popup-ui-micro-b3.c)
		ELSE()
			SET(SRCS ${SRCS} common/popup-ui-micro-b2.c)
		ENDIF()
	ENDIF()

ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/common)

SET(VENDOR "tizen")
SET(PACKAGE "system-syspopup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")

INCLUDE(FindPkgConfig)
pkg_check_modules(system_pkgs REQUIRED
		appcore-efl
		bundle
		dlog
		ecore
		evas
		syspopup
		vconf
		elementary
		deviced
		feedback
		efl-assist
		appsvc
)

FOREACH(flag ${system_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PACKAGE} ${SRCS})

TARGET_LINK_LIBRARIES(${PACKAGE} syspopup ${system_pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(system.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/system.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/system.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/system.edc
)
ADD_DEPENDENCIES(${PACKAGE} system.edj)

INSTALL(TARGETS ${PACKAGE} DESTINATION ${BINDIR})

# install manifest file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})

# install smack efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/packaging/${PKGNAME}.efl DESTINATION /etc/smack/accesses.d)

INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/system.edj DESTINATION ${RESDIR}/edje/system)

INSTALL(FILES ${TABLE_PATH}/system-font.xml  DESTINATION ${RESDIR}/table)
INSTALL(FILES ${TABLE_PATH}/system-color.xml DESTINATION ${RESDIR}/table)

############## END ##############
