find_package(Check REQUIRED)

file(GLOB PK_TEST_SOURCES *.c)

add_executable(pktest ${PK_TEST_SOURCES})

target_include_directories(pktest PRIVATE ${CHECK_INCLUDE_DIRS})

target_link_directories(pktest PRIVATE ${CHECK_LIBRARY_DIRS})

target_link_libraries(pktest ${CHECK_LIBRARIES} pklib)

set_target_properties(pktest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

target_precompile_headers(pktest PRIVATE <check.h>)