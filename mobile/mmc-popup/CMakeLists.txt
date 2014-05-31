
#################### Memory-card ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mmc-popup C)

SET(SRCS ${MOBILE_DIR}/mmc-popup/src/mmc.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${MOBILE_DIR})
INCLUDE_DIRECTORIES(${MOBILE_DIR}/mmc-popup)
INCLUDE_DIRECTORIES(/usr/include/svi)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED appcore-efl elementary devman devman_haptic mm-sound notification sysman)

FOREACH(flag ${pkgs_CFLAGS})
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
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-lsvi")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.mmc-syspopup/bin)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${MOBILE_DIR}/data/org.tizen.mmc-syspopup.xml DESTINATION /usr/share/packages/)

# install ini file
INSTALL(FILES ${MOBILE_DIR}/data/${PROJECT_NAME}.ini DESTINATION /usr/share/process-info)

# instatll rule file
INSTALL(FILES ${MOBILE_DIR}/${PROJECT_NAME}/org.tizen.mmc-syspopup.rule DESTINATION /opt/etc/smack/accesses.d)

# i18n
ADD_SUBDIRECTORY(po_mmc)

###############################################################


