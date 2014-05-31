
#################### Poweroff #########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(poweroff-popup C)

SET(SRCS ${SRCS}
		${WEARABLE_DIR}/poweroff-popup/micro-src/device-options.c
		${WEARABLE_DIR}/poweroff-popup/micro-src/power-off.c
		${WEARABLE_DIR}/poweroff-popup/micro-src/restart.c
		${WEARABLE_DIR}/poweroff-popup/micro-src/sound_outdoor.c
		${WEARABLE_DIR}/share/micro-common.c
)

INCLUDE_DIRECTORIES(${WEARABLE_DIR})
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/poweroff-popup/micro-src)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/share)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/popup-launcher/micro-src)

SET(PKG_MODULES ${PKG_MODULES}
		appcore-efl
		elementary
		dlog
		vconf
		edbus
		deviced
		feedback
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-pie")
ADD_DEFINITIONS("-fpie")

SET(POWEROFF_EDC "micro-poweroff.edc")

ADD_CUSTOM_TARGET(poweroff.edj
		COMMAND edje_cc -id ${WEARABLE_DIR}/edcs/icons
		${WEARABLE_DIR}/edcs/${POWEROFF_EDC}
		${WEARABLE_DIR}/${PROJECT_NAME}/poweroff.edj
		DEPENDS ${WEARABLE_DIR}/edcs/${POWEROFF_EDC}
)
ADD_DEPENDENCIES(${PROJECT_NAME} poweroff.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.poweroff-syspopup/bin)
INSTALL(FILES ${WEARABLE_DIR}/${PROJECT_NAME}/poweroff.edj DESTINATION /usr/apps/org.tizen.poweroff-syspopup/res/edje/poweroff)

# install manifest xml file
INSTALL(FILES ${WEARABLE_DIR}/data/org.tizen.poweroff-syspopup.xml DESTINATION /usr/share/packages/)

# instatll rule file
INSTALL(FILES ${WEARABLE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.poweroff-syspopup.rule DESTINATION /etc/smack/accesses2.d)
INSTALL(FILES ${WEARABLE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.system.popup.poweroff.service DESTINATION /usr/share/dbus-1/services)

