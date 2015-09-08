CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/popup-launcher/src/launcher.c
		 ${CMAKE_SOURCE_DIR}/popup-launcher/src/popup.c
		 ${CMAKE_SOURCE_DIR}/popup-launcher/src/app.c
		 ${CMAKE_SOURCE_DIR}/popup-launcher/src/noti.c
		 ${CMAKE_SOURCE_DIR}/share/common.c
)

SET(APPNAME "popup-launcher")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/popup-launcher/src)

SET(PKG_MODULES ${PKG_MODULES}
		edbus
		dlog
		ecore
		syspopup-caller
		appsvc
		notification
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${PKG_MODULES})


FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g -fno-omit-frame-pointer -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -g -Wall")

ADD_DEFINITIONS("-DAPPNAME=\"${APPNAME}\"")

ADD_EXECUTABLE(${APPNAME} ${SRCS})
TARGET_LINK_LIBRARIES(${APPNAME} ${pkgs_LDFLAGS} "-ldl")

INSTALL(TARGETS ${APPNAME} DESTINATION bin)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/${APPNAME}/src/org.tizen.system.popup.service DESTINATION /usr/share/dbus-1/services)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${APPNAME}/src/system-apps.efl DESTINATION /etc/smack/accesses.d)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/datausage_warning.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/led_torch.png          DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_encryption.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_decryption.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_encryption_error.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/sdcard_decryption_error.png  DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/tima.png               DESTINATION ${SYSTEM_ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/usb.png                DESTINATION ${SYSTEM_ICONDIR})
