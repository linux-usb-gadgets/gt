
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include
			${gadget-tool_SOURCE_DIR}/udc/include
			${gadget-tool_SOURCE_DIR}/config/include
			${gadget-tool_SOURCE_DIR}/function/include
			${gadget-tool_SOURCE_DIR}/gadget/include
			${gadget-tool_SOURCE_DIR}/settings/include )

SET( BASE_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/executable_command.c
	)

add_library(base STATIC ${BASE_SRC} )