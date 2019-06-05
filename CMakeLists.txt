cmake_minimum_required(VERSION 3.7.2)
project(gopher C)

set(CMAKE_C_STANDARD 11)

#add_compile_options(-Wall -Wextra -Wpedantic )
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -pedantic-errors -lm" )

# Include and linking libs
include_directories(${CMAKE_SOURCE_DIR}/general/include)
link_directories(${CMAKE_SOURCE_DIR}/general/lib)

include_directories(${CMAKE_SOURCE_DIR}/windows/include)
link_directories(${CMAKE_SOURCE_DIR}/windows/lib)

include_directories(${CMAKE_SOURCE_DIR}/linux/include)
link_directories(${CMAKE_SOURCE_DIR}/linux/lib)


add_executable(gopher main.c general/lib/utils/utils.c general/include/utils.h)