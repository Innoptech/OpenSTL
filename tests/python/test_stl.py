import pytest, os
import numpy as np
import openstl


@pytest.fixture
def sample_triangles():
    triangle1 = openstl.Triangle([0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3], 3)
    triangle2 = openstl.Triangle([0, 0, 1], [4, 4, 4], [5, 5, 5], [6 ,6, 6], 5)
    return [triangle1, triangle2]

def test_get_vertices_empty():
    empty_triangles = []
    vertices = openstl.get_vertices(empty_triangles)
    assert vertices.shape == (0, 0, 3)  # Check that the shape of the returned vertices array is correct
    assert len(vertices) == 0  # Check that the array is empty

def test_get_vertices(sample_triangles):
    vertices = openstl.get_vertices(sample_triangles)
    assert vertices.shape == (len(sample_triangles),3, 3)
    assert np.allclose(vertices[0,0], [1, 1, 1])
    assert np.allclose(vertices[0,1], [2, 2, 2])
    assert np.allclose(vertices[-1,-1], [6, 6, 6])

def test_write_and_read(sample_triangles):
    filename = "test.stl"

    # Write triangles to file
    assert openstl.write(filename, sample_triangles)

    # Read triangles from file
    triangles_read = openstl.read(filename)
    assert len(triangles_read) == len(sample_triangles)
    for i in range(len(triangles_read)):
        assert np.allclose(triangles_read[i], sample_triangles[i]) # Will compare normal and vertices
        assert triangles_read[i].attribute_byte_count == sample_triangles[i].attribute_byte_count
    # Clean up
    os.remove(filename)

def test_fail_on_read():
    filename = "donoexist.stl"
    triangles_read = openstl.read(filename)
    assert len(triangles_read) == 0

if __name__ == "__main__":
    pytest.main()