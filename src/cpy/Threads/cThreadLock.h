//
// Created by Paul Ross on 22/07/2024.
//

#ifndef PYTHONEXTENSIONPATTERNS_CTHREADLOCK_H
#define PYTHONEXTENSIONPATTERNS_CTHREADLOCK_H

#include <Python.h>
#include "structmember.h"

#ifdef WITH_THREAD
#include "pythread.h"
#endif

#ifdef WITH_THREAD
/* A RAII wrapper around the PyThread_type_lock. */
template<typename T>
class AcquireLock {
public:
    AcquireLock(T *pObject) : m_pObject(pObject) {
        assert(m_pObject);
        assert(m_pObject->lock);
        Py_INCREF(m_pObject);
        if (!PyThread_acquire_lock(m_pObject->lock, NOWAIT_LOCK)) {
            Py_BEGIN_ALLOW_THREADS
                PyThread_acquire_lock(m_pObject->lock, WAIT_LOCK);
            Py_END_ALLOW_THREADS
        }
    }
    ~AcquireLock() {
        assert(m_pObject);
        assert(m_pObject->lock);
        PyThread_release_lock(m_pObject->lock);
        Py_DECREF(m_pObject);
    }
private:
    T *m_pObject;
};

#else
/* Make the class a NOP which should get optimised out. */
template<typename T>
class AcquireLock {
public:
    AcquireLock(T *) {}
};
#endif

// From https://github.com/python/cpython/blob/main/Modules/_bz2module.c
// #define ACQUIRE_LOCK(obj) do { \
//    if (!PyThread_acquire_lock((obj)->lock, 0)) { \
//        Py_BEGIN_ALLOW_THREADS \
//        PyThread_acquire_lock((obj)->lock, 1); \
//        Py_END_ALLOW_THREADS \
//    } } while (0)
//#define RELEASE_LOCK(obj) PyThread_release_lock((obj)->lock)

// /Library/Frameworks/Python.framework/Versions/3.11/include/python3.11/ceval.h
// #define Py_BEGIN_ALLOW_THREADS { \
//                        PyThreadState *_save; \
//                        _save = PyEval_SaveThread();
//#define Py_BLOCK_THREADS        PyEval_RestoreThread(_save);
//#define Py_UNBLOCK_THREADS      _save = PyEval_SaveThread();
//#define Py_END_ALLOW_THREADS    PyEval_RestoreThread(_save); \
//                 }

#endif //PYTHONEXTENSIONPATTERNS_CTHREADLOCK_H
