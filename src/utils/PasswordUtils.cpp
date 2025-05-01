#include "utils/PasswordUtils.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <vector>

namespace PasswordUtils {

std::string generateSalt(size_t length) {
    std::vector<unsigned char> buffer(length);
    RAND_bytes(buffer.data(), length);
    
    std::stringstream ss;
    for (auto& byte : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    
    return ss.str();
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    // Combine password and salt
    std::string combined = password + salt;
    
    // Hash using SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.size());
    SHA256_Final(hash, &sha256);
    
    // Convert to hex string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

std::tuple<std::string, std::string> hashAndSalt(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);
    return std::make_tuple(hash, salt);
}

bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string computedHash = hashPassword(password, salt);
    return computedHash == hash;
}

} // namespace PasswordUtils
