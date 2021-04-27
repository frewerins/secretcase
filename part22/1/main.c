#include <openssl/conf.h>
#include <openssl/evp.h>
#include <string.h>
#include <unistd.h>

void read_salt(int file, char* salt) {
    char rubbish[8];
    read(file, rubbish, 8);
    read(file, salt, 8);
}

int main(int argc, char *argv[]) {
    int file_in = STDIN_FILENO;
    int file_out = STDOUT_FILENO;
    char salt[8];
    read_salt(file_in, salt);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    unsigned char key[EVP_CIPHER_key_length(EVP_aes_256_cbc())];
    unsigned char iv[EVP_CIPHER_iv_length(EVP_aes_256_cbc())];
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, argv[1], strlen(argv[1]), 1, key, iv);
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char input[2048];
    char out[2048];
    int len_in = 0;
    size_t read_bytes = 2048;
    while (read_bytes == 2048) {
        read_bytes = read(file_in, input, 2048);
        EVP_DecryptUpdate(ctx, out, &len_in, input, read_bytes);
        write(file_out, out, len_in);
    }
    int final_len = 0;
    EVP_DecryptFinal_ex(ctx, out + len_in, &final_len);
    write(file_out, out + len_in, final_len);
    EVP_CIPHER_CTX_cleanup(ctx);
    return 0;
}