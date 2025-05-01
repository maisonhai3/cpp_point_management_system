#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

class DBResult {
private:
    std::unique_ptr<pqxx::result> result;
    
public:
    DBResult(std::unique_ptr<pqxx::result> result);
    ~DBResult() = default;
    
    // Prevent copying
    DBResult(const DBResult&) = delete;
    DBResult& operator=(const DBResult&) = delete;
    
    // Allow moving
    DBResult(DBResult&& other) noexcept = default;
    DBResult& operator=(DBResult&& other) noexcept = default;
    
    const pqxx::result* get() const;
    int getRowCount() const;
    int getColumnCount() const;
    std::string getValue(int row, int col) const;
    std::string getColumnName(int col) const;
};

class DBConnection {
private:
    std::unique_ptr<pqxx::connection> connection;
    std::unique_ptr<pqxx::work> transaction;
    std::string lastError;
    
public:
    DBConnection();
    ~DBConnection();
    
    // Prevent copying
    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;
    
    // Connection management
    bool connect(const std::string& connString);
    bool disconnect();
    bool isConnected() const;
    
    // Query execution
    std::unique_ptr<DBResult> executeQuery(const std::string& sql);
    bool executeUpdate(const std::string& sql, int& affectedRows);
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // Error handling
    std::string getLastError() const;
};

#endif // DB_CONNECTION_H
