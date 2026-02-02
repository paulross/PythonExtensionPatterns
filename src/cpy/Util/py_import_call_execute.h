//
//  py_import_call_execute.h
//  PythonSubclassList
//
//  Created by Paul Ross on 10/06/2016.
//  Copyright (c) 2016 Paul Ross. All rights reserved.
//

#ifndef PYTHONEXTENSIONPATTERNS_UTIL_PY_IMPORT_CALL_EXECUTE
#define PYTHONEXTENSIONPATTERNS_UTIL_PY_IMPORT_CALL_EXECUTE

#ifdef __cplusplus
extern "C" {
#endif

const char *current_working_directory(const char *extend);
int initialise_python(int argc, char *const *argv);
int add_path_to_sys_module(const char *path);
int import_call_execute(const char *python_module_name, const char *python_function_name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* defined(PYTHONEXTENSIONPATTERNS_UTIL_PY_IMPORT_CALL_EXECUTE) */
