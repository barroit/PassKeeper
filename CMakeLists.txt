cmake_minimum_required(VERSION 3.14)

project(PassKeeper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(PROJECT_NAME PassKeeper)
set(AUTHOR barroit)
set(DEFAULT_DB_FILE $CREDENTIALS)

configure_file(cmake/config.h.in config.h)

add_subdirectory(src)
add_subdirectory(test)