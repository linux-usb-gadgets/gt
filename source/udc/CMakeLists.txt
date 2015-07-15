
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )

SET( UDC_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/udc.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/udc_libusbg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/udc_gadgetd.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/udc_not_implemented.c
	)

add_library(udc STATIC ${UDC_SRC} )
