#include <Python.h>

#include <stdlib.h>
#include <string.h>
#include "uint256.h"

#include "sha3/sph_shabal.h"

#include <vector>

#ifdef GLOBALDEFINED
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL sph_shabal256_context    z_shabal;

#define fillz() do { \
    sph_shabal256_init(&z_skein); \
} while (0)

#define ZSHABAL   (memcpy(&ctx_shabal,    &z_shabal,    sizeof(z_shabal)))

#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))


template<typename T1>
inline uint256 HashAxiom(const T1 pbegin, const T1 pend)
{

    int R = 2;
    int N = 65536;

    std::vector<uint256> M(N);

    static unsigned char pblank[1];
    pblank[0] = 0;

    sph_shabal256_context   ctx_shabal;

    uint256 hash1;
    sph_shabal256_init(&ctx_shabal);
    sph_shabal256 (&ctx_shabal, (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]));
    sph_shabal256_close(&ctx_shabal, static_cast<void*>(&hash1));
    M[0] = hash1;


    for(int i = 1; i < N; i++)
    {
	sph_shabal256_init(&ctx_shabal);
        sph_shabal256 (&ctx_shabal, (unsigned char*)&M[i - 1], sizeof(M[i - 1]));
        sph_shabal256_close(&ctx_shabal, static_cast<void*>((unsigned char*)&M[i]));
    }

    for(int r = 1; r < R; r ++)
    {
	for(int b = 0; b < N; b++)
	{	    
	    int p = (b - 1 + N) % N;
	    int q = M[p].GetInt() % (N - 1);
	    int j = (b + q) % N;
	    std::vector<uint256> pj(2);
	    
	    pj[0] = M[p];
	    pj[1] = M[j];
	    sph_shabal256_init(&ctx_shabal);
            sph_shabal256 (&ctx_shabal, (unsigned char*)&pj[0], 2 * sizeof(pj[0]));
            sph_shabal256_close(&ctx_shabal, static_cast<void*>((unsigned char*)&M[b]));
	}
    }

    return M[N - 1];

}

static void axiom_hash(const unsigned char *input, int len , char *output)
{
    uint256 hash = HashAxiom(input, input + len);
    memcpy(output, &hash, 64);
}

static PyObject *axiom_getpowhash(PyObject *self, PyObject *args)
{
    char *output;
    PyObject *value;
#if PY_MAJOR_VERSION >= 3
    PyBytesObject *input;
#else
    PyStringObject *input;
#endif
    if (!PyArg_ParseTuple(args, "S", &input))
        return NULL;
    Py_INCREF(input);
    output = (char *)PyMem_Malloc(64);

#if PY_MAJOR_VERSION >= 3
    axiom_hash((unsigned char *)PyBytes_AsString((PyObject*) input), Py_SIZE((PyObject*) input), output);
#else
    axiom_hash((unsigned char *)PyString_AsString((PyObject*) input), Py_SIZE((PyObject*) input), output);
#endif
    Py_DECREF(input);
#if PY_MAJOR_VERSION >= 3
    value = Py_BuildValue("y#", output, 64);
#else
    value = Py_BuildValue("s#", output, 64);
#endif
    PyMem_Free(output);
    return value;
}

static PyMethodDef AxiomMethods[] = {
    { "getPoWHash", axiom_getpowhash, METH_VARARGS, "Returns the proof of work hash using axiom hash" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef AxiomModule = {
    PyModuleDef_HEAD_INIT,
    "axiom_hash",
    "...",
    -1,
    AxiomMethods
};

PyMODINIT_FUNC PyInit_axiom_hash(void) {
    return PyModule_Create(&AxiomModule);
}

#else

PyMODINIT_FUNC initaxiom_hash(void) {
    (void) Py_InitModule("axiom_hash", AxiomMethods);
}
#endif

