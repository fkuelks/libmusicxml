# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

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
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/menu/libmusicxml-git/build

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/menu/libmusicxml-git/build

# Include any dependencies generated for this target.
include CMakeFiles/xmlversion.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/xmlversion.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/xmlversion.dir/flags.make

CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o: CMakeFiles/xmlversion.dir/flags.make
CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o: /Users/menu/libmusicxml-git/samples/xmlversion.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/menu/libmusicxml-git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o -c /Users/menu/libmusicxml-git/samples/xmlversion.cpp

CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/menu/libmusicxml-git/samples/xmlversion.cpp > CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.i

CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/menu/libmusicxml-git/samples/xmlversion.cpp -o CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.s

# Object files for target xmlversion
xmlversion_OBJECTS = \
"CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o"

# External object files for target xmlversion
xmlversion_EXTERNAL_OBJECTS =

bin/xmlversion: CMakeFiles/xmlversion.dir/Users/menu/libmusicxml-git/samples/xmlversion.o
bin/xmlversion: CMakeFiles/xmlversion.dir/build.make
bin/xmlversion: lib/libmusicxml2.a
bin/xmlversion: CMakeFiles/xmlversion.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/menu/libmusicxml-git/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/xmlversion"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/xmlversion.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/xmlversion.dir/build: bin/xmlversion

.PHONY : CMakeFiles/xmlversion.dir/build

CMakeFiles/xmlversion.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/xmlversion.dir/cmake_clean.cmake
.PHONY : CMakeFiles/xmlversion.dir/clean

CMakeFiles/xmlversion.dir/depend:
	cd /Users/menu/libmusicxml-git/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/menu/libmusicxml-git/build /Users/menu/libmusicxml-git/build /Users/menu/libmusicxml-git/build /Users/menu/libmusicxml-git/build /Users/menu/libmusicxml-git/build/CMakeFiles/xmlversion.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/xmlversion.dir/depend
