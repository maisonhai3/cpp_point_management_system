#include "models/Wallet.h"

Wallet::Wallet(const std::string& walletId, int userId, long long balance)
    : walletId(walletId), userId(userId), balance(balance) {}

// Getters
std::string Wallet::getWalletId() const { return walletId; }
int Wallet::getUserId() const { return userId; }
long long Wallet::getBalance() const { return balance; }

// Setters
void Wallet::setWalletId(const std::string& walletId) { this->walletId = walletId; }
void Wallet::setUserId(int userId) { this->userId = userId; }
void Wallet::setBalance(long long balance) { this->balance = balance; }

// Methods
bool Wallet::deposit(long long amount) {
    if (amount <= 0) {
        return false;
    }
    
    balance += amount;
    return true;
}

bool Wallet::withdraw(long long amount) {
    if (amount <= 0 || amount > balance) {
        return false;
    }
    
    balance -= amount;
    return true;
}
