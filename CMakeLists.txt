cmake_minimum_required(VERSION 3.27)

# compiler should set before project()
if("${CMAKE_BUILD_TYPE}" STREQUAL Window)
	set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
	set(CMAKE_SYSTEM_NAME Windows)
	set(CMAKE_SYSTEM_VERSION 10.0.22631)
else()
	set(CMAKE_C_COMPILER gcc)
endif()

project(PassKeeper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(RELEASE_BUILD_TYPES Penguin Window)

# used by config.h
set(PROJECT_NAME PassKeeper)
set(AUTHOR barroit)
set(DEFAULT_DB_FILENAME $CREDENTIALS)

configure_file(cmake/config.h.in config.h)

add_library(develop_config INTERFACE)
target_compile_options(develop_config INTERFACE -Wall -Wextra -O0)

add_library(release_config INTERFACE)
target_compile_options(release_config INTERFACE -Wall -Wextra -Werror -Os)

add_library(debug_config INTERFACE)
target_compile_options(debug_config INTERFACE -g3 -DDEBUG)

add_library(coverage_config INTERFACE)
target_compile_options(coverage_config INTERFACE -g --coverage)
target_link_libraries(coverage_config INTERFACE --coverage)

# we wanna control flags that are applied to target manually
set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_DEBUG "")

add_subdirectory(src)
add_subdirectory(test)