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

project(melisandre-tests)

set(EXECUTABLE_NAME melisandre-tests)
set(MELISANDRE_LIBRARY melisandre)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/*.h ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/gtest-all.cc)

add_executable(
    ${EXECUTABLE_NAME}
    ${SRC_FILES}
)

target_link_libraries(
    ${EXECUTABLE_NAME}
    ${MELISANDRE_LIBRARY} ${SYSTEM_LIBRARIES} ${SDL_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${EMBREE_LIBRARIES} ${OPENEXR_LIBRARIES}
)

c2ba_copy_dll_post_build(${EXECUTABLE_NAME} "${3RD_PARTY_DLL_FILES}")
c2ba_group_sources(${CMAKE_CURRENT_SOURCE_DIR}/src)