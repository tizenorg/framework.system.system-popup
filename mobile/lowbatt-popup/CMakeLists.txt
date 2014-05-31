
#################### Low-battery ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lowbatt-popup C)

SET(SRCS ${MOBILE_DIR}/lowbatt-popup/src/lowbatt.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${MOBILE_DIR})
INCLUDE_DIRECTORIES(${MOBILE_DIR}/lowbatt-popup)
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

ADD_CUSTOM_TARGET(lowbatt.edj
		COMMAND edje_cc -id ${MOBILE_DIR}/../images
		${MOBILE_DIR}/edcs/lowbatt.edc ${MOBILE_DIR}/${PROJECT_NAME}/lowbatt.edj
		DEPENDS ${MOBILE_DIR}/edcs/lowbatt.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} lowbatt.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.lowbat-syspopup/bin)
INSTALL(FILES ${MOBILE_DIR}/${PROJECT_NAME}/lowbatt.edj DESTINATION /usr/apps/org.tizen.lowbat-syspopup/res/edje/lowbatt)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ${MOBILE_DIR}/data/org.tizen.lowbat-syspopup.png DESTINATION /usr/apps/org.tizen.lowbat-syspopup/res/icon)
# install manifest xml file

INSTALL(FILES ${MOBILE_DIR}/data/org.tizen.lowbat-syspopup.xml DESTINATION /usr/share/packages/)

# install ini file
INSTALL(FILES ${MOBILE_DIR}/data/${PROJECT_NAME}.ini DESTINATION /usr/share/process-info)

# instatll rule file
INSTALL(FILES ${MOBILE_DIR}/${PROJECT_NAME}/org.tizen.lowbat-syspopup.rule DESTINATION /opt/etc/smack/accesses.d)

# i18n
ADD_SUBDIRECTORY(po_lowbatt)

###############################################################


