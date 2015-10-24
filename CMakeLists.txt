####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(system-popup C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(SYSTEM_RESDIR "${PREFIX}/share/system-apps/res")
SET(SYSTEM_ICONDIR "${SYSTEM_RESDIR}/icons")
SET(LICENSEDIR "${PREFIX}/share/license")

SET(LOCALE_DIR "/usr/share/locale")
SET(LANG_DOMAIN "system-apps")

IF(PROFILE STREQUAL mobile)
	OPTION(PROFILE_MOBILE "Mobile device" ON)
ENDIF()
IF(PROFILE STREQUAL wearable)
	OPTION(PROFILE_WEARABLE "Wearable device" ON)
ENDIF()
IF(PROFILE STREQUAL tv)
	OPTION(PROFILE_TV "TV device" ON)
ENDIF()

ADD_DEFINITIONS("-DLOCALE_DIR=\"${LOCALE_DIR}\"")
ADD_DEFINITIONS("-DLANG_DOMAIN=\"${LANG_DOMAIN}\"")
ADD_DEFINITIONS("-DSYSTEM_ICONDIR=\"${SYSTEM_ICONDIR}\"")
ADD_DEFINITIONS("-DPROFILE=\"${PROFILE}\"")

SET(COMMON_SRCS
		${CMAKE_SOURCE_DIR}/src/common/core.c
		${CMAKE_SOURCE_DIR}/src/common/popup-common.c
		${CMAKE_SOURCE_DIR}/src/common/popup-ui.c
		${CMAKE_SOURCE_DIR}/src/common/popup-ui-normal.c
)

# SUbmodules
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/poweroff)
ADD_SUBDIRECTORY(src/crash)
ADD_SUBDIRECTORY(src/usbhost-devices)
ADD_SUBDIRECTORY(src/launcher)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(system-servant)
ADD_SUBDIRECTORY(signal-sender)

############## END ##############
