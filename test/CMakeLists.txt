if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_executable(print_tests_string
  print_tests_string.c
)

enable_testing()

add_executable(test_default
  tests.c
)
target_compile_definitions(test_default PRIVATE JSMN_DEFINES)
add_test(NAME test_default COMMAND test_default)

add_executable(test_default_links
  tests.c
)
target_compile_definitions(test_default_links PRIVATE JSMN_DEFINES JSMN_PARENT_LINKS)
add_test(NAME test_default_links COMMAND test_default_links)

add_executable(test_default_short
  tests.c
)
target_compile_definitions(test_default_short PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS)
add_test(NAME test_default_short COMMAND test_default_short)

add_executable(test_default_short_links
  tests.c
)
target_compile_definitions(test_default_short_links PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
add_test(NAME test_default_short_links COMMAND test_default_short_links)

add_executable(test_default_sibling_short
  tests.c
)
target_compile_definitions(test_default_sibling_short PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS)
add_test(NAME test_default_sibling_short COMMAND test_default_sibling_short)

add_executable(test_default_sibling_short_links
  tests.c
)
target_compile_definitions(test_default_sibling_short_links PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
add_test(NAME test_default_sibling_short_links COMMAND test_default_sibling_short_links)

add_executable(test_permissive
  tests.c
)
target_compile_definitions(test_permissive PRIVATE JSMN_DEFINES JSMN_PERMISSIVE)
add_test(NAME test_permissive COMMAND test_permissive)

add_executable(test_permissive_links
  tests.c
)
target_compile_definitions(test_permissive_links PRIVATE JSMN_DEFINES JSMN_PERMISSIVE JSMN_PARENT_LINKS)
add_test(NAME test_permissive_links COMMAND test_permissive_links)

add_executable(test_permissive_short
  tests.c
)
target_compile_definitions(test_permissive_short PRIVATE JSMN_DEFINES JSMN_PERMISSIVE JSMN_SHORT_TOKENS)
add_test(NAME test_permissive_short COMMAND test_permissive_short)

add_executable(test_permissive_short_links
  tests.c
)
target_compile_definitions(test_permissive_short_links PRIVATE JSMN_DEFINES JSMN_PERMISSIVE JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
add_test(NAME test_permissive_short_links COMMAND test_permissive_short_links)

add_executable(test_permissive_sibling_short
  tests.c
)
target_compile_definitions(test_permissive_sibling_short PRIVATE JSMN_DEFINES JSMN_PERMISSIVE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS)
add_test(NAME test_permissive_sibling_short COMMAND test_permissive_sibling_short)

add_executable(test_permissive_sibling_short_links
  tests.c
)
target_compile_definitions(test_permissive_sibling_short_links PRIVATE JSMN_DEFINES JSMN_PERMISSIVE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
add_test(NAME test_permissive_sibling_short_links COMMAND test_permissive_sibling_short_links)

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
