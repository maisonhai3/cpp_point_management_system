#include "../../include/utils/PasswordUtils.h"
#include <openssl/evp.h> // Use EVP API
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <stdexcept> // For exceptions

namespace PasswordUtils {

std::string generateSalt(size_t length) {
    unsigned char buffer[length];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        // Handle error - RAND_bytes failed
        // For simplicity, throwing an exception here. Proper error handling needed.
        throw std::runtime_error("Failed to generate random bytes for salt");
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(buffer[i]);
    }
    return ss.str().substr(0, length * 2); // Return hex string
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = password + salt;
    unsigned char hash[EVP_MAX_MD_SIZE]; // Buffer for the hash
    unsigned int hash_len = 0;

    // Get the EVP_MD structure for SHA256
    const EVP_MD* md = EVP_sha256();
    if (md == NULL) {
        throw std::runtime_error("Failed to get SHA256 message digest");
    }

    // Create and initialize the context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    // Initialize the digest operation
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
         EVP_MD_CTX_free(mdctx);
         throw std::runtime_error("Failed to initialize digest");
    }

    // Provide the message to be hashed
    if (EVP_DigestUpdate(mdctx, combined.c_str(), combined.size()) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to update digest");
    }

    // Finalize the hash
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize digest");
    }

    // Clean up the context
    EVP_MD_CTX_free(mdctx);

    // Convert the binary hash to a hexadecimal string
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hash_len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    return ss.str();
}

bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string newlyHashedPassword = hashPassword(password, salt);
    return newlyHashedPassword == hash;
}

} // namespace PasswordUtils