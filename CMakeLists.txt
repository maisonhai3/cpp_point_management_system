cmake_minimum_required(VERSION 3.10)
project(cpp_point_management_system VERSION 1.0)

# Explicitly use libc++ (common fix for macOS include path issues)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++") # Also for linker


# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Find SQLite3
find_package(SQLite3 REQUIRED)
if(NOT SQLite3_FOUND)
    message(FATAL_ERROR "SQLite3 not found. Please install sqlite3 development package.")
endif()
message(STATUS "Found SQLite3: ${SQLite3_INCLUDE_DIRS} (found version ${SQLite3_VERSION})")

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL development package (e.g., libssl-dev, openssl-devel).")
endif()
message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR} (found version ${OPENSSL_VERSION})")

# Find UUID library

# Directly set the include directory to the known correct location within the SDK
set(EXPECTED_UUID_INCLUDE_DIR "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include")

# Check if the header file actually exists at the expected location
if(EXISTS "${EXPECTED_UUID_INCLUDE_DIR}/uuid/uuid.h")
    set(UUID_INCLUDE_DIR "${EXPECTED_UUID_INCLUDE_DIR}")
    message(STATUS "Forcibly set UUID include directory to SDK path: ${UUID_INCLUDE_DIR}")
else()
    # If it doesn't exist there, something is wrong with the SDK installation.
    # Fall back to find_path as a last resort, though it might pick the wrong path again.
    message(WARNING "Expected SDK UUID header not found at ${EXPECTED_UUID_INCLUDE_DIR}/uuid/uuid.h! Falling back to find_path which may be unreliable.")
    find_path(UUID_INCLUDE_DIR NAMES uuid/uuid.h)
endif()

# Find the library, preferably in Homebrew
find_library(UUID_LIBRARY NAMES uuid
             HINTS /opt/homebrew/lib
             NO_SYSTEM_ENVIRONMENT_PATH
             NO_CMAKE_FIND_ROOT_PATH)

# Fallback for library if hint fails
if(NOT UUID_LIBRARY)
    message(STATUS "UUID library not found in Homebrew hint, searching default paths...")
    find_library(UUID_LIBRARY NAMES uuid)
endif()

# Final check (Ensure both were successfully determined, either by force or fallback)
if(NOT UUID_INCLUDE_DIR OR NOT UUID_LIBRARY)
    message(FATAL_ERROR "UUID library setup failed. Include dir: '${UUID_INCLUDE_DIR}', Library: '${UUID_LIBRARY}'. Please ensure uuid/uuid.h exists (e.g., in Xcode SDK) and libuuid is installed (e.g., 'brew install ossp-uuid' or 'brew install util-linux').")
elseif(NOT EXISTS "${UUID_INCLUDE_DIR}/uuid/uuid.h")
     # If we fell back to find_path and it found something invalid
    message(FATAL_ERROR "UUID include directory found (${UUID_INCLUDE_DIR}), but uuid/uuid.h does not exist there. Check your SDK/installation.")
endif()

message(STATUS "Using UUID: ${UUID_INCLUDE_DIR} - ${UUID_LIBRARY}")


# Source files
file(GLOB SOURCES
    src/*.cpp
    src/models/*.cpp
    src/utils/*.cpp
    src/database/*.cpp
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES}
        include/models/TransactionRecord.h)

# Include directories for the target (Modern CMake approach)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${UUID_INCLUDE_DIR} # Add UUID include directory here
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${UUID_LIBRARY} # Link the found UUID library
)

# --- Add these lines for debugging ---
get_target_property(INCLUDE_DIRS ${PROJECT_NAME} INCLUDE_DIRECTORIES)
message(STATUS "Effective Include Directories for ${PROJECT_NAME}: ${INCLUDE_DIRS}")
# ------------------------------------

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)