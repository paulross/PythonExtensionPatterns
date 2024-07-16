cmake_minimum_required(VERSION 3.24)
project(PythonExtensionPatterns)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


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



FIND_PACKAGE (Python3 3.11 EXACT REQUIRED COMPONENTS Interpreter Development)
#FIND_PACKAGE(PythonLibs 3.11 EXACT REQUIRED)
#SET(PythonLibs_DIR "/Library/Frameworks/Python.framework/Versions/3.8")
#FIND_PACKAGE(PythonLibs 3.8 REQUIRED PATHS ("/Library/Frameworks/Python.framework/Versions/3.8"))
#FindPythonLibs()
IF (Python3_FOUND)
    INCLUDE_DIRECTORIES("${Python3_INCLUDE_DIRS}")
    get_filename_component(PYTHON_LINK_DIRECTORY ${PYTHON_LIBRARY} DIRECTORY)
    # See: https://cmake.org/cmake/help/latest/module/FindPython3.html#module:FindPython3
    message("Python3_VERSION:           ${Python3_VERSION}")
    message("Python3_EXECUTABLE:        ${Python3_EXECUTABLE}")
    message("Python3_INTERPRETER_ID:    ${Python3_INTERPRETER_ID}")
    message("Python3_INCLUDE_DIRS:      ${Python3_INCLUDE_DIRS}")
    message("Python3_STDLIB:            ${Python3_STDLIB}")
    message("Python3_STDARCH:           ${Python3_STDARCH}")
    message("Python3_LINK_OPTIONS:      ${Python3_LINK_OPTIONS}")
    message("Python3_LIBRARIES:         ${Python3_LIBRARIES}")
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
        src/cpy/cParseArgsHelper.cpp
        src/cpy/Pickle/cCustomPickle.c
        src/cpy/cParseArgsHelper.cpp
        src/cpy/File/cFile.cpp
        src/cpy/File/PythonFileWrapper.h
        src/cpy/File/PythonFileWrapper.cpp
        src/cpy/Capsules/spam.c
        src/cpy/Capsules/spam_capsule.h
        src/cpy/Capsules/spam_capsule.c
        src/cpy/Capsules/spam_client.c
        src/cpy/Capsules/datetimetz.c
        src/cpy/cpp/CppCtorDtorInPyObject.cpp
)

link_directories(
        ${PYTHON_LINK_LIBRARY}
)

target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARY})

dump_cmake_variables()
