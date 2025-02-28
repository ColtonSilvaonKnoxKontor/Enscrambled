#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

const size_t CHUNK_SIZE = 16 * 1024; // 16KB chunks
const int AES_KEY_SIZE = 32; // AES-256 requires 32 bytes
const int AES_BLOCK_SIZE = 16; // AES block size
const char FILE_SIGNATURE[] = "SILVASYSTEMS\x01\x00"; // File Signature + version (6 bytes)

// Function to derive a key from a string password
void deriveKey(const string &password, unsigned char *key, unsigned char *iv) {
    unsigned char salt[8] = {0}; // Salt (can be randomized)
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt,
                   (unsigned char *)password.data(), password.size(), 1, key, iv);
}

// Function to encrypt or decrypt a file
void processFile(const string &inputFile, const string &outputFile, const string &password, bool encrypt) {
    ifstream inFile(inputFile, ios::binary);
    ofstream outFile(outputFile, ios::binary);

    if (!inFile || !outFile) {
        cerr << "Error opening file!" << endl;
        return;
    }

    unsigned char key[AES_KEY_SIZE], iv[AES_BLOCK_SIZE];
    deriveKey(password, key, iv);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        cerr << "Error creating cipher context!" << endl;
        return;
    }

    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, encrypt);

    vector<unsigned char> buffer(CHUNK_SIZE);
    vector<unsigned char> outBuffer(CHUNK_SIZE + AES_BLOCK_SIZE); // Extra space for padding
    int outLen;

    if (encrypt) {
        // Write file signature at the beginning
        outFile.write(FILE_SIGNATURE, sizeof(FILE_SIGNATURE));
    } else {
        // Read and check file signature
        char signature[sizeof(FILE_SIGNATURE)];
        inFile.read(signature, sizeof(FILE_SIGNATURE));

        if (strncmp(signature, FILE_SIGNATURE, sizeof(FILE_SIGNATURE)) != 0) {
            cerr << "Error: Invalid or missing file signature!" << endl;
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
    }

    while (inFile) {
        inFile.read((char *)buffer.data(), CHUNK_SIZE);
        streamsize bytesRead = inFile.gcount();

        EVP_CipherUpdate(ctx, outBuffer.data(), &outLen, buffer.data(), bytesRead);
        outFile.write((char *)outBuffer.data(), outLen);
    }

    EVP_CipherFinal_ex(ctx, outBuffer.data(), &outLen);
    outFile.write((char *)outBuffer.data(), outLen);

    EVP_CIPHER_CTX_free(ctx);
    cout << (encrypt ? "Encryption" : "Decryption") << " completed: " << outputFile << endl;
}

int main() {
    string inputFile, outputFile, password;
    int mode;
    cout << "Enscrambled v1.0 by Colton Silva." << endl;
    cout << "A simple anti-copyright detection program." << endl;
    cout << "" << endl;
    cout << "Designed for scrambling/unscrumbling file with signature." << endl;
    cout << "" << endl;


    cout << "Enter input file name: ";
    cin >> inputFile;
    cout << "Enter output file name: ";
    cin >> outputFile;
    cout << "Enter secret key: ";
    cin >> password;

    cout << "Choose mode (1 = Encrypt, 2 = Decrypt): ";
    cin >> mode;

    processFile(inputFile, outputFile, password, mode == 1);

    return 0;
}
