#ifndef OPENSTL_TESTS_TESTUTILS_H
#define OPENSTL_TESTS_TESTUTILS_H
#include "assets/assets_path.h"
#include "openstl/core/stl.h"
#include <string>

namespace openstl {
    namespace testutils {
        enum class TESTOBJECT {
            KEY, BALL, WASHER, COMPROMISED_TRIANGLE_COUNT, EMPTY_FILE
        };

        inline std::string getTestObjectPath(TESTOBJECT obj) {
            std::string basename{};
            switch (obj) {
                default:
                    basename = "KEY.STL";
                    break;
                case TESTOBJECT::BALL:
                    basename = "BALL.STL";
                    break;
                case TESTOBJECT::WASHER:
                    basename = "WASHER.STL";
                    break;
            }
            return OPENSTL_TEST_ASSETSDIR + basename;
        }

        inline std::vector<Triangle> createTestTriangle() {
            Triangle triangle;
            triangle.normal = {0.1f, 0.2f, 1.0f};
            triangle.v0 = {0.0f, 0.0f, 0.0f};
            triangle.v1 = {1.0f, 0.0f, 0.0f};
            triangle.v2 = {0.0f, 1.0f, 0.0f};
            triangle.attribute_byte_count = 0;
            return {triangle};
        }

        inline void createIncompleteTriangleData(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Write header (80 bytes for comments)
            char header[80] = "STL Exported by OpenSTL [https://github.com/Innoptech/OpenSTL]";
            file.write(header, sizeof(header));

            // Write triangle count (4 bytes)
            auto triangleCount = static_cast<uint32_t>(triangles.size());
            file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));

            // Write only half of the triangles to simulate incomplete data
            for (size_t i = 0; i < triangles.size() / 2; ++i) {
                file.write(reinterpret_cast<const char*>(&triangles[i]), sizeof(Triangle));
            }
            file.close();
        }

        inline void createCorruptedHeaderTruncated(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Truncated header (less than 80 bytes)
            char header[40] = "TruncatedHeader";
            file.write(header, sizeof(header)); // Writing only 40 bytes instead of 80

            // Write triangle count and triangles normally
            auto triangleCount = static_cast<uint32_t>(triangles.size());
            file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));
            for (const auto& tri : triangles) {
                file.write(reinterpret_cast<const char*>(&tri), sizeof(Triangle));
            }

            file.close();
        }

        inline void createCorruptedHeaderExcessData(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Correct header followed by garbage data
            char header[80] = "STL Exported by OpenSTL [https://github.com/Innoptech/OpenSTL]";
            file.write(header, sizeof(header));

            // Write some garbage data to corrupt the file
            char garbage[20] = "GARBAGE DATA";
            file.write(garbage, sizeof(garbage));

            // Write triangle count and triangles normally
            auto triangleCount = static_cast<uint32_t>(triangles.size());
            file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));
            for (const auto& tri : triangles) {
                file.write(reinterpret_cast<const char*>(&tri), sizeof(Triangle));
            }

            file.close();
        }

        inline void createExcessiveTriangleCount(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Write header (80 bytes for comments)
            char header[80] = "STL Exported by OpenSTL [https://github.com/Innoptech/OpenSTL]";
            file.write(header, sizeof(header));

            // Write an excessive triangle count (much larger than actual size)
            uint32_t excessiveCount = std::numeric_limits<uint32_t>::max(); // Adding 1000 to the actual count
            file.write(reinterpret_cast<const char*>(&excessiveCount), sizeof(excessiveCount));
            file.close();
        }

        inline void createCorruptedHeaderInvalidChars(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Corrupted header with random invalid characters
            char header[80] = "CorruptedHeader12345!@#$%&*()";
            file.write(header, sizeof(header));

            // Write triangle count and triangles normally
            auto triangleCount = static_cast<uint32_t>(triangles.size());
            file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));
            for (const auto& tri : triangles) {
                file.write(reinterpret_cast<const char*>(&tri), sizeof(Triangle));
            }

            file.close();
        }

        inline void createBufferOverflowOnTriangleCount(const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Write only the header (80 bytes) and close the file
            char header[80] = "STL Exported by OpenSTL [https://github.com/Innoptech/OpenSTL]";
            file.write(header, sizeof(header));
            file.close();
        }

        inline void createStlWithTriangles(const std::vector<Triangle>& triangles, const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);

            // Write a valid header
            char header[80] = "STL Exported by Test";
            file.write(header, sizeof(header));

            // Write the number of triangles
            uint32_t triangleCount = static_cast<uint32_t>(triangles.size());
            file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));

            // Write the triangles
            file.write(reinterpret_cast<const char*>(triangles.data()), triangles.size() * sizeof(Triangle));

            file.close();
        }


        inline void createEmptyStlFile(const std::string& filename) {
            std::ofstream file(filename, std::ios::binary);
            // Simply create the file and close it without writing anything to it
            file.close();
        }

        // Custom equality operator for Vertex struct
        inline bool operator!=(const Vec3& rhs, const Vec3& lhs) {
            return std::tie(rhs.x, rhs.y, rhs.z) != std::tie(lhs.x, lhs.y, lhs.z);
        }

        // Utility function to compare two vectors of triangles
        inline bool checkTrianglesEqual(const std::vector<Triangle>& a, const std::vector<Triangle>& b, bool omit_attribute=false) {
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


    } //namespace testutils
} //namespace openstl
#endif //OPENSTL_TESTS_TESTUTILS_H
