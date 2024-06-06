if (NOT DEFINED ENABLE_MANUAL_PAGE)
	find_program(A2X a2x)
elseif (${ENABLE_MANUAL_PAGE})
	find_program(A2X a2x REQUIRED)
endif()

set(ENABLE_MANUAL_PAGE ON CACHE BOOL "Enable building and installing the manual page")

if (${ENABLE_MANUAL_PAGE} AND NOT A2X)
	message(WARNING "asciidoc a2x not found - manual page generation disabled!")
	set(ENABLE_MANUAL_PAGE OFF)
endif()

if(${ENABLE_MANUAL_PAGE})

ADD_CUSTOM_COMMAND(OUTPUT gt.1.gz
		COMMAND ${A2X} --format=manpage --destination-dir=${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/gt.1.txt
		COMMAND gzip gt.1)

ADD_CUSTOM_TARGET(manpage ALL DEPENDS gt.1.gz)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/gt.1.gz DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

endif()
