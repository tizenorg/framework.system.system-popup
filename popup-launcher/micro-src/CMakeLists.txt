CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/popup-launcher/micro-src/launcher.c
		 ${CMAKE_SOURCE_DIR}/popup-launcher/micro-src/popup.c
		 ${CMAKE_SOURCE_DIR}/popup-launcher/micro-src/app.c
)

SET(APPNAME "popup-launcher")
SET(SYSTEM_APPS "system-apps")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/popup-launcher/micro-src)

SET(PKG_MODULES ${PKG_MODULES}
		edbus
		dlog
		ecore
		feedback
		syspopup-caller
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

INSTALL(FILES ${CMAKE_SOURCE_DIR}/${APPNAME}/micro-src/org.tizen.system.popup.service DESTINATION /usr/share/dbus-1/services)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${APPNAME}/micro-src/system-apps.efl DESTINATION /etc/smack/accesses.d)
