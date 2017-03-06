cmake_minimum_required(VERSION 3.1)
project(miral)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "/usr install prefix" FORCE)
endif()

include (GNUInstallDirs)
include(CheckCXXCompilerFlag)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS off)

set(MIRAL_COMPILER_FLAGS "-pthread -g -Werror -Wall -pedantic -Wextra -fPIC")
set(MIRAL_LINKER_FLAGS   "-Wl,-z,defs")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MIRAL_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MIRAL_COMPILER_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long") # fix spurious FTBFS on X+O
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MIRAL_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${MIRAL_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${MIRAL_LINKER_FLAGS}")

include(FindPkgConfig)

pkg_check_modules(MIRCLIENT mirclient>=0.20 REQUIRED)

include_directories(include SYSTEM ${MIRCLIENT_INCLUDE_DIRS})

set(MIRAL_VERSION_MAJOR 1)
set(MIRAL_VERSION_MINOR 4)
set(MIRAL_VERSION_PATCH 0)

set(MIRAL_VERSION ${MIRAL_VERSION_MAJOR}.${MIRAL_VERSION_MINOR}.${MIRAL_VERSION_PATCH})

add_subdirectory(miral)
add_subdirectory(miral-shell)
add_subdirectory(miral-kiosk)

pkg_check_modules(MIRTEST mirtest)
if(NOT MIRTEST_FOUND)
    message(WARNING "mirtest-dev package not installed - tests cannot be built")
else()
    if (MIRTEST_VERSION VERSION_LESS 0.24)
        # building the tests is disabled by default because the mirtest-dev package is broken on xenial
        # (lp:1583536 and lp:1603080)
        option(MIRAL_ENABLE_TESTS "Build tests" off)
    else()
        option(MIRAL_ENABLE_TESTS "Build tests" on)
    endif()

if (MIRAL_ENABLE_TESTS)
    enable_testing()
    find_package(GMock REQUIRED)
    add_subdirectory(test)
endif()
endif()

include (cmake/Doxygen.cmake)

if(TARGET doc)
    add_custom_target(symbols
        ${CMAKE_SOURCE_DIR}/scripts/process_doxygen_xml.py ${CMAKE_BINARY_DIR}/doc/xml/*.xml > ${CMAKE_SOURCE_DIR}/miral/symbols.map
        DEPENDS doc)
endif()
