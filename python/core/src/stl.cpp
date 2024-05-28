#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
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

/**
 * @brief Template class for creating a strided span over a contiguous sequence of memory.
 *
 * This class provides a strided view over a contiguous sequence of memory, allowing iteration
 * over elements with a specified stride.
 *
 * @tparam VALUETYPE The type of elements stored in the span.
 * @tparam SIZE The stride size (number of elements to skip between each element).
 * @tparam PTRTYPE The type of the pointer to the underlying data.
 */
template<typename VALUETYPE, size_t SIZE, typename PTRTYPE>
class StridedSpan {
    // Iterator type for iterating over elements with stride
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = VALUETYPE;
        using pointer = VALUETYPE*;
        using reference = VALUETYPE&;

        explicit Iterator(const PTRTYPE* ptr) : ptr(ptr) {}

        const VALUETYPE& operator*() const { return *reinterpret_cast<const VALUETYPE*>(ptr); }
        const VALUETYPE* operator->() const { return reinterpret_cast<const VALUETYPE*>(ptr); }

        Iterator& operator++() {
            ptr += SIZE;
            return *this;
        }

        // Equality operator
        bool operator==(const Iterator& other) const { return ptr == other.ptr; }

        // Inequality operator
        bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        const PTRTYPE* ptr;
    };

    const PTRTYPE* data_;
    size_t size_;
public:
    StridedSpan(const PTRTYPE* data, size_t size) : data_(data), size_(size) {}

    Iterator begin() const { return Iterator{data_}; }
    Iterator end() const { return Iterator{data_ + size_ * SIZE}; }
    size_t size() const {return size_;}
    const PTRTYPE* data() const {return data_;}
};


namespace pybind11 { namespace detail {
    template <> struct type_caster<std::vector<Triangle>> {
    public:
    PYBIND11_TYPE_CASTER(std::vector<Triangle>, _("TrianglesArray"));

        bool load(handle src, bool convert)
        {
            if ( (!convert) && (!py::array_t<float, py::array::c_style | py::array::forcecast>::check_(src)) )
                return false;

            auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(src);
            if(!buf)
                return false;

            if (buf.ndim() != 3 || buf.shape(1) != 4 || buf.shape(2) != 3)
                return false;

            std::vector<Triangle> triangles{}; triangles.reserve(buf.shape(0));
            StridedSpan<Triangle, 12, float> stridedIter{buf.data(), (size_t)buf.shape(0)};
            std::copy(std::begin(stridedIter), std::end(stridedIter),
                      std::back_inserter(triangles));

            value = triangles;
            return true;
        }

        static handle cast(const std::vector<Triangle>& src, return_value_policy /*policy*/, handle /* parent */) {
            py::array_t<float, py::array::c_style> array(
                    {static_cast<size_t>(src.size()),
                     static_cast<size_t>(4), static_cast<size_t>(3)},
                    {sizeof(Triangle), sizeof(Vec3), sizeof(float)},
                    (float*)src.data());
            return array.release();
        }
    };
}} // namespace pybind11::detail



void serialize(py::module_ &m) {

    py::enum_<StlFormat>(m, "format")
            .value("ascii", StlFormat::ASCII)
            .value("binary", StlFormat::Binary)
            .export_values();

    m.def("write", [](const std::string &filename,
            const py::array_t<float, py::array::c_style | py::array::forcecast> &array,
            StlFormat format=openstl::StlFormat::Binary){
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << filename << "'." << std::endl;
            return false;
        }

        auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(array);
        if(!buf)
            return false;

        if (buf.ndim() != 3 || buf.shape(1) != 4 || buf.shape(2) != 3)
            return false;

        StridedSpan<Triangle, 12, float> stridedIter{buf.data(), (size_t)buf.shape(0)};
        openstl::serialize(stridedIter, file, format);

        if (file.fail()) {
            std::cerr << "Error: Failed to write to file '" << filename << "'." << std::endl;
        }
        file.close();
        return true;
    },"filename"_a, "triangles"_a, "StlFormat"_a=openstl::StlFormat::Binary, "Serialize a STL to a file");

    m.def("read", [](const std::string &filename) {
        py::scoped_ostream_redirect stream(std::cerr, py::module_::import("sys").attr("stderr"));
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << filename << "'." << std::endl;
            return std::vector<openstl::Triangle>{};
        }

        // Deserialize the triangles in either binary or ASCII format
        return openstl::deserializeStl(file);
    }, "filename"_a, "Deserialize a STl from a file", py::return_value_policy::move);
}


