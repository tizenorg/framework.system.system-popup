
########################### low memory ###########################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/lowmem-popup/src/lowmem.c
		 ${CMAKE_SOURCE_DIR}/share/common.c)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lowmem-popup)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/share)

INCLUDE(FindPkgConfig)
pkg_check_modules(lowmem_pkgs REQUIRED
		appcore-efl
		elementary
		syspopup
		ecore
		deviced
		dlog
		feedback
)

FOREACH(flag ${lowmem_pkgs_CFLAGS})
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
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${lowmem_pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(lowmem.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/edcs/icons
		${CMAKE_SOURCE_DIR}/edcs/lowmem.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowmem.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/edcs/lowmem.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} lowmem.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/org.tizen.lowmem-syspopup/bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lowmem.edj DESTINATION /usr/apps/org.tizen.lowmem-syspopup/res/edje/lowmem)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# install manifest xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/org.tizen.lowmem-syspopup.xml DESTINATION /usr/share/packages/)

# instatll efl file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src/org.tizen.lowmem-syspopup.efl DESTINATION /etc/smack/accesses.d)

################################# End ##############################################

