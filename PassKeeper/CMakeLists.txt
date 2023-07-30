cmake_minimum_required(VERSION 3.26)
project(PassKeeper)

find_package(Boost 1.82.0 COMPONENTS program_options REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Cannot find Boost")
endif ()

find_package(SQLite3)
if (NOT SQLite3_FOUND)
    message(FATAL_ERROR "Cannot find SQLite3")
endif ()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 23)

add_executable(
        PassKeeper
        PassKeeper.cpp
        Command/ArgumentsParser.cpp
        Command/ArgumentsParser.h
        Persistence/Database.cpp
        Persistence/Database.h
)

target_link_libraries(PassKeeper LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(PassKeeper LINK_PUBLIC ${SQLite3_LIBRARIES})
