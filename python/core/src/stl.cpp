#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/iostream.h>
#include <memory>

#include "openstl/core/stl.h"
#include "openstl/core/version.h"

//-------------------------------------------------------------------------------
// PYTHON BINDINGS
//-------------------------------------------------------------------------------
namespace py = pybind11;
using namespace pybind11::literals;
using namespace openstl;


Vec3 makeVec(const py::array_t<float> &vertex)
{
    /* Request a buffer descriptor from Python */
    py::buffer_info info = vertex.request();

    if (info.ndim != 1)
        throw std::runtime_error("Vertex constructed with incompatible buffer dimension");

    if (info.shape[0] != 3)
        throw std::runtime_error("Vertex constructed with incompatible buffer size");

    auto ptr = static_cast<float*>(info.ptr);
    return Vec3{ptr[0], ptr[1], ptr[2]};
}

Vec3 makeVec(const py::list &vertex)
{
    if (vertex.size() != 3)
        throw std::runtime_error("Vertex constructed with incompatible list size");
    return Vec3{static_cast<float>(vertex[0].cast<float>()), static_cast<float>(vertex[1].cast<float>()),
                static_cast<float>(vertex[2].cast<float>())};
}

void stl(py::module_ &m) {

    py::class_<Vec3>(m, "Vec3", py::buffer_protocol())
            .def(py::init([](float x, float y, float z){
                return new Vec3{x,y,z};
            }), "x"_a, "y"_a, "z"_a)
            .def(py::init([](const py::array_t<float>& vertex){
                return Vec3{makeVec(vertex)};
            }))
            .def(py::init([](const py::list& vertex){
                return Vec3{makeVec(vertex)};
            }))
            .def_readwrite("x", &Vec3::x)
            .def_readwrite("y", &Vec3::y)
            .def_readwrite("z", &Vec3::z)
            .def_buffer([](Vec3 &m) -> py::buffer_info {
                return py::buffer_info(
                        &m.x,                                   /* Pointer to buffer */
                        sizeof(float),                          /* Size of one scalar */
                        py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                        1,                                      /* Number of dimensions */
                        {3},                                    /* Buffer dimensions */
                        {sizeof(float)}                         /* Strides (in bytes) for each index */

                );
            })
            .def("__len__",  [](const Vec3 &obj) {
                (void) obj;
                return 3;
            })
            .def("__getitem__",  [](const Vec3 &obj, size_t index) {
                return (&obj.x)[index];
            })
            .def("__repr__",
                 [](const Vec3 &obj) {
                     std::stringstream ss;
                     ss << "[" << obj.x << ", " << obj.y << ", " << obj.z << "]\n";
                     return ss.str();
                 }
            );

    py::class_<Triangle>(m, "Triangle", py::buffer_protocol())
            .def(py::init([](const Vec3& normal, const Vec3& v0, const Vec3& v1,
                    const Vec3& v2, uint16_t attribute_byte_count) {
                return Triangle{normal, v0, v1, v2, attribute_byte_count};
            }), "normal"_a, "v0"_a, "v1"_a, "v2"_a, "attribute_byte_count"_a)
            .def(py::init([](const py::array_t<float>& normal, const py::array_t<float>& v0, const py::array_t<float>& v1,
                             const py::array_t<float>& v2, uint16_t attribute_byte_count) {
                std::vector<Vec3> vertices{}; vertices.reserve(4);
                for (const auto& vertex : {normal, v0, v1, v2})
                    vertices.emplace_back(makeVec(vertex));
                return Triangle{vertices[0], vertices[1], vertices[2], vertices[3], attribute_byte_count};
            }), "normal"_a, "v0"_a, "v1"_a, "v2"_a, "attribute_byte_count"_a)
            .def(py::init([](const py::list& normal, const py::list& v0, const py::list& v1,
                             const py::list& v2, uint16_t attribute_byte_count) {
                std::vector<Vec3> vertices{}; vertices.reserve(4);
                for (const auto& vertex : {normal, v0, v1, v2})
                    vertices.emplace_back(makeVec(vertex));
                return Triangle{vertices[0], vertices[1], vertices[2], vertices[3], attribute_byte_count};
            }), "normal"_a, "v0"_a, "v1"_a, "v2"_a, "attribute_byte_count"_a)
            .def_readwrite("normal", &Triangle::normal)
            .def_readwrite("v0", &Triangle::normal)
            .def_readwrite("v1", &Triangle::normal)
            .def_readwrite("v2", &Triangle::normal)
            .def_readwrite("attribute_byte_count", &Triangle::attribute_byte_count)
            .def_buffer([](Triangle &m) -> py::buffer_info {
                return py::buffer_info(
                        &m.normal,
                        sizeof(float),
                        py::format_descriptor<float>::format(),
                        2,
                        { 3, 4 },
                        { sizeof(float) ,
                          sizeof(float)*3 }
                );
            })
            .def("__len__",  [](const Triangle &obj) {
                (void) obj;
                return 12;
            })
            .def("__getitem__",  [](const Triangle &obj, size_t index) {
                return (&obj.normal.x)[index];
            })
            .def("__repr__",
                 [](const Triangle &obj) {
                std::stringstream ss;
                ss << "Normal:\n" << "\t[" << obj.normal.x << ", " << obj.normal.y << ", " << obj.normal.z << "]\n";
                ss << "Vertices:\n";
                for(const auto vertex : {obj.v0, obj.v1, obj.v2})
                {
                    ss << "\t[" << vertex.x << ", " << vertex.y << ", " << vertex.z << "]\n";
                }
                ss  << "attribute_byte_count: " << obj.attribute_byte_count << "\n";
                return ss.str();
            }
            );

    m.def("get_vertices", [](const std::vector<Triangle>& triangles)
    {
        if (triangles.empty()) {
            // If the vector is empty, return an empty numpy array
            return py::array_t<float>({0,0,3});
        }
        return py::array_t<float>({static_cast<size_t>(triangles.size()),
                                   static_cast<size_t>(3), static_cast<size_t>(3)},
                                  {sizeof(Triangle), sizeof(Vec3), sizeof(float)},
                                   &triangles[0].v0.x);
    });
}

void serialize(py::module_ &m) {

    py::enum_<StlFormat>(m, "StlFormat")
            .value("ASCII", StlFormat::ASCII)
            .value("Binary", StlFormat::Binary);

    m.def("write", [](const std::string &filename, const std::vector<Triangle>& triangles,
            StlFormat format=openstl::StlFormat::Binary){
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
          std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
          return false;
        }
        openstl::serializeStl(triangles, file, format); // Or StlFormat::ASCII

        if (file.fail()) {
          std::cerr << "Error: Failed to write to file " << filename << std::endl;
        }
        file.close();
        return true;
        },"filename"_a, "triangles"_a, "StlFormat"_a=openstl::StlFormat::Binary, "Serialize a STL to a file");

    m.def("read", [](const std::string &filename){
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
            return std::vector<openstl::Triangle>{};
        }

        // Deserialize the triangles in either binary or ASCII format
        std::vector<openstl::Triangle> triangles = openstl::deserializeStl(file);
        file.close();
        return triangles;
        }, "filename"_a, "Deserialize a STl from a file", py::return_value_policy::move);
}

PYBIND11_MODULE(openstl, m) {
    stl(m);
    serialize(m);
    m.attr("__version__") = OPENSTL_PROJECT_VER;
    m.doc() = "A simple STL serializer and deserializer";
}