cmake_minimum_required(VERSION 3.20)
project(ModernISM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for Windows UTF-8 execution character set
if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Empty core library initially (sources added in subsequent tasks)
add_library(minglish_core STATIC
    src/engine/minglish_keymap.cpp
)
target_include_directories(minglish_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(tests)
