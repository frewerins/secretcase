# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /snap/clion/112/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/112/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/anastasia/CLionProjects/caos/part23/1-notmy

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/1_notmy.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/1_notmy.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/1_notmy.dir/flags.make

CMakeFiles/1_notmy.dir/main.c.o: CMakeFiles/1_notmy.dir/flags.make
CMakeFiles/1_notmy.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/1_notmy.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/1_notmy.dir/main.c.o   -c /home/anastasia/CLionProjects/caos/part23/1-notmy/main.c

CMakeFiles/1_notmy.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/1_notmy.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/anastasia/CLionProjects/caos/part23/1-notmy/main.c > CMakeFiles/1_notmy.dir/main.c.i

CMakeFiles/1_notmy.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/1_notmy.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/anastasia/CLionProjects/caos/part23/1-notmy/main.c -o CMakeFiles/1_notmy.dir/main.c.s

# Object files for target 1_notmy
1_notmy_OBJECTS = \
"CMakeFiles/1_notmy.dir/main.c.o"

# External object files for target 1_notmy
1_notmy_EXTERNAL_OBJECTS =

1_notmy: CMakeFiles/1_notmy.dir/main.c.o
1_notmy: CMakeFiles/1_notmy.dir/build.make
1_notmy: CMakeFiles/1_notmy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable 1_notmy"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/1_notmy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/1_notmy.dir/build: 1_notmy

.PHONY : CMakeFiles/1_notmy.dir/build

CMakeFiles/1_notmy.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/1_notmy.dir/cmake_clean.cmake
.PHONY : CMakeFiles/1_notmy.dir/clean

CMakeFiles/1_notmy.dir/depend:
	cd /home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anastasia/CLionProjects/caos/part23/1-notmy /home/anastasia/CLionProjects/caos/part23/1-notmy /home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug /home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug /home/anastasia/CLionProjects/caos/part23/1-notmy/cmake-build-debug/CMakeFiles/1_notmy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/1_notmy.dir/depend

