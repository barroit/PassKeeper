# compile sqlite 3
add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

file(GLOB PK_SOURCES "*.c")
list(REMOVE_ITEM PK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.c")

add_library(libpk STATIC ${PK_SOURCES})

add_executable(pk main.c)

# primarily for config.h
target_include_directories(libpk PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(pk PRIVATE ${PROJECT_BINARY_DIR})

# add debug features if build type is Debug
target_compile_options(pk PRIVATE $<$<CONFIG:Debug>:-g -Wall -Wextra -DDEBUG>)
target_compile_options(libpk PRIVATE $<$<CONFIG:Debug>:-g -Wall -Wextra -DDEBUG>)

target_link_libraries(pk libpk sqlite3)

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR} C_STANDARD 17)
set_target_properties(libpk PROPERTIES OUTPUT_NAME pk C_STANDARD 17)