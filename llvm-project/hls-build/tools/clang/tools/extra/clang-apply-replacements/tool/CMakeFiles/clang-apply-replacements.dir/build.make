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
include tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/progress.make

# Include the compile flags for this target's objects.
include tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/flags.make

tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o: tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/flags.make
tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/clang-apply-replacements/tool/ClangApplyReplacementsMain.cpp
tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o: tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o -MF CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o.d -o CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/clang-apply-replacements/tool/ClangApplyReplacementsMain.cpp

tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/clang-apply-replacements/tool/ClangApplyReplacementsMain.cpp > CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.i

tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/clang-apply-replacements/tool/ClangApplyReplacementsMain.cpp -o CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.s

# Object files for target clang-apply-replacements
clang__apply__replacements_OBJECTS = \
"CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o"

# External object files for target clang-apply-replacements
clang__apply__replacements_EXTERNAL_OBJECTS =

bin/clang-apply-replacements: tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/ClangApplyReplacementsMain.cpp.o
bin/clang-apply-replacements: tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/build.make
bin/clang-apply-replacements: lib/libclangApplyReplacements.a
bin/clang-apply-replacements: lib/libclangBasic.a
bin/clang-apply-replacements: lib/libclangFormat.a
bin/clang-apply-replacements: lib/libclangRewrite.a
bin/clang-apply-replacements: lib/libclangToolingCore.a
bin/clang-apply-replacements: lib/libclangRewrite.a
bin/clang-apply-replacements: lib/libclangAST.a
bin/clang-apply-replacements: lib/libclangLex.a
bin/clang-apply-replacements: lib/libclangBasic.a
bin/clang-apply-replacements: lib/libLLVM-7.0svn.so
bin/clang-apply-replacements: tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../../../../bin/clang-apply-replacements"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/clang-apply-replacements.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/build: bin/clang-apply-replacements
.PHONY : tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/build

tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool && $(CMAKE_COMMAND) -P CMakeFiles/clang-apply-replacements.dir/cmake_clean.cmake
.PHONY : tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/clean

tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/clang-tools-extra/clang-apply-replacements/tool /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/clang/tools/extra/clang-apply-replacements/tool/CMakeFiles/clang-apply-replacements.dir/depend

