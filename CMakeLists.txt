cmake_minimum_required(VERSION 3.0)

# project
set(CMAKE_CXX_STANDARD 11)
add_definitions("-Wall -g")
PROJECT(test_reactor)

# source && bin
set(PROJECT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# include
include_directories(
${CMAKE_CURRENT_SOURCE_DIR}/include
)

# reactor
set(reactor_src
${PROJECT_SRC}/acceptor.cpp
${PROJECT_SRC}/client_socket.cpp
${PROJECT_SRC}/listen_socket.cpp
${PROJECT_SRC}/main_reactor.cpp
${PROJECT_SRC}/reactor_mgr.cpp
${PROJECT_SRC}/reactor.cpp
${PROJECT_SRC}/test_reactor.cpp
${PROJECT_SRC}/utility.cpp
)
# client
set(client_src
${PROJECT_SRC}/client.cpp
${PROJECT_SRC}/utility.cpp
)

# execute
add_executable(${PROJECT_NAME} ${reactor_src})
add_executable(client ${client_src})
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(client pthread)
