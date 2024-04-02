link_libraries(pklib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{BRTOOL_BUILD_PREFIX})

add_executable(t1000-pk_dirname test/t1000-pk_dirname_main.c)

add_executable(t1100-binhex-convert test/t1100-binhex-convert_main.c)