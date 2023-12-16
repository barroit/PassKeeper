find_package(SQLite3 REQUIRED)

add_library(sqlite3 STATIC ${SQLITE3_SOURCE_DIRS}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${SQLITE3_INCLUDE_DIRS})

file(GLOB PK_SOURCES *.c)
list(REMOVE_ITEM PK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

# pkdev
add_library(pkdev STATIC ${PK_SOURCES})
target_link_libraries(pkdev PUBLIC sqlite3 PUBLIC $<IF:$<EQUAL:$<LIST:FIND,${RELEASE_BUILD_TYPES},${CMAKE_BUILD_TYPE}>,-1>,develop_config,release_config>)
if("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
	target_link_libraries(pkdev PUBLIC debug_config)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL Coverage)
	target_link_libraries(pkdev PRIVATE coverage_config)
endif()
target_include_directories(pkdev PUBLIC ${PROJECT_BINARY_DIR} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(pkdev PROPERTIES C_STANDARD 11)

# pk
add_executable(pk main.c)
target_link_libraries(pk pkdev)
set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR} C_STANDARD 11)