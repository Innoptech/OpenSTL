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



