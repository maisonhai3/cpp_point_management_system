#ifndef AUTH_WALLET_SYSTEM_H
#define AUTH_WALLET_SYSTEM_H

#include "models/User.h"
#include "models/Wallet.h"
#include "models/TransactionRecord.h" // <-- Include the new header
#include <memory>
#include <vector>
#include <optional>
#include <sqlite3.h> // Include sqlite3.h

class AuthWalletSystem {
private:
    std::unique_ptr<User> currentUser;
    sqlite3* db = nullptr; // Database handle

    // Internal utility methods
    bool createUserInDB(const User& user, const std::string& hashedPassword, const std::string& salt);
    bool createWalletForUser(int userId, const std::string& walletId);
    bool usernameExists(const std::string& username);
    std::string generateUniqueWalletId(); 
    bool logTransaction(const std::string& fromWallet, const std::string& toWallet, long long amount, const std::string& status); // <-- Added declaration for logging

public:
    AuthWalletSystem();
    ~AuthWalletSystem();

    bool isDbConnected() const;

    // User registration and authentication
    bool registerUser(const std::string& username, const std::string& password,
                     const std::string& fullName, const std::string& contactInfo);
    bool login(const std::string& username, const std::string& password);
    void logout();

    // User information
    std::optional<User> getUserInfo(const std::string& username);
    bool updateUserInfo(const User& user, const std::string& newName, const std::string& newContact);
    bool changePassword(const User& user, const std::string& oldPwd, const std::string& newPwd);

    // Wallet operations
    long long getWalletBalance(const std::string& walletId);
    bool transferPoints(const std::string& senderWalletId, const std::string& recipientWalletId, long long amount); 
    std::vector<TransactionRecord> getTransactionHistory(const std::string& walletId); // <-- Add history declaration
    std::vector<User> adminGetAllUsers();

    // Getters
    User* getCurrentUser() const;
    
    // Check if password change is required
    bool isPasswordChangeRequired() const {
        return currentUser && currentUser->getPasswordStatus() == PasswordStatus::AUTO_GENERATED;
    }
};

#endif // AUTH_WALLET_SYSTEM_H