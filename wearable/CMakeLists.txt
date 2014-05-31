####### System-popup #############

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(SYSTEM_RESDIR "${PREFIX}/share/system-apps/res")
SET(SYSTEM_ICONDIR "${SYSTEM_RESDIR}/icons")
SET(WEARABLE_DIR ${CMAKE_SOURCE_DIR}/wearable)

ADD_DEFINITIONS("-DSYSTEM_ICONDIR=\"${SYSTEM_ICONDIR}\"")

ADD_SUBDIRECTORY(lowbatt-popup)
ADD_SUBDIRECTORY(lowmem-popup)
ADD_SUBDIRECTORY(crash-popup)
ADD_SUBDIRECTORY(popup-launcher)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(poweroff-popup)
ADD_SUBDIRECTORY(sys-device-noti)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME system-popup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.poweroff-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.lowbatt-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.lowmem-syspopup)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.Apache_v2 DESTINATION /usr/share/license RENAME org.tizen.crash-popup)

############## END ##############
