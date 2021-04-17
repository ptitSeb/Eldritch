cmake_minimum_required(VERSION 3.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(../Core/src)

link_directories(${CMAKE_SOURCE_DIR}/Libraries/Core/)

set(glew_src
	src/glew.c
)

add_library(GLEW ${glew_src})

target_link_libraries(GLEW Core)
