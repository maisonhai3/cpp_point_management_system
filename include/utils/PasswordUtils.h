#ifndef PASSWORD_UTILS_H
#define PASSWORD_UTILS_H

#include <string>
#include <tuple>

namespace PasswordUtils {
    // Generate a random salt
    std::string generateSalt(size_t length = 16);
    
    // Hash a password with a given salt using SHA-256
    std::string hashPassword(const std::string& password, const std::string& salt);
    
    // Hash a password and generate a salt
    std::tuple<std::string, std::string> hashAndSalt(const std::string& password);
    
    // Verify a password against a hash and salt
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
}

#endif // PASSWORD_UTILS_H
