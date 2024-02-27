#include <catch2/catch_test_macros.hpp>
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



