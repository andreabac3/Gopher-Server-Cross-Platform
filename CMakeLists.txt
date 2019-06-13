cmake_minimum_required(VERSION 3.7.2)
project(gopher C)

set(CMAKE_C_STANDARD 11)

#add_compile_options(-Wall -Wextra -Wpedantic)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -pedantic-errors -lm" )

# Include and linking libs
include_directories(${CMAKE_SOURCE_DIR}/general/include)
link_directories(${CMAKE_SOURCE_DIR}/general/lib)

if (WIN32)
include_directories(${CMAKE_SOURCE_DIR}/windows/include)
link_directories(${CMAKE_SOURCE_DIR}/windows/lib)
endif (WIN32)

if (UNIX)
include_directories(${CMAKE_SOURCE_DIR}/linux/include)
link_directories(${CMAKE_SOURCE_DIR}/linux/lib)
endif (UNIX)


add_executable(gopher main.c general/lib/utils/utils.c general/include/utils.h general/lib/io/config_file.c general/include/config_file.h linux/lib/utils/linux_portable_lib.c linux/include/linux_portable_lib.h linux/lib/utils/utils.c linux/include/utils.h general/lib/portable/portables.c general/include/portables.h general/include/definitions.h general/lib/io/files_interaction.c general/include/files_interaction.h general/lib/gopher/protocol.c general/include/protocol.h linux/lib/io/linux_files_interaction.c linux/include/linux_files_interaction.h linux/include/linux_dict.h linux/lib/memory_mapping/linux_memory_mapping.c linux/include/linux_memory_mapping.h linux/lib/thread/linux_thread.c linux/include/linux_thread.h linux/lib/process/linux_process.c linux/include/linux_process.h)

if (UNIX)
    target_link_libraries(gopher pthread)
endif (UNIX)

if (WIN32)
    target_link_libraries(gopher ws2_32)
endif (WIN32)
