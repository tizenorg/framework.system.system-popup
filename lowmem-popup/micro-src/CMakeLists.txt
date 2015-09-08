
########################### low memory ###########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(VENDOR "tizen")
SET(PACKAGE "lowmem-syspopup")
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX "${CMAKE_INSTALL_PREFIX}/apps/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(TABLE_PATH ${CMAKE_SOURCE_DIR}/edcs/table)

SET(SRCS ${CMAKE_SOURCE_DIR}/lowmem-popup/micro-src/lowmem.c
		 ${CMAKE_SOURCE_DIR}/share/micro-common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lowmem-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
		appcore-efl
		elementary
		syspopup
		ecore
		deviced
		dlog
		feedback
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

SET(COMMON_EDC "micro-common.edc")
ADD_CUSTOM_TARGET(lowmem.edj COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/${COMMON_EDC} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowmem.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/${COMMON_EDC}
)
ADD_DEPENDENCIES(${PROJECT_NAME} lowmem.edj)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowmem.edj DESTINATION /usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem/)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.lowmem-syspopup/bin)

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.lowmem-syspopup.xml DESTINATION /usr/share/packages/)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.lowmem-syspopup.efl DESTINATION /etc/smack/accesses.d)

INSTALL(FILES ${TABLE_PATH}/system-font.xml  DESTINATION ${RESDIR}/table)
INSTALL(FILES ${TABLE_PATH}/system-color.xml DESTINATION ${RESDIR}/table)

################################# End ##############################################

