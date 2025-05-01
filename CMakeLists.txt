cmake_minimum_required(VERSION 3.10)
project(cpp_point_management_system VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PostgreSQL (still needed for libpqxx)
find_package(PostgreSQL REQUIRED)

# Find pqxx
find_library(PQXX_LIBRARY NAMES pqxx)
find_path(PQXX_INCLUDE_DIR NAMES pqxx/pqxx)

if(NOT PQXX_LIBRARY OR NOT PQXX_INCLUDE_DIR)
    message(FATAL_ERROR "libpqxx not found. Please install libpqxx-dev package.")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIR}
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
target_link_libraries(${PROJECT_NAME} ${PQXX_LIBRARY} ${PostgreSQL_LIBRARIES} crypto uuid)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
