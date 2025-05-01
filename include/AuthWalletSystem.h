#ifndef AUTH_WALLET_SYSTEM_H
#define AUTH_WALLET_SYSTEM_H

#include "database/DBConnection.h"
#include "models/User.h"
#include "models/Wallet.h"
#include <memory>
#include <vector>
#include <optional>

class AuthWalletSystem {
private:
    std::unique_ptr<User> currentUser;
    std::unique_ptr<DBConnection> dbConnection;
    
    // Internal utility methods
    bool createUserInDB(const User& user, const std::string& hashedPassword, const std::string& salt);
    bool createWalletForUser(int userId, const std::string& walletId);
    bool usernameExists(const std::string& username);
    
public:
    AuthWalletSystem();
    ~AuthWalletSystem();
    
    // Database connection
    bool initializeDB(const std::string& connString);
    
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
