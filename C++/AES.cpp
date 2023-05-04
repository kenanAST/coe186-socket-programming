#include <openssl/aes.h>
#include <string>
#include <iostream>
using namespace std;

string aes_encode(const string& input, const string& key) {
    string output;
    AES_KEY aesKey;
    if (AES_set_encrypt_key((const unsigned char*)key.c_str(), 128, &aesKey) < 0) {
        cerr << "Error setting AES encryption key\n";
        return "";
    }
    const size_t inputLength = input.size();
    const size_t bufferSize = inputLength + AES_BLOCK_SIZE;
    output.resize(bufferSize);
    unsigned char* buffer = (unsigned char*)output.data();
    unsigned char iv[AES_BLOCK_SIZE] = { 0 };
    AES_cbc_encrypt((const unsigned char*)input.data(), buffer, inputLength, &aesKey, iv, AES_ENCRYPT);
    return output;
}

string aes_decode(const string& input, const string& key) {
    string output;
    AES_KEY aesKey;
    if (AES_set_decrypt_key((const unsigned char*)key.c_str(), 128, &aesKey) < 0) {
        cerr << "Error setting AES decryption key\n";
        return "";
    }
    const size_t inputLength = input.size();
    const size_t bufferSize = inputLength + AES_BLOCK_SIZE;
    output.resize(bufferSize);
    unsigned char* buffer = (unsigned char*)output.data();
    unsigned char iv[AES_BLOCK_SIZE] = { 0 };
    AES_cbc_encrypt((const unsigned char*)input.data(), buffer, inputLength, &aesKey, iv, AES_DECRYPT);
    output.resize(inputLength - buffer[inputLength - 1]);
    return output;
}
