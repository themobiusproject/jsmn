add_executable(simple_example
  simple.c
)
target_link_libraries(simple_example jsmn)

add_executable(jsondump
  jsondump.c
)
target_link_libraries(jsondump jsmn)
