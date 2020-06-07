if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_executable(test_default
  tests.c
)

add_executable(test_default_links
  tests.c
)
target_compile_definitions(test_default_links PRIVATE JSMN_PARENT_LINKS)

add_executable(test_default_short
  tests.c
)
target_compile_definitions(test_default_short PRIVATE JSMN_SHORT_TOKENS)

add_executable(test_default_short_links
  tests.c
)
target_compile_definitions(test_default_short_links PRIVATE JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)

add_executable(test_default_sibling_short
  tests.c
)
target_compile_definitions(test_default_sibling_short PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS)

add_executable(test_default_sibling_short_links
  tests.c
)
target_compile_definitions(test_default_sibling_short_links PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)


add_executable(test_permissive
  tests.c
)
target_compile_definitions(test_permissive PRIVATE JSMN_PERMISSIVE)

add_executable(test_permissive_links
  tests.c
)
target_compile_definitions(test_permissive_links PRIVATE JSMN_PERMISSIVE JSMN_PARENT_LINKS)

add_executable(test_permissive_short
  tests.c
)
target_compile_definitions(test_permissive_short PRIVATE JSMN_SHORT_TOKENS JSMN_PERMISSIVE)

add_executable(test_permissive_short_links
  tests.c
)
target_compile_definitions(test_permissive_short_links PRIVATE JSMN_SHORT_TOKENS JSMN_PERMISSIVE JSMN_PARENT_LINKS)

add_executable(test_permissive_sibling_short
  tests.c
)
target_compile_definitions(test_permissive_sibling_short PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PERMISSIVE)

add_executable(test_permissive_sibling_short_links
  tests.c
)
target_compile_definitions(test_permissive_sibling_short_links PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PERMISSIVE JSMN_PARENT_LINKS)

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
