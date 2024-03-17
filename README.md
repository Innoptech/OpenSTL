# OpenSTL
An intuitive and fast header-only library to read/write, serialize/deserialize STL (stereolithography) files and data for C++ and Python.

[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow.svg?style=flat-square)](https://conventionalcommits.org)
[![PyPI license](https://img.shields.io/pypi/l/ansicolortags.svg?style=flat-square)](LICENSE)  
[![pypi](https://badge.fury.io/py/openstl.svg?style=flat-square)](https://badge.fury.io/py/openstl)
[![build](https://github.com/Innoptech/OpenSTL/actions/workflows/publish-to-test-pypi.yml/badge.svg?style=flat-square)](https://github.com/Innoptech/OpenSTL/actions/workflows/publish-to-test-pypi.yml)
[![Python](https://img.shields.io/pypi/pyversions/openstl.svg)](https://pypi.org/project/openstl/)

# Performances benchmark
Discover the staggering performance of OpenSTL in comparison to [numpy-stl](https://github.com/wolph/numpy-stl),
 [meshio](https://github.com/nschloe/meshio) and [stl-reader](https://github.com/pyvista/stl-reader), thanks to its powerful C++ backend.
See [benchmark.py](benchmark/benchmark.py). Benchmark performed on an Intel i5-9600KF CPU @ 3.70GHz.

    Performance gains over numpy-stl, meshio and stl-reader
    Write:  1.3 to 4+ times faster
    Read:   1 to 2.3+ times faster
    Rotate: 1 to 12+ times faster
Note: meshio has no straightfoward way of rotating vertices, so it was not benchmarked for this lib. 
![Benchmark Results](benchmark/benchmark.png)

# Python Usage
### Install
`pip install openstl` or `pip install -U git+https://github.com/Innoptech/OpenSTL@main`

### Read and write from a STL file
```python
import openstl
import numpy as np

# Define an array of triangles
# Following the STL standard, each triangle is defined with : normal, v0, v1, v2
quad = np.array([
    [[0.0, 0.0, 1.0], [0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [1.0, 1.0, 0.0]],
    [[0.0, 0.0, 1.0], [0.0, 0.0, 0.0], [0.0, 1.0, 0.0], [1.0, 1.0, 0.0]],
])

# Serialize the triangles to a file
success = openstl.write("quad.stl", quad, openstl.format.binary) # Or openstl.format.ascii (slower but human readable)

if not success:
    raise Exception("Error: Failed to write to the specified file.")

# Deserialize triangles from a file
deserialized_quad = openstl.read("quad.stl")

# Print the deserialized triangles
print("Deserialized Triangles:", deserialized_quad)
```
### Rotate and translate a mesh
```python
import openstl
import numpy as np

quad = openstl.read("quad.stl")

# Rotation
rotation_matrix = np.array([
    [0,-1, 0],
    [1, 0, 0],
    [0, 0, 1]
])
rotated_quad = np.matmul(rotation_matrix, quad.reshape(-1,3).T).T.reshape()

# Translation
translation_vector = np.array([1,1,1])
quad[:,1:4,:] += translation_vector # Avoid translating normals
```

# C++ Usage
### Read STL from file
```c++
std::ifstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
}

// Deserialize the triangles in either binary or ASCII format
std::vector<openstl::Triangle> triangles = openstl::deserializeStl(file);
file.close();
```

### Write STL to a file
```c++
std::ofstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
}

std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, file, openstl::StlFormat::Binary); // Or StlFormat::ASCII

if (file.fail()) {
    std::cerr << "Error: Failed to write to file " << filename << std::endl;
} else {
    std::cout << "File " << filename << " has been successfully written." << std::endl;
}
file.close();
```

### Serialize STL to a stream
```c++
std::stringstream ss;

std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, ss, openstl::StlFormat::Binary); // Or StlFormat::ASCII
```

# Integrate to your codebase
### Smart method
Include this repository with CMAKE Fetchcontent and link your executable/library to `openstl::core` library.   
Choose weither you want to fetch a specific branch or tag using `GIT_TAG`. Use the `main` branch to keep updated with the latest improvements.
```cmake
include(FetchContent)
FetchContent_Declare(
    openstl
    GIT_REPOSITORY https://github.com/Innoptech/OpenSTL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(openstl)
```
### Naïve method
Simply add [stl.h](modules/core/include/openstl/core/stl.h) to your codebase.

# Test
```bash
git clone https://github.com/Innoptech/OpenSTL
mkdir OpenSTL/build && cd OpenSTL/build
cmake -DOPENSTL_BUILD_TESTS=ON .. && cmake --build .
ctest .
```

# Requirements
C++11 or higher.
