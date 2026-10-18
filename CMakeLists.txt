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
    src/engine/word_buffer.cpp
    src/engine/font_profile.cpp
    src/engine/visual_reorderer.cpp
    src/engine/differential_injector.cpp
    src/engine/transpiler_engine.cpp
    src/engine/text_injector.cpp
    src/hook/keyboard_hook.cpp
    src/hook/mouse_hook.cpp
    src/ui/status_pill.cpp
    src/ui/font_switcher_dialog.cpp
    src/ui/cheat_sheet_dialog.cpp
    src/ui/tray_icon.cpp
)
target_include_directories(minglish_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(minglish_core PUBLIC user32 gdi32 shell32)

add_executable(ModernISM WIN32 src/main.cpp)
target_link_libraries(ModernISM PRIVATE minglish_core)

enable_testing()
add_subdirectory(tests)
