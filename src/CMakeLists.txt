if(NOT PK_TEST)
	include(${CMAKE_MODULE_PATH}/FeatureTest.cmake)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL Windows)
	add_compile_definitions(POSIX)

	if(NOT PK_TEST)
		add_compile_definitions(_GNU_SOURCE)
	endif()
endif()

find_package(OpenSSL REQUIRED)
find_package(SqlCipher REQUIRED)

file(GLOB pklib_source *.c)
list(FILTER pklib_source EXCLUDE REGEX passkeeper.c$)
add_library(pklib ${pklib_source})

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