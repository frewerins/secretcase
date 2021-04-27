// openssl enc -aes-256-cbc -salt -in ./inf24-0.c -out data.aes -pass pass:qwerty
// openssl enc -d -aes-256-cbc -in data.aes
// run gcc -Wall inf24-1.c $(python3-config --includes --ldflags)  -lssl -lcrypto -shared -fPIC -fsanitize=address -o aes256.so
#include <Python.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>

static char default_password[256] = "\0";

static int decrypt(const unsigned char* encrypted, int enc_size, unsigned char* decrypted, const unsigned char* password)
{
    unsigned char salt[8];
    memcpy(salt, encrypted + 8, 8);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    uint32_t outl = 0;
    uint32_t templ = 0;

    uint32_t keyl = EVP_CIPHER_key_length(EVP_aes_256_cbc());
    uint32_t ivl = EVP_CIPHER_iv_length(EVP_aes_256_cbc());
    unsigned char key[keyl];
    unsigned char iv[ivl];
    EVP_CIPHER_CTX_init(ctx);

    if (0 == EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, password, strlen((const char*)password), 1, key, iv)) {
        // ERR_print_errors_fp(stderr);
        ERR_error_string(ERR_get_error(), (char*)decrypted);
        EVP_CIPHER_CTX_cleanup(ctx);
        return 1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        // ERR_print_errors_fp(stderr);
        ERR_error_string(ERR_get_error(), (char*)decrypted);
        EVP_CIPHER_CTX_cleanup(ctx);
        return 2;
    }

    if (1 != EVP_DecryptUpdate(ctx, decrypted, (int*)&outl, encrypted + 16, enc_size - 16)) {
        // ERR_print_errors_fp(stderr);
        ERR_error_string(ERR_get_error(), (char*)decrypted);
        EVP_CIPHER_CTX_cleanup(ctx);
        return 3;
    }

    if (1 != EVP_DecryptFinal_ex(ctx, decrypted + outl, (int*)&templ)) {
        // ERR_print_errors_fp(stderr);
        ERR_error_string(ERR_get_error(), (char*)decrypted);
        EVP_CIPHER_CTX_cleanup(ctx);
        return 4;
    }

    decrypted[outl + templ] = '\0';

    EVP_CIPHER_CTX_cleanup(ctx);

    return 0;
}

static PyObject* decode(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = {"encrypted", "bytes", NULL};
    PyObject* val;
    char* val_s = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|z", (char**)kwlist, &val, &val_s)) {
        return NULL;
    }
    // printf("passed password: %s\n", val_s);
    // printf("%s\n", Py_TYPE(val)->tp_name);
    // fflush(stdout);

    char password[256] = "\0";
    if (val_s == NULL) {
        strcpy(password, default_password);
    } else {
        strcpy(password, val_s);
    }
    // printf("password: %s\n", password);

    char* encrypted = PyBytes_AS_STRING(val);
    int encrypted_size = PyBytes_GET_SIZE(val);
    unsigned char* decrypted = malloc(sizeof(char) * encrypted_size);

    int error_code = 0;
    if ((error_code = decrypt((const unsigned char*)encrypted, encrypted_size, decrypted, (const unsigned char*)password)) != 0) {
        // printf("error_code: %d\n", error_code);
        // PyErr_SetString(PyExc_RuntimeError, "Something went wrong during decoding");
        PyErr_SetString(PyExc_RuntimeError, (const char*)decrypted);
        return NULL;
    }
    // printf("%s\n", decrypted);

    PyObject* res = PyBytes_FromString((const char*)decrypted);
    return Py_BuildValue("O", res);
}

static PyObject* set_default_password(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = {"", NULL};
    char* val_s;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", (char**)kwlist, &val_s)) {
        return NULL;
    }
    strcpy(default_password, val_s);
    // printf("%s\n", default_password);

    Py_RETURN_NONE;
}

// Список функций модуля
static PyMethodDef methods[] = {
        {"decode", (PyCFunction)decode, METH_VARARGS | METH_KEYWORDS, "decode"},
        {"set_default_password", (PyCFunction)set_default_password, METH_VARARGS | METH_KEYWORDS, "set_default_password"},
        {NULL, NULL, 0, NULL}};

// Описание модуля
static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT, "aes256", "AES256 operations module", -1, methods};

// Инициализация модуля
PyMODINIT_FUNC PyInit_aes256(void)
{
    PyObject* mod = PyModule_Create(&module);
    return mod;
}