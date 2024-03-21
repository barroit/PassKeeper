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
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(openssl INTERFACE ${OPENSSL_LIBRARIES})
target_link_directories(openssl INTERFACE ${OPENSSL_LIBRARY_DIRS})

find_package(SqlCipher REQUIRED)
add_library(sqlcipher INTERFACE)
target_include_directories(sqlcipher INTERFACE ${SQLCIPHER_INCLUDE_DIRS})
target_link_libraries(sqlcipher INTERFACE ${SQLCIPHER_LIBRARIES})
target_link_directories(sqlcipher INTERFACE ${SQLCIPHER_LIBRARY_DIRS})
target_compile_definitions(sqlcipher INTERFACE SQLITE_HAS_CODEC)

file(GLOB pklib_source *.c)
list(FILTER pklib_source EXCLUDE REGEX passkeeper.c$)
add_library(pklib OBJECT ${pklib_source})
target_include_directories(pklib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pklib PUBLIC openssl sqlcipher)
target_precompile_headers(pklib PUBLIC compat-util.h)

file(GLOB pk_source command/*.c)
add_executable(pk passkeeper.c ${pk_source})
target_include_directories(pk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(pk PRIVATE pklib)
target_precompile_headers(pk PRIVATE [["project-config.h"]])

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})