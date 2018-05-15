cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0063 NEW)

project(jsmn)

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS   "${CMAKE_C99_STANDARD_COMPILE_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX11_STANDARD_COMPILE_OPTION}")

if(!WIN32)
	set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -pg -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -Wall")
endif(!WIN32)

# Add options to GCC Compiler
if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" AND ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ffat-lto-objects")
	set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} --coverage")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
endif(${CMAKE_C_COMPILER_ID} MATCHES "GNU" AND ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_subdirectory(example)
add_subdirectory(test)

add_library(jsmn
  jsmn.c
  json.c
)
target_compile_definitions(jsmn PUBLIC JSMN_PARENT_LINKS JSMN_NEXT_SIBLING JSMN_SHORT_TOKENS JSMN_STRICT)
