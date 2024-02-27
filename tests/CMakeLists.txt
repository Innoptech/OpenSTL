message(STATUS "Adding OpenSTL tests suite")

#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET Catch2::Catch2WithMain)
    message( FATAL_ERROR "catch2 could not be found")
endif()

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------
add_subdirectory(utils) # Test utilities
add_subdirectory(core)
