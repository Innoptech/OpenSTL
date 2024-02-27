message(STATUS "Adding openstl::core module")

#-------------------------------------------------------------------------------
# Ensure requirements
#-------------------------------------------------------------------------------
# No requirements

#-------------------------------------------------------------------------------
# CMAKE OPTIONS
#-------------------------------------------------------------------------------
# No options

#-------------------------------------------------------------------------------
# CMAKE VARIABLES
#-------------------------------------------------------------------------------
# No variable

#-------------------------------------------------------------------------------
# CMAKE CONFIGURATIONS
#-------------------------------------------------------------------------------
configure_file(include/openstl/core/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/openstl/core/version.h)

#-------------------------------------------------------------------------------
# Build module
#-------------------------------------------------------------------------------
add_library(openstl_core INTERFACE)
target_include_directories(openstl_core INTERFACE include/ ${CMAKE_CURRENT_BINARY_DIR}/generated/)
add_library(openstl::core ALIAS openstl_core)