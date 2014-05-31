CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(usbotg-popup C)

SET(SRCS src/usbotg-syspopup.c)

SET(VENDOR "tizen")
SET(PACKAGE "usbotg-syspopup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
			appcore-efl
			elementary
			devman
			syspopup
			syspopup-caller
			ecore-evas
			vconf
			appsvc
			dlog)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall")

ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PACKAGE} ${SRCS})
TARGET_LINK_LIBRARIES(${PACKAGE} syspopup ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PACKAGE} DESTINATION ${BINDIR})

# install manifest xml file
INSTALL(FILES ${MOBILE_DIR}/data/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})

# instatll rule file
INSTALL(FILES ${MOBILE_DIR}/${PROJECT_NAME}/org.tizen.usbotg-syspopup.rule DESTINATION /opt/etc/smack/accesses.d)
