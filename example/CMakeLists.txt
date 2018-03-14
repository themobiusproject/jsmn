if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

add_executable(simple_example
  simple.c
)
target_link_libraries(simple_example jsmn)

add_executable(jsondump
  jsondump.c
)
target_link_libraries(jsondump jsmn)

endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
