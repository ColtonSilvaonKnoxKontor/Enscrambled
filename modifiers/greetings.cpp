#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

std::string base64_decode(const std::string& encoded_string) {
    std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    std::vector<int> T(256, -1);
    for (int k = 0; k < 64; k++)
        T[base64_chars[k]] = k;

    while (in_len-- && (encoded_string[in_] != '=') && T[encoded_string[in_]] != -1) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = T[char_array_4[i]];

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = T[char_array_4[j]];

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}

void backup_motd() {
    const fs::path motd_path = "/etc/motd";
    const fs::path backup_dir = "/etc/malfrost";
    const fs::path backup_path = backup_dir / "motd.bak";

    try {
        if (fs::exists(motd_path)) {
            if (!fs::exists(backup_dir)) {
                fs::create_directory(backup_dir);
            }
            fs::copy_file(motd_path, backup_path, fs::copy_options::overwrite_existing);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error backing up MOTD: " << e.what() << std::endl;
    }
}

void change_motd() {
    const std::string encoded_motd = "WW91ciBzeXN0ZW0gaGFzIGJlZW4gY29tcHJvbWlzZWQuIEZvciBtb3JlIGluZm9ybWF0aW9uLCBjb250YWN0IHlvdXIgYWRtaW5pc3RyYXRvci4K";
    const std::string decoded_motd = base64_decode(encoded_motd);

    try {
        std::ofstream motd_file("/etc/motd", std::ios::trunc);
        if (motd_file.is_open()) {
            motd_file << decoded_motd;
            motd_file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error changing MOTD: " << e.what() << std::endl;
    }
}

void backup_issue() {
    const fs::path issue_path = "/etc/issue";
    const fs::path backup_dir = "/etc/malfrost";
    const fs::path backup_path = backup_dir / "issue.bak";

    try {
        if (fs::exists(issue_path)) {
            if (!fs::exists(backup_dir)) {
                fs::create_directory(backup_dir);
            }
            fs::copy_file(issue_path, backup_path, fs::copy_options::overwrite_existing);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error backing up issue: " << e.what() << std::endl;
    }
}

void change_issue() {
    const std::string encoded_issue = "VGhpcyBzeXN0ZW0gaXMgZm9yIGF1dGhvcml6ZWQgdXNlcnMgb25seS4K";
    const std::string decoded_issue = base64_decode(encoded_issue);

    try {
        std::ofstream issue_file("/etc/issue", std::ios::trunc);
        if (issue_file.is_open()) {
            issue_file << decoded_issue;
            issue_file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error changing issue: " << e.what() << std::endl;
    }
}
