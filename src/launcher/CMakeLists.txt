CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/src/launcher/launcher.c
		 ${CMAKE_SOURCE_DIR}/src/launcher/popup.c
		 ${CMAKE_SOURCE_DIR}/src/launcher/app.c
		 ${CMAKE_SOURCE_DIR}/src/launcher/noti.c
)

IF(NOTIFICATION_SERVICE STREQUAL on)
	SET(SRCS ${SRCS}
			 ${CMAKE_SOURCE_DIR}/src/launcher/noti.c
	)
ENDIF(NOTIFICATION_SERVICE STREQUAL on)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/common)

SET(APPNAME "popup-launcher")

SET(PKG_MODULES ${PKG_MODULES}
		edbus
		dlog
		ecore
		elementary
		syspopup-caller
		appcore-common
		appsvc
		notification
)

INCLUDE(FindPkgConfig)
pkg_check_modules(launcher_pkgs REQUIRED ${PKG_MODULES})


FOREACH(flag ${launcher_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g -fno-omit-frame-pointer -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -g -Wall")

ADD_DEFINITIONS("-DAPPNAME=\"${APPNAME}\"")

ADD_EXECUTABLE(${APPNAME} ${SRCS})
TARGET_LINK_LIBRARIES(${APPNAME} ${launcher_pkgs_LDFLAGS} "-ldl")

INSTALL(TARGETS ${APPNAME} DESTINATION bin)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/launcher/org.tizen.system.popup.service DESTINATION /usr/share/dbus-1/system-services)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/launcher/system-apps.efl DESTINATION /etc/smack/accesses.d)

IF(NOTIFICATION_SERVICE STREQUAL on)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/datausage_warning.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/led_torch.png          DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_encryption.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_decryption.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_encryption_error.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_decryption_error.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/tima.png               DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/usb.png                DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/batt_full_icon.png     DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/batt_full_indicator.png      DESTINATION ${SYSTEM_ICONDIR})
ENDIF(NOTIFICATION_SERVICE STREQUAL on)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION ${LICENSEDIR} RENAME system-popup)
