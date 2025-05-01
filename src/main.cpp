#include "../include/AuthWalletSystem.h"
#include <iostream>
#include <string>
#include <limits>
#include <stdexcept> // For std::stoll exceptions
#include <charconv> // For more modern string-to-int conversion (optional)
#include <iomanip>


// Function declarations
void clearInputBuffer();
bool isValidEmail(const std::string& email);
bool isValidPhone(const std::string& phone);
bool isValidContactInfo(const std::string& contactInfo);
void displayWelcome();
void handleRegistration(AuthWalletSystem& system);
void handleLogin(AuthWalletSystem& system);
void handlePasswordChange(AuthWalletSystem& system, bool forcedChange = false);
void showLoggedInMenu(AuthWalletSystem& system);
void handleViewProfile(AuthWalletSystem& system);
void handleViewBalance(AuthWalletSystem& system); // <-- Add prototype
void handleTransferPoints(AuthWalletSystem& system); // <-- Add prototype

// Function to clear input buffer
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Function to validate email format (basic validation)
bool isValidEmail(const std::string& email) {
    return email.find('@') != std::string::npos;
}

// Function to validate phone number format (basic validation)
bool isValidPhone(const std::string& phone) {
    // Check if all characters are digits
    for (char c : phone) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    
    // Check length (adjust according to your requirements)
    return phone.length() >= 10 && phone.length() <= 15;
}

// Function to validate contact info (email or phone)
bool isValidContactInfo(const std::string& contactInfo) {
    return isValidEmail(contactInfo) || isValidPhone(contactInfo);
}

// Function to display welcome message
void displayWelcome() {
    std::cout << "=======================================" << std::endl;
    std::cout << "  Point Management System" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Enter your choice: ";
}

// Function to handle user registration
void handleRegistration(AuthWalletSystem& system) {
    std::string username, password, confirmPassword, fullName, contactInfo;
    
    std::cout << "\n=== User Registration ===" << std::endl;
    
    // Get username
    while (true) {
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        
        if (username.empty()) {
            std::cout << "Username cannot be empty. Please try again." << std::endl;
            continue;
        }
        
        // Check for spaces in username
        if (username.find(' ') != std::string::npos) {
            std::cout << "Username cannot contain spaces. Please try again." << std::endl;
            continue;
        }
        
        break;
    }
    
    // Get password
    while (true) {
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        
        if (password.empty()) {
            std::cout << "Password cannot be empty. Please try again." << std::endl;
            continue;
        }
        
        if (password.length() < 6) {
            std::cout << "Password must be at least 6 characters long. Please try again." << std::endl;
            continue;
        }
        
        // Confirm password
        std::cout << "Confirm password: ";
        std::getline(std::cin, confirmPassword);
        
        if (password != confirmPassword) {
            std::cout << "Passwords do not match. Please try again." << std::endl;
            continue;
        }
        
        break;
    }
    
    // Get full name
    while (true) {
        std::cout << "Enter full name: ";
        std::getline(std::cin, fullName);
        
        if (fullName.empty()) {
            std::cout << "Full name cannot be empty. Please try again." << std::endl;
            continue;
        }
        
        break;
    }
    
    // Get contact info (email or phone)
    while (true) {
        std::cout << "Enter email or phone number: ";
        std::getline(std::cin, contactInfo);
        
        if (contactInfo.empty()) {
            std::cout << "Contact info cannot be empty. Please try again." << std::endl;
            continue;
        }
        
        if (!isValidContactInfo(contactInfo)) {
            std::cout << "Invalid email or phone format. Please try again." << std::endl;
            continue;
        }
        
        break;
    }
    
    // Attempt to register the user
    std::cout << "\nRegistering user..." << std::endl;
    if (system.registerUser(username, password, fullName, contactInfo)) {
        std::cout << "Registration successful!" << std::endl;
        std::cout << "Username: " << username << std::endl;
        std::cout << "You can now log in with your credentials." << std::endl;
    } else {
        std::cout << "Registration failed. Username might already exist or another error occurred." << std::endl;
    }
    
    // Wait for user input before returning to main menu
    std::cout << "\nPress Enter to continue...";
    // std::cin.get(); // Removed extra cin.get() as getline handles the enter after input
}

