include_directories(${jsmn_SOURCE_DIR})

find_package(CMocka)

enable_testing()

set_source_files_properties(
    ../jsmn.c
    PROPERTIES
        COMPILE_DEFINITIONS UNIT_TESTING)

add_executable(test_default
  ../jsmn.c
  jsmn_test.c
)
target_link_libraries(test_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default COMMAND test_default)

add_executable(test_strict
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_strict PRIVATE JSMN_STRICT)
target_link_libraries(test_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict COMMAND test_strict)

add_executable(test_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_links PRIVATE JSMN_PARENT_LINKS)
target_link_libraries(test_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_links COMMAND test_links)

add_executable(test_strict_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_strict_links PRIVATE JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(test_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict_links COMMAND test_strict_links)

add_executable(test_short_default
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_short_default PRIVATE JSMN_SHORT_TOKENS)
target_link_libraries(test_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_default COMMAND test_short_default)

add_executable(test_short_strict
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_short_strict PRIVATE JSMN_SHORT_TOKENS JSMN_STRICT)
target_link_libraries(test_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict COMMAND test_short_strict)

add_executable(test_short_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_short_links PRIVATE JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
target_link_libraries(test_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_links COMMAND test_short_links)

add_executable(test_short_strict_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_short_strict_links PRIVATE JSMN_SHORT_TOKENS JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(test_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict_links COMMAND test_short_strict_links)

add_executable(test_sibling_short_default
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_default PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS)
target_link_libraries(test_sibling_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_default COMMAND test_sibling_short_default)

add_executable(test_sibling_short_strict
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_strict PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_STRICT)
target_link_libraries(test_sibling_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict COMMAND test_sibling_short_strict)

add_executable(test_sibling_short_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_links PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
target_link_libraries(test_sibling_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_linkst COMMAND test_sibling_short_links)

add_executable(test_sibling_short_strict_links
  ../jsmn.c
  jsmn_test.c
)
target_compile_definitions(test_sibling_short_strict_links PRIVATE JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(test_sibling_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict_links COMMAND test_sibling_short_strict_links)
