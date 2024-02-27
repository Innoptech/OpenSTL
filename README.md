# OpenSTL
A simple header-only library to read/write, serialize/deserialize STL (stereolithography) files and data.

# Usage
```c++
std::ifstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
    return 1;
}

// Deserialize the triangles from both binary or ASCII format
std::vector<Triangle> triangles = openstl::deserializeStl(file);
file.close()
```

```c++
std::ofstream file(filename, std::ios::binary);
if (!file.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << std::endl;
    return 1;
}

// Serialize the triangles in binary format
std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, stream, openstl::StlFormat::Binary); // Or StlFormat::ASCII

if (stream.fail()) {
    std::cerr << "Error: Failed to write to file " << filename << std::endl;
} else {
    std::cout << "File " << filename << " has been successfully written." << std::endl;
}
stream.close();
```

```c++
std::stringstream ss;

// Serialize the triangles in binary format
std::vector<openstl::Triangle> originalTriangles{}; // User triangles
openstl::serializeStl(originalTriangles, ss, openstl::StlFormat::Binary); // Or StlFormat::ASCII
```

# Usage
Simply add [stl.h](modules/core/include/openstl/core/stl.h) to your codebase.

# Test
```bash
git clone https://github.com/Innoptech/OpenSTL
mkdir OpenSTL/build && cd OpenSTL/build
cmake -DOPENSTL_BUILD_TESTS=ON .. && cmake --build .
ctest .
```