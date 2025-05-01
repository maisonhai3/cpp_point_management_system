-- Bảng lưu thông tin người dùng
CREATE TABLE Users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(100) UNIQUE NOT NULL,
    hashed_password VARCHAR(255) NOT NULL,
    salt VARCHAR(100) NOT NULL,
    full_name VARCHAR(255),
    contact_info VARCHAR(255),
    role VARCHAR(20) NOT NULL DEFAULT 'USER'
        CHECK (role IN ('USER', 'ADMIN')),
    password_status VARCHAR(20) NOT NULL DEFAULT 'USER_SET'
        CHECK (password_status IN ('USER_SET', 'AUTO_GENERATED')),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Bảng lưu thông tin ví điểm
CREATE TABLE Wallets (
    wallet_id VARCHAR(100) PRIMARY KEY, -- Nên sinh ngẫu nhiên (UUID hoặc tương tự) khi tạo ví mới
    user_id INT UNIQUE NOT NULL,
    balance BIGINT NOT NULL DEFAULT 0
        CHECK (balance >= 0),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
);

-- Bảng lưu lịch sử giao dịch ví
CREATE TABLE Transactions (
    transaction_id BIGSERIAL PRIMARY KEY,
    from_wallet_id VARCHAR(100),
    to_wallet_id VARCHAR(100) NOT NULL,
    amount BIGINT NOT NULL
        CHECK (amount > 0),
    status VARCHAR(30) NOT NULL
        CHECK (status IN ('PENDING', 'COMPLETED', 'FAILED_OTP', 'FAILED_BALANCE', 'FAILED_WALLET_NOT_FOUND', 'CANCELLED')),
    transaction_timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (from_wallet_id) REFERENCES Wallets(wallet_id) ON DELETE SET NULL,
    FOREIGN KEY (to_wallet_id) REFERENCES Wallets(wallet_id) ON DELETE CASCADE
);

-- Bảng lưu trữ OTP tạm thời
CREATE TABLE OtpStore (
    otp_id SERIAL PRIMARY KEY,
    identifier VARCHAR(255) NOT NULL,
    otp_code VARCHAR(10) NOT NULL,
    action_type VARCHAR(50),
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    is_verified BOOLEAN NOT NULL DEFAULT FALSE
);

-- === Chỉ mục (Indexes) để tăng tốc độ truy vấn ===
CREATE INDEX idx_users_username ON Users(username);
CREATE INDEX idx_wallets_user_id ON Wallets(user_id);
CREATE INDEX idx_transactions_from_wallet ON Transactions(from_wallet_id);
CREATE INDEX idx_transactions_to_wallet ON Transactions(to_wallet_id);
CREATE INDEX idx_transactions_timestamp ON Transactions(transaction_timestamp);
CREATE INDEX idx_otp_identifier ON OtpStore(identifier);
CREATE INDEX idx_otp_expires_at ON OtpStore(expires_at);

-- Trigger để tự động cập nhật updated_at (PostgreSQL)
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_users_modtime BEFORE UPDATE ON Users FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
CREATE TRIGGER update_wallets_modtime BEFORE UPDATE ON Wallets FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
