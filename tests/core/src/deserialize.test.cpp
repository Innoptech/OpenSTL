#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "openstl/tests/testutils.h"
#include "openstl/core/stl.h"
#include <iostream>
#include <sstream>

using namespace openstl;


static std::string oneTriangleBlock(
        const std::string& normal, const std::string& v0, const std::string& v1, const std::string& v2,
        const std::string& outer="outer loop")
{
    std::ostringstream ss;
    ss << "facet normal " << normal << "\n";
    ss << outer << "\n";
    ss << "vertex " << v0 << "\n";
    ss << "vertex " << v1 << "\n";
    ss << "vertex " << v2 << "\n";
    ss << "endloop\n";
    ss << "endfacet\n";
    return ss.str();
}

TEST_CASE("Deserialize ASCII STL: single triangle", "[openstl][ascii]") {
    const std::string stl_text =
            "solid name\n"
            "facet normal 0.1 0.2 1.0\n"
            "outer loop\n"
            "vertex 0.0 0.0 0.0\n"
            "vertex 1.0 0.0 0.0\n"
            "vertex 0.0 1.0 0.0\n"
            "endloop\n"
            "endfacet\n"
            "endsolid name\n";

    std::stringstream ss1(stl_text);
    auto triangles = deserializeAsciiStl(ss1);

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

    std::stringstream ss2(stl_text);
    auto triangles_auto = deserializeStl(ss2);
    REQUIRE(triangles.size() == triangles_auto.size());
}

TEST_CASE("Deserialize ASCII STL: multiple triangles", "[openstl][ascii]") {
    const std::string stl_text =
            "solid name\n"
            "facet normal 0.1 0.2 1.0\n"
            "outer loop\n"
            "vertex 0.0 0.0 0.0\n"
            "vertex 1.0 0.0 0.0\n"
            "vertex 0.0 1.0 0.0\n"
            "endloop\n"
            "endfacet\n"
            "facet normal 0.0 0.0 1.0\n"
            "outer loop\n"
            "vertex 0.0 0.0 0.0\n"
            "vertex 0.0 1.0 0.0\n"
            "vertex 1.0 0.0 0.0\n"
            "endloop\n"
            "endfacet\n"
            "endsolid name\n";

    std::stringstream ss1(stl_text);
    auto triangles = deserializeAsciiStl(ss1);

    REQUIRE(triangles.size() == 2);
    REQUIRE(triangles[0].normal.x == 0.1f);
    REQUIRE(triangles[0].normal.y == 0.2f);
    REQUIRE(triangles[0].normal.z == 1.0f);

    std::stringstream ss2(stl_text);
    auto triangles_auto = deserializeStl(ss2);
    REQUIRE(triangles.size() == triangles_auto.size());
}

TEST_CASE("Deserialize ASCII STL: scientific notation parses (issue #25)", "[openstl][ascii][sci]") {
    std::stringstream ss;
    ss << "solid name\n";
    ss << oneTriangleBlock(
            "3.530327e-01 -3.218319e-01 -8.785170e-01",
            "5.502911e-01 -7.287032e-01 3.099700e-01",
            "2.905658e-01 -3.847714e-01 7.960480e-02",
            "4.099400e-01 -2.538241e-01 7.960480e-02");
    ss << "endsolid name\n";

    auto tris = deserializeAsciiStl(ss);
    REQUIRE(tris.size() == 1);
    REQUIRE_THAT(tris[0].normal.x, Catch::Matchers::WithinAbs( 3.530327e-01f, 1e-6f));
    REQUIRE_THAT(tris[0].normal.y, Catch::Matchers::WithinAbs(-3.218319e-01f, 1e-6f));
    REQUIRE_THAT(tris[0].normal.z, Catch::Matchers::WithinAbs(-8.785170e-01f, 1e-6f));
    REQUIRE_THAT(tris[0].v0.x,     Catch::Matchers::WithinAbs( 5.502911e-01f, 1e-6f));
    REQUIRE_THAT(tris[0].v2.z,     Catch::Matchers::WithinAbs( 7.960480e-02f, 1e-6f));
}

