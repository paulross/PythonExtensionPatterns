cmake_minimum_required(VERSION 3.24)
project(PythonExtensionPatterns)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)


# /Library/Frameworks/Python.framework/Versions/3.11/include/python3.11/cpython/pymem.h:42:39: fatal error: commas at the end of enumerator lists are a C++11 extension [-Wc++11-extensions]
#    PYMEM_ALLOCATOR_PYMALLOC_DEBUG = 6,
#                                      ^

add_compile_options(
        "-Wall"
        "-Wextra"
        "-Wpedantic"
        "-Werror"
        "-Wfatal-errors"
        "-Wno-unused-variable" # Temporary
        "-Wno-unused-parameter" # Temporary
        "-fexceptions"
        # To allow designated initialisers.
        "-Wno-c99-extensions"
        "-Wno-c++11-extensions"
        "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
        # Temporary
        -Wno-unused-function
)

function(dump_cmake_variables)
    message(STATUS "==== dump_cmake_variables()")
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "==== dump_cmake_variables() DONE")
endfunction()



FIND_PACKAGE(PythonLibs 3.11.1 EXACT REQUIRED)
#SET(PythonLibs_DIR "/Library/Frameworks/Python.framework/Versions/3.8")
#FIND_PACKAGE(PythonLibs 3.8 REQUIRED PATHS ("/Library/Frameworks/Python.framework/Versions/3.8"))
#FindPythonLibs()
IF (PYTHONLIBS_FOUND)
    INCLUDE_DIRECTORIES("${PYTHON_INCLUDE_DIRS}")
    get_filename_component(PYTHON_LINK_DIRECTORY ${PYTHON_LIBRARY} DIRECTORY)
    message("PYTHON_LIBS_FOUND:         ${PYTHON_LIBS_FOUND}")
    message("PYTHONLIBS_VERSION_STRING: ${PYTHONLIBS_VERSION_STRING}")
    message("PYTHON_DEBUG_LIBRARIES:    ${PYTHON_DEBUG_LIBRARIES}")
    message("PYTHON_INCLUDE_DIR:        ${PYTHON_INCLUDE_DIR}")
    message("PYTHON_INCLUDE_DIRS:       ${PYTHON_INCLUDE_DIRS}")
    message("PYTHON_INCLUDE_PATH:       ${PYTHON_INCLUDE_PATH}")
    message("PYTHON_LIBRARIES:          ${PYTHON_LIBRARIES}")
    message("PYTHON_LIBRARY:            ${PYTHON_LIBRARY}")
    message("PYTHON_LIBRARY_DEBUG:      ${PYTHON_LIBRARY_DEBUG}")
    message("PYTHON_LIBRARY_RELEASE:    ${PYTHON_LIBRARY_RELEASE}")
    message("PYTHON_LINK_DIRECTORY:     ${PYTHON_LINK_DIRECTORY}")
ELSE ()
    MESSAGE(FATAL_ERROR "Unable to find Python libraries.")
ENDIF ()

include_directories(src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList)

add_executable(PythonExtensionPatterns
        PythonExtensionPatterns/PythonExtensionPatterns/main.c
        PythonExtensionPatterns/PythonExtensionPatterns/PythonExtensionPatterns.c
        src/cpy/cExceptions.c
        src/cpy/cModuleGlobals.c
        src/cpy/cObject.c
        src/cpy/cParseArgs.c
        src/cpy/cPyRefs.c
        # TODO:
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/main.c
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/py_call_super.c
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/py_call_super.h
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/py_import_call_execute.c
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/py_import_call_execute.h
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/SubclassList.c
#        src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList/SubclassList.h
        src/cCanonical.c
        src/scratch.c
        # Legacy code. Removed in the documentation for version 0.2.0.
#        src/cpy/cParseArgsHelper.cpp
        src/cpy/Pickle/cCustomPickle.c)

link_directories(
        ${PYTHON_LINK_LIBRARY}
)

target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARY})

dump_cmake_variables()
