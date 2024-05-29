cmake_minimum_required(VERSION 3.27)

project(PassKeeper VERSION $ENV{PK_VERSION} LANGUAGES C)

set(CMAKE_C_STANDARD 17)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PREFIX})

configure_file(project-config.h project-config.h)

include($ENV{CMAKE_MODULE_PREFIX}/CompilerFlags.cmake)

include($ENV{CMAKE_MODULE_PREFIX}/FeatureTests.cmake)

find_package(OpenSSL REQUIRED)
find_package(SqlCipher REQUIRED)

file(GLOB pklib_source src/*.c src/compat/*.c)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	file(GLOB pklib_source_compat src/compat/win/*.c)
else()
	file(GLOB pklib_source_compat src/compat/uni/*.c)
endif()
list(APPEND pklib_source ${pklib_source_compat})
list(FILTER pklib_source EXCLUDE REGEX src/passkeeper.c$)

add_library(pklib OBJECT ${pklib_source})

target_include_directories(pklib PUBLIC ${SQLCIPHER_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(pklib PUBLIC ${SQLCIPHER_LIBRARIES} ${OPENSSL_LIBRARIES} m)
target_link_directories(pklib PUBLIC ${SQLCIPHER_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})
target_compile_definitions(pklib PUBLIC SQLITE_HAS_CODEC)

target_include_directories(pklib PUBLIC src)
target_precompile_headers(pklib PUBLIC src/compat.h src/message.h src/wrapper.h src/enval.h src/helper.h)

file(GLOB pk_source src/command/*.c)
add_executable(pk src/passkeeper.c ${pk_source})

target_include_directories(pk PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(pk pklib)
target_precompile_headers(pk PRIVATE [["project-config.h"]])

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY $ENV{BRTOOL_BUILD_PREFIX})

if(PK_TEST)
	include($ENV{CMAKE_MODULE_PREFIX}/TestTargets.cmake)
endif()
