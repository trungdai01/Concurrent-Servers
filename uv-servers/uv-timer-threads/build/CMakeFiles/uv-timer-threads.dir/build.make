# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.30

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = D:\Software\CMake\bin\cmake.exe

# The command to remove a file.
RM = D:\Software\CMake\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build"

# Include any dependencies generated for this target.
include CMakeFiles/uv-timer-threads.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/uv-timer-threads.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/uv-timer-threads.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/uv-timer-threads.dir/flags.make

CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj: CMakeFiles/uv-timer-threads.dir/flags.make
CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj: CMakeFiles/uv-timer-threads.dir/includes_C.rsp
CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj: D:/Programming/C,\ C++/Concurrent\ Servers/uv-servers/uv-timer-threads/uv-timer-threads.c
CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj: CMakeFiles/uv-timer-threads.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir="D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj"
	D:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj -MF CMakeFiles\uv-timer-threads.dir\uv-timer-threads.c.obj.d -o CMakeFiles\uv-timer-threads.dir\uv-timer-threads.c.obj -c "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\uv-timer-threads.c"

CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.i"
	D:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\uv-timer-threads.c" > CMakeFiles\uv-timer-threads.dir\uv-timer-threads.c.i

CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.s"
	D:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\uv-timer-threads.c" -o CMakeFiles\uv-timer-threads.dir\uv-timer-threads.c.s

# Object files for target uv-timer-threads
uv__timer__threads_OBJECTS = \
"CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj"

# External object files for target uv-timer-threads
uv__timer__threads_EXTERNAL_OBJECTS =

uv-timer-threads.exe: CMakeFiles/uv-timer-threads.dir/uv-timer-threads.c.obj
uv-timer-threads.exe: CMakeFiles/uv-timer-threads.dir/build.make
uv-timer-threads.exe: D:/Software/C\ Libraries/libuv-git/libuv/build/libuv.a
uv-timer-threads.exe: CMakeFiles/uv-timer-threads.dir/linkLibs.rsp
uv-timer-threads.exe: CMakeFiles/uv-timer-threads.dir/objects1.rsp
uv-timer-threads.exe: CMakeFiles/uv-timer-threads.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir="D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable uv-timer-threads.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\uv-timer-threads.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/uv-timer-threads.dir/build: uv-timer-threads.exe
.PHONY : CMakeFiles/uv-timer-threads.dir/build

CMakeFiles/uv-timer-threads.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\uv-timer-threads.dir\cmake_clean.cmake
.PHONY : CMakeFiles/uv-timer-threads.dir/clean

CMakeFiles/uv-timer-threads.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads" "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads" "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build" "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build" "D:\Programming\C, C++\Concurrent Servers\uv-servers\uv-timer-threads\build\CMakeFiles\uv-timer-threads.dir\DependInfo.cmake" "--color=$(COLOR)"
.PHONY : CMakeFiles/uv-timer-threads.dir/depend

