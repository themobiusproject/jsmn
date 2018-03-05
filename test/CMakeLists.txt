add_executable(test_default
  tests.c
)
target_link_libraries(test_default jsmn)

add_executable(test_strict
  tests.c
)
target_compile_definitions(test_strict PRIVATE JSMN_STRICT=1)
target_link_libraries(test_strict jsmn)

add_executable(test_links
  tests.c
)
target_compile_definitions(test_links PRIVATE JSMN_PARENT_LINKS=1)
target_link_libraries(test_links jsmn)

add_executable(test_strict_links
  tests.c
)
target_compile_definitions(test_strict_links PRIVATE JSMN_STRICT=1 JSMN_PARENT_LINKS=1)
target_link_libraries(test_strict_links jsmn)
