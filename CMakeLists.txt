cmake_minimum_required(VERSION 3.27)

if(CMAKE_BUILD_TYPE STREQUAL "Window")
	set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
	set(CMAKE_SYSTEM_NAME Windows)
	set(CMAKE_SYSTEM_VERSION 10.0.22631)
else()
	set(CMAKE_C_COMPILER gcc)
endif()

project(PassKeeper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)

# config pkgconfig
set(ENV{PKG_CONFIG_PATH} "${INSTALL_PREFIX}/lib64/pkgconfig:${INSTALL_PREFIX}/lib/pkgconfig")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# define release build
list(APPEND RELEASE_BUILD_TYPES "Penguin" "Window")

# used by config.h.in
set(AUTHOR_NAME "Jiamu Sun")
set(CONTACT_ADDRESS barroit@linux.com)
set(PROJECT_NAME passkeeper)
set(CREDENTIAL_ENVNAME CREDENTIAL_PATH)
set(CREDENTIAL_KEY_ENVNAME CREDENTIAL_KEY_PATH)

configure_file(cmake/config.h.in projcfg.h)

# common flags
set(CMAKE_C_FLAGS "-DSQLITE_HAS_CODEC -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-DSQLITE_HAS_CODEC -Wall -Wextra")

add_library(develop_config INTERFACE)
target_compile_options(develop_config INTERFACE -O0)

add_library(release_config INTERFACE)
target_compile_options(release_config INTERFACE -Werror -Os -DNDEBUG)

add_library(debug_config INTERFACE)
target_compile_options(debug_config INTERFACE -g3 -DPK_IS_DEBUG)

add_library(coverage_config INTERFACE)
target_compile_options(coverage_config INTERFACE -g --coverage)
target_link_libraries(coverage_config INTERFACE --coverage)

add_subdirectory(src)
add_subdirectory(test)