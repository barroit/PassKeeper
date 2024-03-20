add_compile_definitions(NO_SETENV NO_STRCHRNUL NO_DIRNAME PKTEST)

add_library(pklib "$ENV{SRC_PREFIX}/compat.c" "$ENV{SRC_PREFIX}/usage.c")

target_include_directories(pklib PUBLIC $ENV{SRC_PREFIX} ${PROJECT_BINARY_DIR})

target_precompile_headers(pklib PUBLIC "$ENV{SRC_PREFIX}/compat-util.h")

link_libraries(pklib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(t1000-pk_dirname "t1000-pk_dirname_main.c")