find_package(SqlCipher REQUIRED)

file(GLOB PK_SOURCES "*.c")
list(REMOVE_ITEM PK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interface.c")

# pkdev
add_library(pkdev STATIC ${PK_SOURCES})

target_link_directories(pkdev PUBLIC ${SQLCIPHER_LIBRARY_DIRS})
target_link_libraries(pkdev PUBLIC ${SQLCIPHER_LIBRARIES})

if(CMAKE_BUILD_TYPE IN_LIST RELEASE_BUILD_TYPES)
	target_link_libraries(pkdev PUBLIC release_config) # -Wall -Wextra -Werror -Os
else()
	target_link_libraries(pkdev PUBLIC develop_config) # -Wall -Wextra -O0
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Window") # linux-only, for arc4random generator
	find_package(LibBSD REQUIRED)
	target_link_libraries(pkdev PRIVATE ${LIBBSD_LIBRARIES})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(pkdev PUBLIC debug_config)
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
	target_link_libraries(pkdev PRIVATE coverage_config)
endif()

target_include_directories(pkdev PUBLIC ${PROJECT_BINARY_DIR} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${SQLCIPHER_INCLUDE_DIRS})

# pk
add_executable(pk interface.c)

target_link_libraries(pk pkdev)

set_target_properties(pk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})