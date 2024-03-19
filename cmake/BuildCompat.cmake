include(CheckFunctionExists)

check_function_exists(setenv HAVE_SETENV)
if(NOT HAVE_SETENV)
	add_compile_definitions(-DNO_SETENV)
endif()

check_function_exists(strchrnul HAVE_STRCHRNUL)
if(NOT HAVE_STRCHRNUL)
	add_compile_definitions(-DNO_STRCHRNUL)
endif()

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_compile_definitions(-DPOSIX)
endif()