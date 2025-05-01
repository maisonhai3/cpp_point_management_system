#ifndef AUTH_WALLET_SYSTEM_H
#define AUTH_WALLET_SYSTEM_H

#include "models/User.h"
#include "models/Wallet.h"
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
    std::string generateUniqueWalletId(); // <-- Declaration moved here

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
    std::vector<User> adminGetAllUsers();

    // Getters
    User* getCurrentUser() const;
};

#endif // AUTH_WALLET_SYSTEM_H