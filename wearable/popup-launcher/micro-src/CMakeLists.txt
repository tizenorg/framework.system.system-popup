CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${WEARABLE_DIR}/popup-launcher/micro-src/launcher.c
		 ${WEARABLE_DIR}/popup-launcher/micro-src/popup.c
		 ${WEARABLE_DIR}/share/micro-common.c
)

SET(APPNAME "popup-launcher")

INCLUDE_DIRECTORIES(${WEARABLE_DIR}/share)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/popup-launcher/micro-src)

SET(PKG_MODULES ${PKG_MODULES}
		edbus
		dlog
		ecore
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

INSTALL(FILES ${WEARABLE_DIR}/${APPNAME}/micro-src/org.tizen.system.popup.service DESTINATION /usr/share/dbus-1/services)

# instatll rule file
INSTALL(FILES ${WEARABLE_DIR}/${APPNAME}/micro-src/system-apps.rule DESTINATION /etc/smack/accesses2.d)