TEST_CASE("Deserialize ASCII STL: keywords are case-insensitive", "[openstl][ascii][case]") {
    std::stringstream ss;
    ss << "solid s\n";
    ss << "FACET NORMAL 1E+00 0E+00 0E+00\n";
    ss << "OUTER LOOP\n";
    ss << "VERTEX 0E+00 0E+00 0E+00\n";
    ss << "VERTEX 1E+00 0E+00 0E+00\n";
    ss << "VERTEX 0E+00 1E+00 0E+00\n";
    ss << "ENDLOOP\nENDFACET\nENDSOLID s\n";

    auto tris = deserializeAsciiStl(ss);
    REQUIRE(tris.size() == 1);
    REQUIRE_THAT(tris[0].normal.x, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
}

TEST_CASE("Deserialize ASCII STL: Windows CRLF endings are tolerated", "[openstl][ascii][crlf]") {
    std::string text;
    text  = "solid s\r\n";
    text += oneTriangleBlock("1.0 0.0 0.0", "0 0 0", "1 0 0", "0 1 0");
    std::replace(text.begin(), text.end(), '\n', '\r'); // make everything CR
    // Ensure CRLF pairs exist (simulate typical CRLF): we’ll craft quickly:
    // For simplicity, rebuild with \r\n pairs:
    text  = "solid s\r\n";
    text += "facet normal 1.0 0.0 0.0\r\n";
    text += "outer loop\r\n";
    text += "vertex 0 0 0\r\n";
    text += "vertex 1 0 0\r\n";
    text += "vertex 0 1 0\r\n";
    text += "endloop\r\nendfacet\r\nendsolid s\r\n";

    std::stringstream ss(text);
    auto tris = deserializeAsciiStl(ss);
    REQUIRE(tris.size() == 1);
    REQUIRE_THAT(tris[0].v1.x, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
}

TEST_CASE("Deserialize ASCII STL: extra tokens after numbers are ignored", "[openstl][ascii][garbage]") {
    std::stringstream ss;
    ss << "solid s\n";
    ss << "facet normal 0 0 1 extra tokens here\n";
    ss << "outer loop\n";
    ss << "vertex 0 0 0 trailing\n";
    ss << "vertex 1 0 0 garbage\n";
    ss << "vertex 0 1 0 more_garbage\n";
    ss << "endloop\nendfacet\nendsolid s\n";

    auto tris = deserializeAsciiStl(ss);
    REQUIRE(tris.size() == 1);
    REQUIRE_THAT(tris[0].normal.z, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
}

TEST_CASE("Deserialize ASCII STL: malformed vertex fails fast (missing coord)", "[openstl][ascii][error]") {
    std::stringstream ss;
    ss << "solid s\n";
    ss << "facet normal 0 0 1\n";
    ss << "outer loop\n";
    ss << "vertex 0 0\n";        // <-- missing Z
    ss << "vertex 1 0 0\n";
    ss << "vertex 0 1 0\n";
    ss << "endloop\nendfacet\nendsolid s\n";

    REQUIRE_THROWS_AS(deserializeAsciiStl(ss), std::runtime_error);
}

TEST_CASE("Deserialize ASCII STL: unexpected EOF fails fast", "[openstl][ascii][eof]") {
    std::stringstream ss;
    ss << "solid s\n";
    ss << "facet normal 0 0 1\n";
    ss << "outer loop\n";
    ss << "vertex 0 0 0\n";
    // stream ends abruptly before vertex 2/3
    REQUIRE_THROWS_AS(deserializeAsciiStl(ss), std::runtime_error);
}

TEST_CASE("Deserialize ASCII STL: non-facet text is ignored (0 triangles)", "[openstl][ascii][ignore]") {
    std::stringstream ss;
    ss << "solid s\n";
    ss << "this is a comment\n";
    ss << "endsolid s\n";
    auto tris = deserializeAsciiStl(ss);
    REQUIRE(tris.empty());
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
