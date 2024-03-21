target_compile_definitions(pklib PUBLIC NO_SETENV NO_STRCHRNUL NO_DIRNAME PKTEST)
link_libraries(pklib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(t1000-pk_dirname "t1000-pk_dirname_main.c")

add_executable(t1100-binhex-convert "t1100-binhex-convert_main.c")