add_executable(minglish_tests
    test_main.cpp
    test_minglish_keymap.cpp
    test_word_buffer.cpp
    test_font_profile.cpp
    test_visual_reorderer.cpp
    test_differential_injector.cpp
)

target_include_directories(minglish_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(minglish_tests PRIVATE minglish_core)

add_test(NAME minglish_tests COMMAND minglish_tests)
set_tests_properties(minglish_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
