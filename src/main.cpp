#include "../include/AuthWalletSystem.h"
#include <iostream>
#include <string>
#include <limits>

// Function declarations
void clearInputBuffer();
bool isValidEmail(const std::string& email);
bool isValidPhone(const std::string& phone);
bool isValidContactInfo(const std::string& contactInfo);
void displayWelcome();
void handleRegistration(AuthWalletSystem& system);
void handleLogin(AuthWalletSystem& system);
void handlePasswordChange(AuthWalletSystem& system, bool forcedChange = false); // Placeholder added
void showLoggedInMenu(AuthWalletSystem& system); // Placeholder added

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

// Placeholder for the main menu shown after successful login
void showLoggedInMenu(AuthWalletSystem& system) {
    std::cout << "\n--- Logged In Menu ---" << std::endl;
    // TODO: Implement menu based on user role (EndUser/Admin)
    // Example options:
    // 1. View Balance (UC-WALLET-01)
    // 2. View Transactions (UC-WALLET-02)
    // 3. Transfer Points (UC-WALLET-03)
    // 4. View Profile (UC-INFO-01)
    // 5. Edit Profile (UC-INFO-02)
    // 6. Change Password (UC-AUTH-06)
    // 7. Logout (UC-AUTH-04)
    // If Admin:
    // 8. View All Users (UC-INFO-03)
    // 9. Create User (UC-AUTH-02) 
    // 10. Edit Other User (UC-INFO-04) 

    std::cout << "Post Logged-in features not implemented yet." << std::endl;
    std::cout << "You will be logged out." << std::endl;
    std::cout << "Logging out..." << std::endl;
    system.logout(); // Automatically log out for now
}


// Main function
int main() {
    // Create an instance of the AuthWalletSystem
    AuthWalletSystem system;
    if (!system.isDbConnected()) {
        std::cerr << "FATAL ERROR: Could not connect to the database. Exiting." << std::endl;
        return 1; // Exit with an error code if DB connection fails
    }


    std::string choice;
    bool running = true;
    
    while (running) {
        // Clear screen (optional, platform-dependent)
        // system("clear"); // Linux/macOS
        // system("cls"); // Windows
        
        displayWelcome();
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            // Register
            handleRegistration(system);
        } else if (choice == "2") {
            // Login
            handleLogin(system);
        } else if (choice == "3") {
            // Exit
            std::cout << "Exiting the system. Goodbye!" << std::endl;
            running = false;
        } else {
            std::cout << "Invalid choice. Please try again." << std::endl;
            // std::cout << "Press Enter to continue..."; // Redundant due to loop
            // std::cin.get(); // Redundant
        }
        
         // Add a small pause or clear screen before showing the menu again, unless exiting
        if (running) {
             // std::cout << "\nPress Enter to return to the main menu...";
             // clearInputBuffer(); // Clear buffer before next getline
             // std::cin.get(); 
        }
    }
    
    return 0;
}