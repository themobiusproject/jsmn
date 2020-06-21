find_package(CMocka)

add_executable(print_jsmn_test_functions
  print_jsmn_test_functions.c
)

enable_testing()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-invalid-source-encoding")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

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
target_compile_definitions(jsmn_test_default PRIVATE)
target_link_libraries(jsmn_test_default ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default COMMAND jsmn_test_default)

add_executable(jsmn_test_default_low_memory
  jsmn_test.c
)
target_compile_definitions(jsmn_test_default_low_memory PRIVATE JSMN_LOW_MEMORY)
target_link_libraries(jsmn_test_default_low_memory ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default_low_memory COMMAND jsmn_test_default_low_memory)

add_executable(jsmn_test_default_mult_json_fail
  jsmn_test.c
)
target_compile_definitions(jsmn_test_default_mult_json_fail PRIVATE JSMN_MULTIPLE_JSON_FAIL)
target_link_libraries(jsmn_test_default_mult_json_fail ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_default_mult_json_fail COMMAND jsmn_test_default_mult_json_fail)


add_executable(jsmn_test_permissive
  jsmn_test.c
)
target_compile_definitions(jsmn_test_permissive PRIVATE JSMN_PERMISSIVE)
target_link_libraries(jsmn_test_permissive ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_permissive COMMAND jsmn_test_permissive)

add_executable(jsmn_test_permissive_low_memory
  jsmn_test.c
)
target_compile_definitions(jsmn_test_permissive_low_memory PRIVATE JSMN_PERMISSIVE JSMN_LOW_MEMORY)
target_link_libraries(jsmn_test_permissive_low_memory ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_permissive_low_memory COMMAND jsmn_test_permissive_low_memory)

add_executable(jsmn_test_permissive_mult_json_fail
  jsmn_test.c
)
target_compile_definitions(jsmn_test_permissive_mult_json_fail PRIVATE JSMN_PERMISSIVE JSMN_MULTIPLE_JSON_FAIL)
target_link_libraries(jsmn_test_permissive_mult_json_fail ${CMOCKA_LIBRARY})
add_test(NAME jsmn_test_permissive_mult_json_fail COMMAND jsmn_test_permissive_mult_json_fail)
