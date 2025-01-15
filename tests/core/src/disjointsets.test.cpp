#include <catch2/catch_test_macros.hpp>
#include "openstl/core/stl.h"

using namespace openstl;

TEST_CASE("DisjointSet basic operations", "[DisjointSet]") {
    DisjointSet ds(10);

    SECTION("Initial state") {
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(ds.find(i) == i);
        }
    }

    SECTION("Union operation") {
        ds.unite(0, 1);
        ds.unite(2, 3);
        ds.unite(1, 3);

        REQUIRE(ds.connected(0, 3));
        REQUIRE(ds.connected(1, 2));
        REQUIRE(!ds.connected(0, 4));
    }

    SECTION("Find with path compression") {
        ds.unite(4, 5);
        ds.unite(5, 6);
        REQUIRE(ds.find(6) == ds.find(4));
        REQUIRE(ds.find(5) == ds.find(4));
    }

    SECTION("Disconnected sets") {
        ds.unite(7, 8);
        REQUIRE(!ds.connected(7, 9));
        REQUIRE(ds.connected(7, 8));
    }
}

TEST_CASE("Find connected components of faces", "[findConnectedComponents]") {
    std::vector<std::array<float, 3>> vertices = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.5f, 0.5f, 1.0f},
    };

    std::vector<std::array<size_t, 3>> faces = {
        {0, 1, 2},
        {1, 3, 2},
        {2, 3, 4},
    };

    SECTION("Single connected component") {
        auto connectedComponents = findConnectedComponents(vertices, faces);
        REQUIRE(connectedComponents.size() == 1);
        REQUIRE(connectedComponents[0].size() == 3);
    }

    SECTION("Multiple disconnected components") {
        faces.push_back({5, 6, 7});
        vertices.push_back({2.0f, 2.0f, 0.0f});
        vertices.push_back({3.0f, 2.0f, 0.0f});
        vertices.push_back({2.5f, 3.0f, 0.0f});

        auto connectedComponents = findConnectedComponents(vertices, faces);
        REQUIRE(connectedComponents.size() == 2);
        REQUIRE(connectedComponents[0].size() == 3);
        REQUIRE(connectedComponents[1].size() == 1);
    }

    SECTION("No faces provided") {
        faces.clear();
        auto connectedComponents = findConnectedComponents(vertices, faces);
        REQUIRE(connectedComponents.empty());
    }

    SECTION("Single face") {
        faces = {{0, 1, 2}};
        auto connectedComponents = findConnectedComponents(vertices, faces);
        REQUIRE(connectedComponents.size() == 1);
        REQUIRE(connectedComponents[0].size() == 1);
        REQUIRE(connectedComponents[0][0] == std::array<size_t, 3>{0, 1, 2});
    }

    SECTION("Disconnected vertices") {
        vertices.push_back({10.0f, 10.0f, 10.0f}); // Add an isolated vertex
        auto connectedComponents = findConnectedComponents(vertices, faces);
        REQUIRE(connectedComponents.size() == 1);
        REQUIRE(connectedComponents[0].size() == 3); // Only faces contribute
    }
}