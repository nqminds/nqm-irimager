// contains example code adapted from https://opensource.com/article/22/11/extend-c-python

#include <Python.h>
#include "structmember.h"

#include <iostream>

// A struct contains the definition of a module
PyModuleDef irimager = {
    PyModuleDef_HEAD_INIT,
    "irimager", // Module name
    R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)", // docstring
    -1,      // Optional size of the module state memory
    nullptr, // Optional module methods
    nullptr, // Optional slot definitions
    nullptr, // Optional traversal function
    nullptr, // Optional clear function
    nullptr  // Optional module deallocation function
};

class IRImager {
    public:
    int test() {
        return 42;
    }
};

// Python object that stores an IRImager
typedef struct {
    PyObject_HEAD
    IRImager* m_myclass;
} IRImagerObject;

PyObject* IRImager_test(PyObject *self, PyObject *args){
    assert(self);

    IRImagerObject* _self = reinterpret_cast<IRImagerObject*>(self);
    int val = _self->m_myclass->test();
    return PyLong_FromLong(val);
}

static PyMethodDef IRImager_methods[] = {
    {"test", (PyCFunction)IRImager_test, METH_NOARGS, PyDoc_STR("Return the number 42")},
    {0, nullptr} /* Sentinel */
};

static PyMemberDef IRImager_members[] = {
    // currently no members
    {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

PyObject *IRImager_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    std::cout << "IRImager_new() called!" << std::endl;

    IRImagerObject *self = (IRImagerObject*) type->tp_alloc(type, 0);
    if(self != nullptr){ // -> allocation successfull
        // assign initial values
        self->m_myclass = nullptr;
    }
    return (PyObject*) self;
}

int IRImager_init(PyObject *self, PyObject *args, PyObject *kwds){
    IRImagerObject* m = (IRImagerObject*)self;
    m->m_myclass = (IRImager*)PyObject_Malloc(sizeof(IRImager));

    if(!m->m_myclass){
        PyErr_SetString(PyExc_RuntimeError, "Memory allocation failed");
        return -1;
    }

    try {
        new (m->m_myclass) IRImager();
    } catch (const std::exception& ex) {
        PyObject_Free(m->m_myclass);
        m->m_myclass = nullptr;
        PyErr_SetString(PyExc_RuntimeError, ex.what());
        return -1;
    } catch(...) {
        PyObject_Free(m->m_myclass);
        m->m_myclass = nullptr;
        PyErr_SetString(PyExc_RuntimeError, "Initialization failed");
        return -1;
    }

    return 0;
}

void IRImager_dealloc(IRImagerObject *self){
    std::cout << "IRImager_dealloc() called!" << std::endl;
    PyTypeObject *tp = Py_TYPE(self);

    IRImagerObject* m = reinterpret_cast<IRImagerObject*>(self);

    if(m->m_myclass){
        m->m_myclass->~IRImager();
        PyObject_Free(m->m_myclass);
    }

    tp->tp_free(self);
    Py_DECREF(tp);
};

PyDoc_STRVAR(IRImager_doc, R"(IRImager object - interfaces with a camera.)");

static PyType_Slot IRImager_slots[] = {
    {Py_tp_new,     (void*)IRImager_new},
    {Py_tp_init,    (void*)IRImager_init},
    {Py_tp_dealloc, (void*)IRImager_dealloc},
    {Py_tp_members, IRImager_members},
    {Py_tp_methods, IRImager_methods},
    {Py_tp_doc,     (void*)IRImager_doc},
    {0, nullptr} /* Sentinel */
};

static PyType_Spec spec_IrImager = {
    "nqm.irimager.IRImager",                    // name
    sizeof(IRImagerObject) + sizeof(IRImager),  // basicsize
    0,                                          // itemsize
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   // flags
    IRImager_slots                              // slots
};

PyMODINIT_FUNC
PyInit_irimager(void) {
    PyObject* module = PyModule_Create(&irimager);

    PyObject *irimager_class = PyType_FromSpec(&spec_IrImager);
    if (irimager_class == NULL){
        std::cerr << "Failed to make irimager_class!" << std::endl;
        return nullptr;
    }
    Py_INCREF(irimager_class);

    if(PyModule_AddObject(module, "IRImager", irimager_class) < 0){
        std::cerr << "Failed to add IRImager!" << std::endl;
        Py_DECREF(irimager_class);
        Py_DECREF(module);
        return nullptr;
    }
    return module;
}
