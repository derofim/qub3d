#
#	 Copyright (C) 2018 Qub³d Engine Group.
#	 All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without modification,
#  are permitted provided that the following conditions are met:
# 
#  1. Redistributions of source code must retain the above copyright notice, this
#  list of conditions and the following disclaimer.
#  
#  2. Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation and/or
#  other materials provided with the distribution.
#  
#  3. Neither the name of the copyright holder nor the names of its contributors
#  may be used to endorse or promote products derived from this software without
#  specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Use this version because it adds some options that we use.
cmake_minimum_required(VERSION 3.0)

project(qub3d-engine)

set(src       ${PROJECT_SOURCE_DIR}/source/src)
set(headerDir ${PROJECT_SOURCE_DIR}/source/include)

set(sources 
    ${src}/gameIOManager.cpp
    ${src}/logging/logging.cpp
    ${src}/gui/gameStateManager.cpp
    ${src}/gui/states/stateMap.cpp
    ${src}/settingsManager.cpp
)

set(headers
    ${headerDir}/types.hpp
    ${headerDir}/gameIOManager.hpp
    ${headerDir}/logging/logging.hpp
    ${headerDir}/settingsManager.hpp
)

set(libdeps ${CMAKE_CURRENT_LIST_DIR}/../libdeps)

set(include_dirs
    ${libdeps}/glew/include
    ${libdeps}/glm
    ${libdeps}/SDL2/include
    ${libdeps}/yaml-cpp/include
    ${src}/../include
)

set(libs
    glew_s
	yaml-cpp
    SDL2-static
)

# These libraries are required if on Unix systems
if (UNIX AND NOT APPLE)
	find_package(X11 REQUIRED)
	find_package(Threads REQUIRED)
	list(APPEND libs
		${CMAKE_DL_LIBS}
		${CMAKE_THREAD_LIBS_INIT}
		${X11_LIBRARIES}
		${X11_Xxf86vm_LIB}
		GL
	)
endif()

if (WIN32)
    list(APPEND libs
        imm32.lib
        ole32.lib
        oleaut32.lib
        winmm.lib
        version.lib
        opengl32.lib
    )
endif()

set(LIBRARY_OUTPUT_PATH ../../COMPILE/lib)

# Link the engine library.
add_library(${PROJECT_NAME} STATIC ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} ${libs})
include_directories(${include_dirs})
