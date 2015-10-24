####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(SYSTEM_POPUP STREQUAL off)
	RETURN()
ENDIF()

SET(TABLE_PATH ${CMAKE_SOURCE_DIR}/edcs/table)

#common module
SET(SRCS ${COMMON_SRCS})

IF(WATCHDOG_POPUP STREQUAL on)
	SET(SRCS ${SRCS} watchdog/watchdog.c)
ENDIF()

IF(BATTERY_POPUP STREQUAL on)
	IF(PROFILE_WEARABLE)
		SET(SRCS ${SRCS} battery/lowbattery-micro.c)
	ENDIF()
	IF (PROFILE_MOBILE)
		SET(SRCS ${SRCS} battery/lowbattery-mobile.c)
	ENDIF()
ENDIF(BATTERY_POPUP STREQUAL on)

IF(STORAGE_POPUP STREQUAL on)
	IF(PROFILE_WEARABLE)
		SET(SRCS ${SRCS} storage/lowstorage-micro.c)
	ENDIF()
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} storage/lowstorage-mobile.c)
	ENDIF()
ENDIF(STORAGE_POPUP STREQUAL on)

IF(COOLDOWN_POPUP STREQUAL on)
	IF(PROFILE_WEARABLE)
		SET(SRCS ${SRCS} cooldown/cooldown-micro.c)
	ENDIF()
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} cooldown/cooldown-mobile.c)
	ENDIF()
ENDIF(COOLDOWN_POPUP STREQUAL on)

IF(DATAUSAGE_POPUP STREQUAL on)
	IF(PROFILE_WEARABLE)
		SET(SRCS ${SRCS} datausage/datausage-micro.c)
	ENDIF()
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} datausage/datausage-mobile.c)
	ENDIF()
ENDIF(DATAUSAGE_POPUP STREQUAL on)

IF(MMC_POPUP STREQUAL on)
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} mmc/mmc-mobile.c)
	ENDIF()
ENDIF(MMC_POPUP STREQUAL on)

IF(ODE_POPUP STREQUAL on)
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} ode/ode-mobile.c)
	ENDIF()
ENDIF(ODE_POPUP STREQUAL on)

IF(USB_POPUP STREQUAL on)
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} usb/usb-mobile.c
						 usb/usbotg-mobile.c
						 usb/usb-device.c)
	ENDIF()
ENDIF(USB_POPUP STREQUAL on)

IF(RECOVERY_POPUP STREQUAL on)
	IF(PROFILE_MOBILE)
		SET(SRCS ${SRCS} recovery/recovery-mobile.c)
	ENDIF()
ENDIF(RECOVERY_POPUP STREQUAL on)


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
		efl-extension
		appsvc
)

FOREACH(flag ${system_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall -fpie")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PACKAGE} ${SRCS})

TARGET_LINK_LIBRARIES(${PACKAGE} syspopup ${system_pkgs_LDFLAGS})

INSTALL(TARGETS ${PACKAGE} DESTINATION ${BINDIR})

# install manifest file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})

# install smack efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/${PKGNAME}.efl DESTINATION /etc/smack/accesses.d)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION ${LICENSEDIR} RENAME ${PKGNAME})

############## END ##############
