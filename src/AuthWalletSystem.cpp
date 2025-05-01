#include "../include/AuthWalletSystem.h"
#include <iostream>
#include <sqlite3.h>
#include <string> // Required for std::to_string
#include "AuthWalletSystem.h" // Include header của lớp
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <chrono>
#include <algorithm>
#include "../include/utils/PasswordUtils.h"
#include "../include/utils/UUIDGenerator.h" // <-- Add include for UUID generation

// ... other includes and using directives ...

// Include User model
#include "../include/models/User.h" // Ensure User is included
// Include AuthWalletSystem header
#include "../include/AuthWalletSystem.h" 

// Remove the free function declaration if it exists here:
// std::string generateUniqueWalletId(); // <-- REMOVE THIS LINE


// --- Implementation of generateUniqueWalletId as a member function ---
std::string AuthWalletSystem::generateUniqueWalletId() {
    if (!db) { // Ensure DB is connected
        std::cerr << "ERROR: Database not connected in generateUniqueWalletId.\n";
        return ""; // Return empty string indicates error
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql_check = "SELECT COUNT(*) FROM Wallets WHERE wallet_id = ?;";
    std::string candidateId;
    int count = 1; // Initialize to 1 to ensure the loop runs at least once

    do {
        // 1. Generate a new potential ID using the project's UUID generator
        candidateId = UUIDGenerator::generateUUID();

        // 2. Prepare the SQL statement to check if the ID exists
        int rc = sqlite3_prepare_v2(db, sql_check, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "ERROR preparing wallet ID check statement: " << sqlite3_errmsg(db) << std::endl;
            if (stmt) sqlite3_finalize(stmt); // Clean up if partially prepared
            return ""; // Indicate error
        }

        // 3. Bind the candidate ID to the prepared statement
        sqlite3_bind_text(stmt, 1, candidateId.c_str(), -1, SQLITE_STATIC);

        // 4. Execute the statement and check the count
        count = 0; // Reset count for this check
        rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW) {
            // A row was returned, get the count
            count = sqlite3_column_int(stmt, 0);
        } else if (rc != SQLITE_DONE) {
            // An error occurred during execution (SQLITE_DONE means no rows found, which is good if count is 0)
            std::cerr << "ERROR executing wallet ID check statement: " << sqlite3_errmsg(db) << std::endl;
            count = 1; // Force loop retry or indicate error state
                       // Consider returning "" here as well for critical errors
        }
        // else: rc == SQLITE_DONE, meaning ID was not found (count remains 0)

        // 5. Finalize the statement for this iteration
        sqlite3_finalize(stmt);
        stmt = nullptr; // Reset for the next potential iteration

    } while (count > 0); // Loop only if the count was greater than 0 (ID already exists)

    // 6. If the loop exits, candidateId is unique
    return candidateId;
}


// --- Constructor, Destructor, isDbConnected remain the same ---
AuthWalletSystem::AuthWalletSystem() : db(nullptr), currentUser(nullptr) {
    std::cout << "Initializing AuthWalletSystem..." << std::endl;

    // Define the database path as a constant
    const char* dbPath = "../database/wallet_app.db";

    // Mở kết nối CSDL SQLite using the constant
    int rc = sqlite3_open(dbPath, &db);

    if (rc != SQLITE_OK) {
        // Nếu mở lỗi, db sẽ là nullptr hoặc chứa thông tin lỗi
        // Use the constant in the error message
        std::cerr << "ERROR: Cannot open database '" << dbPath << "': " << sqlite3_errmsg(db) << std::endl;
        // db có thể vẫn cần được close nếu lỗi xảy ra sau khi cấp phát một phần
        if (db) {
           sqlite3_close(db); // Cố gắng đóng
           db = nullptr;       // Đặt lại là nullptr
        }
        // Bạn có thể muốn ném một ngoại lệ ở đây để báo hiệu lỗi khởi tạo nghiêm trọng
        // throw std::runtime_error("Failed to open database");
    } else {
        // Use the constant in the success message
        std::cout << "Database opened successfully: " << dbPath << std::endl;

        // Bật kiểm tra khóa ngoại (QUAN TRỌNG!)
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "ERROR: Failed to enable foreign keys: " << errMsg << std::endl;
            sqlite3_free(errMsg); // Giải phóng bộ nhớ thông báo lỗi
            // Xem xét việc đóng CSDL hoặc ném ngoại lệ nếu bật FK là bắt buộc
            sqlite3_close(db);
            db = nullptr;
            // throw std::runtime_error("Failed to enable foreign keys");
        } else {
            std::cout << "Foreign key enforcement enabled." << std::endl;
        }
    }
}