namespace openstl
{
    enum class Convert { VERTICES_AND_FACES=0, TRIANGLES};
};

void convertSubmodule(py::module_ &_m)
{
    auto m = _m.def_submodule("convert", "A submodule to convert mesh representations");

    m.def("verticesandfaces", [](
            const py::array_t<float, py::array::c_style | py::array::forcecast> &array
    )
            -> std::tuple<py::array_t<float, py::array::c_style>,
                    py::array_t<size_t, py::array::c_style>>
    {
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(array);
        if(!buf){
            std::cerr << "Input array cannot be interpreted as a mesh.\n";
            return {};
        }
        if (buf.ndim() != 3 || buf.shape(1) != 4 || buf.shape(2) != 3){
            std::cerr << "Input array cannot be interpreted as a mesh.\n";
            return {};
        }

        StridedSpan<Triangle, 12, float> stridedIter{buf.data(), (size_t)buf.shape(0)};
        const auto& verticesAndFaces = convertToVerticesAndFaces(stridedIter);
        const auto& vertices = std::get<0>(verticesAndFaces);
        const auto& faces = std::get<1>(verticesAndFaces);

        return std::make_tuple(
                py::array_t<float, py::array::c_style>(
                        {static_cast<size_t>(vertices.size()),static_cast<size_t>(3)},
                        {sizeof(Vec3), sizeof(float)},
                        (const float*)vertices.data()),
                py::array_t<size_t, py::array::c_style>(
                        {static_cast<size_t>(faces.size()),static_cast<size_t>(3)},
                        {sizeof(Face), sizeof(size_t)},
                        (const size_t*)faces.data())
        );
    }, "triangles"_a, "Convert the mesh to a format 'vertices-and-face-indices'");


    m.def("triangles", [](
            const py::array_t<float, py::array::c_style | py::array::forcecast> &vertices,
            const py::array_t<size_t, py::array::c_style | py::array::forcecast> &faces
    ) -> std::vector<Triangle>
    {
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        auto vbuf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(vertices);
        if(!vbuf){
            std::cerr << "Vertices input array cannot be interpreted as a mesh.\n";
            return {};
        }
        if (vbuf.ndim() != 2 || vbuf.shape(1) != 3){
            std::cerr << "Vertices input array cannot be interpreted as a mesh. Shape must be N x 3.\n";
            return {};
        }

        auto fbuf = py::array_t<size_t , py::array::c_style | py::array::forcecast>::ensure(faces);
        if(!fbuf){
            std::cerr << "Faces input array cannot be interpreted as a mesh.\n";
            return {};
        }
        if (fbuf.ndim() != 2 || vbuf.shape(1) != 3){
            std::cerr << "Faces input array cannot be interpreted as a mesh.\n";
            std::cerr << "Shape must be N x 3 (v0, v1, v2).\n";
            return {};
        }

        StridedSpan<Vec3,3, float> verticesIter{vbuf.data(), (size_t)vbuf.shape(0)};
        StridedSpan<Face,3,size_t> facesIter{fbuf.data(), (size_t)fbuf.shape(0)};
        return convertToTriangles(verticesIter, facesIter);
    }, "vertices"_a,"faces"_a, "Convert the mesh from vertices and faces to triangles");
}

PYBIND11_MODULE(openstl, m) {
    serialize(m);
    convertSubmodule(m);
    m.attr("__version__") = OPENSTL_PROJECT_VER;
    m.doc() = "A simple STL serializer and deserializer";

    PYBIND11_NUMPY_DTYPE(Vec3, x, y, z);
    PYBIND11_NUMPY_DTYPE(Triangle, normal, v0, v1, v2, attribute_byte_count);
}