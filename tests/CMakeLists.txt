add_executable(minglish_tests
    test_main.cpp
    test_minglish_keymap.cpp
    test_word_buffer.cpp
)

target_include_directories(minglish_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(minglish_tests PRIVATE minglish_core)

add_test(NAME minglish_tests COMMAND minglish_tests)