AuthWalletSystem::~AuthWalletSystem() {
    std::cout << "Destroying AuthWalletSystem..." << std::endl;
    // Đóng kết nối CSDL nếu nó đã được mở thành công
    if (db != nullptr) {
        int rc = sqlite3_close(db);
        if (rc == SQLITE_BUSY) {
             std::cerr << "WARNING: Database connection was busy. Could not close immediately." << std::endl;
             // Có thể bạn còn prepared statement chưa finalize. Cần xử lý triệt để hơn.
             // Trong ứng dụng đơn giản, lỗi này ít xảy ra nếu code đúng.
        } else if (rc != SQLITE_OK) {
            std::cerr << "ERROR: Failed to close database cleanly: " << sqlite3_errmsg(db) << std::endl;
        } else {
            std::cout << "Database closed successfully." << std::endl;
        }
        // Dù có lỗi hay không, không còn truy cập được db nữa
        db = nullptr;
    }
    // std::unique_ptr<User> currentUser sẽ tự động được giải phóng (nếu nó quản lý User)
}

bool AuthWalletSystem::isDbConnected() const {
    return db != nullptr;
}

// --- Make sure registerUser calls the member function ---
void AuthWalletSystem::logout() {
    currentUser.reset(); // Reset the unique_ptr, effectively deleting the User object and clearing the session state.
    std::cout << "You have been logged out." << std::endl; 
}

bool AuthWalletSystem::login(const std::string& username, const std::string& password) {
    if (!db) {
        std::cerr << "Database connection not established!" << std::endl;
        return false;
    }

    // Prepare SQL statement to retrieve user data
    const char* sql = "SELECT user_id, username, hashed_password, salt, full_name, contact_info, "
                     "role, password_status FROM Users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Bind the username parameter
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind username parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    
    bool loginSuccess = false;
    
    // Execute the query and process the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        std::string dbUsername = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string hashedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string contactInfo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        UserRole role = (roleStr == "ADMIN") ? UserRole::ADMIN : UserRole::USER;
        
        std::string pwdStatusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        PasswordStatus pwdStatus = (pwdStatusStr == "AUTO_GENERATED") ? PasswordStatus::AUTO_GENERATED : PasswordStatus::USER_SET;
        
        // Get wallet ID for the user
        std::string walletId;
        sqlite3_stmt* walletStmt;
        const char* walletSql = "SELECT wallet_id FROM Wallets WHERE user_id = ?";
        
        if (sqlite3_prepare_v2(db, walletSql, -1, &walletStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_bind_int(walletStmt, 1, userId) == SQLITE_OK) {
                if (sqlite3_step(walletStmt) == SQLITE_ROW) {
                    walletId = reinterpret_cast<const char*>(sqlite3_column_text(walletStmt, 0));
                }
            }
            sqlite3_finalize(walletStmt);
        }
        
        // Create a new User object with retrieved data
        auto user = std::make_unique<User>(userId, dbUsername, hashedPassword, salt, 
                                          fullName, contactInfo, role, walletId, pwdStatus);
        
        // Verify password using PasswordUtils
        #include "utils/PasswordUtils.h"
        
        // Compare the password with stored hash
        if (PasswordUtils::verifyPassword(password, hashedPassword, salt)) {
            // Password is correct, set the current user
            currentUser = std::move(user);
            loginSuccess = true;
        }
    }
    
    sqlite3_finalize(stmt);
    
    // Check if user needs to change password
    if (loginSuccess && currentUser && currentUser->getPasswordStatus() == PasswordStatus::AUTO_GENERATED) {
        std::cout << "Login successful, but you need to change your auto-generated password." << std::endl;
        // Notify caller that password change is required
        // This would typically set a flag or return a specific code to indicate
        // the system should redirect to UC-AUTH-05 (password change)
    } else if (loginSuccess) {
        std::cout << "Login successful!" << std::endl;
    } else {
        std::cout << "Invalid username or password." << std::endl;
    }
    
    return loginSuccess;
}

