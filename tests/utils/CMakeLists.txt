#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
# No dependencies

#-------------------------------------------------------------------------------
# CMAKE CONFIGURATIONS
#-------------------------------------------------------------------------------
set(OPENSTL_TEST_ASSETSDIR "${CMAKE_CURRENT_BINARY_DIR}/generated/tests/assets/")
set(SOURCE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(DEST_ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/tests/assets")
file(GLOB_RECURSE ASSET_FILES "${SOURCE_ASSETS_DIR}/*")
list(FILTER ASSET_FILES EXCLUDE REGEX "\\.in$")
file(COPY ${ASSET_FILES} DESTINATION ${DEST_ASSETS_DIR})
configure_file(assets/assets_path.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/tests/assets/assets_path.h)

#-------------------------------------------------------------------------------
# Add test executable
#-------------------------------------------------------------------------------
add_library(tests_utils INTERFACE)
target_link_libraries(tests_utils INTERFACE openstl::core)
target_include_directories(tests_utils INTERFACE include/ ${CMAKE_CURRENT_BINARY_DIR}/generated/tests/)
add_library(openstl::testutils ALIAS tests_utils)