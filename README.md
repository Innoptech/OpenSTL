# OpenSTL
A simple header-only library to read/write, serialize/deserialize STL (stereolithography) files and data.

[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg)](http://commitizen.github.io/cz-cli/)
[![PyPI license](https://img.shields.io/pypi/l/ansicolortags.svg)](LICENSE)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow.svg?style=flat-square)](https://conventionalcommits.org)

# Integrate to your codebase
## Method 1
Simply add [stl.h](modules/core/include/openstl/core/stl.h) to your codebase.

## Method 2
Include this repository with CMAKE Fetchcontent and link your executable/library to openstl::core library.
```cmake
include(FetchContent)
FetchContent_Declare(
openstl
GIT_REPOSITORY https://github.com/Innoptech/OpenSTL.git
GIT_TAG main
GIT_SHALLOW TRUE
GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(openstl)
```

# Usage
## Read STL from file
```c++
std::ifstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
    return 1;
}

// Deserialize the triangles in either binary or ASCII format
std::vector<openstl::Triangle> triangles = openstl::deserializeStl(file);
file.close()
```

## Write STL to a file
```c++
std::ofstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
    return 1;
}

std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, stream, openstl::StlFormat::Binary); // Or StlFormat::ASCII

if (stream.fail()) {
    std::cerr << "Error: Failed to write to file " << filename << std::endl;
} else {
    std::cout << "File " << filename << " has been successfully written." << std::endl;
}
stream.close();
```

## Serialize STL to a stream
```c++
std::stringstream ss;

std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, ss, openstl::StlFormat::Binary); // Or StlFormat::ASCII
```

# Test
```bash
git clone https://github.com/Innoptech/OpenSTL
mkdir OpenSTL/build && cd OpenSTL/build
cmake -DOPENSTL_BUILD_TESTS=ON .. && cmake --build .
ctest .
```
