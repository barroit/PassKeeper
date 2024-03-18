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

set(AUTHOR "Jiamu Sun")
set(CONTACT "barroit@linux.com")
set(PROJNAME "BR passkeeper")
set(LICENSE_NOTIES [=[Copyright (C) 2023, 2024 Jiamu Sun\nLicense GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n]=])

set(ARCHITECTURE $ENV{ARCHITECTURE})
set(BUILD_COMMIT $ENV{BUILD_COMMIT})

set(CREDENTIAL_ENVNAME "CREDENTIAL_PATH")
set(CREDENTIAL_KEY_ENVNAME "CREDENTIAL_KEY_PATH")

configure_file(cmake/project-config.h project-config.h)

# common flags
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Werror -O3 -DNDEBUG")

add_subdirectory(src)