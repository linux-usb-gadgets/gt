find_program(A2X_EXE a2x)

set(ENABLE_MANUAL_PAGE ON CACHE BOOL "Enable building and installing the manual page")
if(NOT A2X_EXE)
	message(WARNING "asciidoc a2x not found - manual generation disabled...")
	set(ENABLE_MANUAL_PAGE OFF CACHE BOOL "Enable building and installing the manual page" FORCE)
endif()

if(${ENABLE_MANUAL_PAGE})

ADD_CUSTOM_COMMAND(OUTPUT gt.1.gz
		COMMAND ${A2X_EXE} --format=manpage --destination-dir=${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/gt.1.txt
		COMMAND gzip gt.1)

ADD_CUSTOM_TARGET(manpage ALL DEPENDS gt.1.gz)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/gt.1.gz TYPE MAN)

endif()
