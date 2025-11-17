#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <random>

#define PORT 8080

// --- Key Pool Management ---
std::map<std::string, std::string> key_status;
std::map<std::string, std::string> key_passwords;
std::mutex key_status_mutex;

void load_key_status() {
    std::ifstream status_file("keys/key_status.csv");
    if (!status_file.is_open()) {
        std::cerr << "FATAL: Cannot open keys/key_status.csv. Did you generate keys?" << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(status_file, line)) {
        std::stringstream ss(line);
        std::string key_id, status, password;
        std::getline(ss, key_id, ',');
        std::getline(ss, status, ',');
        std::getline(ss, password);
        key_status[key_id] = status;
        key_passwords[key_id] = password;
    }
    std::cout << "Loaded status for " << key_status.size() << " keys." << std::endl;
}

void update_key_status_file_locked() {
    std::ofstream status_file("keys/key_status.csv", std::ios::trunc);
    if (!status_file.is_open()) {
        std::cerr << "FATAL: Cannot open keys/key_status.csv for writing." << std::endl;
        return;
    }
    for (const auto& pair : key_status) {
        status_file << pair.first << "," << pair.second << "," << key_passwords[pair.first] << std::endl;
    }
}

// --- Cryptography ---
RSA* load_private_key(const std::string& key_id) {
    std::string key_path = "keys/" + key_id + ".priv";
    FILE* fp = fopen(key_path.c_str(), "rb");
    if (!fp) {
        std::cerr << "Error: Could not open private key file: " << key_path << std::endl;
        return nullptr;
    }
    std::string password = key_passwords[key_id];
    RSA* rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, (void*)password.c_str());
    fclose(fp);
    if (!rsa) {
        std::cerr << "Error: Failed to read private key " << key_id << std::endl;
        ERR_print_errors_fp(stderr);
    }
    return rsa;
}

std::string rsa_decrypt(RSA* rsa, const std::vector<unsigned char>& encrypted_data) {
    std::vector<unsigned char> decrypted_data(RSA_size(rsa));
    int result = RSA_private_decrypt(encrypted_data.size(), encrypted_data.data(), decrypted_data.data(), rsa, RSA_PKCS1_PADDING);
    if (result == -1) {
        ERR_print_errors_fp(stderr);
        return "";
    }
    return std::string(reinterpret_cast<char*>(decrypted_data.data()), result);
}

// --- Key Generation (from previous step) ---
std::string generate_password(int length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.length() - 1);
    std::string password;
    for (int i = 0; i < length; ++i) {
        password += chars[distribution(generator)];
    }
    return password;
}

void generate_key_pool(int count) {
    mkdir("keys", 0755);
    std::ofstream status_file("keys/key_status.csv");
    if (!status_file.is_open()) {
        std::cerr << "Error: Could not create key_status.csv" << std::endl;
        exit(1);
    }
    std::cout << "Generating " << count << " RSA key pairs..." << std::endl;
    for (int i = 1; i <= count; ++i) {
        std::string key_id = "key_" + std::to_string(i);
        std::string private_key_path = "keys/" + key_id + ".priv";
        std::string public_key_path = "keys/" + key_id + ".pub";
        std::string password = generate_password(50);
        std::string gen_private_cmd = "openssl genrsa -aes256 -passout pass:'" + password + "' -out " + private_key_path + " 2048 > /dev/null 2>&1";
        if (system(gen_private_cmd.c_str()) != 0) {
            std::cerr << "Error generating private key " << key_id << std::endl;
            exit(1);
        }
        std::string gen_public_cmd = "openssl rsa -in " + private_key_path + " -passin pass:'" + password + "' -pubout -out " + public_key_path + " > /dev/null 2>&1";
        if (system(gen_public_cmd.c_str()) != 0) {
            std::cerr << "Error extracting public key " << key_id << std::endl;
            exit(1);
        }
        status_file << key_id << ",available," << password << std::endl;
        if (i % 10 == 0 || i == count) {
            std::cout << "Generated " << i << "/" << count << " key pairs." << std::endl;
        }
    }
    status_file.close();
    std::cout << "Successfully generated key pool." << std::endl;
}

