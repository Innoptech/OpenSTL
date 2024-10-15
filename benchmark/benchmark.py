import openstl
import torch
import numpy as np
import meshio
import timeit
from stl import mesh, Mode
import stl_reader
import matplotlib.pyplot as plt

#-----------------------------------------------------
# openstl
#-----------------------------------------------------
def create_triangles(num_triangles):
    triangles = []
    tri = np.array([[0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3]])
    return np.tile(tri[np.newaxis,:,:], (num_triangles,1,1))

def benchmark_write_openstl(num_triangles, filename):
    triangles = create_triangles(num_triangles)
    result = timeit.timeit(lambda: openstl.write(filename, triangles, openstl.format.binary), number=5)
    return result

def benchmark_read_openstl(filename):
    result = timeit.timeit(lambda: openstl.read(filename), number=5)
    return result

def benchmark_rotate_openstl(num_triangles):
    triangles = create_triangles(num_triangles)
    cos, sin = np.cos(np.pi/3), np.sin(np.pi/3)
    matrix = np.array([[cos, -sin, 0],[sin, cos, 0], [0,0,1]]) # rotation of pi/3 around z axis
    result = timeit.timeit(lambda: np.matmul(matrix, triangles.reshape(-1,3).T), number=5)
    return result

def benchmark_rotate_openstl_torch(num_triangles):
    triangles = torch.Tensor(create_triangles(num_triangles)).to('cuda')
    cos, sin = np.cos(np.pi/3), np.sin(np.pi/3)
    matrix = torch.Tensor([[cos, -sin, 0],[sin, cos, 0], [0,0,1]]).to('cuda') # rotation of pi/3 around z axis
    result = timeit.timeit(lambda: torch.matmul(matrix, triangles.reshape(-1,3).T), number=5)
    return result

#-----------------------------------------------------
# nympy-stl
#-----------------------------------------------------
def create_nympystl(num_triangles):
    triangles = np.zeros(num_triangles, dtype=mesh.Mesh.dtype) 
    mesh_data = mesh.Mesh(triangles)
    return mesh_data

def benchmark_read_numpy_stl(filename):
    result = timeit.timeit(lambda: mesh.Mesh.from_file(filename, mode=Mode.BINARY), number=5)
    return result

def benchmark_write_numpy_stl(num_triangles, filename):
    mesh_data = create_nympystl(num_triangles)
    result = timeit.timeit(lambda: mesh_data.save(filename, mode=Mode.BINARY), number=5)
    return result

def benchmark_rotate_openstl_numpy(num_triangles):
    mesh_data = create_nympystl(num_triangles)
    result = timeit.timeit(lambda: mesh_data.rotate([0.0, 0.0, 1.0], np.pi/3), number=5)
    return result

#-----------------------------------------------------
# meshio
#-----------------------------------------------------
def create_meshio_triangles(num_triangles):
    points = np.array([[1, 1, 1], [2, 2, 2], [3, 3, 3]])
    cells = [("triangle", np.tile([0,1,2], (num_triangles, 1)))]
    return meshio.Mesh(points, cells)

def benchmark_write_meshio(num_triangles, filename):
    mesh = create_meshio_triangles(num_triangles)
    result = timeit.timeit(lambda: meshio.write(filename, mesh, "stl"), number=5)
    return result

def benchmark_read_meshio(filename):
    result = timeit.timeit(lambda: meshio.read(filename), number=5)
    return result

#-----------------------------------------------------
# stl-reader
#-----------------------------------------------------
def benchmark_read_stl_reader(filename):
    result = timeit.timeit(lambda: stl_reader.read(filename), number=5)
    return result

#-----------------------------------------------------
# benchmark utils
#-----------------------------------------------------
def benchmark_library(num_triangles_list, write_func, read_func, rotate_func=None, library_name="Library"):
    write_times = []
    read_times = []
    rotate_times = [] if rotate_func else None

    # Warm-up to exclude initialization overhead
    write_func(1000, "warmup.stl")
    read_func("warmup.stl")

    for num_triangles in num_triangles_list:
        write_times.append(write_func(num_triangles, f"benchmark_{num_triangles}.stl"))

    for num_triangles in num_triangles_list:
        read_times.append(read_func(f"benchmark_{num_triangles}.stl"))

    if rotate_func:
        for num_triangles in num_triangles_list:
            rotate_times.append(rotate_func(num_triangles))

    return write_times, read_times, rotate_times

def calculate_speedup(openstl_times, other_times):
    speedup = np.array(other_times) / np.array(openstl_times)
    return speedup.round(3)

def display_speedup_results(name, write_speedup, read_speedup, rotate_speedup=None):
    print(f"Write:\tOpenSTL is {write_speedup.min()} to {write_speedup.max()} X faster than {name}")
    print(f"Read:\tOpenSTL is {read_speedup.min()} to {read_speedup.max()} X faster than {name}")
    if rotate_speedup is not None:
        print(f"Rotate:\tOpenSTL is {rotate_speedup.min()} to {rotate_speedup.max()} X faster than {name}")

