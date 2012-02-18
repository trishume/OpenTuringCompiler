#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.8)
#Name your project here
project(TuringCompiler)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules )

set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR}/bin)

# LLVM
find_package(LLVM REQUIRED)

if( NOT LLVM_FOUND )
  message(FATAL_ERROR "LLVM package can't be found. Set CMAKE_PREFIX_PATH variable to LLVM's installation prefix.")
endif()

include_directories( ${LLVM_INCLUDE_DIR} )
link_directories( ${LLVM_LIB_DIR} )

add_subdirectory(src)

enable_testing()

add_test (Compiler13Test src/compiler ${PROJECT_SOURCE_DIR}/testcompile13.txt)