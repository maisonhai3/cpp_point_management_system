#ifndef TRANSACTION_RECORD_H
#define TRANSACTION_RECORD_H

#include <string>

struct TransactionRecord {
    int transactionId;
    std::string fromWalletId; // Can be empty/null if from system
    std::string toWalletId;
    long long amount;
    std::string status;
    std::string timestamp; // Store as string for simplicity for now

    TransactionRecord(int id, const std::string& from, const std::string& to,
                      long long amt, const std::string& stat, const std::string& time)
        : transactionId(id), fromWalletId(from), toWalletId(to),
          amount(amt), status(stat), timestamp(time) {}
};

#endif // TRANSACTION_RECORD_H