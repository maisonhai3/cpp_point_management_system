#ifndef WALLET_H
#define WALLET_H

#include <string>

class Wallet {
private:
    std::string walletId;
    int userId;
    long long balance;

public:
    // Constructors
    Wallet() = default;
    Wallet(const std::string& walletId, int userId, long long balance = 0);
    
    // Getters
    std::string getWalletId() const;
    int getUserId() const;
    long long getBalance() const;
    
    // Setters
    void setWalletId(const std::string& walletId);
    void setUserId(int userId);
    void setBalance(long long balance);
    
    // Methods
    bool deposit(long long amount);
    bool withdraw(long long amount);
};

#endif // WALLET_H
