#!/bin/bash
#
# (c)2017 Michael Tesch. tesch1@gmail.com
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

: ${CMAKE=cmake}
: ${CMAKE_FLAGS=""}

CMDLINE="$0 $@"
SCRIPT=$(basename "$0")
SRCDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OnError() {
    echo "$SCRIPT: Error on line ${BASH_LINENO[0]}, exiting."
    exit 1
}
trap OnError ERR

#CMAKE_FLAGS="${CMAKE_FLAGS} -DSPIND_REAL_FLOAT=1" --trace --debug-output
if command -v nproc &> /dev/null ; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null ; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=1
fi

# if available, use cmake3
if command -v cmake3 >/dev/null 2>&1 ; then
    CMAKE="cmake3"
fi

# check we're in the right directory
if [ ! -d src/gumbo ]; then
    echo "run build.sh from base source dir"
    exit -1
fi

mkdir -p Build
cd Build
$CMAKE ..
make -j$NPROC

