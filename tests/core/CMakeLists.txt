#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET openstl::testutils)
    message( FATAL_ERROR "openstl::testutils could not be found")
endif()

#-------------------------------------------------------------------------------
# CMAKE CONFIGURATIONS
#-------------------------------------------------------------------------------
# No configuration

#-------------------------------------------------------------------------------
# Add test executable
#-------------------------------------------------------------------------------
file(GLOB_RECURSE tests_src src/*.test.cpp)
add_executable(tests_core ${tests_src})
target_link_libraries(tests_core PRIVATE openstl::testutils Catch2::Catch2WithMain)
target_include_directories(tests_core PRIVATE include/ ${CMAKE_CURRENT_BINARY_DIR}/generated/)
catch_discover_tests(tests_core)