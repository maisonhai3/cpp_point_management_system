#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h> // Header chính của SQLite
#include <functional> // Cho std::hash (ví dụ hashing đơn giản)
#include <random>    // Cho tạo salt, wallet id
#include <chrono>    // Cho tạo wallet id
#include <algorithm>

// --- Giả định có các hàm trợ giúp này (cần implement thực tế) ---
std::string generateSalt(size_t length = 16) {
    // Implement tạo chuỗi ngẫu nhiên an toàn
    // Ví dụ đơn giản (KHÔNG DÙNG CHO PRODUCTION):
    std::string s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(s.begin(), s.end(), generator);
    return s.substr(0, length);
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    // Implement hàm băm an toàn (vd: Argon2, bcrypt, hoặc ít nhất là SHA256)
    // Ví dụ dùng std::hash (RẤT KHÔNG AN TOÀN - CHỈ ĐỂ MINH HỌA):
    std::hash<std::string> hasher;
    size_t hashed = hasher(password + salt);
    return std::to_string(hashed);
}

std::string generateUniqueWalletId() {
    // Implement tạo ID duy nhất (ví dụ: "W_" + timestamp + random)
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distrib(1000, 9999);
    return "W_" + std::to_string(now_ms) + std::to_string(distrib(generator));
}

// --- Hàm thực hiện Use Case UC-AUTH-01 ---

bool registerUser() {
    std::string username, password, confirmPassword, fullName, contactInfo;

    // 1. Nhận Input từ End-user
    std::cout << "--- Dang Ky Tai Khoan Moi ---\n";
    std::cout << "Nhap ten dang nhap: ";
    std::getline(std::cin >> std::ws, username);
    // Thêm kiểm tra username hợp lệ (không trống, không chứa ký tự đặc biệt...)

    std::cout << "Nhap mat khau: ";
    std::getline(std::cin >> std::ws, password);
    // Thêm kiểm tra độ mạnh mật khẩu

    std::cout << "Xac nhan mat khau: ";
    std::getline(std::cin >> std::ws, confirmPassword);
    if (password != confirmPassword) {
        std::cerr << "Loi: Mat khau xac nhan khong khop!\n";
        return false;
    }

    std::cout << "Nhap ho va ten: ";
    std::getline(std::cin >> std::ws, fullName);

    std::cout << "Nhap Email hoac SĐT: ";
    std::getline(std::cin >> std::ws, contactInfo);

    // 2. Kết nối CSDL SQLite
    sqlite3* db;
    int rc = sqlite3_open("wallet_app.db", &db); // Mở hoặc tạo file DB
    if (rc != SQLITE_OK) {
        std::cerr << "Loi khong the mo CSDL: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db); // Dù lỗi cũng nên thử đóng
        return false;
    }
    std::cout << "Mo CSDL thanh cong.\n";

    // Bật kiểm tra khóa ngoại (QUAN TRỌNG!)
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Loi bat khoa ngoai: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }


    // 3. Kiểm tra Username tồn tại
    sqlite3_stmt* stmt_check;
    const char* sql_check = "SELECT COUNT(*) FROM Users WHERE username = ?;";
    rc = sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Loi chuan bi truy van kiem tra username: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    // Gán giá trị username vào placeholder '?' (index 1)
    sqlite3_bind_text(stmt_check, 1, username.c_str(), -1, SQLITE_STATIC);

    int user_count = 0;
    if (sqlite3_step(stmt_check) == SQLITE_ROW) { // Lấy kết quả
        user_count = sqlite3_column_int(stmt_check, 0);
    } else {
         std::cerr << "Loi thuc thi truy van kiem tra username: " << sqlite3_errmsg(db) << std::endl;
         sqlite3_finalize(stmt_check);
         sqlite3_close(db);
         return false;
    }
    sqlite3_finalize(stmt_check); // Giải phóng statement

    if (user_count > 0) {
        std::cerr << "Loi: Ten dang nhap '" << username << "' da ton tai!\n";
        sqlite3_close(db);
        return false;
    }

    // 4. Chuẩn bị Dữ liệu User và Wallet
    std::string salt = generateSalt();
    std::string hashedPassword = hashPassword(password, salt);
    std::string walletId = generateUniqueWalletId();
    std::string userRole = "USER"; // Mặc định
    std::string passStatus = "USER_SET"; // Người dùng tự đặt

    // 5. Bắt đầu Transaction
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Loi bat dau transaction: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_stmt* stmt_insert_user = nullptr;
    sqlite3_stmt* stmt_insert_wallet = nullptr;
    bool success = true;
    long long last_user_id = -1; // Lưu user_id vừa insert

    // 6. Insert User
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
            last_user_id = sqlite3_last_insert_rowid(db); // Lấy ID của user vừa insert
            std::cout << "Them User thanh cong (ID: " << last_user_id << ").\n";
        }
    } else {
        std::cerr << "Loi chuan bi insert User: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }
    sqlite3_finalize(stmt_insert_user); // Luôn giải phóng statement

    // 7. Insert Wallet (chỉ thực hiện nếu Insert User thành công)
    if (success && last_user_id != -1) {
        const char* sql_insert_wallet = "INSERT INTO Wallets (wallet_id, user_id, balance) VALUES (?, ?, ?);";
        rc = sqlite3_prepare_v2(db, sql_insert_wallet, -1, &stmt_insert_wallet, nullptr);
        if (rc == SQLITE_OK) {
            long long initialBalance = 0;
            sqlite3_bind_text(stmt_insert_wallet, 1, walletId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt_insert_wallet, 2, last_user_id); // Dùng user_id vừa lấy
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
    } else {
        success = false; // Đảm bảo không commit nếu User insert lỗi hoặc không lấy được ID
    }

    // 8. Commit hoặc Rollback Transaction
    if (success) {
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Loi commit transaction: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            // Dù commit lỗi thì dữ liệu cũng có thể đã bị rollback bởi CSDL
            success = false; // Đánh dấu là thất bại
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

    // 9. Đóng kết nối CSDL
    sqlite3_close(db);
    std::cout << "Dong CSDL.\n";

    // 10. Thông báo kết quả
    if (success) {
        std::cout << "\n*** Dang ky tai khoan thanh cong! ***\n";
        return true;
    } else {
        std::cout << "\n*** Dang ky tai khoan THAT BAI! ***\n";
        return false;
    }
}