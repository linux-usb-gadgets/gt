cmake_minimum_required(VERSION 2.8)

#project name
project (gadget-tool)

INCLUDE_DIRECTORIES( ${gadget-tool_SOURCE_DIR}/base/include )

SET(CMAKE_C_FLAGS -g)

add_subdirectory(config)
add_subdirectory(function)
add_subdirectory(gadget)
add_subdirectory(settings)
add_subdirectory(udc)
add_subdirectory(base)

SET( MAIN_SRC main.c)

add_executable( gt ${MAIN_SRC})

target_link_libraries( gt
	base
	udc
	config
	function
	gadget
	settings
	)