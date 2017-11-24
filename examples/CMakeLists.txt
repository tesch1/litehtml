#
# (c)2016 Michael Tesch. tesch1@gmail.com
# 
# Contributed to litehtml project under APACHE-2.0:
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

#
# Setup thirdparty libs
#
file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty")

# generator name
if (NOT "${CMAKE_EXTRA_GENERATOR}" STREQUAL "")
  set (GENERATOR_STRING "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else ()
  set (GENERATOR_STRING "${CMAKE_GENERATOR}")
endif ()

# configure the thirdparty build dir
set (CMD_ARGS ${CMD_ARGS} -G${GENERATOR_STRING})
set (CMD_ARGS ${CMD_ARGS} -DANDROID=${ANDROID})
set (CMD_ARGS ${CMD_ARGS} -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES})
set (CMD_ARGS ${CMD_ARGS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
set (CMD_ARGS ${CMD_ARGS} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM})

execute_process (
  COMMAND ${CMAKE_COMMAND} ${CMD_ARGS} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty"
  RESULT_VARIABLE THIRDPARTY_CONFIG_RESULT
  )
if (THIRDPARTY_CONFIG_RESULT)
  message (FATAL_ERROR "Configuring dependencies failed: ${THIRDPARTY_CONFIG_RESULT}")
endif ()

# build the thirdparty dir
execute_process (
  COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/thirdparty"
  RESULT_VARIABLE THIRDPARTY_BUILD_RESULT
  )
if (THIRDPARTY_BUILD_RESULT)
  message (FATAL_ERROR "Building dependencies failed: ${THIRDPARTY_BUILD_RESULT}")
endif ()

add_subdirectory ("thirdparty")
message ("FREETYPE_SRC_DIR: ${FREETYPE_SRC_DIR}")

#
# Freetype configuration
#
set (WITH_ZLIB OFF)
set (WITH_BZip2 OFF)
set (WITH_PNG OFF)
set (WITH_HarfBuzz OFF)
set (FREETYPE_NO_DIST ON)
set (SKIP_INSTALL_ALL ON)
add_subdirectory ("${FREETYPE_SRC_DIR}" "thirdparty_freetype")
set (SKIP_INSTALL_ALL OFF)
set (FREETYPE_INCLUDE_DIR "${FREETYPE_SRC_DIR}/include")

#
# SDL2
#
find_library (SDL2_LIBRARIES SDL2 REQUIRED)
find_path (SDL2_INCLUDE_DIRS "SDL2/SDL.h" PATH_SUFFIXES "SDL" "SDL2")

#
# SDL2 Browser App
#
set (BROWSER_SRCS
  browser.cpp
  sdl-container.cpp)

add_executable (browser ${BROWSER_SRCS})
target_link_libraries (browser litehtml)
target_link_libraries (browser ${SDL2_LIBRARIES})
target_include_directories (browser PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories (browser PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories (browser PUBLIC ${FREETYPE_INCLUDE_DIR})
