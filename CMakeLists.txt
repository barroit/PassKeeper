cmake_minimum_required(VERSION 3.27)

if(CMAKE_TARGET_PLATFORM STREQUAL "Window")
	set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
	set(CMAKE_SYSTEM_NAME Windows)
	set(CMAKE_SYSTEM_VERSION 10.0.22631)
else()
	set(CMAKE_C_COMPILER gcc)
endif()

project(PassKeeper VERSION 0.3.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)

set(ENV{PKG_CONFIG_PATH} "${LIB_INSTALL_PREFIX}/lib64/pkgconfig:${LIB_INSTALL_PREFIX}/lib/pkgconfig")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

configure_file(cmake/project-config.h project-config.h)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Werror -O3 -DNDEBUG")

add_subdirectory(src)

if(DEFINED PK_TEST)
	add_subdirectory(test)
endif()