// Function to handle user login
void handleLogin(AuthWalletSystem& system) {
    std::string username, password;
    
    std::cout << "\n=== User Login ===" << std::endl;
    
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    
    if (system.login(username, password)) {
        // Login successful
        std::cout << "\nLogin successful! Welcome, " << system.getCurrentUser()->getFullName() << "!" << std::endl;
        
        // Check if password change is required (auto-generated password) UC-AUTH-03 Luồng phụ
        if (system.isPasswordChangeRequired()) {
            std::cout << "\n*** Important: You must change your auto-generated password before proceeding. ***" << std::endl;
            handlePasswordChange(system, true); // true indicates forced password change (UC-AUTH-05)
            // Check if password was actually changed (or user potentially quit)
            if (!system.getCurrentUser() || system.isPasswordChangeRequired()) {
                 std::cout << "Password change was required but not completed. Logging out." << std::endl;
                 if(system.getCurrentUser()) system.logout(); // Ensure logout if still technically logged in
                 return; // Return to main menu
            }
            std::cout << "Password changed successfully." << std::endl;
        }
        
        // Proceed to the logged-in user menu
        showLoggedInMenu(system); // Call the main menu for logged-in users

    } else {
        std::cout << "\nLogin failed. Invalid username or password." << std::endl;
    }
     // Wait for user input before returning to main menu
    std::cout << "\nPress Enter to continue...";
    // std::cin.get(); // Removed extra cin.get() as getline handles the enter after input
}

// Function to handle password change (Placeholder for UC-AUTH-05, UC-AUTH-06)
void handlePasswordChange(AuthWalletSystem& system, bool forcedChange) {
     std::cout << "\n=== " << (forcedChange ? "Required " : "") << "Password Change ===" << std::endl;
     
     if (!system.getCurrentUser()) {
         std::cout << "Error: No user logged in." << std::endl;
         return;
     }

    std::string oldPassword = ""; // Not needed for forced change
    std::string newPassword, confirmNewPassword;

    if (!forcedChange) {
         // For UC-AUTH-06, ask for the old password first
         std::cout << "Enter current password: ";
         std::getline(std::cin, oldPassword);
         // TODO: Add validation for the old password using system.changePassword logic later
         // TODO: Add OTP verification step here (UC-AUTH-07)
    }

    while (true) {
        std::cout << "Enter new password: ";
        std::getline(std::cin, newPassword);

        if (newPassword.empty()) {
            std::cout << "Password cannot be empty. Please try again." << std::endl;
            continue;
        }
        if (newPassword.length() < 6) {
            std::cout << "Password must be at least 6 characters long. Please try again." << std::endl;
            continue;
        }
         // In a real scenario, add more complexity checks here

        std::cout << "Confirm new password: ";
        std::getline(std::cin, confirmNewPassword);

        if (newPassword != confirmNewPassword) {
            std::cout << "New passwords do not match. Please try again." << std::endl;
            continue;
        }
        break; // Passwords match and meet basic criteria
    }

     // Placeholder for the actual password change call
     // In the real implementation, call system.changePassword()
     // bool success = system.changePassword(*system.getCurrentUser(), oldPassword, newPassword);
     bool success = true; // Assume success for now
     
     if (success) {
        std::cout << "Password change process placeholder - simulating success." << std::endl;
        // The actual system.changePassword would update the DB and the user object's status
        // For now, just pretend it worked if forced.
        if (forcedChange && system.getCurrentUser()) {
             // Manually update status in the current object for demo purposes
             // This would normally be handled within AuthWalletSystem::changePassword
             system.getCurrentUser()->setPasswordStatus(PasswordStatus::USER_SET); 
        }
     } else {
         std::cout << "Password change failed." << (forcedChange ? " Logging out." : "") << std::endl;
         // If forced change fails, we should probably log the user out.
         if (forcedChange && system.getCurrentUser()) {
             system.logout();
         }
     }
}

