#include <Python.h>
#include <stdio.h>
#include <stdbool.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

static char default_password[256] = "\0";

static PyObject* set_default_password(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"", NULL};
    char* password;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", (char**)kwlist, &password)) {
        return NULL;
    }
    strcpy(default_password, password);
    Py_RETURN_NONE;
}

static PyObject* decode(PyObject* self, PyObject* args, PyObject* kwargs) {
    char password[256];
    strcpy(password, default_password);
    PyObject* arg1;
    char* arg2 = NULL;
    static const char* kwlist[] = {"encrypted", "password", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|z", (char**)kwlist, &arg1, &arg2)) {
        return NULL;
    }
    if (arg2 != NULL) {
        strcpy(password, arg2);
    }
    char* encrypted = PyBytes_AS_STRING(arg1);
    unsigned char* out = malloc(sizeof(char)* PyBytes_GET_SIZE(arg1));
    unsigned char salt[8];
    memcpy(salt, encrypted + 8, 8);
    unsigned char key[EVP_CIPHER_key_length(EVP_aes_256_cbc())];
    unsigned char iv[EVP_CIPHER_iv_length(EVP_aes_256_cbc())];
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    int len_in = 0;
    int final_len = 0;
    bool error = false;
    if (EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, password, strlen((const char*)password), 1, key, iv) == 0) {
        error = true;
        ERR_error_string(ERR_get_error(), (char*)out);
    } else if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        error = true;
        ERR_error_string(ERR_get_error(), (char*)out);
    } else if (EVP_DecryptUpdate(ctx, out, &len_in, encrypted + 16, PyBytes_GET_SIZE(arg1) - 16) != 1) {
        error = true;
        ERR_error_string(ERR_get_error(), (char*)out);
    } else if (EVP_DecryptFinal_ex(ctx, out + len_in, &final_len) != 1) {
        error = true;
        ERR_error_string(ERR_get_error(), (char*)out);
    } else {
        out[final_len + len_in] = '\0';
    }
    EVP_CIPHER_CTX_cleanup(ctx);
    if (error) {
        PyErr_SetString(PyExc_RuntimeError, (char*)out);
        free(out);
        return NULL;
    }
    PyObject* answer = PyBytes_FromString((const char*)out);
    free(out);
    return Py_BuildValue("O", answer);
}

static PyMethodDef methods[] = {
        {"decode", (PyCFunction)decode, METH_VARARGS | METH_KEYWORDS, "decode"},
        {"set_default_password", (PyCFunction)set_default_password, METH_VARARGS | METH_KEYWORDS, "set_default_password"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT, "aes256", "", -1, methods
};

PyMODINIT_FUNC PyInit_aes256(void) {
    return PyModule_Create(&module);
}