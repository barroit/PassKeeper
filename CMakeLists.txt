cmake_minimum_required(VERSION 3.22)

project(passkeeper)

set(CMAKE_CXX_STANDARD 20)

# Include Dependencies
add_subdirectory(external/CLI11)
add_subdirectory(external/sqlite3)

# Add the include directory to the include path
include_directories(${CMAKE_SOURCE_DIR}/include)

# Define executable
add_executable(
		pk
		src/main.cpp
		include/command.hpp
		include/orm.hpp
		include/pkerr.hpp
		src/command/create.cpp
		src/command/read.cpp
		src/command/update.cpp
		src/command/delete.cpp
		src/orm.cpp
)

# Link Dependencies
target_link_libraries(pk PRIVATE sqlite3)
target_link_libraries(pk PRIVATE CLI11)

# Enable precompiled headers
target_precompile_headers(pk PRIVATE include/pch.hpp)
