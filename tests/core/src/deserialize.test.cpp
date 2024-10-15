#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "openstl/tests/testutils.h"
#include "openstl/core/stl.h"
#include <iostream>
#include <sstream>

using namespace openstl;

TEST_CASE("Deserialize ASCII STL", "[openstl]") {
    SECTION("Single triangle")
    {
        std::stringstream stream;
        stream << "solid name\n";
        stream << "facet normal 0.1 0.2 1.0\n";
        stream << "outer loop\n";
        stream << "vertex 0.0 0.0 0.0\n";
        stream << "vertex 1.0 0.0 0.0\n";
        stream << "vertex 0.0 1.0 0.0\n";
        stream << "endloop\n";
        stream << "endfacet\n";
        stream << "endsolid name\n";

        auto triangles = deserializeAsciiStl(stream);

        REQUIRE(triangles.size() == 1);
        REQUIRE(triangles[0].normal.x == 0.1f);
        REQUIRE(triangles[0].normal.y == 0.2f);
        REQUIRE(triangles[0].normal.z == 1.0f);

        REQUIRE(triangles[0].v0.x == 0.0f);
        REQUIRE(triangles[0].v0.y == 0.0f);
        REQUIRE(triangles[0].v0.z == 0.0f);

        REQUIRE(triangles[0].v1.x == 1.0f);
        REQUIRE(triangles[0].v1.y == 0.0f);
        REQUIRE(triangles[0].v1.z == 0.0f);

        REQUIRE(triangles[0].v2.x == 0.0f);
        REQUIRE(triangles[0].v2.y == 1.0f);
        REQUIRE(triangles[0].v2.z == 0.0f);

        stream.clear(); stream.seekg(0);
        auto triangles_auto = deserializeStl(stream);
        REQUIRE(triangles.size() == triangles_auto.size());
    }
    SECTION("Multiple triangles")
    {
        std::stringstream stream;
        stream << "solid name\n";
        stream << "facet normal 0.1 0.2 1.0\n";
        stream << "outer loop\n";
        stream << "vertex 0.0 0.0 0.0\n";
        stream << "vertex 1.0 0.0 0.0\n";
        stream << "vertex 0.0 1.0 0.0\n";
        stream << "endloop\n";
        stream << "endfacet\n";
        stream << "facet normal 0.0 0.0 1.0\n";
        stream << "outer loop\n";
        stream << "vertex 0.0 0.0 0.0\n";
        stream << "vertex 0.0 1.0 0.0\n";
        stream << "vertex 1.0 0.0 0.0\n";
        stream << "endloop\n";
        stream << "endfacet\n";
        stream << "endsolid name\n";

        auto triangles = deserializeAsciiStl(stream);

        REQUIRE(triangles.size() == 2);
        REQUIRE(triangles[0].normal.x == 0.1f);
        REQUIRE(triangles[0].normal.y == 0.2f);
        REQUIRE(triangles[0].normal.z == 1.0f);

        stream.clear(); stream.seekg(0);
        auto triangles_auto = deserializeStl(stream);
        REQUIRE(triangles.size() == triangles_auto.size());
    }
}

TEST_CASE("Deserialize Binary STL", "[openstl]") {

    SECTION("KEY")
    {
        std::ifstream file(testutils::getTestObjectPath(testutils::TESTOBJECT::KEY), std::ios::binary);
        REQUIRE(file.is_open());
        auto triangles = deserializeBinaryStl(file);
        REQUIRE(triangles.size() == 12);
        REQUIRE_THAT(triangles.at(0).normal.x, Catch::Matchers::WithinAbs(-1.0, 1e-6));
        REQUIRE_THAT(triangles.at(1).normal.z, Catch::Matchers::WithinAbs(-1.0, 1e-6));
        REQUIRE(triangles.at(0).attribute_byte_count == 0);

        file.clear(); file.seekg(0);
        auto triangles_auto = deserializeStl(file);
        REQUIRE(triangles.size() == triangles_auto.size());
    }
    SECTION("BALL")
    {
        std::ifstream file(testutils::getTestObjectPath(testutils::TESTOBJECT::BALL), std::ios::binary);
        REQUIRE(file.is_open());
        auto triangles = deserializeBinaryStl(file);
        REQUIRE(triangles.size() == 6162);

        file.clear(); file.seekg(0);
        auto triangles_auto = deserializeStl(file);
        REQUIRE(triangles.size() == triangles_auto.size());
    }
    SECTION("WASHER")
    {
        std::ifstream file(testutils::getTestObjectPath(testutils::TESTOBJECT::WASHER), std::ios::binary);
        REQUIRE(file.is_open());
        auto triangles = deserializeBinaryStl(file);
        REQUIRE(triangles.size() == 424);

        file.clear(); file.seekg(0);
        auto triangles_auto = deserializeStl(file);
        REQUIRE(triangles.size() == triangles_auto.size());
    }
}

