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

# A macro adding all GLSL shaders from a directory as custom targets for the generated solution.
# The compilation target for glsl shaders is a copy in a "glsl" folder located in the executable directory, with the same file path layout
# Recognized extensions:
#  - *.vs.glsl : vertex shader
#  - *.fs.glsl : fragment shader
#  - *.gs.glsl : geometry shader
#  - *.cs.glsl : compute shader
#  - *.xs.glsl : unspecified shader
macro(c2ba_add_shader_directory directory)
    file(GLOB_RECURSE relative_files RELATIVE ${directory} ${directory}/*.vs.glsl ${directory}/*.fs.glsl ${directory}/*.cs.glsl ${directory}/*.gs.glsl ${directory}/*.xs.glsl)
    file(GLOB_RECURSE files ${directory}/*.vs.glsl ${directory}/*.fs.glsl ${directory}/*.cs.glsl ${directory}/*.gs.glsl ${directory}/*.xs.glsl)

    if(files)
        list(LENGTH files file_count)
        math(EXPR range_end "${file_count} - 1")

        foreach(idx RANGE ${range_end})
            list(GET files ${idx} file)
            list(GET relative_files ${idx} relative_file)

            if(MSVC)
                set(SHADER_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}//\$\(Configuration\)/glsl/${relative_file})
            else()
                set(SHADER_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/glsl/${relative_file})
            endif()

            add_custom_command(
                OUTPUT ${SHADER_OUTPUT_PATH}
                COMMAND ${CMAKE_COMMAND} -E copy ${file} ${SHADER_OUTPUT_PATH}
                MAIN_DEPENDENCY ${file}
            )
        endforeach()
    endif()
endmacro()