#include <catch2/catch_test_macros.hpp>
#include "openstl/core/stl.h"
#include <unordered_set>
#include <algorithm>

using namespace openstl;



TEST_CASE("findInverseMap function test", "[openstl::core]") {
    SECTION("Empty input vector") {
        std::vector<Triangle> triangles;
        auto inverseMap = findInverseMap(triangles);
        REQUIRE(inverseMap.empty());
    }

    SECTION("Input vector with one triangle") {
        std::vector<Triangle> triangles = {
                Triangle{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0}
        };
        auto inverseMap = findInverseMap(triangles);
        REQUIRE(inverseMap.size() == 3);
        REQUIRE(inverseMap[triangles[0].v0].size() == 1);
        REQUIRE(inverseMap[triangles[0].v1].size() == 1);
        REQUIRE(inverseMap[triangles[0].v2].size() == 1);
    }

    SECTION("Input vector with multiple triangles") {
        std::vector<Triangle> triangles = {
                Triangle{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0},
                Triangle{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0},
                Triangle{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0}
        };
        auto inverseMap = findInverseMap(triangles);
        REQUIRE(inverseMap.size() == 3);
        REQUIRE(inverseMap[triangles[0].v0].size() == 3);  // All vertices are similar
        REQUIRE(inverseMap[triangles[0].v1].size() == 3);
        REQUIRE(inverseMap[triangles[0].v2].size() == 3);
    }
    SECTION("Multiples vertices and different triangles")
    {
        const Vec3 v0{1.0f, 2.0f, 3.0f}, v1{4.0f, 5.0f, 6.0f}, v2{7.0f, 8.0f, 9.0f}, v3{10.0f, 20.0f, 30.0f};
        const Vec3 normal{0.f, 0.f, 1.f};
        std::vector<Triangle> triangles = {
                {normal, v0, v1, v2, 0},
                {normal, v2, v1, v0, 0},  // Duplicate vertices
                {normal, v0, v1, v3, 0},
        };
        const auto& inverseMap = findInverseMap(triangles);
        REQUIRE(inverseMap.size() == 4);

        // Check if specific vertices are present in the set
        REQUIRE(inverseMap.count(v0) == 1);
        REQUIRE(inverseMap.count(v1) == 1);
        REQUIRE(inverseMap.count(v2) == 1);
        REQUIRE(inverseMap.count(v3) == 1);
        REQUIRE(inverseMap.count(normal) == 0); // Not a vertex, a normal

        // Check the number of face indices per vertex
        REQUIRE(inverseMap.at(v0).size() == 3);
        REQUIRE(inverseMap.at(v1).size() == 3);
        REQUIRE(inverseMap.at(v2).size() == 2);
        REQUIRE(inverseMap.at(v3).size() == 1);
    }
}

