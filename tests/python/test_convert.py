import pytest, os
import numpy as np
import typing
import openstl

from .testutils import sample_triangles, are_faces_equal

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