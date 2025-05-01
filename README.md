# cpp_point_management_system

## Introduction
A C++ system for managing user points, wallets, and transactions, supporting OTP-based security.

## Who we are?
A student team at PTIT.

## What we do?
We provide a secure point management system with user, wallet, and transaction management.

## Prerequisites
- C++17 compiler (GCC or Clang)
- CMake 3.10+
- PostgreSQL 12+
- libpq (PostgreSQL C client library)
- OpenSSL
- UUID library

## Database Setup
1. Install PostgreSQL if not already installed.
2. Create a new database:
   ```sh
   sudo -u postgres psql
   CREATE DATABASE point_management_system;
   \c point_management_system
   ```
3. Apply the schema from `database_schemas/schema.sql`:
   ```sh
   sudo -u postgres psql -d point_management_system -f database_schemas/schema.sql
   ```

## How to build and run this project
1. Install required libraries:
   ```sh
   # Ubuntu/Debian
   sudo apt-get install libpq-dev libssl-dev uuid-dev
   
   # macOS (with Homebrew)
   brew install postgresql openssl ossp-uuid
   ```

2. Build the project:
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```
   
3. Run the executable:
   ```sh
   ./cpp_point_management_system
   ```

## Project Structure
- `src/` - Source code files
  - `models/` - Data models (User, Wallet, Transaction)
  - `utils/` - Utility functions (password hashing, UUID generation)
  - `database/` - Database connection and operations
- `include/` - Header files
- `database_schemas/` - Database schema
- `docs/` - Documentation including use cases and class diagrams

## Configuration
Update the database connection settings in `src/main.cpp` to match your PostgreSQL setup:

