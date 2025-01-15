import numpy as np
import pytest
from openstl.topology import find_connected_components

@pytest.fixture
def sample_vertices_and_faces():
    vertices = np.array([
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
        [1.0, 1.0, 0.0],
        [0.5, 0.5, 1.0],
    ])
    faces = np.array([
        [0, 1, 2],
        [1, 3, 2],
        [2, 3, 4],
    ])
    return vertices, faces


def test_single_connected_component(sample_vertices_and_faces):
    vertices, faces = sample_vertices_and_faces
    connected_components = find_connected_components(vertices, faces)

    # Expect one connected component containing all faces
    assert len(connected_components) == 1
    assert len(connected_components[0]) == 3


def test_multiple_disconnected_components(sample_vertices_and_faces):
    vertices, faces = sample_vertices_and_faces
    # Add disconnected component
    faces = np.vstack([faces, [5, 6, 7]])
    vertices = np.vstack([
        vertices,
        [2.0, 2.0, 0.0],
        [3.0, 2.0, 0.0],
        [2.5, 3.0, 0.0]
    ])

    connected_components = find_connected_components(vertices, faces)

    # Expect two connected components
    assert len(connected_components) == 2
    assert len(connected_components[0]) == 3
    assert len(connected_components[1]) == 1


def test_no_faces():
    vertices = np.array([
        [0.0, 0.0, 0.0],
        [1.0, 1.0, 1.0],
    ])
    faces = np.array([]).reshape(0, 3)

    connected_components = find_connected_components(vertices, faces)

    # Expect no connected components
    assert len(connected_components) == 0


def test_single_face():
    vertices = np.array([
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
    ])
    faces = np.array([[0, 1, 2]])

    connected_components = find_connected_components(vertices, faces)

    # Expect one connected component with one face
    assert len(connected_components) == 1
    assert len(connected_components[0]) == 1
    assert np.array_equal(connected_components[0][0], [0, 1, 2])


def test_disconnected_vertices(sample_vertices_and_faces):
    vertices, faces = sample_vertices_and_faces
    vertices = np.vstack([vertices, [10.0, 10.0, 10.0]])  # Add disconnected vertex

    connected_components = find_connected_components(vertices, faces)

    # Expect one connected component (disconnected vertex ignored)
    assert len(connected_components) == 1
    assert len(connected_components[0]) == 3  # Only faces contribute
