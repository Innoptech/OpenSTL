#include <catch2/catch_test_macros.hpp>
#include "openstl/core/stl.h"

using namespace openstl;


// Custom equality operator for Vertex struct
bool operator!=(const Vec3& rhs, const Vec3& lhs) {
    return std::tie(rhs.x, rhs.y, rhs.z) != std::tie(lhs.x, lhs.y, lhs.z);
}

// Utility function to compare two vectors of triangles
bool compareTriangles(const std::vector<Triangle>& a, const std::vector<Triangle>& b, bool omit_attribute=false) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].normal != b[i].normal ||
            a[i].v0 != b[i].v0 ||
            a[i].v1 != b[i].v1 ||
            a[i].v2 != b[i].v2 ||
            ((a[i].attribute_byte_count != b[i].attribute_byte_count) & !omit_attribute)) {
            return false;
        }
    }
    return true;
}

TEST_CASE("Serialize STL triangles", "[openstl]") {
    // Generate some sample triangles
    std::vector<Triangle> originalTriangles{
            {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, 1u},
            {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, 2u}
    };

    SECTION("Binary Format") {
        std::string filename{"test_binary.stl"};
        std::ofstream stream(filename, std::ios::binary);
        if (!stream.is_open()) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            return;
        }
        REQUIRE(stream.is_open());

        // Serialize the triangles in binary format
        serialize(originalTriangles, stream, StlFormat::Binary);
        REQUIRE_FALSE(stream.fail());
        stream.close();

        // Deserialize the serialized triangles
        std::ifstream inFile(filename, std::ios::binary);
        REQUIRE(inFile.is_open());
        auto deserializedTriangles = deserializeBinaryStl(inFile);

        // Validate deserialized triangles against original triangles
        REQUIRE(compareTriangles(deserializedTriangles, originalTriangles));
    }

    SECTION("Binary Format stringstream") {
        std::stringstream ss;

        // Serialize the triangles in binary format
        serialize(originalTriangles, ss, StlFormat::Binary);

        // Deserialize the serialized triangles
        ss.seekg(0);
        auto deserializedTriangles = deserializeBinaryStl(ss);

        // Validate deserialized triangles against original triangles
        REQUIRE(compareTriangles(deserializedTriangles, originalTriangles));
    }

    SECTION("ASCII Format") {
        std::string filename{"test_ascii.stl"};
        std::ofstream stream(filename, std::ios::binary);
        if (!stream.is_open()) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            return;
        }
        REQUIRE(stream.is_open());

        // Serialize the triangles in ASCII format
        serialize(originalTriangles, stream, StlFormat::ASCII);
        REQUIRE_FALSE(stream.fail());
        stream.close();

        // Deserialize the serialized triangles
        std::ifstream inFile(filename);
        REQUIRE(inFile.is_open());
        auto deserializedTriangles = deserializeAsciiStl(inFile);

        // Validate deserialized triangles against original triangles
        REQUIRE(compareTriangles(deserializedTriangles, originalTriangles, true));
    }
}