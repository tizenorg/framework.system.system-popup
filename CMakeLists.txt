####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(system-popup C)

IF(${SYSTEM_APPS_MICRO} STREQUAL "yes")
ADD_DEFINITIONS("-DTIZEN_MICRO")
ADD_SUBDIRECTORY(wearable)
ELSE()
ADD_SUBDIRECTORY(mobile)
ENDIF()

############## END ##############
