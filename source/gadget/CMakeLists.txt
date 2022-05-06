
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include
			${PROJECT_SOURCE_DIR}/function/include
			${PROJECT_SOURCE_DIR}/config/include )

SET( GADGET_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_libusbg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_not_implemented.c
	)

IF (DEFINED CMAKE_WITH_GADGETD)
	LIST(APPEND GADGET_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/gadget_gadgetd.c
	)
ENDIF ()

add_library(gadget STATIC ${GADGET_SRC} )
