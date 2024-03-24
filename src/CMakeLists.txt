find_package(OpenSSL REQUIRED)
find_package(SqlCipher REQUIRED)

file(GLOB pklib_source *.c)
list(FILTER pklib_source EXCLUDE REGEX passkeeper.c$)
if (CMAKE_SYSTEM_NAME STREQUAL Windows)
	file(GLOB pklib_source_pf win/*.c)
else()
	file(GLOB pklib_source_pf uni/*.c)
endif()
list(APPEND pklib_source ${pklib_source_pf})

add_library(pklib OBJECT ${pklib_source})

if(PK_TEST)
	target_compile_definitions(pklib PUBLIC PKTEST)
endif()

if(PK_TEST OR CMAKE_SYSTEM_NAME STREQUAL Windows)
	target_compile_definitions(pklib PUBLIC NO_SETENV NO_STRCHRNUL NO_DIRNAME)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL Windows)
	target_compile_definitions(pklib PUBLIC POSIX)

	if(NOT PK_TEST)
		target_compile_definitions(pklib PUBLIC _GNU_SOURCE)
	endif()
endif()

target_include_directories(pklib PUBLIC ${SQLCIPHER_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(pklib PUBLIC ${SQLCIPHER_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_directories(pklib PUBLIC ${SQLCIPHER_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})
target_compile_definitions(pklib PUBLIC SQLITE_HAS_CODEC)

target_include_directories(pklib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(pklib PUBLIC compat-util.h wrapper.h)

file(GLOB pk_source command/*.c)
add_executable(pk passkeeper.c ${pk_source})

target_include_directories(pk PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(pk pklib)
target_precompile_headers(pk PRIVATE [["project-config.h"]])

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY $ENV{BUILD_PREFIX})