CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS src/usb-syspopup.c
		 ${CMAKE_SOURCE_DIR}/share/common.c)

SET(VENDOR "tizen")
SET(PACKAGE "usb-syspopup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(MANIFESTDIR "${CMAKE_INSTALL_PREFIX}/share/packages")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
				appcore-efl
				bundle
				dlog
				ecore
				elementary
				evas
				syspopup
				appsvc
				vconf
				deviced)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PACKAGE} ${SRCS})
TARGET_LINK_LIBRARIES(${PACKAGE} syspopup  ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PACKAGE} DESTINATION ${BINDIR})

# install icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/usb_icon.png DESTINATION ${RESDIR}/icons/)

# install manifest file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/${PKGNAME}.xml DESTINATION ${MANIFESTDIR})

# install smack efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/packaging/${PKGNAME}.efl DESTINATION /etc/smack/accesses.d)
