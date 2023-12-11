cmake_minimum_required(VERSION 3.14)

project(
	PassKeeper
	VERSION		0.1.0
	LANGUAGES	C
)

set(PROJECT_NAME PassKeeper)
set(AUTHOR barroit)
set(DEFAULT_DB_FILE $CREDENTIALS)

configure_file(config.h.in config.h)

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -DDEBUG")

include_directories(${sqlite3_SOURCE_DIR})

file(GLOB PK_SOURCES "src/*.c")

add_executable(
	pk
	${PK_SOURCES}
	${sqlite3_SOURCE_DIR}/sqlite3.c
)

target_include_directories(pk PRIVATE ${sqlite3_SOURCE_DIR})
target_include_directories(pk PRIVATE "${PROJECT_BINARY_DIR}")

set_target_properties(pk PROPERTIES C_STANDARD 17)