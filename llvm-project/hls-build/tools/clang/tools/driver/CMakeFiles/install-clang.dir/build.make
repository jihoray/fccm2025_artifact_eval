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

# Utility rule file for install-clang.

# Include any custom commands dependencies for this target.
include tools/clang/tools/driver/CMakeFiles/install-clang.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/clang/tools/driver/CMakeFiles/install-clang.dir/progress.make

tools/clang/tools/driver/CMakeFiles/install-clang: bin/clang-7.0
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/driver && /usr/bin/cmake -DCMAKE_INSTALL_COMPONENT="clang" -P /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/cmake_install.cmake

install-clang: tools/clang/tools/driver/CMakeFiles/install-clang
install-clang: tools/clang/tools/driver/CMakeFiles/install-clang.dir/build.make
.PHONY : install-clang

# Rule to build all files generated by this target.
tools/clang/tools/driver/CMakeFiles/install-clang.dir/build: install-clang
.PHONY : tools/clang/tools/driver/CMakeFiles/install-clang.dir/build

tools/clang/tools/driver/CMakeFiles/install-clang.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/driver && $(CMAKE_COMMAND) -P CMakeFiles/install-clang.dir/cmake_clean.cmake
.PHONY : tools/clang/tools/driver/CMakeFiles/install-clang.dir/clean

tools/clang/tools/driver/CMakeFiles/install-clang.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/driver /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/driver /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/driver/CMakeFiles/install-clang.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/clang/tools/driver/CMakeFiles/install-clang.dir/depend

