import pytest, os
import numpy as np
import openstl
import gc

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
    gc.disable()
    filename = "test.stl"
    
    # Write triangles to file
    assert openstl.write(filename, sample_triangles, openstl.format.ascii)

    # Read triangles from file
    triangles_read = openstl.read(filename)
    gc.collect()
    assert len(triangles_read) == len(sample_triangles)
    for i in range(len(triangles_read)):
        assert np.allclose(triangles_read[i], sample_triangles[i]) # Will compare normal and vertices

    # Clean up
    os.remove(filename)

def test_fail_on_read():
    filename = "donoexist.stl"
    triangles_read = openstl.read(filename)
    assert len(triangles_read) == 0


# Define Face and Vec3 as tuples
Face = tuple[int, int, int]  # v0, v1, v2
Vec3 = tuple[float, float, float]

def are_all_unique(arr: list) -> bool:
    """Check if all elements in the array are unique."""
    seen = set()
    for element in arr:
        if element in seen:
            return False
        seen.add(element)
    return True


def are_faces_equal(face1: Face, face2: Face, v1: list[Vec3], v2: list[Vec3]) -> bool:
    """Check if two Face objects are equal."""
    # Vertices v0, v1, v2 can be shuffled between two equal faces
    assert len(np.unique(face1)) == len(np.unique(face2))
    for i in face1:
        if not any((v1[i] == v2[j]).all() for j in face2):
            return False
    return True


def all_faces_valid(faces: list[Face], final_faces: list[Face], vertices: list[Vec3], final_vertices: list[Vec3]) -> bool:
    """Check if all original faces are present in the final faces."""
    return all(any(are_faces_equal(face, final_f, vertices, final_vertices) for final_f in final_faces) for face in faces)


@pytest.fixture
def sample_vertices_and_faces():
    # Define vertices and faces
    vertices = np.array([
        [0.0, 0.0, 0.0],
        [1.0, 1.0, 1.0],
        [2.0, 2.0, 2.0],
        [3.0, 3.0, 3.0],
    ])
    faces = np.array([
        [0, 1, 2],  # Face 1 
        [1, 3, 2]   # Face 2 
    ])
    return vertices, faces


def test_convert_to_vertices_and_faces_on_empty():
    empty_triangles = np.array([[]])
    vertices, faces = openstl.convert.verticesandfaces(empty_triangles)
    # Test if vertices and faces are empty
    assert len(vertices) == 0
    assert len(faces) == 0

def test_convert_to_vertices_and_faces(sample_triangles):
    vertices, faces = openstl.convert.verticesandfaces(sample_triangles)
    # Convert vertices to tuples to make them hashable
    vertices = [tuple(vertex) for vertex in vertices]

    # Test if vertices and faces are extracted correctly
    assert len(vertices) == 3
    assert len(faces) == 1000

    # Test if each face contains three indices
    for face in faces:
        assert len(face) == 3

    # Test for uniqueness of vertices
    unique_vertices = set(vertices)
    assert len(unique_vertices) == len(vertices)

    # Test if all indices in faces are valid
    for face in faces:
        for vertex_idx in face:
            assert vertex_idx >= 0
            assert vertex_idx < len(vertices)


def test_convertToVerticesAndFaces_integration(sample_vertices_and_faces):
    # Extract vertices and faces
    vertices, faces = sample_vertices_and_faces

    # Convert vertices and faces to triangles
    triangles = openstl.convert.triangles(vertices, faces)

    # Convert triangles back to vertices and faces
    result_vertices, result_faces = openstl.convert.verticesandfaces(triangles)

    # Check if the number of vertices and faces are preserved
    assert len(vertices) == len(result_vertices)
    assert len(faces) == len(result_faces)

    # Check if each vertices are preserved. 
    found_set: list[int] = []
    for i, result_vertex in enumerate(result_vertices):
        for ref_vertex in vertices:
            if (ref_vertex == result_vertex).all():
                found_set.append(i)
                break
    assert len(found_set) == result_vertices.shape[0]

    # Check if each face is correctly preserved
    for face, result_face in zip(faces, result_faces):
        assert are_faces_equal(face, result_face, vertices, result_vertices)

if __name__ == "__main__":
    pytest.main()