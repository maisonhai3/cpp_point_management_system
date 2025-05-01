#include "models/User.h"

User::User(const std::string& username, const std::string& fullName, const std::string& contactInfo)
    : username(username), fullName(fullName), contactInfo(contactInfo), 
      role(UserRole::USER), passwordStatus(PasswordStatus::USER_SET) {}

User::User(int userId, const std::string& username, const std::string& hashedPassword, 
           const std::string& salt, const std::string& fullName, const std::string& contactInfo,
           UserRole role, const std::string& walletId, PasswordStatus passwordStatus)
    : userId(userId), username(username), hashedPassword(hashedPassword), salt(salt),
      fullName(fullName), contactInfo(contactInfo), role(role), 
      walletId(walletId), passwordStatus(passwordStatus) {}

// Getters
int User::getUserId() const { return userId; }
std::string User::getUsername() const { return username; }
std::string User::getHashedPassword() const { return hashedPassword; }
std::string User::getSalt() const { return salt; }
std::string User::getFullName() const { return fullName; }
std::string User::getContactInfo() const { return contactInfo; }
UserRole User::getRole() const { return role; }
std::string User::getWalletId() const { return walletId; }
PasswordStatus User::getPasswordStatus() const { return passwordStatus; }

// Setters
void User::setUserId(int userId) { this->userId = userId; }
void User::setUsername(const std::string& username) { this->username = username; }
void User::setHashedPassword(const std::string& hashedPassword) { this->hashedPassword = hashedPassword; }
void User::setSalt(const std::string& salt) { this->salt = salt; }
void User::setFullName(const std::string& fullName) { this->fullName = fullName; }
void User::setContactInfo(const std::string& contactInfo) { this->contactInfo = contactInfo; }
void User::setRole(UserRole role) { this->role = role; }
void User::setWalletId(const std::string& walletId) { this->walletId = walletId; }
void User::setPasswordStatus(PasswordStatus passwordStatus) { this->passwordStatus = passwordStatus; }

// Methods
bool User::isAdmin() const {
    return role == UserRole::ADMIN;
}

void User::setHashAndSalt(const std::string& hash, const std::string& salt) {
    this->hashedPassword = hash;
    this->salt = salt;
}

bool User::checkPassword(const std::string& password) const {
    // This is a placeholder. Actual implementation requires hashing the password with the salt
    // and comparing it to the stored hash.
    return false;
}

bool User::changePassword(const std::string& oldPwd, const std::string& newPwd) {
    // Placeholder for actual implementation
    return false;
}

bool User::updateInfo(const std::string& newName, const std::string& newContact) {
    // Placeholder for actual implementation
    fullName = newName;
    contactInfo = newContact;
    return true;
}
