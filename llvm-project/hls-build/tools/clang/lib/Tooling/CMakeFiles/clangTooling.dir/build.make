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
include tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/progress.make

# Include the compile flags for this target's objects.
include tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/AllTUsExecution.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o -MF CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o.d -o CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/AllTUsExecution.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/AllTUsExecution.cpp > CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/AllTUsExecution.cpp -o CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/ArgumentsAdjusters.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o -MF CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o.d -o CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/ArgumentsAdjusters.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/ArgumentsAdjusters.cpp > CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/ArgumentsAdjusters.cpp -o CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CommonOptionsParser.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o -MF CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o.d -o CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CommonOptionsParser.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CommonOptionsParser.cpp > CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CommonOptionsParser.cpp -o CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CompilationDatabase.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o -MF CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o.d -o CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CompilationDatabase.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CompilationDatabase.cpp > CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/CompilationDatabase.cpp -o CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Execution.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o -MF CMakeFiles/clangTooling.dir/Execution.cpp.o.d -o CMakeFiles/clangTooling.dir/Execution.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Execution.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/Execution.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Execution.cpp > CMakeFiles/clangTooling.dir/Execution.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/Execution.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Execution.cpp -o CMakeFiles/clangTooling.dir/Execution.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FileMatchTrie.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o -MF CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o.d -o CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FileMatchTrie.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FileMatchTrie.cpp > CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FileMatchTrie.cpp -o CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FixIt.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o -MF CMakeFiles/clangTooling.dir/FixIt.cpp.o.d -o CMakeFiles/clangTooling.dir/FixIt.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FixIt.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/FixIt.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FixIt.cpp > CMakeFiles/clangTooling.dir/FixIt.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/FixIt.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/FixIt.cpp -o CMakeFiles/clangTooling.dir/FixIt.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/JSONCompilationDatabase.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o -MF CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o.d -o CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/JSONCompilationDatabase.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/JSONCompilationDatabase.cpp > CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/JSONCompilationDatabase.cpp -o CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Refactoring.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o -MF CMakeFiles/clangTooling.dir/Refactoring.cpp.o.d -o CMakeFiles/clangTooling.dir/Refactoring.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Refactoring.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/Refactoring.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Refactoring.cpp > CMakeFiles/clangTooling.dir/Refactoring.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/Refactoring.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Refactoring.cpp -o CMakeFiles/clangTooling.dir/Refactoring.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/RefactoringCallbacks.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o -MF CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o.d -o CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/RefactoringCallbacks.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/RefactoringCallbacks.cpp > CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/RefactoringCallbacks.cpp -o CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/StandaloneExecution.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o -MF CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o.d -o CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/StandaloneExecution.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/StandaloneExecution.cpp > CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/StandaloneExecution.cpp -o CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.s

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/flags.make
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Tooling.cpp
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o -MF CMakeFiles/clangTooling.dir/Tooling.cpp.o.d -o CMakeFiles/clangTooling.dir/Tooling.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Tooling.cpp

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clangTooling.dir/Tooling.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Tooling.cpp > CMakeFiles/clangTooling.dir/Tooling.cpp.i

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clangTooling.dir/Tooling.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling/Tooling.cpp -o CMakeFiles/clangTooling.dir/Tooling.cpp.s

# Object files for target clangTooling
clangTooling_OBJECTS = \
"CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o" \
"CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o" \
"CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o" \
"CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o" \
"CMakeFiles/clangTooling.dir/Execution.cpp.o" \
"CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o" \
"CMakeFiles/clangTooling.dir/FixIt.cpp.o" \
"CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o" \
"CMakeFiles/clangTooling.dir/Refactoring.cpp.o" \
"CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o" \
"CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o" \
"CMakeFiles/clangTooling.dir/Tooling.cpp.o"

# External object files for target clangTooling
clangTooling_EXTERNAL_OBJECTS =

lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/AllTUsExecution.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/ArgumentsAdjusters.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CommonOptionsParser.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/CompilationDatabase.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Execution.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FileMatchTrie.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/FixIt.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/JSONCompilationDatabase.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Refactoring.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/RefactoringCallbacks.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/StandaloneExecution.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/Tooling.cpp.o
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/build.make
lib/libclangTooling.a: tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Linking CXX static library ../../../../lib/libclangTooling.a"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && $(CMAKE_COMMAND) -P CMakeFiles/clangTooling.dir/cmake_clean_target.cmake
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/clangTooling.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/build: lib/libclangTooling.a
.PHONY : tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/build

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling && $(CMAKE_COMMAND) -P CMakeFiles/clangTooling.dir/cmake_clean.cmake
.PHONY : tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/clean

tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang/lib/Tooling /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/clang/lib/Tooling/CMakeFiles/clangTooling.dir/depend

