#include <catch2/catch_test_macros.hpp>
#include "openstl/tests/testutils.h"
#include "openstl/core/stl.h"

using namespace openstl;

TEST_CASE("Find unique triangles", "[openstl]") {
    Vec3 v0{1.0f, 2.0f, 3.0f}, v1{4.0f, 5.0f, 6.0f}, v2{7.0f, 8.0f, 9.0f}, v3{10.0f, 20.0f, 30.0f};
    Vec3 normal{0.f, 0.f, 1.f};
    std::vector<Triangle> triangles = {
            {normal, v0, v1, v2, 0},
            {normal, v0, v1, v2, 0},  // Duplicate vertices
            {normal, v3, v3, v3, 0},
    };

    auto uniqueVertices = findUniqueVertices(triangles);
    REQUIRE(uniqueVertices.size() == 4);

    // Check if specific vertices are present in the set
    REQUIRE(uniqueVertices.count(v0) == 1);
    REQUIRE(uniqueVertices.count(v1) == 1);
    REQUIRE(uniqueVertices.count(v2) == 1);
    REQUIRE(uniqueVertices.count(v3) == 1);
    REQUIRE(uniqueVertices.count(normal) == 0); // Not a vertice, a normal
}