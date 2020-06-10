if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_executable(print_tests_string
  print_tests_string.c
)

enable_testing()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-invalid-source-encoding")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

add_executable(test_default
  tests.c
)
target_compile_definitions(test_default PRIVATE)
add_test(NAME test_default COMMAND test_default)

add_executable(test_default_low_memory
  tests.c
)
target_compile_definitions(test_default_low_memory PRIVATE JSMN_LOW_MEMORY)
add_test(NAME test_default_low_memory COMMAND test_default_low_memory)


add_executable(test_permissive
  tests.c
)
target_compile_definitions(test_permissive PRIVATE JSMN_PERMISSIVE)
add_test(NAME test_permissive COMMAND test_permissive)

add_executable(test_permissive_low_memory
  tests.c
)
target_compile_definitions(test_permissive_low_memory PRIVATE JSMN_PERMISSIVE JSMN_LOW_MEMORY)
add_test(NAME test_permissive_low_memory COMMAND test_permissive_low_memory)

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
