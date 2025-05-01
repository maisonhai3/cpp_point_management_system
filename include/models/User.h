#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole {
    USER,
    ADMIN
};

enum class PasswordStatus {
    USER_SET,
    AUTO_GENERATED
};

class User {
private:
    int userId;
    std::string username;
    std::string hashedPassword;
    std::string salt;
    std::string fullName;
    std::string contactInfo;
    UserRole role;
    std::string walletId;
    PasswordStatus passwordStatus;

public:
    // Constructors
    User() = default;
    User(const std::string& username, const std::string& fullName, const std::string& contactInfo);
    User(int userId, const std::string& username, const std::string& hashedPassword, 
         const std::string& salt, const std::string& fullName, const std::string& contactInfo,
         UserRole role, const std::string& walletId, PasswordStatus passwordStatus);
    
    // Getters
    int getUserId() const;
    std::string getUsername() const;
    std::string getHashedPassword() const;
    std::string getSalt() const;
    std::string getFullName() const;
    std::string getContactInfo() const;
    UserRole getRole() const;
    std::string getWalletId() const;
    PasswordStatus getPasswordStatus() const;
    
    // Setters
    void setUserId(int userId);
    void setUsername(const std::string& username);
    void setHashedPassword(const std::string& hashedPassword);
    void setSalt(const std::string& salt);
    void setFullName(const std::string& fullName);
    void setContactInfo(const std::string& contactInfo);
    void setRole(UserRole role);
    void setWalletId(const std::string& walletId);
    void setPasswordStatus(PasswordStatus passwordStatus);
    
    // Methods
    bool isAdmin() const;
    bool changePassword(const std::string& oldPwd, const std::string& newPwd);
    bool updateInfo(const std::string& newName, const std::string& newContact);
    void setHashAndSalt(const std::string& hash, const std::string& salt);
    bool checkPassword(const std::string& password) const;
};

#endif // USER_H
