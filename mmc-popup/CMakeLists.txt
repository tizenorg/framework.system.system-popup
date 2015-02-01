
#################### Memory-card ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mmc-popup C)

SET(SRCS ${CMAKE_SOURCE_DIR}/mmc-popup/src/mmc.c
		 ${CMAKE_SOURCE_DIR}/share/common.c
)

SET(PKG_MODULES
		appcore-efl
		elementary
		ecore
		notification
		deviced
		dlog
		edbus
		feedback
		syspopup
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/mmc-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(mmc_pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${mmc_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g ")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${mmc_pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.mmc-syspopup/bin)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.mmc-syspopup.xml DESTINATION /usr/share/packages/)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.mmc-syspopup.efl DESTINATION /etc/smack/accesses.d)

###############################################################


