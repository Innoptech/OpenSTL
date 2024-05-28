message(STATUS "Adding OpenSTL python binding")

#-------------------------------------------------------------------------------
# External libraries
#-------------------------------------------------------------------------------
find_package(Python COMPONENTS Interpreter Development)

#-------------------------------------------------------------------------------
# Internal libraries
#-------------------------------------------------------------------------------
if (NOT TARGET openstl::core)
    message( FATAL_ERROR "openstl::core module could not be found")
endif()

#-------------------------------------------------------------------------------
# Build Python Binding
#-------------------------------------------------------------------------------
if(NOT PYTHON_FOUND)
    message( FATAL_ERROR "PYTHON could not be found")
endif()


IF(PYTHON3_FOUND)
    file(GLOB_RECURSE python_SRC core/*.cpp)
    pybind11_add_module(openstl MODULE ${python_SRC})
    target_include_directories(openstl PRIVATE ${PYBIND11_SUBMODULE}/include)
    target_link_libraries(openstl PRIVATE openstl::core pybind11::headers)
    target_compile_definitions(openstl PRIVATE VERSION_INFO=${PROJECT_VERSION})
    set_target_properties(openstl PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION ON
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON)
ENDIF()