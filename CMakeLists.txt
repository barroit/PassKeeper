cmake_minimum_required(VERSION 3.14)

project(
	PassKeeper
	VERSION		0.1.0
	LANGUAGES	C
)

include_directories(${sqlite3_SOURCE_DIR})

file(GLOB PK_SOURCES "src/*.c")

add_executable(
	pk
	${PK_SOURCES}
	${sqlite3_SOURCE_DIR}/sqlite3.c
)

target_include_directories(pk PRIVATE ${sqlite3_SOURCE_DIR})

set_target_properties(pk PROPERTIES C_STANDARD 17)