// --- Server Logic ---
void handle_connection(int client_socket) {
    char buffer[4096] = {0};
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    std::string request(buffer, bytes_read);

    if (request == "GET_KEY") {
        std::string key_to_vend_id;
        {
            std::lock_guard<std::mutex> lock(key_status_mutex);
            for (auto& pair : key_status) {
                if (pair.second == "available") {
                    key_to_vend_id = pair.first;
                    pair.second = "assigned";
                    update_key_status_file_locked();
                    break;
                }
            }
        }

        if (!key_to_vend_id.empty()) {
            std::string pub_key_path = "keys/" + key_to_vend_id + ".pub";
            std::ifstream pub_key_file(pub_key_path);
            if (pub_key_file.is_open()) {
                std::stringstream pub_key_stream;
                pub_key_stream << pub_key_file.rdbuf();
                std::string response = key_to_vend_id + "\n" + pub_key_stream.str();
                send(client_socket, response.c_str(), response.length(), 0);
                std::cout << "INFO: Vended key '" << key_to_vend_id << "' to a client." << std::endl;
            }
        } else {
            const char* response = "NO_KEYS_AVAILABLE";
            send(client_socket, response, strlen(response), 0);
            std::cerr << "WARN: A client requested a key, but none are available." << std::endl;
        }
    }
    else if (request.rfind("VERIFY ", 0) == 0) {
        std::stringstream ss(request.substr(7));
        std::string key_id;
        ss >> key_id;
        
        int header_len = 7 + key_id.length() + 1;
        std::vector<unsigned char> encrypted_data(buffer + header_len, buffer + bytes_read);

        RSA* private_rsa = load_private_key(key_id);
        if (private_rsa) {
            std::string decrypted_passcode = rsa_decrypt(private_rsa, encrypted_data);
            RSA_free(private_rsa);

            if (key_passwords.count(key_id) && decrypted_passcode == key_passwords[key_id]) {
                send(client_socket, "Success", 7, 0);
                std::cout << "INFO: Verification success for key '" << key_id << "'." << std::endl;
                {
                    std::lock_guard<std::mutex> lock(key_status_mutex);
                    key_status[key_id] = "used";
                    update_key_status_file_locked();
                }
            } else {
                send(client_socket, "Failure", 7, 0);
                std::cout << "INFO: Verification failed for key '" << key_id << "'. Tried password: '" << decrypted_passcode << "'" << std::endl;
                {
                    std::lock_guard<std::mutex> lock(key_status_mutex);
                    key_status[key_id] = "available";
                    update_key_status_file_locked();
                }
            }
        } else {
            send(client_socket, "Failure", 7, 0);
            std::cerr << "ERROR: Could not load private key for '" << key_id << "' during verification." << std::endl;
        }
    }

    // --- Finalization of the connection ---
    // Set a linger timeout to ensure data is sent before the connection is closed.
    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = 5; // Wait up to 5 seconds
    setsockopt(client_socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    // Gracefully close the write-end of the socket
    shutdown(client_socket, SHUT_WR);
    close(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--generate-keys") {
        int count = 1;
        if (argc > 3 && std::string(argv[2]) == "--how-many") {
            try {
                count = std::stoi(argv[3]);
            } catch (const std::exception&) { /* ignore */ }
        }
        generate_key_pool(count > 0 ? count : 1);
        return 0;
    }

    load_key_status();

    int server_fd;
    struct sockaddr_in address;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    while (true) {
        int new_socket;
        socklen_t addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue;
        }
        // For now, handle connections sequentially. A multi-threaded approach would be better for production.
        handle_connection(new_socket);
    }

    close(server_fd);
    return 0;
}
