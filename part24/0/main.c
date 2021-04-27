#include <Python.h>

static double value(PyObject* m, size_t i, size_t j) {
    if (PyList_Size(m) > i) {
        PyObject *row = PyList_GetItem(m, i);
        if (PyList_Size(row) > j) {
            return PyFloat_AsDouble(PyList_GetItem(row, j));
        }
    }
    return 0;
}

static PyObject* dot(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* left;
    PyObject* right;
    long int n = 0;
    static const char* kwlist[] = {"", "", "", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iOO", (char**)kwlist, &n, &left, &right)) {
        return NULL;
    }
    PyObject* result = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        PyObject* row = PyList_New(n);
        PyList_SetItem(result, i, row);
        for (size_t j = 0; j < n; ++j) {
            double cur_result = 0;
            for (size_t h = 0; h < n; ++h) {
                cur_result += value(left, i, h) * value(right, h, j);
            }
            PyList_SetItem(row, j, PyFloat_FromDouble(cur_result));
        }
    }
    return Py_BuildValue("O", result);
}

static PyMethodDef methods[] = {
        {"dot", (PyCFunction)dot, METH_VARARGS | METH_KEYWORDS, "matrix dot"},
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef module = {PyModuleDef_HEAD_INIT, "matrix", "", -1, methods};

PyMODINIT_FUNC PyInit_matrix(void) {
    return PyModule_Create(&module);
}