// Function to handle viewing user profile (UC-INFO-01)
void handleViewProfile(AuthWalletSystem& system) {
    std::cout << "\n--- Your Profile Information ---" << std::endl;
    
    User* currentUser = system.getCurrentUser();
    
    if (currentUser) {
        std::cout << "Username:     " << currentUser->getUsername() << std::endl;
        std::cout << "Full Name:    " << currentUser->getFullName() << std::endl;
        std::cout << "Contact Info: " << currentUser->getContactInfo() << std::endl;
        
        // Convert UserRole enum to string for display
        std::string roleStr = (currentUser->getRole() == UserRole::ADMIN) ? "Administrator" : "End-User";
        std::cout << "Role:         " << roleStr << std::endl;
        
        std::cout << "Wallet ID:    " << currentUser->getWalletId() << std::endl;
    } else {
        // This case should ideally not happen if called from showLoggedInMenu
        std::cout << "Error: Could not retrieve user information. Please log in again." << std::endl;
    }
    
    std::cout << "\nPress Enter to return to the menu...";
    // std::cin.get(); // Might consume the newline from previous input, use ignore
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer before waiting
    std::cin.get(); // Wait for user to press Enter
}

// Function to handle viewing wallet balance (UC-WALLET-01)
void handleViewBalance(AuthWalletSystem& system) {
    std::cout << "\n--- View Wallet Balance ---" << std::endl;

    User* currentUser = system.getCurrentUser();

    if (!currentUser) {
        std::cout << "Error: You must be logged in to view your balance." << std::endl;
        // Wait before returning
        std::cout << "\nPress Enter to return to the menu...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
        std::cin.get();
        return;
    }

    std::string walletId = currentUser->getWalletId();
    if (walletId.empty()) {
         std::cout << "Error: Could not find wallet ID for the current user." << std::endl;
         // Wait before returning
         std::cout << "\nPress Enter to return to the menu...";
         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
         std::cin.get();
         return;
    }

    long long balance = system.getWalletBalance(walletId);

    if (balance >= 0) {
        std::cout << "Your current wallet balance is: " << balance << " points." << std::endl;
    } else {
        // getWalletBalance returns -1 on error or if not found
        std::cout << "Could not retrieve wallet balance. The wallet might not exist or an error occurred." << std::endl;
    }

    std::cout << "\nPress Enter to return to the menu...";
    // Need to handle potential leftover newline from previous getline if called directly after another getline action
    // Using ignore here should be safer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
    std::cin.get(); // Wait for user to press Enter
}

