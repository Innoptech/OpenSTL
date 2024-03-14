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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iterator>

#include <unordered_set>
#include <vector>
#include <tuple>
#include <cstddef>

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
    template<typename Stream>
    void serializeAsciiStl(const std::vector<Triangle>& triangles, Stream& stream) {
        stream << "solid\n";
        for (const auto& triangle : triangles) {
            stream << "facet normal " << triangle.normal.x << " " << triangle.normal.y << " " << triangle.normal.z << std::endl;
            stream << "outer loop" << std::endl;
            stream << "vertex " << triangle.v0.x << " " << triangle.v0.y << " " << triangle.v0.z << std::endl;
            stream << "vertex " << triangle.v1.x << " " << triangle.v1.y << " " << triangle.v1.z << std::endl;
            stream << "vertex " << triangle.v2.x << " " << triangle.v2.y << " " << triangle.v2.z << std::endl;
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
    template<typename Stream>
    void serializeBinaryStl(const std::vector<Triangle>& triangles, Stream& stream) {
        // Write header (80 bytes for comments)
        char header[80] = {0};
        stream.write(header, 80);

        // Write triangle count (4 bytes)
        auto triangleCount = static_cast<uint32_t>(triangles.size());
        stream.write((const char*)&triangleCount, sizeof(triangleCount));

        // Write triangles
        stream.write((const char*)triangles.data(), sizeof(Triangle)*triangles.size());
    }

    /**
     * @brief Serialize a vector of triangles in the specified STL format and write to a stream.
     *
     * @tparam Stream The type of the output stream.
     * @param triangles The vector of triangles to serialize.
     * @param stream The output stream to write the serialized data.
     * @param format The format of the STL file (ASCII or binary).
     */
    template <typename Stream>
    inline void serializeStl(const std::vector<Triangle>& triangles, Stream& stream, StlFormat format) {
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
     * @brief Finds unique vertices from a vector of triangles.
     *
     * @param triangles The vector of triangles from which to find unique vertices.
     * @return An unordered_set containing unique Vec3 vertices.
     */
    inline std::unordered_set<Vec3, Vec3Hash> findUniqueVertices(const std::vector<Triangle>& triangles) {
        std::unordered_set<Vec3, Vec3Hash> uniqueVertices;

        for (const auto& triangle : triangles) {
            uniqueVertices.insert({triangle.v0.x, triangle.v0.y, triangle.v0.z});
            uniqueVertices.insert({triangle.v1.x, triangle.v1.y, triangle.v1.z});
            uniqueVertices.insert({triangle.v2.x, triangle.v2.y, triangle.v2.z});
        }

        return uniqueVertices;
    }
} //namespace openstl
#endif //OPENSTL_OPENSTL_SERIALIZE_H