def plot_benchmark_results(num_triangles_list, results, labels, filename="benchmark.png"):
    plt.figure(figsize=(10, 6))

    for result, label, style in results:
        plt.plot(num_triangles_list, result, label=label, **style)

    plt.xlabel("Number of Triangles", fontsize=12)
    plt.ylabel("Time (seconds)", fontsize=12)
    plt.title("Python Benchmark Results", fontsize=14)
    plt.xscale("log")
    plt.yscale("log")
    plt.legend(fontsize=10, handlelength=5, loc='upper left', ncol=3)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()
    plt.xlim(num_triangles_list[0], num_triangles_list[-1] * 1.1)
    plt.savefig(filename)

if __name__ == "__main__":
    filename = "benchmark.stl"
    num_triangles_list = np.logspace(1, 5, 15).round().astype(int)

    # OpenSTL Benchmark
    openstl_write, openstl_read, openstl_rotate = benchmark_library(num_triangles_list, benchmark_write_openstl, benchmark_read_openstl, benchmark_rotate_openstl, "OpenSTL")

    # OpenSTL + PyTorch Benchmark (if available)
    if torch.cuda.is_available():
        _, _, openstl_rotate_torch = benchmark_library(num_triangles_list, lambda *args: None, lambda *args: None, benchmark_rotate_openstl_torch, "OpenSTL + PyTorch")

    # Numpy STL Benchmark
    numpy_write, numpy_read, numpy_rotate = benchmark_library(num_triangles_list, benchmark_write_numpy_stl, benchmark_read_numpy_stl, benchmark_rotate_openstl_numpy, "Numpy STL")

    # Meshio Benchmark
    meshio_write, meshio_read, _ = benchmark_library(num_triangles_list, benchmark_write_meshio, benchmark_read_meshio, library_name="Meshio")

    # STL Reader Benchmark (Read-only)
    _, stl_reader_read, _ = benchmark_library(num_triangles_list, benchmark_write_openstl, benchmark_read_stl_reader, library_name="STL Reader")

    # Calculate and display speedup
    write_speedup_np = calculate_speedup(openstl_write, numpy_write)
    read_speedup_np = calculate_speedup(openstl_read, numpy_read)
    rotate_speedup_np = calculate_speedup(openstl_rotate, numpy_rotate)

    display_speedup_results("numpy-stl", write_speedup_np, read_speedup_np, rotate_speedup_np)

    if torch.cuda.is_available():
        rotate_speedup_torch = calculate_speedup(openstl_rotate_torch, numpy_rotate)
        print(f"Rotate:\tOpenSTL + PyTorch is {rotate_speedup_torch.min()} to {rotate_speedup_torch.max()} X faster than numpy-stl")

    write_speedup_meshio = calculate_speedup(openstl_write, meshio_write)
    read_speedup_meshio = calculate_speedup(openstl_read, meshio_read)

    display_speedup_results("meshio", write_speedup_meshio, read_speedup_meshio)

    read_speedup_stl_reader = calculate_speedup(openstl_read, stl_reader_read)
    print(f"Read:\tOpenSTL is {read_speedup_stl_reader.min()} to {read_speedup_stl_reader.max()} X faster than stl_reader")

    # Plot results
    styles = [
        (openstl_write, "Write (OpenSTL)", {"color": "green", "linestyle": "-", "marker": "s", "markersize": 7, "linewidth": 3}),
        (openstl_read, "Read (OpenSTL)", {"color": "blue", "linestyle": "-", "marker": "s", "markersize": 7, "linewidth": 3}),
        (openstl_rotate, "Rotate (OpenSTL)", {"color": "red", "linestyle": "-", "marker": "s", "markersize": 7, "linewidth": 3})
    ]

    if torch.cuda.is_available():
        styles.append((openstl_rotate_torch, "Rotate (OpenSTL + PyTorch)", {"color": "purple", "linestyle": "-", "marker": "s", "markersize": 7, "linewidth": 3}))

    styles += [
        (numpy_write, "Write (numpy-stl)", {"color": "green", "linestyle": "--", "marker": "o", "markersize": 5, "alpha": 0.5}),
        (numpy_read, "Read (numpy-stl)", {"color": "blue", "linestyle": "--", "marker": "o", "markersize": 5, "alpha": 0.5}),
        (numpy_rotate, "Rotate (numpy-stl)", {"color": "red", "linestyle": "--", "marker": "o", "markersize": 5, "alpha": 0.5}),
        (meshio_write, "Write (meshio)", {"color": "green", "linestyle": ":", "marker": "^", "markersize": 5, "alpha": 0.5}),
        (meshio_read, "Read (meshio)", {"color": "blue", "linestyle": ":", "marker": "^", "markersize": 5, "alpha": 0.5}),
        (stl_reader_read, "Read (stl_reader)", {"color": "blue", "linestyle": "-.", "marker": "x", "markersize": 5, "alpha": 0.5}),
    ]

    plot_benchmark_results(num_triangles_list, styles, "benchmark.png")