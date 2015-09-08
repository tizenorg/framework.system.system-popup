
#################### Low-battery ###############################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lowbatt-popup C)

IF(${SYSTEM_APPS_MICRO} STREQUAL "yes")
ADD_SUBDIRECTORY(micro-src)
ELSE()
ADD_SUBDIRECTORY(src)
ENDIF()
