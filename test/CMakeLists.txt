if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_executable(test_default
  tests.c
)

add_executable(test_strict
  tests.c
)
target_compile_definitions(test_strict PRIVATE JSMN_STRICT)

add_executable(test_links
  tests.c
)
target_compile_definitions(test_links PRIVATE JSMN_PARENT_LINKS)

add_executable(test_strict_links
  tests.c
)
target_compile_definitions(test_strict_links PRIVATE JSMN_STRICT JSMN_PARENT_LINKS)

add_executable(test_short_default
  tests.c
)
target_compile_definitions(test_short_default PRIVATE JSMN_SHORT_TOKENS)

add_executable(test_short_strict
  tests.c
)
target_compile_definitions(test_short_strict PRIVATE JSMN_SHORT_TOKENS JSMN_STRICT)

add_executable(test_short_links
  tests.c
)
target_compile_definitions(test_short_links PRIVATE JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)

add_executable(test_short_strict_links
  tests.c
)
target_compile_definitions(test_short_strict_links PRIVATE JSMN_SHORT_TOKENS JSMN_STRICT JSMN_PARENT_LINKS)

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
