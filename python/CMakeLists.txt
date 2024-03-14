message(STATUS "Adding OpenSTL python binding")

#-------------------------------------------------------------------------------
# External libraries
#-------------------------------------------------------------------------------
FIND_PACKAGE(PythonInterp 3)
FIND_PACKAGE(PythonLibs   3)

#-------------------------------------------------------------------------------
# Internal libraries
#-------------------------------------------------------------------------------
if (NOT TARGET openstl::core)
    message( FATAL_ERROR "openstl::core module could not be found")
endif()

#-------------------------------------------------------------------------------
# Build Python Binding
#-------------------------------------------------------------------------------
if(NOT PYTHONINTERP_FOUND)
    message( FATAL_ERROR "PYTHONINTERP could not be found")
endif()
if(NOT PYTHONLIBS_FOUND)
    message( FATAL_ERROR "PYTHONLIBS could not be found")
endif()


IF(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    file(GLOB_RECURSE python_SRC core/*.cpp)
    pybind11_add_module(openstl MODULE ${python_SRC})
    target_include_directories(openstl PRIVATE ${PYBIND11_SUBMODULE}/include)
    target_link_libraries(openstl PRIVATE openstl::core)
    target_compile_definitions(openstl PRIVATE VERSION_INFO=${PROJECT_VERSION})
ENDIF()