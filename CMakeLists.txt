cmake_minimum_required(VERSION 3.10)
project(cpp_point_management_system VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SQLite3
find_package(SQLite3 REQUIRED)
if(NOT SQLite3_FOUND)
    message(FATAL_ERROR "SQLite3 not found. Please install sqlite3 development package.")
endif()

message(STATUS "Found SQLite3: ${SQLite3_INCLUDE_DIRS} (found version ${SQLite3_VERSION})")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# Source files
file(GLOB SOURCES 
    src/*.cpp
    src/models/*.cpp
    src/utils/*.cpp
    src/database/*.cpp
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${SQLite3_LIBRARIES} crypto uuid)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
