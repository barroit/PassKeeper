find_package(Check REQUIRED)

file(GLOB PK_TEST_SOURCES *.c)

add_executable(pk_test ${PK_TEST_SOURCES})

target_include_directories(pk_test PRIVATE ${CHECK_INCLUDE_DIRS})
target_link_directories(pk_test PRIVATE ${CHECK_LIBRARY_DIRS})
target_link_libraries(pk_test ${CHECK_LIBRARIES} pkdev)
set_target_properties(pk_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR} C_STANDARD 11)

enable_testing()
add_test(NAME test COMMAND pk_test)