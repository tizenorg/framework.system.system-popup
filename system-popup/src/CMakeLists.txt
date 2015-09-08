####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS core.c
		 recovery.c
		 watchdog.c
		 usbotg_mount_failed.c
		 usbotg_removed_unsafe.c
		 ode_error.c
		 cooldown_poweron.c
		 cooldown_poweroff.c
		 safemode.c
		 accessibility.c
		 acc-tts.c
		 acc-assistive-light.c
		 acc-negative-colors.c
		 acc-zoom.c
		 ${CMAKE_SOURCE_DIR}/share/common.c
		 )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/system-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/popup-launcher/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

SET(VENDOR "tizen")
SET(PACKAGE "system-syspopup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
		appcore-efl
		bundle
		dlog
		ecore
		evas
		syspopup
		vconf
		elementary
		deviced
		)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PACKAGE} ${SRCS})

TARGET_LINK_LIBRARIES(${PACKAGE} syspopup  ${pkgs_LDFLAGS})

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

############## END ##############
