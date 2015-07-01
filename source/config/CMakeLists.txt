
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )

SET( CONFIG_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/configuration_gadgetd.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/configuration_libusbg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/configuration_not_implemented.c
	)

add_library(config STATIC ${CONFIG_SRC} )
