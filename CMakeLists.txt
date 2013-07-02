####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(system-popup C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")

# SUbmodules
ADD_SUBDIRECTORY(poweroff-popup)
ADD_SUBDIRECTORY(lowbatt-popup)
ADD_SUBDIRECTORY(lowmem-popup)
ADD_SUBDIRECTORY(mmc-popup)
ADD_SUBDIRECTORY(usb-popup)
ADD_SUBDIRECTORY(usbotg-popup)
ADD_SUBDIRECTORY(sys_device_noti)

############## END ##############
