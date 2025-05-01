#include "../include/AuthWalletSystem.h"
#include <iostream>
#include <string>
#include <limits>

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
        std::cout << "Registration failed. Please try again later." << std::endl;
    }
    
    // Wait for user input before returning to main menu
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

// Main function
int main() {
    // Create an instance of the AuthWalletSystem
    AuthWalletSystem system;
    
    // Initialize the database connection
    std::string dbConnString = "host=localhost port=5432 dbname=point_management_system user=postgres password=postgres";
    if (!system.initializeDB(dbConnString)) {
        std::cerr << "Failed to connect to the database. Please check your connection settings." << std::endl;
        return 1;
    }
    
    std::string choice;
    bool running = true;
    
    while (running) {
        displayWelcome();
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            // Register
            handleRegistration(system);
        } else if (choice == "2") {
            // Login (to be implemented)
            std::cout << "Login functionality not implemented yet." << std::endl;
            std::cout << "Press Enter to continue...";
            std::cin.get();
        } else if (choice == "3") {
            // Exit
            std::cout << "Exiting the system. Goodbye!" << std::endl;
            running = false;
        } else {
            std::cout << "Invalid choice. Please try again." << std::endl;
            std::cout << "Press Enter to continue...";
            std::cin.get();
        }
    }
    
    return 0;
}
