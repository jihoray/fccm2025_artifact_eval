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
include tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/progress.make

# Include the compile flags for this target's objects.
include tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/flags.make

tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o: tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/flags.make
tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/include-fixer/find-all-symbols/tool/FindAllSymbolsMain.cpp
tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o: tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o -MF CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o.d -o CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/include-fixer/find-all-symbols/tool/FindAllSymbolsMain.cpp

tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/include-fixer/find-all-symbols/tool/FindAllSymbolsMain.cpp > CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.i

tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/include-fixer/find-all-symbols/tool/FindAllSymbolsMain.cpp -o CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.s

# Object files for target find-all-symbols
find__all__symbols_OBJECTS = \
"CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o"

# External object files for target find-all-symbols
find__all__symbols_EXTERNAL_OBJECTS =

bin/find-all-symbols: tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/FindAllSymbolsMain.cpp.o
bin/find-all-symbols: tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/build.make
bin/find-all-symbols: lib/libclangAST.a
bin/find-all-symbols: lib/libclangASTMatchers.a
bin/find-all-symbols: lib/libclangBasic.a
bin/find-all-symbols: lib/libclangFrontend.a
bin/find-all-symbols: lib/libclangLex.a
bin/find-all-symbols: lib/libclangTooling.a
bin/find-all-symbols: lib/libfindAllSymbols.a
bin/find-all-symbols: lib/libclangTooling.a
bin/find-all-symbols: lib/libclangASTMatchers.a
bin/find-all-symbols: lib/libclangFrontend.a
bin/find-all-symbols: lib/libclangParse.a
bin/find-all-symbols: lib/libclangSerialization.a
bin/find-all-symbols: lib/libclangSema.a
bin/find-all-symbols: lib/libclangEdit.a
bin/find-all-symbols: lib/libclangAnalysis.a
bin/find-all-symbols: lib/libclangDriver.a
bin/find-all-symbols: lib/libclangFormat.a
bin/find-all-symbols: lib/libclangToolingCore.a
bin/find-all-symbols: lib/libclangAST.a
bin/find-all-symbols: lib/libclangRewrite.a
bin/find-all-symbols: lib/libclangLex.a
bin/find-all-symbols: lib/libclangBasic.a
bin/find-all-symbols: lib/libLLVM-7.0svn.so
bin/find-all-symbols: tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../../../../../bin/find-all-symbols"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/find-all-symbols.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/build: bin/find-all-symbols
.PHONY : tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/build

tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool && $(CMAKE_COMMAND) -P CMakeFiles/find-all-symbols.dir/cmake_clean.cmake
.PHONY : tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/clean

tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/include-fixer/find-all-symbols/tool /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/clang/tools/extra/include-fixer/find-all-symbols/tool/CMakeFiles/find-all-symbols.dir/depend

