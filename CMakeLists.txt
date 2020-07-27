cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0063 NEW)

project(JSMN)

file(STRINGS "${PROJECT_SOURCE_DIR}/jsmn.h"
	${CMAKE_PROJECT_NAME}_SPEC REGEX "^#[ \t]*define[ \t]+${CMAKE_PROJECT_NAME}_VERSION_[A-Z]+[ \t]+[0-9]+.*$")
if(${CMAKE_PROJECT_NAME}_SPEC)
#	message(STATUS "${${CMAKE_PROJECT_NAME}_SPEC}")
	foreach(item IN ITEMS MAJOR MINOR PATCH)
		string(REGEX REPLACE ".*#[ \t]*define[ \t]+${CMAKE_PROJECT_NAME}_VERSION_${item}+[ \t]+([0-9]+).*$"
			"\\1" XYZ ${${CMAKE_PROJECT_NAME}_SPEC})
		set("${CMAKE_PROJECT_NAME}_VERSION_${item}" ${XYZ} CACHE STRING "Version number for the build of the ${CMAKE_PROJECT_NAME} library.")
	endforeach()
	set("${CMAKE_PROJECT_NAME}_PROJECT_VERSION"
	    ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH}
			CACHE STRING "Full version number for the build of the ${CMAKE_PROJECT_NAME} library.")
endif()

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS   "${CMAKE_C89_STANDARD_COMPILE_OPTION}")
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

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(jsmn_obj OBJECT
	jsmn.c
	jsmn_utils.c
)
set_target_properties(jsmn_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(jsmn
	$<TARGET_OBJECTS:jsmn_obj>
)
target_include_directories(jsmn
	PUBLIC ${PROJECT_SOURCE_DIR}
)

set_target_properties(jsmn
                      PROPERTIES
                      VERSION ${${CMAKE_PROJECT_NAME}_PROJECT_VERSION}
                      SOVERSION ${${CMAKE_PROJECT_NAME}_PROJECT_VERSION}
)

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	add_subdirectory(example)
#	add_subdirectory(test)
	add_subdirectory(tests)
endif(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)
if (DOXYGEN_FOUND)
	# set input and output files
	set(DOXYGEN_${CMAKE_PROJECT_NAME}_DIR ${PROJECT_SOURCE_DIR}/.doxygen)
	set(DOXYGEN_${CMAKE_PROJECT_NAME}_IN ${DOXYGEN_${CMAKE_PROJECT_NAME}_DIR}/Doxyfile.in)
	set(DOXYGEN_${CMAKE_PROJECT_NAME}_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

	# request to configure the file
	configure_file(${DOXYGEN_${CMAKE_PROJECT_NAME}_IN} ${DOXYGEN_${CMAKE_PROJECT_NAME}_OUT} @ONLY)

	# note the option ALL which allows to build the docs together with the application
	add_custom_target(jsmn_doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
	set_directory_properties(${CMAKE_CURRENT_BINARY_DIR}
		PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
			doxygen
			Doxyfile)
else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
