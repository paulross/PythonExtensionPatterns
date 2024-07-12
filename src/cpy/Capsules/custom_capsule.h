//
// Created by Paul Ross on 18/06/2021.
//

#ifndef PYTHONEXTENSIONSBASIC_CUSTOM_CAPSULE_H
#define PYTHONEXTENSIONSBASIC_CUSTOM_CAPSULE_H
#ifdef __cplusplus
extern "C" {
#endif

/* Define structure for C API. */
typedef struct {
    /* type objects */
    PyTypeObject *CustomType;
//    PyTypeObject *DateTimeType;
//    PyTypeObject *TimeType;
//    PyTypeObject *DeltaType;
//    PyTypeObject *TZInfoType;
//
//    /* singletons */
//    PyObject *TimeZone_UTC;
//
//    /* constructors */
//    PyObject *(*Date_FromDate)(int, int, int, PyTypeObject*);
//    PyObject *(*DateTime_FromDateAndTime)(int, int, int, int, int, int, int,
//                                          PyObject*, PyTypeObject*);
//    PyObject *(*Time_FromTime)(int, int, int, int, PyObject*, PyTypeObject*);
//    PyObject *(*Delta_FromDelta)(int, int, int, int, PyTypeObject*);
//    PyObject *(*TimeZone_FromTimeZone)(PyObject *offset, PyObject *name);
//
//    /* constructors for the DB API */
//    PyObject *(*DateTime_FromTimestamp)(PyObject*, PyObject*, PyObject*);
//    PyObject *(*Date_FromTimestamp)(PyObject*, PyObject*);
//
//    /* PEP 495 constructors */
//    PyObject *(*DateTime_FromDateAndTimeAndFold)(int, int, int, int, int, int, int,
//                                                 PyObject*, int, PyTypeObject*);
//    PyObject *(*Time_FromTimeAndFold)(int, int, int, int, PyObject*, int, PyTypeObject*);
//
} PyCustom_CAPI;

#define PyCustom_CAPSULE_NAME "custom3_capsule.CAPI"

#ifdef CUSTOM_CAPSULE
/* Code that is used for a standard build of custom such as done by setup.py. */

#else
/* Code that provides a C API to custom. */
static void **PyCustom_API;

#define PySpam_System \
 (*(PySpam_System_RETURN (*)PySpam_System_PROTO) PySpam_API[PySpam_System_NUM])

/* Return -1 on error, 0 on success.
 * PyCapsule_Import will set an exception if there's an error.
 */
static int
import_custom(void)
{
    PyCustom_API = (void **)PyCapsule_Import("custom._C_API", 0);
    return (PyCustom_API != NULL) ? 0 : -1;
}

#endif

#ifdef __cplusplus
}
#endif
#endif //PYTHONEXTENSIONSBASIC_CUSTOM_CAPSULE_H
