cmake_minimum_required(VERSION 3.1)
project(MIMUW-CPCPP CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror")

# http://stackoverflow.com/questions/10555706/
macro(add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT})
    endif ()
endmacro()


add_subdirectory(tests)

install(TARGETS DESTINATION PageRank)
