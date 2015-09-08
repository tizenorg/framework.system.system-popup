CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(usbhost-devices C)

SET(SRCS ${CMAKE_SOURCE_DIR}/usbhost-devices/host-devices.c
		 ${CMAKE_SOURCE_DIR}/share/common.c)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

SET(VENDOR "tizen")
SET(PACKAGE "host-devices")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")

SET(PKG_MODULES
	appcore-efl
	ui-gadget-1
	elementary
	ecore
	vconf
	dlog
	bundle
	efl-assist
	feedback
	appsvc
	deviced
	syspopup
	syspopup-caller
	notification
	tts
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -g -Wall")
MESSAGE("FLAGS: ${CMAKE_C_FLAGS}")

ADD_DEFINITIONS("-DPKGNAME=\"${PKGNAME}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PKGNAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PKGNAME} ${pkgs_LDFLAGS} "-ldl")

ADD_CUSTOM_TARGET(host-devices.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/host-devices.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/host-devices.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/host-devices.edc
)
ADD_DEPENDENCIES(${PKGNAME} host-devices.edj)

INSTALL(TARGETS ${PKGNAME} DESTINATION ${BINDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/host-devices.edj DESTINATION ${RESDIR}/edje/host-devices)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/packaging/${PKGNAME}.efl DESTINATION /etc/smack/accesses.d)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})
