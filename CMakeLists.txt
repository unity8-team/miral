cmake_minimum_required(VERSION 3.0)
project(miral)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -g -Werror -Wall -pedantic -Wextra -fPIC -fuse-ld=gold")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g -std=c++14 -Werror -Wall -fno-strict-aliasing -pedantic -Wnon-virtual-dtor -Wextra -fPIC -fuse-ld=gold")
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs -fuse-ld=gold")
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,defs -fuse-ld=gold")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,defs -fuse-ld=gold")


include(FindPkgConfig)

pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)
pkg_check_modules(MIRCLIENT mirclient>=0.20 REQUIRED)
pkg_check_modules(MIRRENDERERGLDEV mir-renderer-gl-dev>=0.20 REQUIRED)

include_directories(include
        SYSTEM ${MIRCOMMON_INCLUDE_DIRS}
        SYSTEM ${MIRSERVER_INCLUDE_DIRS}
        SYSTEM ${MIRRENDERERGLDEV_INCLUDE_DIRS}
)

add_subdirectory(miral)
add_subdirectory(smirsh)
