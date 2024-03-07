# provides SQLCIPHER_INCLUDE_DIRS SQLCIPHER_LIBRARIES SQLCIPHER_LIBRARY_DIRS
find_package(SqlCipher REQUIRED)

# provides OPENSSL_INCLUDE_DIRS OPENSSL_LIBRARIES OPENSSL_LIBRARY_DIRS
find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE PK_SOURCES "*.c")
list(REMOVE_ITEM PK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/passkeeper.c")

# pk
add_library(pklib STATIC ${PK_SOURCES})

target_link_directories(pklib INTERFACE ${SQLCIPHER_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})

target_link_libraries(pklib INTERFACE ${SQLCIPHER_LIBRARIES} ${OPENSSL_LIBRARIES})

if(CMAKE_BUILD_TYPE IN_LIST RELEASE_BUILD_TYPES)
	target_link_libraries(pklib PUBLIC release_config) # -Wall -Wextra -Werror -Os
else()
	target_link_libraries(pklib PUBLIC develop_config) # -Wall -Wextra -O0
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(pklib PUBLIC debug_config)
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
	target_link_libraries(pklib PRIVATE coverage_config)
endif()

target_include_directories(pklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR} ${SQLCIPHER_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})

target_precompile_headers(pklib PUBLIC <stddef.h> <stdlib.h> <string.h> <stdio.h> <stdbool.h> <stdint.h>)

# pk
add_executable(pk passkeeper.c)

target_link_libraries(pk pklib)

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})