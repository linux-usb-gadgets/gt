set(ENABLE_MANUAL_PAGE ON CACHE BOOL "Enable building and installing the manual page")

if(${ENABLE_MANUAL_PAGE})

ADD_CUSTOM_COMMAND(OUTPUT gt.1.gz
		COMMAND a2x --format=manpage --destination-dir=${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/gt.1.txt
		COMMAND gzip gt.1)

ADD_CUSTOM_TARGET(manpage ALL DEPENDS gt.1.gz)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/gt.1.gz DESTINATION ${MANDIR})

endif()
