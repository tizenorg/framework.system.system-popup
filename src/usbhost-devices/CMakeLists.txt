CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(USBHOST_LIST_APP STREQUAL off)
	RETURN()
ENDIF(USBHOST_LIST_APP STREQUAL off)

SET(SRCS host-devices.c)

SET(DIRNAME "usbhost-devices")
SET(VENDOR "tizen")
SET(PACKAGE "host-devices")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")
SET(SRCDIR "${CMAKE_SOURCE_DIR}/src/${DIRNAME}")

SET(PKG_MODULES
	appcore-efl
	ui-gadget-1
	elementary
	ecore
	vconf
	dlog
	bundle
	efl-extension
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
		COMMAND edje_cc -id ${SRCDIR}/edcs
		${SRCDIR}/edcs/host-devices.edc ${SRCDIR}/host-devices.edj
		DEPENDS ${SRCDIR}/edcs/host-devices.edc
)
ADD_DEPENDENCIES(${PKGNAME} host-devices.edj)

INSTALL(TARGETS ${PKGNAME} DESTINATION ${BINDIR})
INSTALL(FILES ${SRCDIR}/host-devices.edj DESTINATION ${RESDIR}/edje/host-devices)
INSTALL(FILES ${SRCDIR}/${PKGNAME}.efl DESTINATION /etc/smack/accesses.d)
INSTALL(FILES ${SRCDIR}/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION ${LICENSEDIR} RENAME ${PKGNAME})
