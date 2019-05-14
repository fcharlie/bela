# top cmake
cmake_minimum_required(VERSION 3.13)

project(bela CXX)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with Privexec.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  message(STATUS "IPO/LTO enabled")
else()
  message(STATUS "IPO/LTO not supported: <${error}>")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(
  ./include
)

add_subdirectory(src/bela)