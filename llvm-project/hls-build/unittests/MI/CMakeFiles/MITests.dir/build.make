# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build

# Include any dependencies generated for this target.
include unittests/MI/CMakeFiles/MITests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include unittests/MI/CMakeFiles/MITests.dir/compiler_depend.make

# Include the progress variables for this target.
include unittests/MI/CMakeFiles/MITests.dir/progress.make

# Include the compile flags for this target's objects.
include unittests/MI/CMakeFiles/MITests.dir/flags.make

unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o: unittests/MI/CMakeFiles/MITests.dir/flags.make
unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/MI/LiveIntervalTest.cpp
unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o: unittests/MI/CMakeFiles/MITests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o -MF CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o.d -o CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/MI/LiveIntervalTest.cpp

unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MITests.dir/LiveIntervalTest.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/MI/LiveIntervalTest.cpp > CMakeFiles/MITests.dir/LiveIntervalTest.cpp.i

unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MITests.dir/LiveIntervalTest.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/MI/LiveIntervalTest.cpp -o CMakeFiles/MITests.dir/LiveIntervalTest.cpp.s

# Object files for target MITests
MITests_OBJECTS = \
"CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o"

# External object files for target MITests
MITests_EXTERNAL_OBJECTS =

unittests/MI/MITests: unittests/MI/CMakeFiles/MITests.dir/LiveIntervalTest.cpp.o
unittests/MI/MITests: unittests/MI/CMakeFiles/MITests.dir/build.make
unittests/MI/MITests: lib/libgtest_main.a
unittests/MI/MITests: lib/libgtest.a
unittests/MI/MITests: lib/libLLVM-7.0svn.so
unittests/MI/MITests: unittests/MI/CMakeFiles/MITests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable MITests"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MITests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
unittests/MI/CMakeFiles/MITests.dir/build: unittests/MI/MITests
.PHONY : unittests/MI/CMakeFiles/MITests.dir/build

unittests/MI/CMakeFiles/MITests.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI && $(CMAKE_COMMAND) -P CMakeFiles/MITests.dir/cmake_clean.cmake
.PHONY : unittests/MI/CMakeFiles/MITests.dir/clean

unittests/MI/CMakeFiles/MITests.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/MI /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/MI/CMakeFiles/MITests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : unittests/MI/CMakeFiles/MITests.dir/depend

