
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include
			${CMAKE_SOURCE_DIR}/function/include )

SET( GADGET_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_libusbg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_gadgetd.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_not_implemented.c
	)

add_library(gadget STATIC ${GADGET_SRC} )
