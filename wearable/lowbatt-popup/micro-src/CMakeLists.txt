
#################### Low-battery ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${WEARABLE_DIR}/lowbatt-popup/micro-src/lowbatt.c
		 ${WEARABLE_DIR}/share/micro-common.c)

SET(PACKAGE ${PROJECT_NAME})

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${WEARABLE_DIR})
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/lowbatt-popup)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/popup-launcher/micro-src)
INCLUDE_DIRECTORIES(${WEARABLE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
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

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.lowbat-syspopup/bin)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${WEARABLE_DIR}/data/org.tizen.lowbat-syspopup.xml DESTINATION /usr/share/packages/)

# instatll rule file
INSTALL(FILES ${WEARABLE_DIR}/${PROJECT_NAME}/micro-src/org.tizen.lowbat-syspopup.rule DESTINATION /etc/smack/accesses2.d)

###############################################################


