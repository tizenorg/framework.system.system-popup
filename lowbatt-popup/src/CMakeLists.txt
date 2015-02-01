
#################### Low-battery ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/lowbatt-popup/src/lowbatt.c
		 ${CMAKE_SOURCE_DIR}/share/common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lowbatt-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(lowbat_pkgs REQUIRED
		appcore-efl
		elementary
		deviced
		dlog
		feedback
		syspopup
		efl-assist
		syspopup-caller
		capi-media-sound-manager
		capi-media-wav-player
)

FOREACH(flag ${lowbat_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
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
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${lowbat_pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(lowbatt.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/lowbatt.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowbatt.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/lowbatt.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} lowbatt.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.lowbat-syspopup/bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowbatt.edj DESTINATION /usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.lowbat-syspopup.xml DESTINATION /usr/share/packages/)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src/org.tizen.lowbat-syspopup.efl DESTINATION /etc/smack/accesses.d)

###############################################################


