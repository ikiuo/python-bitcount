#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif /* PY_SSIZE_T_CLEAN */

#include <Python.h>

#if (defined(i386) ||       \
     defined(__i386) ||     \
     defined(__i386__) ||   \
     defined(__i486__) ||   \
     defined(__i586__) ||   \
     defined(__i686__) ||   \
     defined(__amd64)   ||  \
     defined(__amd64__) ||  \
     defined(__x86_64) ||   \
     defined(__x86_64__) || \
     0)
#define ISA_X86  1
#else
#define ISA_X86  0
#endif

#ifndef USE_POPCOUNT
#  if defined(__clang__) && defined(__has_builtin) && \
    __has_builtin(__builtin_popcount)
#    define HAS_POPCOUNT  1
#    define USE_POPCOUNT  1
#  endif
#else
#  define HAS_POPCOUNT  0
#endif

/*
 * Method: bit_count
 */

#if PyLong_SHIFT <= 16

inline static digit
stdc_bit_count(digit x)
{
    x = (x & 0x5555) + ((x >> 1) & 0x5555);
    x = (x & 0x3333) + ((x >> 2) & 0x3333);
    x = (x & 0x0f0f) + ((x >> 4) & 0x0f0f);
    x = (x & 0x00ff) + ((x >> 8) & 0x00ff);
    return x;
}

#elif PyLong_SHIFT <= 32

inline static digit
stdc_bit_count(digit x)
{
    x = (x & 0x55555555) + ((x >>  1) & 0x55555555);
    x = (x & 0x33333333) + ((x >>  2) & 0x33333333);
    x = (x & 0x0f0f0f0f) + ((x >>  4) & 0x0f0f0f0f);
    x = (x & 0x00ff00ff) + ((x >>  8) & 0x00ff00ff);
    x = (x & 0x0000ffff) + ((x >> 16) & 0x0000ffff);
    return x;
}

#else
#error "unknown PyLong_SHIFT"
#endif

#if HAS_POPCOUNT

static int has_popcnt = 1;
inline static digit
extc_bit_count(digit x)
{
    return __builtin_popcount(x);
}

#if ISA_X86
#define HAS_CHECK_POPCNT  1

typedef struct cpuid_result {
    unsigned int eax; /* reg:00 */
    unsigned int ecx; /* reg:01 */
    unsigned int edx; /* reg:10 */
    unsigned int ebx; /* reg:11 */
} cpuid_result;

inline static void
check_popcnt()
{
    cpuid_result res;

    res.eax = 1;
    res.ecx = 0;
    res.edx = 0;
    res.ebx = 0;

    __asm__ volatile (
        "mov %0,%%eax;"
        "mov %1,%%ecx;"
        "xor %%edx,%%edx;"
        "xor %%ebx,%%ebx;"
        "cpuid;"
        "mov %%eax,%0;"
        "mov %%ecx,%1;"
        "mov %%edx,%2;"
        "mov %%ebx,%3;"
        : "+m" (res.eax),
          "+m" (res.ecx),
          "+m" (res.edx),
          "+m" (res.ebx)
        : /**/
        : "eax", "ecx", "edx", "ebx");

    has_popcnt = (res.ecx >> 23) & 1;
}

#endif /* ISA_X86 */

#else  /* !HAS_POPCOUNT */

static int has_popcnt = 0;

inline static digit
extc_bit_count(digit x)
{
    return stdc_bit_count(x);
}

#endif /* HAS_POPCOUNT */

static PyObject *
method_bit_count(PyObject *unused, PyObject *const *args, Py_ssize_t size)
{
    PyLongObject *x;
    Py_ssize_t xsize;
    Py_ssize_t i;
    unsigned long count;

    (void) unused;

    if (size != 1) {
        PyErr_SetString(PyExc_TypeError, "bit_count expected at 1 argument");
        return NULL;
    }
    if (!PyLong_CheckExact(args[0])) {
        PyErr_SetObject(PyExc_TypeError, args[0]);
        return NULL;
    }

    x = (PyLongObject *) args[0];
    xsize = Py_ABS(Py_SIZE(x));

    count = 0;
    if (has_popcnt) {
        for (i = 0; i < xsize; i++)
            count += extc_bit_count(x->ob_digit[i]);
    }
    else {
        for (i = 0; i < xsize; i++)
            count += stdc_bit_count(x->ob_digit[i]);
    }
    return PyLong_FromUnsignedLong(count);
}

/*
 * Module: bit_count
 */

static PyMethodDef bit_count_methods[] = {
    {"bit_count", (PyCFunction) method_bit_count, METH_FASTCALL,
     "bit_count(x: int) -> int: same as bin(x).count('1')\n"},
    {NULL, NULL, 0, NULL}, /* end */
};

static PyModuleDef bit_count_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "bit_count",
    .m_doc = NULL,
    .m_size = -1,
    .m_methods = bit_count_methods,
};

PyMODINIT_FUNC
PyInit_bit_count(void)
{
#if HAS_CHECK_POPCNT
    check_popcnt();
#endif /* HAS_CHECK_POPCNT */
#ifndef NDEBUG
    printf("has_popcnt: %d\n", has_popcnt);
#endif
    return PyModule_Create(&bit_count_def);
}

/*
 * Local Variables:
 * c-file-style: "PEP7"
 * End:
 */