bool AuthWalletSystem::registerUser(const std::string& username, const std::string& password,
                                   const std::string& fullName, const std::string& contactInfo)
{
    // --- Bỏ phần mở/đóng CSDL và PRAGMA ở đây ---

    // *** THÊM KIỂM TRA KẾT NỐI ***
    if (!isDbConnected()) { // Hoặc if (!db)
         std::cerr << "Loi: Khong co ket noi CSDL de dang ky!\n";
         return false;
    }

    std::string confirmPassword; // Vẫn cần confirm password
    std::cout << "Xac nhan mat khau (trong ham registerUser): "; // Có thể bạn muốn tách logic input ra ngoài
    std::getline(std::cin >> std::ws, confirmPassword);
    if (password != confirmPassword) {
        std::cerr << "Loi: Mat khau xac nhan khong khop!\n";
        return false;
    }


    // 3. Kiểm tra Username tồn tại (Dùng biến thành viên 'db')
    sqlite3_stmt* stmt_check;
    const char* sql_check = "SELECT COUNT(*) FROM Users WHERE username = ?;";
    int rc = sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, nullptr); // Sử dụng 'db' của lớp
    if (rc != SQLITE_OK) {
        std::cerr << "Loi chuan bi truy van kiem tra username: " << sqlite3_errmsg(db) << std::endl;
        // Không cần close db ở đây nữa
        return false;
    }

    // ... (Phần bind, step, finalize của kiểm tra username như cũ, dùng biến 'db') ...
     sqlite3_bind_text(stmt_check, 1, username.c_str(), -1, SQLITE_STATIC);
     int user_count = 0;
     if (sqlite3_step(stmt_check) == SQLITE_ROW) {
         user_count = sqlite3_column_int(stmt_check, 0);
     } else {
          std::cerr << "Loi thuc thi truy van kiem tra username: " << sqlite3_errmsg(db) << std::endl;
          sqlite3_finalize(stmt_check);
          return false;
     }
     sqlite3_finalize(stmt_check);
     if (user_count > 0) {
         std::cerr << "Loi: Ten dang nhap '" << username << "' da ton tai!\n";
         return false;
     }


    // 4. Chuẩn bị Dữ liệu User và Wallet
    std::string salt = PasswordUtils::generateSalt();
    std::string hashedPassword = PasswordUtils::hashPassword(password, salt);
    std::string walletId = generateUniqueWalletId(); // <-- Now calls the member function
    if (walletId.empty()) { // Check if ID generation failed
        std::cerr << "Loi: Khong the tao Wallet ID duy nhat!\n";
        return false; // Abort registration if we couldn't get a wallet ID
    }
    std::string userRole = "USER";
    std::string passStatus = "USER_SET";

    // 5. Bắt đầu Transaction (Dùng biến thành viên 'db')
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Loi bat dau transaction: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    sqlite3_stmt* stmt_insert_user = nullptr;
    sqlite3_stmt* stmt_insert_wallet = nullptr;
    bool success = true;
    long long last_user_id = -1;

    // 6. Insert User (Dùng biến thành viên 'db')
    // ... (Code prepare, bind, step, finalize cho insert user như cũ, dùng biến 'db') ...
    const char* sql_insert_user = "INSERT INTO Users (username, hashed_password, salt, full_name, contact_info, role, password_status) VALUES (?, ?, ?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql_insert_user, -1, &stmt_insert_user, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt_insert_user, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 3, salt.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 4, fullName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 5, contactInfo.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 6, userRole.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert_user, 7, passStatus.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt_insert_user) != SQLITE_DONE) {
            std::cerr << "Loi insert User: " << sqlite3_errmsg(db) << std::endl;
            success = false;
        } else {
            last_user_id = sqlite3_last_insert_rowid(db);
            std::cout << "Them User thanh cong (ID: " << last_user_id << ").\n";
        }
    } else {
        std::cerr << "Loi chuan bi insert User: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }
    sqlite3_finalize(stmt_insert_user);


    // 7. Insert Wallet (Dùng biến thành viên 'db')
    // ... (Code prepare, bind, step, finalize cho insert wallet như cũ, dùng biến 'db') ...
     if (success && last_user_id != -1) {
        const char* sql_insert_wallet = "INSERT INTO Wallets (wallet_id, user_id, balance) VALUES (?, ?, ?);";
        rc = sqlite3_prepare_v2(db, sql_insert_wallet, -1, &stmt_insert_wallet, nullptr);
        if (rc == SQLITE_OK) {
            long long initialBalance = 0;
            // Use the generated walletId here
            sqlite3_bind_text(stmt_insert_wallet, 1, walletId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt_insert_wallet, 2, last_user_id);
            sqlite3_bind_int64(stmt_insert_wallet, 3, initialBalance);

            if (sqlite3_step(stmt_insert_wallet) != SQLITE_DONE) {
                std::cerr << "Loi insert Wallet: " << sqlite3_errmsg(db) << std::endl;
                success = false;
            } else {
                 std::cout << "Them Wallet thanh cong (ID: " << walletId << ").\n";
            }
        } else {
             std::cerr << "Loi chuan bi insert Wallet: " << sqlite3_errmsg(db) << std::endl;
             success = false;
        }
        sqlite3_finalize(stmt_insert_wallet);
    } else if (last_user_id == -1 && success) { // Trường hợp insert user thành công nhưng không lấy được ID
         std::cerr << "Loi: Khong lay duoc User ID sau khi insert.\n";
         success = false;
    }
     else {
        success = false;
    }


    // 8. Commit hoặc Rollback Transaction (Dùng biến thành viên 'db')
    // ... (Code commit/rollback như cũ, dùng biến 'db') ...
     if (success) {
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Loi commit transaction: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            success = false;
        } else {
            std::cout << "Transaction committed.\n";
        }
    } else {
        std::cerr << "Gap loi, dang rollback transaction...\n";
        rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
         if (rc != SQLITE_OK) {
            std::cerr << "Loi rollback transaction: " << errMsg << std::endl;
            sqlite3_free(errMsg);
         } else {
             std::cout << "Transaction rolled back.\n";
         }
    }

    // --- Bỏ phần đóng CSDL ở đây ---

    // 10. Thông báo kết quả
    if (success) {
        std::cout << "\n*** Dang ky tai khoan thanh cong! ***\n";
        return true;
    } else {
        std::cout << "\n*** Dang ky tai khoan THAT BAI! ***\n";
        return false;
    }
}

