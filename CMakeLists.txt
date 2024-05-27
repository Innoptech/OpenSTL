cmake_minimum_required(VERSION 3.15.0)


#-------------------------------------------------------------------------------
# Project Definitions
#-------------------------------------------------------------------------------
project(OPENSTL
        DESCRIPTION "A simple STL serializer and deserializer"
        LANGUAGES CXX)

#-------------------------------------------------------------------------------
# VERSIONING
#-------------------------------------------------------------------------------
add_subdirectory(cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(PYPROJECT_PATH "${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml")
configure_file("${PROJECT_SOURCE_DIR}/pyproject.toml" ${PYPROJECT_PATH})
ReadVersion(${PYPROJECT_PATH})

#-------------------------------------------------------------------------------
# COMPILATION
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-math-errno")
    set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
endif()

# Do not allow to build in main repo
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).
    Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Set the default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release")
endif()

#-------------------------------------------------------------------------------
# CMAKE OPTIONS
#-------------------------------------------------------------------------------
option(OPENSTL_BUILD_TESTS "Enable the compilation of the test files." OFF)
option(OPENSTL_BUILD_PYTHON "Enable the compilation of the python binding." OFF)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

#-------------------------------------------------------------------------------
# Add external components
#-------------------------------------------------------------------------------
# No external component at runtime

#-------------------------------------------------------------------------------
# Add module components 
#-------------------------------------------------------------------------------
set(OPENSTL_MODULES core)
foreach(module ${OPENSTL_MODULES})
    add_subdirectory(modules/${module})
endforeach()

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------
if(OPENSTL_BUILD_TESTS)
    ReadDependencyVersion(catch2 ${PYPROJECT_PATH})
    add_subdirectory(extern/catch2)
    # Configure automatic test registration
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    add_subdirectory(tests)
endif()

#-------------------------------------------------------------------------------
# Python
#-------------------------------------------------------------------------------
if(OPENSTL_BUILD_PYTHON)
    ReadDependencyVersion(pybind11 ${PYPROJECT_PATH})
    add_subdirectory(extern/pybind11)
    add_subdirectory(python)
endif()

