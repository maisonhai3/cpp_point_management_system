#include "AuthWalletSystem.h"
#include "utils/PasswordUtils.h"
#include "utils/UUIDGenerator.h"
#include <iostream>
#include <sstream>

AuthWalletSystem::AuthWalletSystem() : dbConnection(std::make_unique<DBConnection>()) {}

AuthWalletSystem::~AuthWalletSystem() {
    logout();
}

bool AuthWalletSystem::initializeDB(const std::string& connString) {
    return dbConnection->connect(connString);
}

bool AuthWalletSystem::registerUser(const std::string& username, const std::string& password, 
                                    const std::string& fullName, const std::string& contactInfo) {
    // Check if username already exists
    if (usernameExists(username)) {
        std::cout << "Username already exists!" << std::endl;
        return false;
    }
    
    // Create user object
    User user(username, fullName, contactInfo);
    
    // Hash and salt the password
    auto [hashedPassword, salt] = PasswordUtils::hashAndSalt(password);
    
    // Begin a transaction for atomicity
    if (!dbConnection->beginTransaction()) {
        std::cout << "Failed to begin transaction: " << dbConnection->getLastError() << std::endl;
        return false;
    }
    
    // Create the user in the database
    if (!createUserInDB(user, hashedPassword, salt)) {
        dbConnection->rollbackTransaction();
        return false;
    }
    
    // Get the user's ID from the database
    std::stringstream ss;
    ss << "SELECT user_id FROM Users WHERE username = '" << username << "'";
    auto result = dbConnection->executeQuery(ss.str());
    
    if (!result || result->getRowCount() == 0) {
        std::cout << "Failed to retrieve user ID: " << dbConnection->getLastError() << std::endl;
        dbConnection->rollbackTransaction();
        return false;
    }
    
    int userId = std::stoi(result->getValue(0, 0));
    
    // Generate a wallet ID and create a wallet for the user
    std::string walletId = UUIDGenerator::generateUUID();
    if (!createWalletForUser(userId, walletId)) {
        dbConnection->rollbackTransaction();
        return false;
    }
    
    // Commit the transaction
    if (!dbConnection->commitTransaction()) {
        std::cout << "Failed to commit transaction: " << dbConnection->getLastError() << std::endl;
        dbConnection->rollbackTransaction();
        return false;
    }
    
    std::cout << "User registered successfully!" << std::endl;
    return true;
}

bool AuthWalletSystem::createUserInDB(const User& user, const std::string& hashedPassword, const std::string& salt) {
    std::stringstream ss;
    ss << "INSERT INTO Users (username, hashed_password, salt, full_name, contact_info, role, password_status) VALUES ("
       << "'" << user.getUsername() << "', "
       << "'" << hashedPassword << "', "
       << "'" << salt << "', "
       << "'" << user.getFullName() << "', "
       << "'" << user.getContactInfo() << "', "
       << "'USER', 'USER_SET')";
    
    int affectedRows = 0;
    if (!dbConnection->executeUpdate(ss.str(), affectedRows) || affectedRows != 1) {
        std::cout << "Failed to create user: " << dbConnection->getLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool AuthWalletSystem::createWalletForUser(int userId, const std::string& walletId) {
    std::stringstream ss;
    ss << "INSERT INTO Wallets (wallet_id, user_id, balance) VALUES ("
       << "'" << walletId << "', "
       << userId << ", "
       << "0)";
    
    int affectedRows = 0;
    if (!dbConnection->executeUpdate(ss.str(), affectedRows) || affectedRows != 1) {
        std::cout << "Failed to create wallet: " << dbConnection->getLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool AuthWalletSystem::usernameExists(const std::string& username) {
    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM Users WHERE username = '" << username << "'";
    
    auto result = dbConnection->executeQuery(ss.str());
    if (!result || result->getRowCount() == 0) {
        return false;
    }
    
    int count = std::stoi(result->getValue(0, 0));
    return count > 0;
}

bool AuthWalletSystem::login(const std::string& username, const std::string& password) {
    // Placeholder for actual implementation
    return false;
}

void AuthWalletSystem::logout() {
    currentUser.reset();
}

std::optional<User> AuthWalletSystem::getUserInfo(const std::string& username) {
    // Placeholder for actual implementation
    return std::nullopt;
}

bool AuthWalletSystem::updateUserInfo(const User& user, const std::string& newName, const std::string& newContact) {
    // Placeholder for actual implementation
    return false;
}

bool AuthWalletSystem::changePassword(const User& user, const std::string& oldPwd, const std::string& newPwd) {
    // Placeholder for actual implementation
    return false;
}

long long AuthWalletSystem::getWalletBalance(const std::string& walletId) {
    // Placeholder for actual implementation
    return 0;
}

std::vector<User> AuthWalletSystem::adminGetAllUsers() {
    // Placeholder for actual implementation
    return {};
}

User* AuthWalletSystem::getCurrentUser() const {
    return currentUser.get();
}
