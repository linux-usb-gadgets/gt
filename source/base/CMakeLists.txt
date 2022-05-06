INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include
			${PROJECT_SOURCE_DIR}/udc/include
			${PROJECT_SOURCE_DIR}/config/include
			${PROJECT_SOURCE_DIR}/function/include
			${PROJECT_SOURCE_DIR}/gadget/include
			${PROJECT_SOURCE_DIR}/settings/include )

SET( BASE_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/backend.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/executable_command.c
	)

add_library(base STATIC ${BASE_SRC} )