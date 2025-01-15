import typing
import numpy as np
import pytest

# Define Face and Vec3 as tuples
Face = typing.Tuple[int, int, int]  # v0, v1, v2
Vec3 = typing.Tuple[float, float, float]

@pytest.fixture
def sample_triangles():
    triangle = np.array([[0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3]])
    return np.stack([triangle]*1000)

def are_all_unique(arr: list) -> bool:
    """Check if all elements in the array are unique."""
    seen = set()
    for element in arr:
        if element in seen:
            return False
        seen.add(element)
    return True


def are_faces_equal(face1: Face, face2: Face, v1: typing.List[Vec3], v2: typing.List[Vec3]) -> bool:
    """Check if two Face objects are equal."""
    # Vertices v0, v1, v2 can be shuffled between two equal faces
    assert len(np.unique(face1)) == len(np.unique(face2))
    for i in face1:
        if not any((v1[i] == v2[j]).all() for j in face2):
            return False
    return True


def all_faces_valid(faces: typing.List[Face], final_faces: typing.List[Face],
                    vertices: typing.List[Vec3], final_vertices: typing.List[Vec3]) -> bool:
    """Check if all original faces are present in the final faces."""
    return all(any(are_faces_equal(face, final_f, vertices, final_vertices) for final_f in final_faces) for face in faces)