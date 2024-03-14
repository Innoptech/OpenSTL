# OpenSTL
A simple yet fast header-only library to read/write, serialize/deserialize STL (stereolithography) files and data for C++ and Python.

[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow.svg?style=flat-square)](https://conventionalcommits.org)
[![PyPI license](https://img.shields.io/pypi/l/ansicolortags.svg?style=flat-square)](LICENSE)  
[![pypi](https://badge.fury.io/py/openstl.svg?style=flat-square)](https://badge.fury.io/py/openstl)
[![build](https://github.com/Innoptech/OpenSTL/actions/workflows/publish-to-test-pypi.yml/badge.svg?style=flat-square)](https://github.com/Innoptech/OpenSTL/actions/workflows/publish-to-test-pypi.yml)
[![Python](https://img.shields.io/pypi/pyversions/openstl.svg)](https://pypi.org/project/openstl/)

# Performances benchmark
The benchmark results indicate that the performance of multiplying OpenSTL triangles is comparable to that of 
native NumPy array multiplication.
![Benchmark Results](benchmark/benchmark.png)

# Python Usage
### Install
`pip install openstl` or `pip install -U git+https://github.com/Innoptech/OpenSTL@main`

### Read and write from a STL file
```python
import openstl

# Define a list of triangles
# Following the STL standard, each triangle is defined with : normal, v0, v1, v2, attribute_byte_count
# Note: attribute_byte_count can be used for coloring
triangles = [
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3),
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3),
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3)
]

# Serialize the triangles to a file
openstl.write("output.stl", triangles, openstl.StlFormat.Binary)

# Deserialize triangles from a file
deserialized_triangles = openstl.read("output.stl")

# Print the deserialized triangles
print("Deserialized Triangles:", deserialized_triangles)
```

Notes:
- The format `openstl.StlFormat.ASCII` can be used to make the file human-readable. 
It is however slower than Binary for reading and writing.
- `attribute_byte_count` can only be written in **Binary** format. 
The STL standard do not include it in the ASCII format.

### Access the Triangle attributes
```python
import openstl

# Following the STL standard, each triangle is defined with : normal, v0, v1, v2, attribute_byte_count
triangle = openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3)

# Access individual vertices
print("Normal:", triangle.normal)
print("Vertex 0:", triangle.v0)
print("Vertex 1:", triangle.v1)
print("Vertex 2:", triangle.v2)

# Access normal and attribute_byte_count
print("Attribute Byte Count:", triangle.attribute_byte_count)

# Get length of Triangle (number of elements in buffer)
print("Length:", len(triangle))

# Get elements using indexing (inefficient, instead use get_vertices)
print("Elements:", [triangle[i] for i in range(len(triangle))])

# Print Triangle object
print("Triangle:", triangle)
```

### Get an efficient view of the vertices
```python
import openstl

# Define a list of triangles
triangles = [
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3),
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3),
    openstl.Triangle([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 0.0], 3)
]

# Get a view of the vertices (v0, v1, v2) of all triangles by skipping normals and attribute_byte_count
vertices = openstl.get_vertices(triangles)

# Print the shape of the array
print("Shape of vertices array:", vertices.shape)

# Print the vertices array
print("Vertices:", vertices)
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
