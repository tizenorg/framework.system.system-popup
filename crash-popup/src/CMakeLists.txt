
########################### low memory ###########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(crash-popup C)

SET(SRCS
		${CMAKE_SOURCE_DIR}/crash-popup/src/crash.c
		${CMAKE_SOURCE_DIR}/share/common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)
pkg_check_modules(crash_pkgs REQUIRED
		appcore-efl
		elementary
		syspopup
		ecore
		deviced
		dlog
		pkgmgr-info
		efl-assist
		feedback
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/crash-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

FOREACH(flag ${crash_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g -I/usr/include/elementary-0 ")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${crash_pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(crash.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/crash.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/crash.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/crash.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} crash.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.crash-popup/bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/crash.edj DESTINATION /usr/apps/org.tizen.crash-popup/res/edje/crash)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.crash-popup.xml DESTINATION /usr/share/packages/)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/crash-popup/src/org.tizen.crash-popup.efl DESTINATION /etc/smack/accesses.d)

################################# End ##############################################

