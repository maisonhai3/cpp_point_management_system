#include "database/DBConnection.h"
#include <iostream>

// DBResult implementation
DBResult::DBResult(std::unique_ptr<pqxx::result> result) : result(std::move(result)) {}

const pqxx::result* DBResult::get() const {
    return result.get();
}

int DBResult::getRowCount() const {
    return result ? result->size() : 0;
}

int DBResult::getColumnCount() const {
    return result && !result->empty() ? result->columns() : 0;
}

std::string DBResult::getValue(int row, int col) const {
    if (!result || row >= static_cast<int>(result->size()) || col >= static_cast<int>(result->columns())) {
        return "";
    }
    
    const auto& r = (*result)[row];
    if (r[col].is_null()) {
        return "";
    }
    return r[col].as<std::string>();
}

std::string DBResult::getColumnName(int col) const {
    return result && col < static_cast<int>(result->columns()) ? result->column_name(col) : "";
}

// DBConnection implementation
DBConnection::DBConnection() : connection(nullptr), transaction(nullptr) {}

DBConnection::~DBConnection() {
    disconnect();
}

bool DBConnection::connect(const std::string& connString) {
    try {
        connection = std::make_unique<pqxx::connection>(connString);
        return isConnected();
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Connection error: " << lastError << std::endl;
        return false;
    }
}

bool DBConnection::disconnect() {
    if (transaction) {
        try {
            transaction->abort();
        } catch (const std::exception& e) {
            lastError = e.what();
            std::cerr << "Error aborting transaction: " << lastError << std::endl;
        }
        transaction.reset();
    }
    
    connection.reset();
    return true;
}

bool DBConnection::isConnected() const {
    return connection && connection->is_open();
}

std::unique_ptr<DBResult> DBConnection::executeQuery(const std::string& sql) {
    if (!isConnected()) {
        lastError = "Not connected to database";
        return nullptr;
    }
    
    try {
        pqxx::work work(*connection);
        auto result = std::make_unique<pqxx::result>(work.exec(sql));
        work.commit();
        return std::make_unique<DBResult>(std::move(result));
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Query failed: " << lastError << std::endl;
        return nullptr;
    }
}

bool DBConnection::executeUpdate(const std::string& sql, int& affectedRows) {
    if (!isConnected()) {
        lastError = "Not connected to database";
        return false;
    }
    
    try {
        if (transaction) {
            // If we're in a transaction, use it
            pqxx::result result = transaction->exec(sql);
            affectedRows = result.affected_rows();
        } else {
            // Otherwise create a temporary work object
            pqxx::work work(*connection);
            pqxx::result result = work.exec(sql);
            affectedRows = result.affected_rows();
            work.commit();
        }
        return true;
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Update failed: " << lastError << std::endl;
        return false;
    }
}

bool DBConnection::beginTransaction() {
    if (!isConnected()) {
        lastError = "Not connected to database";
        return false;
    }
    
    if (transaction) {
        lastError = "Transaction already in progress";
        return false;
    }
    
    try {
        transaction = std::make_unique<pqxx::work>(*connection);
        return true;
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Failed to begin transaction: " << lastError << std::endl;
        return false;
    }
}

bool DBConnection::commitTransaction() {
    if (!isConnected()) {
        lastError = "Not connected to database";
        return false;
    }
    
    if (!transaction) {
        lastError = "No transaction in progress";
        return false;
    }
    
    try {
        transaction->commit();
        transaction.reset();
        return true;
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Failed to commit transaction: " << lastError << std::endl;
        return false;
    }
}

bool DBConnection::rollbackTransaction() {
    if (!isConnected()) {
        lastError = "Not connected to database";
        return false;
    }
    
    if (!transaction) {
        lastError = "No transaction in progress";
        return false;
    }
    
    try {
        transaction->abort();
        transaction.reset();
        return true;
    } catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "Failed to rollback transaction: " << lastError << std::endl;
        return false;
    }
}

std::string DBConnection::getLastError() const {
    return lastError.empty() && connection ? connection->error_message() : lastError;
}