// Function to handle transferring points (UC-WALLET-03)
void handleTransferPoints(AuthWalletSystem& system) {
    std::cout << "\n--- Transfer Points ---" << std::endl;

    // Declare variables at the top of the scope
    User* currentUser = system.getCurrentUser();
    std::string senderWalletId = ""; // Initialize senderWalletId
    std::string recipientWalletId = ""; // Initialize recipientWalletId
    long long amount = 0; // Initialize amount

    if (!currentUser) {
        std::cout << "Error: You must be logged in to transfer points." << std::endl;
        goto end_transfer; // Jump is now okay as declarations are above
    }

    // Assign senderWalletId after checking currentUser
    senderWalletId = currentUser->getWalletId();
    if (senderWalletId.empty()) {
        std::cout << "Error: Could not determine your wallet ID." << std::endl;
        goto end_transfer; // Jump is okay
    }

    // Get Recipient Wallet ID
    while (true) {
        std::cout << "Enter recipient's wallet ID: ";
        std::getline(std::cin, recipientWalletId);
        if (recipientWalletId.empty()) {
            std::cout << "Recipient wallet ID cannot be empty. Please try again." << std::endl;
        } else if (recipientWalletId == senderWalletId) {
             std::cout << "You cannot transfer points to your own wallet. Please enter a different ID." << std::endl;
        }
         else {
            break; // Valid input
        }
    }

    // Get Amount
    while (true) {
        std::cout << "Enter amount to transfer (positive integer): ";
        std::string amountStr;
        std::getline(std::cin, amountStr);

        try {
            size_t processedChars = 0;
            amount = std::stoll(amountStr, &processedChars); // Use stoll for long long

            // Check if the entire string was processed and the amount is positive
            if (processedChars != amountStr.length() || amount <= 0) {
                 std::cout << "Invalid input. Please enter a positive whole number." << std::endl;
            } else {
                 break; // Valid positive amount
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Input out of range for a valid amount." << std::endl;
        }
    }

    // --- Placeholder for getting OTP from user ---
    // std::cout << "Enter OTP received: ";
    // std::string otp;
    // std::getline(std::cin, otp);
    // --- End OTP Placeholder ---


    std::cout << "\nProcessing transfer..." << std::endl;
    // Call the system function to perform the transfer
    if (system.transferPoints(senderWalletId, recipientWalletId, amount)) {
        std::cout << "Transfer successful! " << amount << " points transferred to wallet " << recipientWalletId << "." << std::endl;
        // Optionally, display the new balance
        long long newBalance = system.getWalletBalance(senderWalletId);
        if (newBalance >= 0) {
            std::cout << "Your new balance: " << newBalance << " points." << std::endl;
        }
    } else {
        std::cout << "Transfer failed. Please check the details and your balance." << std::endl;
        // Specific error messages should have been printed by transferPoints()
    }

// Label for the common exit point
end_transfer:
    std::cout << "\nPress Enter to return to the menu...";
    // Clear potential leftover newline before waiting for Enter
    if(std::cin.peek() == '\n') std::cin.ignore();
    std::cin.get(); // Wait for user to press Enter
}

void handleViewHistory(AuthWalletSystem& system) {
    std::cout << "\n--- Transaction History ---" << std::endl;

    User* currentUser = system.getCurrentUser();
    if (!currentUser) {
        std::cerr << "Error: No user is currently logged in.\n";
    }
    std::string userWalletId = currentUser->getWalletId();

    std::vector<TransactionRecord> history = system.getTransactionHistory(userWalletId);

    if (!currentUser) {
        std::cout << "Error: You must be logged in to view history." << std::endl;
        goto end_history;
    }

    if (userWalletId.empty()) {
        std::cout << "Error: Could not determine your wallet ID." << std::endl;
        goto end_history;
    }


    if (history.empty()) {
        std::cout << "No transaction history found for wallet " << userWalletId << "." << std::endl;
    } else {
        std::cout << "History for Wallet: " << userWalletId << std::endl;
        std::cout << "--------------------------------------------------------------------------------------------------" << std::endl;
        // Header - Adjust widths as needed
        std::cout << std::left << std::setw(10) << "ID"
                  << std::setw(22) << "Timestamp"
                  << std::setw(18) << "Type"
                  << std::setw(22) << "From / To"
                  << std::right << std::setw(12) << "Amount"
                  << " " << std::left << std::setw(15) << "Status" << std::endl;
        std::cout << "--------------------------------------------------------------------------------------------------" << std::endl;

        for (const auto& record : history) {
             std::string type;
             std::string otherParty;

            if (record.fromWalletId == userWalletId) {
                 type = "Sent";
                 otherParty = "To: " + record.toWalletId;
             } else if (record.toWalletId == userWalletId) {
                 type = "Received";
                 otherParty = "From: " + record.fromWalletId;
             } else {
                 // Should not happen based on query, but handle defensively
                 type = "Unknown";
                 otherParty = "From:" + record.fromWalletId + "/To:" + record.toWalletId;
             }

            std::cout << std::left << std::setw(10) << record.transactionId
                      << std::setw(22) << record.timestamp // Assuming timestamp is reasonably formatted
                      << std::setw(18) << type
                      << std::setw(22) << otherParty
                      << std::right << std::setw(12) << record.amount
                      << " " << std::left << std::setw(15) << record.status << std::endl;
        }
        std::cout << "--------------------------------------------------------------------------------------------------" << std::endl;
    }

end_history:
    std::cout << "\nPress Enter to return to the menu...";
    if(std::cin.peek() == '\n') std::cin.ignore();
    std::cin.get();
}

// Add this function somewhere near your other handlers
void handleViewTransactions(AuthWalletSystem& system) {
    User* user = system.getCurrentUser();
    if (!user) {
        std::cout << "User not logged in.\n";
        return;
    }
    std::string walletId = user->getWalletId();
    std::vector<TransactionRecord> history = system.getTransactionHistory(walletId);

    std::cout << "\n=== Transaction History (UC-WALLET-02) ===\n";
    if (history.empty()) {
        std::cout << "No transactions found.\n";
    } else {
        std::cout << "ID   | Time                  | From         | To           | Amount | Status\n";
        std::cout << "-----|-----------------------|--------------|--------------|--------|-------------\n";
        for (const auto& tx : history) {
            std::cout << tx.transactionId << " | "
                      << tx.timestamp << " | "
                      << (tx.fromWalletId.empty() ? "SYSTEM" : tx.fromWalletId) << " | "
                      << tx.toWalletId << " | "
                      << tx.amount << " | "
                      << tx.status << '\n';
        }
    }
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

// Menu shown after successful login
void showLoggedInMenu(AuthWalletSystem& system) {
    bool loggedIn = true;
    std::string choice;

    while(loggedIn && system.getCurrentUser()) { // Check if still logged in
        std::cout << "\n--- Logged In Menu ---" << std::endl;
        std::cout << "Welcome, " << system.getCurrentUser()->getFullName() << "!" << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "1. View Profile (UC-INFO-01)" << std::endl;
        std::cout << "2. View Balance (UC-WALLET-01)" << std::endl;
        std::cout << "3. Transfer Points (UC-WALLET-03)" << std::endl; // <-- Add menu option
        std::cout << "4. View Transactions (UC-WALLET-02)" << std::endl; // <-- Added menu option
        // TODO: Add other options based on role
        // 4. View Transactions (UC-WALLET-02)
        // 5. Edit Profile (UC-INFO-02)
        // 6. Change Password (UC-AUTH-06)
        std::cout << "7. Logout (UC-AUTH-04)" << std::endl; // <-- Renumber logout
        std::cout << "----------------------" << std::endl;
        std::cout << "Enter your choice (or type 'logout'): ";
        
        std::getline(std::cin, choice);

        if (choice == "1") {
            handleViewProfile(system);
        } else if (choice == "2") { 
            handleViewBalance(system);
        } else if (choice == "3") { // <-- Add case for transfer points
             handleTransferPoints(system);
        } else if (choice == "4") {
            handleViewTransactions(system); // <-- Handle transaction history viewing
        } else if (choice == "logout" || choice == "7") { // Check for logout input (updated number)
             std::cout << "Logging out..." << std::endl; 
             system.logout(); 
             loggedIn = false; 
        } 
        // TODO: Add cases for other menu options here
        /* Example structure:
        else if (choice == "6") {
            handlePasswordChange(system, false); 
        } 
        */
         else {
             std::cout << "Invalid choice. Please try again." << std::endl;
             std::cout << "Press Enter to continue...";
             if(std::cin.peek() == '\n') std::cin.ignore(); // Clear buffer if needed
             std::cin.get(); 
         }

        // Small pause or clear screen before showing menu again if still logged in
        if (loggedIn) {
            // system("clear"); // or cls
        }
    } 
     std::cout << "Returning to main menu." << std::endl; 
}

// Main function
int main() {
   // ... (rest of main function remains the same) ...
   AuthWalletSystem system;
    if (!system.isDbConnected()) {
        std::cerr << "FATAL ERROR: Could not connect to the database. Exiting." << std::endl;
        return 1; 
    }

    std::string choice;
    bool running = true;
    
    while (running) {        
        displayWelcome();
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            handleRegistration(system);
        } else if (choice == "2") {
            handleLogin(system); 
        } else if (choice == "3") {
            std::cout << "Exiting the system. Goodbye!" << std::endl;
            running = false;
        } else {
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
        
        if (running) {
             std::cout << "\nPress Enter to return to the main menu...";
             // Handle potential leftover newline depending on previous input function
             if (std::cin.peek() == '\n') {
                 std::cin.ignore();
             }
             std::cin.get(); 
        }
    }
    
    return 0;
}