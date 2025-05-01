#include "AuthWalletSystem.h" // Include header của lớp
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <chrono>
#include <algorithm>
#include <sqlite3.h> // Make sure this is included (likely via AuthWalletSystem.h)
#include "../include/utils/PasswordUtils.h"
#include "../include/utils/UUIDGenerator.h" // <-- Add include for UUID generation

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

    // Mở kết nối CSDL SQLite
    // Thay "wallet_app.db" bằng đường dẫn thực tế nếu cần
    int rc = sqlite3_open("wallet_app.db", &db);

    if (rc != SQLITE_OK) {
        // Nếu mở lỗi, db sẽ là nullptr hoặc chứa thông tin lỗi
        std::cerr << "ERROR: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        // db có thể vẫn cần được close nếu lỗi xảy ra sau khi cấp phát một phần
        if (db) {
           sqlite3_close(db); // Cố gắng đóng
           db = nullptr;       // Đặt lại là nullptr
        }
        // Bạn có thể muốn ném một ngoại lệ ở đây để báo hiệu lỗi khởi tạo nghiêm trọng
        // throw std::runtime_error("Failed to open database");
    } else {
        std::cout << "Database opened successfully: wallet_app.db" << std::endl;

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