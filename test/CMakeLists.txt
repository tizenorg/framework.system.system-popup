CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SRCS ${CMAKE_SOURCE_DIR}/test/test.c
		 ${CMAKE_SOURCE_DIR}/share/common.c)
SET(UTILNAME "systemfw-app-test")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test
					${CMAKE_SOURCE_DIR}/share
					${CMAKE_SOURCE_DIR}/popup-launcher/src)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED edbus appcore-efl bundle elementary)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g -fno-omit-frame-pointer -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -g -Wall")

ADD_EXECUTABLE(${UTILNAME} ${SRCS})
TARGET_LINK_LIBRARIES(${UTILNAME} ${pkgs_LDFLAGS} "-ldl")

INSTALL(TARGETS ${UTILNAME} DESTINATION bin)
