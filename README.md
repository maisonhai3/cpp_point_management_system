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
- SQLite3 library
- OpenSSL
- UUID library

## Database Setup
The application will automatically create an SQLite database file in the `data` directory when it's first run. No manual setup is required.

The database schema will be automatically created with tables for:
- Users
- Wallets
- Transactions

## How to build and run this project
1. Install required libraries:
   ```sh
   # Ubuntu/Debian
   sudo apt-get install libsqlite3-dev libssl-dev uuid-dev
   
   # macOS (with Homebrew)
   brew install sqlite openssl ossp-uuid
   
   # Windows (with vcpkg)
   vcpkg install sqlite3 openssl libuuid
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
- `data/` - Contains the SQLite database file (created automatically)
- `docs/` - Documentation including use cases and class diagrams

## Database Location
The application creates and uses an SQLite database file at `data/point_management.db`. This file will be created automatically when the program is first run. You can directly examine the database using the SQLite command line tool:

```sh
sqlite3 data/point_management.db

