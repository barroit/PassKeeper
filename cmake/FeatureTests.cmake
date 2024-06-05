if(PK_TEST)
	add_compile_definitions(PK_TEST)
	add_compile_definitions(NO_SETENV NO_STRCHRNUL NO_DIRNAME)
	return()
endif()

include(CheckSymbolExists)

check_symbol_exists(setenv stdlib.h have_setenv)
if(NOT have_setenv)
	add_compile_definitions(NO_SETENV)
endif()

check_symbol_exists(unsetenv stdlib.h have_unsetenv)
if(NOT have_unsetenv)
	add_compile_definitions(NO_UNSETENV)
endif()

check_symbol_exists(strchrnul string.h have_strchrnul)
if(NOT have_strchrnul)
	add_compile_definitions(NO_STRCHRNUL)
endif()

check_symbol_exists(dirname libgen.h have_dirname)
if(NOT have_dirname)
	add_compile_definitions(NO_DIRNAME)
endif()

check_symbol_exists(strdup string.h have_strdup)
if(NOT have_strdup)
	add_compile_definitions(NO_STRDUP)
endif()

check_symbol_exists(strndup string.h have_strndup)
if(NOT have_strndup)
	add_compile_definitions(NO_STRNDUP)
endif()

check_symbol_exists(getline stdio.h have_getline)
if(NOT have_getline)
	add_compile_definitions(NO_GETLINE)
endif()

check_symbol_exists(strnlen string.h have_strnlen)
if(NOT have_strnlen)
	add_compile_definitions(NO_STRNLEN)
endif()

check_symbol_exists(mempcpy string.h have_mempcpy)
if(NOT have_mempcpy)
	add_compile_definitions(NO_MEMPCPY)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 3.1)
	set(have_always_inline 1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 2.6)
	set(have_always_inline 1)
endif()

if (NOT have_always_inline)
	add_compile_definitions(NO_ALWAYS_INLINE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 3.1)
	set(have_unused 1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 2.6)
	set(have_unused 1)
endif()

if (NOT have_unused)
	add_compile_definitions(NO_UNUSED)
endif()
