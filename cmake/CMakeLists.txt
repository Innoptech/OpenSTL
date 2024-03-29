macro(ReadVersion version_file)
    file(READ ${version_file} ver)
    string(REGEX MATCH "version\ *=\ *\"([0-9]+).([0-9]+).([0-9]*)\"" _ ${ver})
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
    set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
endmacro()

macro(ReadDependencyVersion dependency_name version_file)
    file(READ ${version_file} ver)
    string(REGEX MATCH "${dependency_name}\ *=\ *\"?([^\"]*)\"?" _ ${ver})
    set(${dependency_name}_VERSION "${CMAKE_MATCH_1}")
    message(STATUS "${dependency_name} version: ${${dependency_name}_VERSION}")
endmacro()