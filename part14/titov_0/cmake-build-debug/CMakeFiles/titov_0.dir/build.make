# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /snap/clion/103/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/103/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/anastasia/CLionProjects/caos/part14/titov_0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/titov_0.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/titov_0.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/titov_0.dir/flags.make

CMakeFiles/titov_0.dir/main.c.o: CMakeFiles/titov_0.dir/flags.make
CMakeFiles/titov_0.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/titov_0.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/titov_0.dir/main.c.o   -c /home/anastasia/CLionProjects/caos/part14/titov_0/main.c

CMakeFiles/titov_0.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/titov_0.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/anastasia/CLionProjects/caos/part14/titov_0/main.c > CMakeFiles/titov_0.dir/main.c.i

CMakeFiles/titov_0.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/titov_0.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/anastasia/CLionProjects/caos/part14/titov_0/main.c -o CMakeFiles/titov_0.dir/main.c.s

# Object files for target titov_0
titov_0_OBJECTS = \
"CMakeFiles/titov_0.dir/main.c.o"

# External object files for target titov_0
titov_0_EXTERNAL_OBJECTS =

titov_0: CMakeFiles/titov_0.dir/main.c.o
titov_0: CMakeFiles/titov_0.dir/build.make
titov_0: CMakeFiles/titov_0.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable titov_0"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/titov_0.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/titov_0.dir/build: titov_0

.PHONY : CMakeFiles/titov_0.dir/build

CMakeFiles/titov_0.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/titov_0.dir/cmake_clean.cmake
.PHONY : CMakeFiles/titov_0.dir/clean

CMakeFiles/titov_0.dir/depend:
	cd /home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anastasia/CLionProjects/caos/part14/titov_0 /home/anastasia/CLionProjects/caos/part14/titov_0 /home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug /home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug /home/anastasia/CLionProjects/caos/part14/titov_0/cmake-build-debug/CMakeFiles/titov_0.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/titov_0.dir/depend

