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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test

# Include any dependencies generated for this target.
include CMakeFiles/rtpreceiver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rtpreceiver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rtpreceiver.dir/flags.make

CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o: CMakeFiles/rtpreceiver.dir/flags.make
CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o: src/receiver_def.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o   -c /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/src/receiver_def.c

CMakeFiles/rtpreceiver.dir/src/receiver_def.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rtpreceiver.dir/src/receiver_def.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/src/receiver_def.c > CMakeFiles/rtpreceiver.dir/src/receiver_def.c.i

CMakeFiles/rtpreceiver.dir/src/receiver_def.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rtpreceiver.dir/src/receiver_def.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/src/receiver_def.c -o CMakeFiles/rtpreceiver.dir/src/receiver_def.c.s

# Object files for target rtpreceiver
rtpreceiver_OBJECTS = \
"CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o"

# External object files for target rtpreceiver
rtpreceiver_EXTERNAL_OBJECTS =

librtpreceiver.a: CMakeFiles/rtpreceiver.dir/src/receiver_def.c.o
librtpreceiver.a: CMakeFiles/rtpreceiver.dir/build.make
librtpreceiver.a: CMakeFiles/rtpreceiver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library librtpreceiver.a"
	$(CMAKE_COMMAND) -P CMakeFiles/rtpreceiver.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rtpreceiver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rtpreceiver.dir/build: librtpreceiver.a

.PHONY : CMakeFiles/rtpreceiver.dir/build

CMakeFiles/rtpreceiver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rtpreceiver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rtpreceiver.dir/clean

CMakeFiles/rtpreceiver.dir/depend:
	cd /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test /home/vl/桌面/lab2-rtp-vllimpid/Lab2-RTP-Test/CMakeFiles/rtpreceiver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rtpreceiver.dir/depend

