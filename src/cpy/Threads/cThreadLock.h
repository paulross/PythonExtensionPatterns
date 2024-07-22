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

#endif //PYTHONEXTENSIONPATTERNS_CTHREADLOCK_H
