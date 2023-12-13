include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Check REQUIRED)

link_directories(${CHECK_LIBRARY_DIRS})

file(GLOB PK_TEST_SOURCES "*.c")

add_executable(pk_test ${PK_TEST_SOURCES})

target_include_directories(pk_test PRIVATE ${CHECK_INCLUDE_DIRS})
target_link_libraries(pk_test libpk ${CHECK_LIBRARIES})