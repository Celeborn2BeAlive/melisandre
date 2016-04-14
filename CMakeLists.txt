# Copyright (c) 2015 Laurent Noël

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required(VERSION 2.8)

project(melisandre)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Select the default type of build Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Set path to cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include various cmake modules
include(c2ba-glsl-shaders)
include(c2ba-group-sources)
include(c2ba-copy-dll-post-build)

# Change output directories for libraries and binaries
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(WIN32)
    if(MSVC)
        set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third-party/lib/windows/x64/)
        add_definitions(-DNOMINMAX)

        set(SDL_LIBRARIES ${CMAKE_LIBRARY_PATH}/SDL2.lib ${CMAKE_LIBRARY_PATH}/SDL2main.lib)
        set(GLEW_LIBRARIES ${CMAKE_LIBRARY_PATH}/glew32.lib)
        set(EMBREE_LIBRARIES ${CMAKE_LIBRARY_PATH}/embree.lib)
        set(OPENEXR_LIBRARIES ${CMAKE_LIBRARY_PATH}/IlmImf-2_2.lib ${CMAKE_LIBRARY_PATH}/IlmImfUtil-2_2.lib ${CMAKE_LIBRARY_PATH}/Imath-2_2.lib ${CMAKE_LIBRARY_PATH}/IexMath-2_2.lib ${CMAKE_LIBRARY_PATH}/Half.lib ${CMAKE_LIBRARY_PATH}/Iex-2_2.lib ${CMAKE_LIBRARY_PATH}/IlmThread-2_2.lib)

        #set(GLFW_LIBRARY glfw3dll.lib)
        set(ASSIMP_LIBRARY ${CMAKE_LIBRARY_PATH}/assimp-vc130-mt.lib)
        set(SYSTEM_LIBRARIES Shlwapi)
        
        #add_definitions(-DGLEW_NO_GLU -DGLEW_STATIC -DNOMINMAX)

        set(DLL_3RD ${CMAKE_SOURCE_DIR}/third-party/bin/x64)
        file(GLOB 3RD_PARTY_DLL_FILES ${DLL_3RD}/*.dll)
    else()
        message(FATAL_ERROR "Only MSVC 2015 supported for windows builds")
    endif()
elseif(UNIX)
    if(APPLE)
        message(FATAL_ERROR "Apple platform unsupported")
    else()
        set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third-party/lib/linux-x64/)
        
        #set(GLEW_LIBRARY GLEW)
        #set(GLFW_LIBRARY glfw)
        #set(EMBREE_LIBRARY embree)
        #set(ASSIMP_LIBRARY assimp)
        #set(BONEZ_LIBRARY bonez)
        #set(OPENEXR_LIBRARIES libIlmImf-2_2.so libIlmImfUtil-2_2.so Imath-2_2 IexMath-2_2 Half Iex-2_2 IlmThread-2_2 pthread)

        #set(CMAKE_CXX_COMPILER "g++")
        #set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_CXX_FLAGS "-Wall -fPIC -fvisibility-inlines-hidden -fvisibility=hidden -std=c++14 -fno-reciprocal-math")
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/melisandre/src)
include_directories(${CMAKE_SOURCE_DIR}/third-party/include)

# Find libraries
find_package(OpenGL REQUIRED)

# Run CMake for subdirectories
add_subdirectory(melisandre)
add_subdirectory(applications)