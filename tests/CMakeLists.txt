find_package(CMocka)

enable_testing()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/local/include)

set_source_files_properties(
    jsmn_test.c
    json_test.c
    PROPERTIES
        COMPILE_DEFINITIONS UNIT_TESTING)

add_executable(jsmn_test_default
  jsmn_test.c
)
target_compile_definitions(jsmn_test_default PRIVATE JSMN_DEFINES)
target_link_libraries(jsmn_test_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default COMMAND jsmn_test_default)

add_executable(jsmn_test_strict
  jsmn_test.c
)
target_compile_definitions(jsmn_test_strict PRIVATE JSMN_DEFINES JSMN_STRICT)
target_link_libraries(jsmn_test_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict COMMAND jsmn_test_strict)

add_executable(jsmn_test_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_links PRIVATE JSMN_DEFINES JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_links COMMAND jsmn_test_links)

add_executable(jsmn_test_strict_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_strict_links PRIVATE JSMN_DEFINES JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_strict_links COMMAND jsmn_test_strict_links)

add_executable(jsmn_test_short_default
  jsmn_test.c
)
target_compile_definitions(jsmn_test_short_default PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS)
target_link_libraries(jsmn_test_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_default COMMAND jsmn_test_short_default)

add_executable(jsmn_test_short_strict
  jsmn_test.c
)
target_compile_definitions(jsmn_test_short_strict PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS JSMN_STRICT)
target_link_libraries(jsmn_test_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict COMMAND jsmn_test_short_strict)

add_executable(jsmn_test_short_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_short_links PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_links COMMAND jsmn_test_short_links)

add_executable(jsmn_test_short_strict_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_short_strict_links PRIVATE JSMN_DEFINES JSMN_SHORT_TOKENS JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_short_strict_links COMMAND jsmn_test_short_strict_links)

add_executable(jsmn_test_sibling_short_default
  jsmn_test.c
)
target_compile_definitions(jsmn_test_sibling_short_default PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS)
target_link_libraries(jsmn_test_sibling_short_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_default COMMAND jsmn_test_sibling_short_default)

add_executable(jsmn_test_sibling_short_strict
  jsmn_test.c
)
target_compile_definitions(jsmn_test_sibling_short_strict PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_STRICT)
target_link_libraries(jsmn_test_sibling_short_strict ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict COMMAND jsmn_test_sibling_short_strict)

add_executable(jsmn_test_sibling_short_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_sibling_short_links PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_sibling_short_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_links COMMAND jsmn_test_sibling_short_links)

add_executable(jsmn_test_sibling_short_strict_links
  jsmn_test.c
)
target_compile_definitions(jsmn_test_sibling_short_strict_links PRIVATE JSMN_DEFINES JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_STRICT JSMN_PARENT_LINKS)
target_link_libraries(jsmn_test_sibling_short_strict_links ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_sibling_short_strict_links COMMAND jsmn_test_sibling_short_strict_links)
