# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/tools/apps/CLion/ch-0/183.5429.37/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /opt/tools/apps/CLion/ch-0/183.5429.37/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /opt/projects/gopher

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /opt/projects/gopher/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/gopher.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/gopher.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/gopher.dir/flags.make

CMakeFiles/gopher.dir/main.c.o: CMakeFiles/gopher.dir/flags.make
CMakeFiles/gopher.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/opt/projects/gopher/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/gopher.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/gopher.dir/main.c.o   -c /opt/projects/gopher/main.c

CMakeFiles/gopher.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gopher.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /opt/projects/gopher/main.c > CMakeFiles/gopher.dir/main.c.i

CMakeFiles/gopher.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gopher.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /opt/projects/gopher/main.c -o CMakeFiles/gopher.dir/main.c.s

CMakeFiles/gopher.dir/general/lib/utils/utils.c.o: CMakeFiles/gopher.dir/flags.make
CMakeFiles/gopher.dir/general/lib/utils/utils.c.o: ../general/lib/utils/utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/opt/projects/gopher/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/gopher.dir/general/lib/utils/utils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/gopher.dir/general/lib/utils/utils.c.o   -c /opt/projects/gopher/general/lib/utils/utils.c

CMakeFiles/gopher.dir/general/lib/utils/utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gopher.dir/general/lib/utils/utils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /opt/projects/gopher/general/lib/utils/utils.c > CMakeFiles/gopher.dir/general/lib/utils/utils.c.i

CMakeFiles/gopher.dir/general/lib/utils/utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gopher.dir/general/lib/utils/utils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /opt/projects/gopher/general/lib/utils/utils.c -o CMakeFiles/gopher.dir/general/lib/utils/utils.c.s

# Object files for target gopher
gopher_OBJECTS = \
"CMakeFiles/gopher.dir/main.c.o" \
"CMakeFiles/gopher.dir/general/lib/utils/utils.c.o"

# External object files for target gopher
gopher_EXTERNAL_OBJECTS =

gopher: CMakeFiles/gopher.dir/main.c.o
gopher: CMakeFiles/gopher.dir/general/lib/utils/utils.c.o
gopher: CMakeFiles/gopher.dir/build.make
gopher: CMakeFiles/gopher.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/opt/projects/gopher/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable gopher"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gopher.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/gopher.dir/build: gopher

.PHONY : CMakeFiles/gopher.dir/build

CMakeFiles/gopher.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/gopher.dir/cmake_clean.cmake
.PHONY : CMakeFiles/gopher.dir/clean

CMakeFiles/gopher.dir/depend:
	cd /opt/projects/gopher/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/projects/gopher /opt/projects/gopher /opt/projects/gopher/cmake-build-debug /opt/projects/gopher/cmake-build-debug /opt/projects/gopher/cmake-build-debug/CMakeFiles/gopher.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/gopher.dir/depend

