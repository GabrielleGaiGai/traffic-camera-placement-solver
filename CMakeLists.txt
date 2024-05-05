cmake_minimum_required(VERSION 2.8.12)
project(project)

add_subdirectory(minisat)

# enable c++11 support
set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/run)

include_directories(${CMAKE_SOURCE_DIR}/minisat)

option(WITH_ASAN "Enabled AddressSanitizer to detect memory faults" OFF)
if(WITH_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined" )
endif()


add_executable(solver solver.cpp)
add_executable(parser parser.cpp graph.cpp)
add_executable(rgen rgen.cpp)
# link MiniSAT libraries
target_link_libraries(parser minisat-lib-static)

install(PROGRAMS traffic.py DESTINATION bin)
install(TARGETS solver parser rgen RUNTIME DESTINATION bin)