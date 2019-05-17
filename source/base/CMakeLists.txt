INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include
			${CMAKE_SOURCE_DIR}/udc/include
			${CMAKE_SOURCE_DIR}/ffs/include
			${CMAKE_SOURCE_DIR}/config/include
			${CMAKE_SOURCE_DIR}/function/include
			${CMAKE_SOURCE_DIR}/gadget/include
			${CMAKE_SOURCE_DIR}/settings/include )

SET( BASE_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/backend.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/executable_command.c
	)

add_library(base STATIC ${BASE_SRC} )