-- Bảng lưu thông tin người dùng
CREATE TABLE Users (
                       user_id INTEGER PRIMARY KEY AUTOINCREMENT,     -- Khóa chính tự tăng kiểu SQLite
                       username VARCHAR(100) UNIQUE NOT NULL,         -- Tên đăng nhập, duy nhất, không rỗng
                       hashed_password VARCHAR(255) NOT NULL,        -- Mật khẩu đã được băm
                       salt VARCHAR(100) NOT NULL,                   -- Chuỗi salt cho việc băm mật khẩu
                       full_name VARCHAR(255),                       -- Họ và tên
                       contact_info VARCHAR(255),                    -- Email hoặc SĐT
                       role VARCHAR(20) NOT NULL DEFAULT 'USER'      -- Vai trò ('USER' hoặc 'ADMIN')
                           CHECK (role IN ('USER', 'ADMIN')),
                       password_status VARCHAR(20) NOT NULL DEFAULT 'USER_SET' -- Trạng thái MK ('USER_SET', 'AUTO_GENERATED')
                           CHECK (password_status IN ('USER_SET', 'AUTO_GENERATED')),
                       created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Thời gian tạo tài khoản
                       updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP  -- Thời gian cập nhật lần cuối (sẽ được cập nhật bởi trigger)
);

-- Bảng lưu thông tin ví điểm
CREATE TABLE Wallets (
                         wallet_id VARCHAR(100) PRIMARY KEY,          -- Mã ví duy nhất (do ứng dụng tạo), khóa chính
                         user_id INTEGER UNIQUE NOT NULL,             -- Khóa ngoại liên kết tới Users, đảm bảo 1 User chỉ có 1 Wallet
                         balance INTEGER NOT NULL DEFAULT 0           -- Số dư điểm (dùng INTEGER của SQLite, tương đương BIGINT)
                             CHECK (balance >= 0),                    -- Ràng buộc số dư không âm
                         created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
                         updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Sẽ được cập nhật bởi trigger
                         FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE -- Nếu xóa User thì xóa cả Wallet liên quan
);

-- Bảng lưu lịch sử giao dịch ví
CREATE TABLE Transactions (
                              transaction_id INTEGER PRIMARY KEY AUTOINCREMENT, -- Khóa chính tự tăng kiểu SQLite
                              from_wallet_id VARCHAR(100),                 -- Mã ví gửi (NULL nếu từ hệ thống/MASTER)
                              to_wallet_id VARCHAR(100) NOT NULL,          -- Mã ví nhận, không rỗng
                              amount INTEGER NOT NULL                      -- Số điểm giao dịch (dùng INTEGER của SQLite)
                                  CHECK (amount > 0),                      -- Ràng buộc số tiền phải dương
                              status VARCHAR(30) NOT NULL
                              CHECK (status IN ('PENDING', 'COMPLETED', 'FAILED_OTP', 'FAILED_BALANCE', 'FAILED_WALLET_NOT_FOUND', 'CANCELLED')),
                              transaction_timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Thời điểm giao dịch
                              FOREIGN KEY (from_wallet_id) REFERENCES Wallets(wallet_id) ON DELETE SET NULL, -- Nếu ví gửi bị xóa, đặt FK thành NULL
                              FOREIGN KEY (to_wallet_id) REFERENCES Wallets(wallet_id) ON DELETE CASCADE   -- Nếu ví nhận bị xóa, xóa cả giao dịch liên quan
);

-- Bảng lưu trữ OTP tạm thời (Tùy chọn)
CREATE TABLE OtpStore (
                          otp_id INTEGER PRIMARY KEY AUTOINCREMENT,    -- Khóa chính tự tăng kiểu SQLite
                          identifier VARCHAR(255) NOT NULL,            -- Username hoặc định danh khác liên quan
                          otp_code VARCHAR(10) NOT NULL,               -- Mã OTP đã tạo
                          action_type VARCHAR(50),                     -- Loại hành động (CHANGE_PASSWORD, TRANSFER, UPDATE_INFO)
                          expires_at TIMESTAMP NOT NULL,               -- Thời gian hết hạn
                          created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
                          is_verified INTEGER NOT NULL DEFAULT 0       -- Đã được xác thực hay chưa (0 = false, 1 = true)
);

-- === Chỉ mục (Indexes) để tăng tốc độ truy vấn ===
-- (Syntax CREATE INDEX tương tự PostgreSQL và hoạt động tốt trong SQLite)
CREATE INDEX idx_users_username ON Users(username);
CREATE INDEX idx_wallets_user_id ON Wallets(user_id);
CREATE INDEX idx_transactions_from_wallet ON Transactions(from_wallet_id);
CREATE INDEX idx_transactions_to_wallet ON Transactions(to_wallet_id);
CREATE INDEX idx_transactions_timestamp ON Transactions(transaction_timestamp);
CREATE INDEX idx_otp_identifier ON OtpStore(identifier);
CREATE INDEX idx_otp_expires_at ON OtpStore(expires_at);

-- === Trigger để tự động cập nhật updated_at (Syntax cho SQLite) ===
-- Trigger cho bảng Users
CREATE TRIGGER update_users_updated_at AFTER UPDATE ON Users
    FOR EACH ROW
BEGIN
    UPDATE Users SET updated_at = CURRENT_TIMESTAMP WHERE user_id = OLD.user_id;
END;

-- Trigger cho bảng Wallets
CREATE TRIGGER update_wallets_updated_at AFTER UPDATE ON Wallets
    FOR EACH ROW
BEGIN
    UPDATE Wallets SET updated_at = CURRENT_TIMESTAMP WHERE wallet_id = OLD.wallet_id;
END;