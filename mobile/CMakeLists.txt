####### System-popup #############
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(MOBILE_DIR ${CMAKE_SOURCE_DIR}/mobile)

# SUbmodules
ADD_SUBDIRECTORY(poweroff-popup)
ADD_SUBDIRECTORY(lowbatt-popup)
ADD_SUBDIRECTORY(lowmem-popup)
ADD_SUBDIRECTORY(mmc-popup)
ADD_SUBDIRECTORY(usb-popup)
ADD_SUBDIRECTORY(usbotg-popup)
ADD_SUBDIRECTORY(sys_device_noti)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME system-popup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.poweroff-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.lowbatt-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.lowmem-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.mmc-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.usb-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.usbotg-syspopup)

############## END ##############
