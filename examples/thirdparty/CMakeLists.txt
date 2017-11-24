#
# (c)2016 Michael Tesch. tesch1@gmail.com
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

cmake_minimum_required (VERSION 2.8)
project (thirdparty NONE)

get_directory_property (hasParent PARENT_DIRECTORY)
include (ExternalProject)

#
# Freetype
#
ExternalProject_Add (freetype_
  PREFIX freetype
  URL http://download.savannah.gnu.org/releases/freetype/freetype-2.8.1.tar.gz
  URL_HASH MD5=c947b6b1c513e814cc9d7617a44bc6cf
  DOWNLOAD_DIR "$ENV{HOME}/Downloads"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
  )
if (hasParent)
  ExternalProject_Get_Property (freetype_ source_dir)
  set (FREETYPE_SRC_DIR "${source_dir}" PARENT_SCOPE)
  message ("FREETYPE_SRC_DIR: ${FREETYPE_SRC_DIR}")
endif()
