
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )

SET( FUNCTION_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/function.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/function_libusbg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/function_gadgetd.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/function_not_implemented.c
	)

add_library(function STATIC ${FUNCTION_SRC} )