TEST_CASE("Binary STL Serialization/Deserialization Security and Integrity Tests",
          "[openstl][security][stl][serialization][deserialization][security]")
{
    SECTION("Incomplete triangle data - incomplete_triangle_data.stl") {
        const std::vector<Triangle> &triangles = testutils::createTestTriangle();
        const std::string filename{"incomplete_triangle_data.stl"};
        testutils::createIncompleteTriangleData(triangles, filename);

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Test deserialization with corrupted header (invalid characters)") {
        const std::vector<Triangle>& triangles = testutils::createTestTriangle();
        const std::string filename = "corrupted_header.stl";
        testutils::createCorruptedHeaderInvalidChars(triangles, filename);  // Generate the file with invalid characters in the header

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());

        std::vector<Triangle> deserialized_triangles;
        CHECK_NOTHROW(deserialized_triangles = deserializeBinaryStl(file));
        REQUIRE(testutils::checkTrianglesEqual(deserialized_triangles, triangles));
    }
    SECTION("Test deserialization with corrupted header (excess data after header)") {
        const std::vector<Triangle> &triangles = testutils::createTestTriangle();
        const std::string filename{"excess_data_after_header.stl"};
        testutils::createCorruptedHeaderExcessData(triangles,
                                                   filename);  // Generate the file with excess data after the header

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Test deserialization with excessive triangle count") {
        const std::vector<Triangle> &triangles = testutils::createTestTriangle();
        const std::string filename{"excessive_triangle_count.stl"};
        testutils::createExcessiveTriangleCount(triangles,filename);  // Generate the file with an excessive triangle count

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Test deserialization with the maximum number of triangles") {
        const std::string filename = "max_triangles.stl";

        // Create a file with exactly MAX_TRIANGLES triangles
        std::vector<Triangle> triangles(MAX_TRIANGLES);
        testutils::createStlWithTriangles(triangles, filename);

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());

        // Test that deserialization works correctly for MAX_TRIANGLES
        std::vector<Triangle> deserialized_triangles;
        CHECK_NOTHROW(deserialized_triangles = deserializeBinaryStl(file));
        REQUIRE(deserialized_triangles.size() == MAX_TRIANGLES);
    }
    SECTION("Test deserialization exceeding the maximum number of triangles") {
        const std::string filename = "exceeding_triangles.stl";

        // Create a file with more than MAX_TRIANGLES triangles
        std::vector<Triangle> triangles(MAX_TRIANGLES+1);
        testutils::createStlWithTriangles(triangles, filename);

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());

        // Test that deserialization throws an exception for exceeding MAX_TRIANGLES
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Test deserialization exceeding the maximum number of triangles with deactivated safety") {
        const std::string filename = "exceeding_triangles.stl";

        // Create a file with more than MAX_TRIANGLES triangles
        std::vector<Triangle> triangles(MAX_TRIANGLES+1);
        testutils::createStlWithTriangles(triangles, filename);

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());

        // Deactivate buffer overflow safety
        activateOverflowSafety() = false;
        CHECK_NOTHROW(deserializeBinaryStl(file));
    }
    SECTION("Test deserialization with an empty file") {
        const std::string filename{"empty_triangles.stl"};
        testutils::createEmptyStlFile(filename);  // Generate an empty file

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Buffer overflow on triangle count - buffer_overflow_triangle_count.stl") {
        std::string filename = "buffer_overflow_triangle_count.stl";
        testutils::createBufferOverflowOnTriangleCount(filename);

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());
        CHECK_THROWS_AS(deserializeBinaryStl(file), std::runtime_error);
    }
    SECTION("Test deserialization with corrupted header (invalid characters) - corrupted_header_invalid_chars.stl") {
        const std::vector<Triangle>& triangles = testutils::createTestTriangle();
        const std::string filename = "corrupted_header_invalid_chars.stl";
        testutils::createCorruptedHeaderInvalidChars(triangles, filename);  // Generate the file with invalid characters in the header

        std::ifstream file(filename, std::ios::binary);
        REQUIRE(file.is_open());

        // Deserialize the STL file, ignoring the header content
        auto deserialized_triangles = deserializeBinaryStl(file);

        // Check that the deserialized triangles match the expected count and data
        testutils::checkTrianglesEqual(deserialized_triangles, triangles);
    }
}