User* AuthWalletSystem::getCurrentUser() const {
    return currentUser.get(); // Return the raw pointer from the unique_ptr
}

long long AuthWalletSystem::getWalletBalance(const std::string& walletId) {
    if (!db || walletId.empty()) {
        std::cerr << "Error: Database not connected or invalid wallet ID." << std::endl;
        return -1; // Indicate error or not found
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT balance FROM Wallets WHERE wallet_id = ?;";
    long long balance = -1; // Default to error/not found

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    // Bind the walletId parameter
    if (sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind wallet ID: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    // Execute the statement and retrieve the balance
    int stepResult = sqlite3_step(stmt);
    if (stepResult == SQLITE_ROW) {
        balance = sqlite3_column_int64(stmt, 0); // Get balance from the first column
    } else if (stepResult == SQLITE_DONE) {
        // No row found, wallet ID likely doesn't exist (or balance is NULL?)
        // Keep balance as -1
         std::cerr << "Wallet ID not found: " << walletId << std::endl;
    } else {
        std::cerr << "Failed to step statement: " << sqlite3_errmsg(db) << std::endl;
        // Keep balance as -1
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);

    return balance;
}

// Function to execute simple SQL commands like BEGIN, COMMIT, ROLLBACK
bool executeSimpleSQL(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error (" << sql << "): " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}


bool AuthWalletSystem::transferPoints(const std::string& senderWalletId, const std::string& recipientWalletId, long long amount) {
    if (!db) {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }
    if (senderWalletId == recipientWalletId) {
        std::cerr << "Error: Sender and recipient wallet IDs cannot be the same." << std::endl;
        return false;
    }
    if (amount <= 0) {
        std::cerr << "Error: Transfer amount must be positive." << std::endl;
        return false;
    }

    // --- Begin Transaction ---
    if (!executeSimpleSQL(db, "BEGIN TRANSACTION;")) {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    bool success = false; // Assume failure initially

    // 1. Check sender balance
    long long senderBalance = getWalletBalance(senderWalletId);
    if (senderBalance < amount) {
        std::cerr << "Error: Insufficient balance. Available: " << senderBalance << ", Required: " << amount << std::endl;
        executeSimpleSQL(db, "ROLLBACK;");
        // Log failed transaction (insufficient balance)
        logTransaction(senderWalletId, recipientWalletId, amount, "FAILED_BALANCE");
        return false;
    }

    // 2. Check if recipient wallet exists
    long long recipientBalanceCheck = getWalletBalance(recipientWalletId);
    if (recipientBalanceCheck < 0) {
        std::cerr << "Error: Recipient wallet ID '" << recipientWalletId << "' not found or error checking balance." << std::endl;
        executeSimpleSQL(db, "ROLLBACK;");
        // Log failed transaction (recipient wallet not found)
        logTransaction(senderWalletId, recipientWalletId, amount, "FAILED_WALLET_NOT_FOUND");
        return false;
    }

    // 3. Debit sender
    const char* debitSql = "UPDATE Wallets SET balance = balance - ? WHERE wallet_id = ?;";
    if (sqlite3_prepare_v2(db, debitSql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare debit statement: " << sqlite3_errmsg(db) << std::endl;
        executeSimpleSQL(db, "ROLLBACK;");
        return false;
    }
    sqlite3_bind_int64(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, senderWalletId.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute debit: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        executeSimpleSQL(db, "ROLLBACK;");
        // Log failed transaction (debit failed)
        logTransaction(senderWalletId, recipientWalletId, amount, "FAILED_BALANCE");
        return false;
    }
    sqlite3_finalize(stmt);

    // 4. Credit recipient
    const char* creditSql = "UPDATE Wallets SET balance = balance + ? WHERE wallet_id = ?;";
    if (sqlite3_prepare_v2(db, creditSql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare credit statement: " << sqlite3_errmsg(db) << std::endl;
        executeSimpleSQL(db, "ROLLBACK;");
        // Log failed transaction (internal error)
        logTransaction(senderWalletId, recipientWalletId, amount, "FAILED_WALLET_NOT_FOUND");
        return false;
    }
    sqlite3_bind_int64(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, recipientWalletId.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute credit: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        executeSimpleSQL(db, "ROLLBACK;");
        // Log failed transaction (credit failed)
        logTransaction(senderWalletId, recipientWalletId, amount, "FAILED_BALANCE");
        return false;
    }
    sqlite3_finalize(stmt);

    // --- Log the successful transaction ---
    bool logSuccess = logTransaction(senderWalletId, recipientWalletId, amount, "COMPLETED");
    if (!logSuccess) {
        std::cerr << "CRITICAL ERROR: Transaction completed but failed to log! Manual intervention required." << std::endl;
        // For now, proceed with commit.
    }

    // --- Commit Transaction ---
    if (executeSimpleSQL(db, "COMMIT;")) {
        success = true;
    } else {
        executeSimpleSQL(db, "ROLLBACK;");
    }

    return success;
}

bool AuthWalletSystem::logTransaction(const std::string& fromWallet, const std::string& toWallet, long long amount, const std::string& status) {
    if (!db) return false;

    const char* sql = R"(
        INSERT INTO Transactions (from_wallet_id, to_wallet_id, amount, status)
        VALUES (?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare logTransaction: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    if (!fromWallet.empty())
        sqlite3_bind_text(stmt, 1, fromWallet.c_str(), -1, SQLITE_STATIC);
    else
        sqlite3_bind_null(stmt, 1);
    sqlite3_bind_text(stmt, 2, toWallet.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, amount);
    sqlite3_bind_text(stmt, 4, status.c_str(), -1, SQLITE_STATIC);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok) {
        std::cerr << "Failed to log transaction: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return ok;
}

#include "AuthWalletSystem.h"
#include <sqlite3.h>
#include <vector>
#include <string>
#include <iostream>

std::vector<TransactionRecord> AuthWalletSystem::getTransactionHistory(const std::string& walletId) {
    std::vector<TransactionRecord> records;
    if (!db) {
        std::cerr << "Database not connected.\n";
        return records;
    }

    const char* sql =
        "SELECT transaction_id, from_wallet_id, to_wallet_id, amount, status, transaction_timestamp "
        "FROM Transactions "
        "WHERE from_wallet_id = ?1 OR to_wallet_id = ?1 "
        "ORDER BY transaction_timestamp DESC";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return records;
    }

    sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int transactionId = sqlite3_column_int(stmt, 0);

        const unsigned char* fromWalPtr = sqlite3_column_text(stmt, 1);
        const unsigned char* toWalPtr = sqlite3_column_text(stmt, 2);

        std::string fromWalletId = fromWalPtr ? reinterpret_cast<const char*>(fromWalPtr) : "";
        std::string toWalletId = toWalPtr ? reinterpret_cast<const char*>(toWalPtr) : "";

        long long amount = sqlite3_column_int64(stmt, 3);

        const unsigned char* statusPtr = sqlite3_column_text(stmt, 4);
        std::string status = statusPtr ? reinterpret_cast<const char*>(statusPtr) : "";

        const unsigned char* tsPtr = sqlite3_column_text(stmt, 5);
        std::string timestamp = tsPtr ? reinterpret_cast<const char*>(tsPtr) : "";

        records.emplace_back(transactionId, fromWalletId, toWalletId, amount, status, timestamp);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Error while reading transaction history: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return records;
}

// Implementation of the password change method
bool AuthWalletSystem::changePassword(const User& user, const std::string& oldPwd, const std::string& newPwd) {
    if (!db) {
        std::cerr << "Database connection not established!" << std::endl;
        return false;
    }

    // In case of forced change (AUTO_GENERATED), we don't need to verify oldPwd
    bool isAutoGenerated = user.getPasswordStatus() == PasswordStatus::AUTO_GENERATED;
    
    // For regular password change, verify that the old password matches
    if (!isAutoGenerated && !user.checkPassword(oldPwd)) {
        std::cerr << "Current password verification failed." << std::endl;
        return false;
    }

    // For auto-generated passwords, ensure the new password is different
    if (isAutoGenerated && user.checkPassword(newPwd)) {
        std::cerr << "New password cannot be the same as the auto-generated password." << std::endl;
        return false;
    }

    // Generate new salt and hash for the new password
    std::string newSalt = PasswordUtils::generateSalt();
    std::string newHashedPassword = PasswordUtils::hashPassword(newPwd, newSalt);
    
    // Update the database
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Users SET hashed_password = ?, salt = ?, password_status = ? WHERE user_id = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare password update statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, newHashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, newSalt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, "USER_SET", -1, SQLITE_STATIC); // Always set to USER_SET after change
    sqlite3_bind_int(stmt, 4, user.getUserId());
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        success = true;
        
        // If this is the current user, update the currentUser object too
        if (currentUser && currentUser->getUserId() == user.getUserId()) {
            currentUser->setHashedPassword(newHashedPassword);
            currentUser->setSalt(newSalt);
            currentUser->setPasswordStatus(PasswordStatus::USER_SET);
            
            // Log the password change
            std::cout << "Password for user " << currentUser->getUsername() 
                      << " changed " << (isAutoGenerated ? "(from auto-generated)" : "")
                      << " at " << std::time(nullptr) << std::endl;
        }
    } else {
        std::cerr << "Failed to update password: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

// Implementation of adminCreateUser for UC-AUTH-02
bool AuthWalletSystem::adminCreateUser(const std::string& username, const std::string& fullName,
                                     const std::string& contactInfo, bool generateRandomPassword,
                                     std::string& password) {
    if (!db) {
        std::cerr << "Database connection not established!" << std::endl;
        return false;
    }
    
    // Check if current user is an admin
    if (!currentUser || !currentUser->isAdmin()) {
        std::cerr << "Error: Only administrators can create user accounts." << std::endl;
        return false;
    }
    
    // Check if username already exists
    sqlite3_stmt* stmt_check;
    const char* sql_check = "SELECT COUNT(*) FROM Users WHERE username = ?;";
    int rc = sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing username check query: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt_check, 1, username.c_str(), -1, SQLITE_STATIC);
    int user_count = 0;
    if (sqlite3_step(stmt_check) == SQLITE_ROW) {
        user_count = sqlite3_column_int(stmt_check, 0);
    } else {
        std::cerr << "Error executing username check query: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt_check);
        return false;
    }
    sqlite3_finalize(stmt_check);
    
    if (user_count > 0) {
        std::cerr << "Error: Username '" << username << "' already exists!" << std::endl;
        return false;
    }
    
    // Generate random password if requested
    std::string salt;
    std::string hashedPassword;
    std::string passwordStatus;
    
    if (generateRandomPassword) {
        // Generate a random password (8 characters)
        const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, chars.size() - 1);
        
        password.clear();
        for (int i = 0; i < 8; ++i) {
            password += chars[distribution(generator)];
        }
        
        passwordStatus = "AUTO_GENERATED";
    } else {
        // Use the provided password
        passwordStatus = "USER_SET";
    }
    
    // Generate salt and hash the password
    salt = PasswordUtils::generateSalt();
    hashedPassword = PasswordUtils::hashPassword(password, salt);
    
    // Generate a wallet ID
    std::string walletId = generateUniqueWalletId();
    if (walletId.empty()) {
        std::cerr << "Error: Could not generate unique wallet ID!" << std::endl;
        return false;
    }
    
    // Begin transaction
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error starting transaction: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    // Insert user
    sqlite3_stmt* stmt_insert_user = nullptr;
    const char* sql_insert_user = "INSERT INTO Users (username, hashed_password, salt, full_name, contact_info, role, password_status) VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    rc = sqlite3_prepare_v2(db, sql_insert_user, -1, &stmt_insert_user, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing user insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    
    sqlite3_bind_text(stmt_insert_user, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_user, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_user, 3, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_user, 4, fullName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_user, 5, contactInfo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_user, 6, "USER", -1, SQLITE_STATIC); // Always creating USER role
    sqlite3_bind_text(stmt_insert_user, 7, passwordStatus.c_str(), -1, SQLITE_STATIC);
    
    bool success = true;
    long long last_user_id = -1;
    
    if (sqlite3_step(stmt_insert_user) != SQLITE_DONE) {
        std::cerr << "Error inserting user: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    } else {
        last_user_id = sqlite3_last_insert_rowid(db);
        std::cout << "User created successfully (ID: " << last_user_id << ")." << std::endl;
    }
    sqlite3_finalize(stmt_insert_user);
    
    // Insert wallet if user creation was successful
    if (success && last_user_id != -1) {
        sqlite3_stmt* stmt_insert_wallet = nullptr;
        const char* sql_insert_wallet = "INSERT INTO Wallets (wallet_id, user_id, balance) VALUES (?, ?, ?);";
        
        rc = sqlite3_prepare_v2(db, sql_insert_wallet, -1, &stmt_insert_wallet, nullptr);
        if (rc == SQLITE_OK) {
            long long initialBalance = 0; // Start with zero balance
            
            sqlite3_bind_text(stmt_insert_wallet, 1, walletId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt_insert_wallet, 2, last_user_id);
            sqlite3_bind_int64(stmt_insert_wallet, 3, initialBalance);
            
            if (sqlite3_step(stmt_insert_wallet) != SQLITE_DONE) {
                std::cerr << "Error inserting wallet: " << sqlite3_errmsg(db) << std::endl;
                success = false;
            } else {
                std::cout << "Wallet created successfully (ID: " << walletId << ")." << std::endl;
            }
        } else {
            std::cerr << "Error preparing wallet insert statement: " << sqlite3_errmsg(db) << std::endl;
            success = false;
        }
        sqlite3_finalize(stmt_insert_wallet);
    }
    
    // Commit or rollback transaction
    if (success) {
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Error committing transaction: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            success = false;
        }
    } else {
        std::cerr << "Error encountered, rolling back transaction..." << std::endl;
        rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Error rolling back transaction: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
    
    return success;
}

