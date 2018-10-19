include_directories(${jsmn_SOURCE_DIR})

find_package(CMocka)

if(DEFINED CMOCKA_LIBRARY)

enable_testing()

set_source_files_properties(
    jsmn_test.c
    PROPERTIES
        COMPILE_DEFINITIONS UNIT_TESTING)

add_executable(test_default
  jsmn_test.c
)
target_link_libraries(test_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default COMMAND test_default)

add_executable(test_strict
  jsmn_test.c
)
target_compile_definitions(test_strict PRIVATE JSMNM_STRICT)
target_link_libraries(test_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict COMMAND test_strict)

add_executable(test_links
  jsmn_test.c
)
target_compile_definitions(test_links PRIVATE JSMNM_PARENT_LINKS)
target_link_libraries(test_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_links COMMAND test_links)

add_executable(test_strict_links
  jsmn_test.c
)
target_compile_definitions(test_strict_links PRIVATE JSMNM_STRICT JSMNM_PARENT_LINKS)
target_link_libraries(test_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict_links COMMAND test_strict_links)

add_executable(test_short_default
  jsmn_test.c
)
target_compile_definitions(test_short_default PRIVATE JSMNM_SHORT_TOKENS)
target_link_libraries(test_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_default COMMAND test_short_default)

add_executable(test_short_strict
  jsmn_test.c
)
target_compile_definitions(test_short_strict PRIVATE JSMNM_SHORT_TOKENS JSMNM_STRICT)
target_link_libraries(test_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict COMMAND test_short_strict)

add_executable(test_short_links
  jsmn_test.c
)
target_compile_definitions(test_short_links PRIVATE JSMNM_SHORT_TOKENS JSMNM_PARENT_LINKS)
target_link_libraries(test_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_links COMMAND test_short_links)

add_executable(test_short_strict_links
  jsmn_test.c
)
target_compile_definitions(test_short_strict_links PRIVATE JSMNM_SHORT_TOKENS JSMNM_STRICT JSMNM_PARENT_LINKS)
target_link_libraries(test_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict_links COMMAND test_short_strict_links)

add_executable(test_sibling_short_default
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_default PRIVATE JSMNM_NEXT_SIBLING JSMNM_SHORT_TOKENS)
target_link_libraries(test_sibling_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_default COMMAND test_sibling_short_default)

add_executable(test_sibling_short_strict
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_strict PRIVATE JSMNM_NEXT_SIBLING JSMNM_SHORT_TOKENS JSMNM_STRICT)
target_link_libraries(test_sibling_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict COMMAND test_sibling_short_strict)

add_executable(test_sibling_short_links
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_links PRIVATE JSMNM_NEXT_SIBLING JSMNM_SHORT_TOKENS JSMNM_PARENT_LINKS)
target_link_libraries(test_sibling_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_linkst COMMAND test_sibling_short_links)

add_executable(test_sibling_short_strict_links
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_strict_links PRIVATE JSMNM_NEXT_SIBLING JSMNM_SHORT_TOKENS JSMNM_STRICT JSMNM_PARENT_LINKS)
target_link_libraries(test_sibling_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict_links COMMAND test_sibling_short_strict_links)

endif(DEFINED CMOCKA_LIBRARY)
