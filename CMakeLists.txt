cmake_minimum_required(VERSION 3.0)

# project
set(CMAKE_CXX_STANDARD 11)
add_definitions("-Wall -g")
PROJECT(test_reactor)

# source && bin
SET(PROJECT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(PROJECT_BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)
aux_source_directory(${PROJECT_SRC} SRC)


# include
include_directories(
${CMAKE_CURRENT_SOURCE_DIR}/include
)

# execute
add_executable(${PROJECT_NAME} 
${SRC}
)
target_link_libraries(${PROJECT_NAME} pthread)
