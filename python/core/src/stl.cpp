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


// Custom stride container for py::array_t<float, c_style | forcecast> -> Iterator of Triangles
class StridedTriangleSpan {
    // Iterator type for iterating over elements with stride
    class Iterator {
    public:
        // Define iterator category
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Triangle;
        using pointer = Triangle*;
        using reference = Triangle&;

        explicit Iterator(const float* ptr) : ptr(ptr) {}

        // Dereference operator
        const Triangle& operator*() const { return *reinterpret_cast<const Triangle*>(ptr); }
        const Triangle* operator->() const { return reinterpret_cast<const Triangle*>(ptr); }

        // Increment operator
        Iterator& operator++() {
            ptr += 12;
            return *this;
        }

        // Equality operator
        bool operator==(const Iterator& other) const { return ptr == other.ptr; }

        // Inequality operator
        bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        const float* ptr;
    };

    const float* data_;
    size_t size_;
public:

    // Constructor takes a pointer to the data, the number of elements, and the stride
    StridedTriangleSpan(const float* data, size_t size) : data_(data), size_(size) {}

    // Begin iterator
    Iterator begin() const { return Iterator{data_}; }

    // End iterator
    Iterator end() const { return Iterator{data_ + size_ * 12}; }

    size_t size() const {return size_;}
};


namespace pybind11 { namespace detail {
    template <> struct type_caster<std::vector<Triangle>> {
    public:
    PYBIND11_TYPE_CASTER(std::vector<Triangle>, _("TrianglesArray"));

        bool load(handle src, bool convert)
        {
            if ( !convert and !py::array_t<float, py::array::c_style | py::array::forcecast>::check_(src) )
                return false;

            auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(src);
            if(!buf)
                return false;

            if (buf.ndim() != 3 || buf.shape(1) != 4 || buf.shape(2) != 3)
                return false;

            std::vector<Triangle> triangles{}; triangles.reserve(buf.shape(0));
            StridedTriangleSpan stridedIter{buf.data(), (size_t)buf.shape(0)};
            std::copy(std::begin(stridedIter), std::end(stridedIter),
                      std::back_inserter(triangles));

            value = triangles;
            return true;
        }

        static handle cast(const std::vector<Triangle>& src, return_value_policy /*policy*/, handle /* parent */) {
            py::array_t<float, py::array::c_style | py::array::forcecast> array(
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
            .export_values();;

    m.def("write", [](const std::string &filename,
            const py::array_t<float, py::array::c_style | py::array::forcecast> &array,
            StlFormat format=openstl::StlFormat::Binary){
        py::scoped_ostream_redirect stream(std::cerr,py::module_::import("sys").attr("stderr"));
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
            return false;
        }

        auto buf = py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(array);
        if(!buf)
            return false;

        if (buf.ndim() != 3 || buf.shape(1) != 4 || buf.shape(2) != 3)
            return false;

        StridedTriangleSpan stridedIter{buf.data(), (size_t)buf.shape(0)};
        openstl::serialize(stridedIter, file, format);

        if (file.fail()) {
            std::cerr << "Error: Failed to write to file " << filename << std::endl;
        }
        file.close();
        return true;
    },"filename"_a, "triangles"_a, "StlFormat"_a=openstl::StlFormat::Binary, "Serialize a STL to a file");

    m.def("read", [](const std::string &filename) {
        py::scoped_ostream_redirect stream(std::cerr, py::module_::import("sys").attr("stderr"));
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
            return std::vector<openstl::Triangle>{};
        }

        // Deserialize the triangles in either binary or ASCII format
        return openstl::deserializeStl(file);
    }, "filename"_a, "Deserialize a STl from a file", py::return_value_policy::move);
}

PYBIND11_MODULE(openstl, m) {
    //stl(m);
    serialize(m);
    m.attr("__version__") = OPENSTL_PROJECT_VER;
    m.doc() = "A simple STL serializer and deserializer";

    PYBIND11_NUMPY_DTYPE(Vec3, x, y, z);
    PYBIND11_NUMPY_DTYPE(Triangle, normal, v0, v1, v2, attribute_byte_count);
}