import pytest, os
import numpy as np
import openstl


@pytest.fixture
def sample_triangles():
    triangle = np.array([[0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3]])
    return np.stack([triangle]*1000)

def test_get_vertices(sample_triangles):
    vertices = sample_triangles
    assert vertices.shape == (len(vertices),4,3)
    assert np.allclose(vertices[0,0], [0, 0, 1])
    assert np.allclose(vertices[0,1], [1, 1, 1])
    assert np.allclose(vertices[-1,-1], [3, 3, 3])

def test_write_and_read(sample_triangles):
    filename = "test.stl"
    
    # Write triangles to file
    assert openstl.write(filename, sample_triangles, openstl.format.ascii)

    # Read triangles from file
    triangles_read = openstl.read(filename)
    assert len(triangles_read) == len(sample_triangles)
    for i in range(len(triangles_read)):
        assert np.allclose(triangles_read[i], sample_triangles[i]) # Will compare normal and vertices

    # Clean up
    os.remove(filename)

def test_fail_on_read():
    filename = "donoexist.stl"
    triangles_read = openstl.read(filename)
    assert len(triangles_read) == 0

if __name__ == "__main__":
    pytest.main()