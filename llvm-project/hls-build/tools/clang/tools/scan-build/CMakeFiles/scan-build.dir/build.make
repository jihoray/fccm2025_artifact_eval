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

# Utility rule file for scan-build.

# Include any custom commands dependencies for this target.
include tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/progress.make

tools/clang/tools/scan-build/CMakeFiles/scan-build: bin/scan-build
tools/clang/tools/scan-build/CMakeFiles/scan-build: libexec/ccc-analyzer
tools/clang/tools/scan-build/CMakeFiles/scan-build: libexec/c++-analyzer
tools/clang/tools/scan-build/CMakeFiles/scan-build: share/man/man1/scan-build.1
tools/clang/tools/scan-build/CMakeFiles/scan-build: share/scan-build/scanview.css
tools/clang/tools/scan-build/CMakeFiles/scan-build: share/scan-build/sorttable.js

bin/scan-build: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/bin/scan-build
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ../../../../bin/scan-build"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/bin
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/bin/scan-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/bin/

libexec/c++-analyzer: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/libexec/c++-analyzer
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating ../../../../libexec/c++-analyzer"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/libexec
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/libexec/c++-analyzer /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/libexec/

libexec/ccc-analyzer: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/libexec/ccc-analyzer
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating ../../../../libexec/ccc-analyzer"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/libexec
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/libexec/ccc-analyzer /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/libexec/

share/man/man1/scan-build.1: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/man/scan-build.1
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Generating ../../../../share/man/man1/scan-build.1"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/man/man1
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/man/scan-build.1 /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/man/man1/

share/scan-build/scanview.css: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/share/scan-build/scanview.css
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Generating ../../../../share/scan-build/scanview.css"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/scan-build
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/share/scan-build/scanview.css /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/scan-build/

share/scan-build/sorttable.js: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/share/scan-build/sorttable.js
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Generating ../../../../share/scan-build/sorttable.js"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E make_directory /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/scan-build
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && /usr/bin/cmake -E copy /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build/share/scan-build/sorttable.js /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/share/scan-build/

scan-build: bin/scan-build
scan-build: libexec/c++-analyzer
scan-build: libexec/ccc-analyzer
scan-build: share/man/man1/scan-build.1
scan-build: share/scan-build/scanview.css
scan-build: share/scan-build/sorttable.js
scan-build: tools/clang/tools/scan-build/CMakeFiles/scan-build
scan-build: tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/build.make
.PHONY : scan-build

# Rule to build all files generated by this target.
tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/build: scan-build
.PHONY : tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/build

tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build && $(CMAKE_COMMAND) -P CMakeFiles/scan-build.dir/cmake_clean.cmake
.PHONY : tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/clean

tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/tools/scan-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/clang/tools/scan-build/CMakeFiles/scan-build.dir/depend

