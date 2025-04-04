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
include lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/progress.make

# Include the compile flags for this target's objects.
include lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoadStoreVectorizer.cpp
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o -MF CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o.d -o CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoadStoreVectorizer.cpp

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoadStoreVectorizer.cpp > CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.i

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoadStoreVectorizer.cpp -o CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.s

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoopVectorize.cpp
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o -MF CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o.d -o CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoopVectorize.cpp

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoopVectorize.cpp > CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.i

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/LoopVectorize.cpp -o CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.s

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o -MF CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o.d -o CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp > CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.i

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp -o CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.s

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/Vectorize.cpp
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o -MF CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o.d -o CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/Vectorize.cpp

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/Vectorize.cpp > CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.i

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/Vectorize.cpp -o CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.s

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/flags.make
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o: /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/VPlan.cpp
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o -MF CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o.d -o CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o -c /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/VPlan.cpp

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LLVMVectorize.dir/VPlan.cpp.i"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/VPlan.cpp > CMakeFiles/LLVMVectorize.dir/VPlan.cpp.i

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LLVMVectorize.dir/VPlan.cpp.s"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize/VPlan.cpp -o CMakeFiles/LLVMVectorize.dir/VPlan.cpp.s

# Object files for target LLVMVectorize
LLVMVectorize_OBJECTS = \
"CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o" \
"CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o" \
"CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o" \
"CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o" \
"CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o"

# External object files for target LLVMVectorize
LLVMVectorize_EXTERNAL_OBJECTS =

lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoadStoreVectorizer.cpp.o
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/LoopVectorize.cpp.o
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/SLPVectorizer.cpp.o
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/Vectorize.cpp.o
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/VPlan.cpp.o
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/build.make
lib/libLLVMVectorize.a: lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library ../../libLLVMVectorize.a"
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && $(CMAKE_COMMAND) -P CMakeFiles/LLVMVectorize.dir/cmake_clean_target.cmake
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/LLVMVectorize.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/build: lib/libLLVMVectorize.a
.PHONY : lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/build

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/clean:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize && $(CMAKE_COMMAND) -P CMakeFiles/LLVMVectorize.dir/cmake_clean.cmake
.PHONY : lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/clean

lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/depend:
	cd /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/llvm/lib/Transforms/Vectorize /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize /usr/scratch/jkim3998/bc_test/fccm2025_artifact_eval/llvm-project/hls-build/lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/Transforms/Vectorize/CMakeFiles/LLVMVectorize.dir/depend

