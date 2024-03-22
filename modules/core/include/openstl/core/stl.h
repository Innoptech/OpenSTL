/*
MIT License

Copyright (c) 2024 Innoptech

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef OPENSTL_OPENSTL_SERIALIZE_H
#define OPENSTL_OPENSTL_SERIALIZE_H
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iterator>
#include <unordered_map>
#include <tuple>
#include <cmath>
#include <algorithm>

namespace openstl
{
// Disable padding for the structure
#pragma pack(push, 1)
    struct Vec3 {
        float x, y, z;
    };

    struct Triangle {
        Vec3 normal, v0, v1, v2;
        uint16_t attribute_byte_count;
    };
#pragma pack(pop)

    //---------------------------------------------------------------------------------------------------------
    // Serialize
    //---------------------------------------------------------------------------------------------------------
    enum class StlFormat { ASCII, Binary };

    /**
     * @brief Serialize a vector of triangles to an ASCII STL format and write it to the provided stream.
     *
     * This function writes the vector of triangles to the stream in ASCII STL format, where each triangle
     * is represented by its normal vector and three vertices.
     *
     * @tparam Stream The type of the output stream.
     * @param triangles The vector of triangles to serialize.
     * @param stream The output stream to write the serialized data to.
     */
    template<typename Stream, typename Container>
    void serializeAsciiStl(const Container& triangles, Stream& stream) {
        stream << "solid\n";
        for (const auto& tri : triangles) {
            stream << "facet normal " << tri.normal.x << " " << tri.normal.y << " " << tri.normal.z << std::endl;
            stream << "outer loop" << std::endl;
            stream << "vertex " << tri.v0.x << " " << tri.v0.y << " " << tri.v0.z << std::endl;
            stream << "vertex " << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << std::endl;
            stream << "vertex " << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << std::endl;
            stream << "endloop" << std::endl;
            stream << "endfacet" << std::endl;
        }
        stream << "endsolid\n";
    }

    /**
     * @brief Serialize a vector of triangles in binary STL format and write to a stream.
     *
     * @tparam Stream The type of the output stream.
     * @param triangles The vector of triangles to serialize.
     * @param stream The output stream to write the serialized data.
     */
    template<typename Stream, typename Container>
    void serializeBinaryStl(const Container& triangles, Stream& stream) {
        // Write header (80 bytes for comments)
        char header[80] = "STL Exported by OpenSTL [https://github.com/Innoptech/OpenSTL]";
        stream.write(header, 80);

        // Write triangle count (4 bytes)
        auto triangleCount = static_cast<uint32_t>(triangles.size());
        stream.write((const char*)&triangleCount, sizeof(triangleCount));

        // Write triangles
        for (const auto& tri : triangles)
            stream.write((const char*)&tri, sizeof(Triangle));
    }

    /**
     * @brief Serialize a vector of triangles in the specified STL format and write to a stream.
     *
     * @tparam Stream The type of the output stream.
     * @param triangles The vector of triangles to serialize.
     * @param stream The output stream to write the serialized data.
     * @param format The format of the STL file (ASCII or binary).
     */
    template <typename Stream, typename Container>
    inline void serialize(const Container& triangles, Stream& stream, StlFormat format) {
        switch (format) {
            case StlFormat::ASCII:
                serializeAsciiStl(triangles, stream);
                break;
            case StlFormat::Binary:
                serializeBinaryStl(triangles, stream);
                break;
        }
    }

    //---------------------------------------------------------------------------------------------------------
    // Deserialize
    //---------------------------------------------------------------------------------------------------------

    /**
     * @brief Read a vertex from a stream.
     *
     * @tparam Stream The type of the input stream.
     * @param stream The input stream from which to read the vertex.
     * @param vertex The Vec3 object to store the read vertex.
     */
    template <typename Stream>
    inline void readVertex(Stream& stream, Vec3& vertex)
    {
        std::string line;
        std::getline(stream, line);
        if (line.find("vertex") != std::string::npos) {
            std::istringstream iss(line);
            iss.ignore(7); // Ignore "vertex "
            iss >> vertex.x >> vertex.y >> vertex.z;
        }
    }

    /**
     * @brief Deserialize an ASCII STL file from a stream and convert it to a vector of triangles.
     *
     * @tparam Stream The type of the input stream.
     * @param stream The input stream from which to read the ASCII STL data.
     * @return A vector of triangles representing the geometry from the ASCII STL file.
     */
    template <typename Stream>
    inline std::vector<Triangle> deserializeAsciiStl(Stream& stream)
    {
        std::vector<Triangle> triangles;
        std::string line;
        while (std::getline(stream, line)) {
            if (line.find("facet normal") != std::string::npos) {
                Triangle tri{};
                {
                    std::istringstream iss(line);
                    iss.ignore(13); // Ignore "facet normal "
                    iss >> tri.normal.x >> tri.normal.y >> tri.normal.z;
                }

                std::getline(stream, line); // Skip 'outer loop' line
                readVertex(stream, tri.v0);
                readVertex(stream, tri.v1);
                readVertex(stream, tri.v2);
                triangles.push_back(tri);
            }
        }

        return triangles;
    }

    /**
     * @brief Deserialize a binary STL file from a stream and convert it to a vector of triangles.
     *
     * @tparam Stream The type of the input stream.
     * @param stream The input stream from which to read the binary STL data.
     * @return A vector of triangles representing the geometry from the binary STL file.
     */
    template <typename Stream>
    inline std::vector<Triangle> deserializeBinaryStl(Stream& stream)
    {
        // Read header
        stream.ignore(80); // Ignore the header
        uint32_t triangle_qty;
        stream.read((char*)&triangle_qty, sizeof(triangle_qty));

        // Read triangles
        std::vector<Triangle> triangles(triangle_qty);
        stream.read((char*)triangles.data(), sizeof(Triangle)*triangle_qty);
        return triangles;
    }

    /**
     * @brief Check if the given stream contains ASCII STL data.
     *
     * @tparam Stream The type of the input stream.
     * @param stream The input stream to check for ASCII STL data.
     * @return True if the stream contains ASCII STL data, false otherwise.
     */
    template <typename Stream>
    inline bool isAscii(Stream& stream)
    {
        std::string line;
        std::getline(stream, line);
        bool condition = (line.find("solid") != std::string::npos);
        std::getline(stream, line);
        condition &= (line.find("facet normal") != std::string::npos);
        stream.clear();
        stream.seekg(0);
        return condition;
    }

    /**
     * @brief Deserialize an STL file from a stream and convert it to a vector of triangles.
     *
     * This function detects the format of the STL file (ASCII or binary) by examining the content
     * of the input stream and calls the appropriate deserialization function accordingly.
     *
     * @tparam Stream The type of the input stream.
     * @param stream The input stream from which to read the STL data.
     * @return A vector of triangles representing the geometry from the STL file.
     */
    template <typename Stream>
    inline std::vector<Triangle> deserializeStl(Stream& stream)
    {
        if (isAscii(stream)) {
            return deserializeAsciiStl(stream);
        }
        return deserializeBinaryStl(stream);
    }

    //---------------------------------------------------------------------------------------------------------
    // Transformation Utils
    //---------------------------------------------------------------------------------------------------------
    using Face = std::array<size_t, 3>; // v0, v1, v2

    inline bool operator==(const Vec3& rhs, const Vec3& lhs) {
        return std::tie(rhs.x, rhs.y, rhs.z) == std::tie(lhs.x, lhs.y, lhs.z);
    }

    struct Vec3Hash {
        std::size_t operator()(const Vec3& vertex) const {
            // Combine hashes of x, y, and z using bitwise XOR
            return std::hash<float>{}(vertex.x) ^ std::hash<float>{}(vertex.y) ^ std::hash<float>{}(vertex.z);
        }
    };

    /**
     * @brief  Find the inverse map: vertex -> face idx
     * @param triangles The container of triangles from which to find unique vertices
     * @return A hash map that maps: for each unique vertex -> a vector of corresponding face indices
     */
    template<typename Container>
    inline std::unordered_map<Vec3, std::vector<size_t>, Vec3Hash> findInverseMap(const Container& triangles)
    {
        std::unordered_map<Vec3, std::vector<size_t>, Vec3Hash> map{};
        size_t triangleIdx{0};
        for (const auto& tri : triangles) {
            for(const auto vertex : {&tri.v0, &tri.v1, &tri.v2})
            {
                auto it = map.find(*vertex);
                if (it != std::end(map)) {
                    it->second.emplace_back(triangleIdx);
                    continue;
                }
                map[*vertex] = {triangleIdx};
            }
            ++triangleIdx;
        }
        return map;
    }


    /**
     * @brief Finds unique vertices from a vector of triangles
     * @param triangles The container of triangles to convert
     * @return An tuple containing respectively the vector of vertices and the vector of face indices
     */
    template<typename Container>
    inline std::tuple<std::vector<Vec3>, std::vector<Face>>
    convertToVerticesAndFaces(const Container& triangles) {
        const auto& inverseMap = findInverseMap(triangles);
        auto verticesNum = inverseMap.size();
        std::vector<Vec3> vertices{}; vertices.reserve(verticesNum);
        std::vector<Face> faces(triangles.size());
        std::vector<uint8_t> vertexPositionInFace(triangles.size(), 0u);
        size_t vertexIdx{0};
        for(const auto& item : inverseMap) {
            vertices.emplace_back(item.first);
            // Multiple faces can have the same vertex index
            for(const auto faceIdx : item.second)
                faces[faceIdx][vertexPositionInFace[faceIdx]++] = vertexIdx;
            ++vertexIdx;
        }
        return std::make_tuple(std::move(vertices), std::move(faces));
    }

    inline Vec3 operator-(const Vec3& rhs, const Vec3& lhs) {
        return {rhs.x - lhs.x, rhs.y - lhs.y, rhs.z - lhs.z};
    }

    inline Vec3 crossProduct(const Vec3& a, const Vec3& b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    /**
     * @brief Convert vertices and faces to triangles.
     * @param vertices The container of vertices.
     * @param faces The container of faces.
     * @return A vector of triangles constructed from the vertices and faces.
     */
    template<typename ContainerA, typename ContainerB>
    inline std::vector<Triangle> convertToTriangles(const ContainerA& vertices, const ContainerB& faces)
    {
        if (faces.size() == 0)
            return {};

        std::vector<Triangle> triangles; triangles.reserve(faces.size());
        auto getVertex = [&vertices](std::size_t index) {
            return std::next(std::begin(vertices), index);
        };
        auto minmax = std::max_element(&std::begin(faces)->at(0), &std::begin(faces)->at(0)+faces.size()*3);

        // Check if the minimum and maximum indices are within the bounds of the vector
        if (*minmax >= static_cast<int>(vertices.size())) {
            throw std::out_of_range("Face index out of range");
        }

        for (const auto& face : faces) {
            auto v0 = getVertex(face[0]);
            auto v1 = getVertex(face[1]);
            auto v2 = getVertex(face[2]);
            const auto normal = crossProduct(*v1 - *v0, *v2 - *v0);
            triangles.emplace_back(Triangle{normal, *v0, *v1, *v2, 0u});
        }
        return triangles;
    }
} //namespace openstl
#endif //OPENSTL_OPENSTL_SERIALIZE_H
