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
include unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/compiler_depend.make

# Include the progress variables for this target.
include unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/progress.make

# Include the compile flags for this target's objects.
include unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/flags.make

unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o: unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/flags.make
unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/Support/DynamicLibrary/PipSqueak.cxx
unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o: unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o -MF CMakeFiles/SecondLib.dir/PipSqueak.cxx.o.d -o CMakeFiles/SecondLib.dir/PipSqueak.cxx.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/Support/DynamicLibrary/PipSqueak.cxx

unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SecondLib.dir/PipSqueak.cxx.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/Support/DynamicLibrary/PipSqueak.cxx > CMakeFiles/SecondLib.dir/PipSqueak.cxx.i

unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SecondLib.dir/PipSqueak.cxx.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/Support/DynamicLibrary/PipSqueak.cxx -o CMakeFiles/SecondLib.dir/PipSqueak.cxx.s

# Object files for target SecondLib
SecondLib_OBJECTS = \
"CMakeFiles/SecondLib.dir/PipSqueak.cxx.o"

# External object files for target SecondLib
SecondLib_EXTERNAL_OBJECTS =

unittests/Support/DynamicLibrary/SecondLib.so: unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/PipSqueak.cxx.o
unittests/Support/DynamicLibrary/SecondLib.so: unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/build.make
unittests/Support/DynamicLibrary/SecondLib.so: unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library SecondLib.so"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SecondLib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/build: unittests/Support/DynamicLibrary/SecondLib.so
.PHONY : unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/build

unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary && $(CMAKE_COMMAND) -P CMakeFiles/SecondLib.dir/cmake_clean.cmake
.PHONY : unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/clean

unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/unittests/Support/DynamicLibrary /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : unittests/Support/DynamicLibrary/CMakeFiles/SecondLib.dir/depend