TEST_CASE("convertToVerticesAndFaces function test", "[convertToVerticesAndFaces]") {
    SECTION("Empty input vector") {
        std::vector<Triangle> triangles;
        auto result = convertToVerticesAndFaces(triangles);
        REQUIRE(std::get<0>(result).empty());
        REQUIRE(std::get<1>(result).empty());
    }

    SECTION("Input vector with one triangle") {
        std::vector<Triangle> triangles = {
                Triangle{{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0}
        };
        auto result = convertToVerticesAndFaces(triangles);
        REQUIRE(std::get<0>(result).size() == 3);
        REQUIRE(std::get<1>(result).size() == 1);
    }
    SECTION("Input vector with multiple triangles") {
        std::vector<Triangle> triangles = {
                Triangle{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0},
                Triangle{{0.0f, 0.0f, 2.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0},
                Triangle{{0.0f, 0.0f, 3.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 1.0f, 0.0f}, {1.0f, 2.0f, 0.0f}, 0}
        };
        auto result = convertToVerticesAndFaces(triangles);
        const auto& vertices = std::get<0>(result);
        const auto& faces = std::get<1>(result);

        REQUIRE(vertices.size() == 6); // There are 9 unique vertices in the triangles
        REQUIRE(faces.size() == 3); // There are 3 triangles

        // Check if each face has three vertices
        for (const auto& face : faces) {
            REQUIRE(face.size() == 3); // v0, v1, v2
            // Check if all indices in each face are unique
            std::unordered_set<size_t> uniqueIndices(std::begin(face), std::end(face));
            REQUIRE(uniqueIndices.size() == face.size());
        }

        // Check the correctness of vertices and faces
        // Here we are just checking if the vertices and faces are correctly extracted
        std::unordered_set<Vec3, Vec3Hash> uniqueVertices(std::begin(vertices), std::end(vertices));
        REQUIRE(uniqueVertices.size() == vertices.size()); // Check for uniqueness of vertices

        // Check if each face contains valid indices to the vertices
        for (const auto& face : faces) {
            for (size_t vertexIdx : face) {
                REQUIRE(vertexIdx >= 0);
                REQUIRE(vertexIdx < vertices.size());
            }
        }
    }
}

TEST_CASE("convertToTriangles function test", "[convertToTriangles]") {
    SECTION("Face index out of range") {
        std::vector<Vec3> vertices = {
                {0.0f, 0.0f, 1.0f}
        };
        std::vector<Face> faces = {
                {0, 1, 2}
        };
        REQUIRE_THROWS_AS(convertToTriangles(vertices, faces), std::out_of_range);
    }
    SECTION("Valid input") {
        Vec3 v0{0.0f, 0.0f, 0.0f};
        Vec3 v1{1.0f, 0.0f, 0.0f};
        Vec3 v2{0.0f, 1.0f, 0.0f};
        std::vector<Vec3> vertices = {
                v0,v1,v2,
                {0.0f, 5.0f, 0.0f} // extra vertex, not indexed
        };

        std::vector<Face> faces = {
                {0, 1, 2} // v0,v1,v2
        };

        auto triangles = convertToTriangles(vertices, faces);

        REQUIRE(triangles.size() == 1);

        const auto& triangle = triangles[0];
        REQUIRE(triangle.v0 == v0);
        REQUIRE(triangle.v1 == v1);
        REQUIRE(triangle.v2 == v2);
    }
}

template<typename T, size_t N>
bool areAllUnique(const std::array<T, N>& arr) {
    std::unordered_set<T> seen;
    for (const auto& element : arr) {
        if (!seen.insert(element).second) {
            // If insertion fails, the element is not unique
            return false;
        }
    }
    // If the loop completes, all elements are unique
    return true;
}

// Helper function to check if two Vec3 objects are equal
bool areVec3Equal(const Vec3& v1, const Vec3& v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

// Helper function to check if two Face objects are equal.
// Vertices v0, v1, v2 can be shuffled between two equal faces
bool areFacesEqual(const Face& f1, const Face& f2, const std::vector<Vec3> &v1, const std::vector<Vec3> &v2) {
    assert(areAllUnique(f1) && areAllUnique(f2));
    return std::all_of(
            std::begin(f1),std::end(f1),
            [&](const size_t &idx_f1) {
                return std::any_of(
                        std::begin(f2), std::end(f2),
                        [&](const size_t &idx_f2) { return areVec3Equal(v1[idx_f1], v2[idx_f2]); }
                );
            }
    );
}

TEST_CASE("convertToVerticesAndFaces <-> convertToTriangles integration test", "[integration]") {
    std::vector<Vec3> vertices = {
            {0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 1.0f},
    };

    // Convert vertices to faces
    std::vector<Face> faces = {
            {0, 1, 2}, // indices for: v0, v1, v2
            {1, 3, 2},
            {2, 3, 4},
    };

    // Convert faces to triangles
    auto triangles = convertToTriangles(vertices, faces);

    // Convert triangles back to vertices and faces
    auto result = convertToVerticesAndFaces(triangles);
    const auto& finalVertices = std::get<0>(result);
    const auto& finalFaces = std::get<1>(result);

    // Check if all original vertices are present in the final vertices
    bool allVerticesFound = std::all_of(
            std::begin(vertices),std::end(vertices),
            [&finalVertices](const Vec3 &vertex) {
                return std::any_of(
                        std::begin(finalVertices), std::end(finalVertices),
                        [&vertex](const Vec3 &final_v) { return areVec3Equal(vertex, final_v); }
                );
            }
    );
    REQUIRE(allVerticesFound);


    // Check if all original faces are present in the final faces
    bool allFacesValid = std::all_of(
            std::begin(faces),std::end(faces),
            [&](const Face &face) {
                return std::any_of(
                        std::begin(finalFaces), std::end(finalFaces),
                        [&](const Face &final_f) { return areFacesEqual(face, final_f, vertices, finalVertices); }
                );
            }
    );
    REQUIRE(allFacesValid);
}