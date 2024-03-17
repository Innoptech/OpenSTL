import openstl
import numpy as np
import timeit
from stl import mesh, Mode
import matplotlib.pyplot as plt

def create_triangles(num_triangles):
    triangles = []
    tri = np.array([[0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3]]) # normal, vertice 1,2,3
    for _ in range(num_triangles):
        triangles.append(tri)
    return np.array(triangles)

def benchmark_write(num_triangles, filename):
    triangles = create_triangles(num_triangles)
    result = timeit.timeit(lambda: openstl.write(filename, triangles, openstl.format.binary), number=5)
    return result

def benchmark_read(filename):
    result = timeit.timeit(lambda: openstl.read(filename), number=5)
    return result

def benchmark_rotate(num_triangles):
    triangles = create_triangles(num_triangles)
    cos, sin = np.cos(np.pi/3), np.sin(np.pi/3)
    matrix = np.array([[cos, -sin, 0],[sin, cos, 0], [0,0,1]]) # rotation of pi/3 around z axis
    result = timeit.timeit(lambda: np.matmul(matrix, triangles.reshape(-1,3).T), number=5)
    return result

def create_nympystl(num_triangles):
    triangles = np.zeros(num_triangles, dtype=mesh.Mesh.dtype) 
    mesh_data = mesh.Mesh(triangles)
    return mesh_data

def benchmark_read_numpy_stl(filename):
    result = timeit.timeit(lambda: mesh.Mesh.from_file(filename, mode=Mode.BINARY), number=5)
    return result

def benchmark_write_numpy_stl(num_triangles, filename):
    mesh_data = create_nympystl(num_triangles)
    result = timeit.timeit(lambda: mesh_data.save(filename), number=5)
    return result

def benchmark_rotate_numpy(num_triangles):
    mesh_data = create_nympystl(num_triangles)
    result = timeit.timeit(lambda: mesh_data.rotate([0.0, 0.0, 1.0], np.pi/3), number=5)
    return result

if __name__ == "__main__":
    # Benchmark parameters
    filename = "benchmark.stl"
    num_triangles_list = np.logspace(1,7, 20).round().astype(int)

    # Benchmarking
    write_times_openstl = []
    read_times_openstl = []
    rotate_openstl = []
    read_numpy_stl_times = []
    write_numpy_stl_times = []
    rotate_numpy_stl = []

    # Exclude 1rst operation (openstl dyn lib loading)
    write_time = benchmark_write(1, filename)

    for num_triangles in num_triangles_list:
        write_time = benchmark_write(num_triangles, filename)
        read_time = benchmark_read(filename)
        rotate_time = benchmark_rotate(num_triangles)
        write_times_openstl.append(write_time)
        read_times_openstl.append(read_time)
        rotate_openstl.append(rotate_time)

        write_time = benchmark_write_numpy_stl(num_triangles, filename)
        read_time = benchmark_read_numpy_stl(filename)
        rotate_time = benchmark_rotate_numpy(num_triangles)
        read_numpy_stl_times.append(read_time)
        write_numpy_stl_times.append(write_time)
        rotate_numpy_stl.append(rotate_time)

    # Results
    write_diff = (np.array(write_numpy_stl_times) / np.array(write_times_openstl)).round(3)
    read_diff =  (np.array(read_numpy_stl_times) / np.array(read_times_openstl)).round(3)
    rotate_diff = (np.array(rotate_numpy_stl) / np.array(rotate_openstl)).round(3)
    print(f"Write:\tOpenSTL is { np.min(write_diff)} to {np.max(write_diff)} X faster than numpy-stl")
    print(f"Read:\tOpenSTL is {np.min(read_diff)} to {np.max(read_diff)} X faster than numpy-stl")
    print(f"Rotate:\tOpenSTL is {np.min(rotate_diff)} to {np.max(rotate_diff)} X faster than numpy-stl")

    # Plotting
    plt.figure(figsize=(10, 6))  # Set figure size for better visualization
    plt.plot(num_triangles_list, write_times_openstl, label="Write (OpenSTL)", color="green", linestyle="-", marker="s", markersize=5)
    plt.plot(num_triangles_list, read_times_openstl, label="Read (OpenSTL)", color="blue", linestyle="-", marker="s", markersize=5)
    plt.plot(num_triangles_list, rotate_openstl, label="Rotate (OpenSTL)", color="red", linestyle="-", marker="s", markersize=5)
    plt.plot(num_triangles_list, write_numpy_stl_times, label="Write (numpy-stl)", color="green", linestyle="--", marker="o", markersize=5)
    plt.plot(num_triangles_list, read_numpy_stl_times, label="Read (numpy-stl)", color="blue", linestyle="--", marker="o", markersize=5)
    plt.plot(num_triangles_list, rotate_numpy_stl, label="Rotate (numpy-stl)", color="red", linestyle="--", marker="o", markersize=5)

    plt.xlabel("Number of Triangles", fontsize=12)
    plt.ylabel("Time (seconds)", fontsize=12)
    plt.title("Python Benchmark Results", fontsize=14)
    plt.xscale("log")
    plt.yscale("log")
    plt.legend(fontsize=10, handlelength=5)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)  # Add grid lines with dashed style
    plt.tight_layout()  # Adjust layout to prevent clipping of labels

    # Adjusting axes limits and scaling
    plt.xlim(num_triangles_list[0], num_triangles_list[-1]*1.1)  # Extend x-axis range by 10%
    plt.ylim(min(min(write_times_openstl), min(read_times_openstl), min(read_numpy_stl_times), min(write_numpy_stl_times)) * 0.5,
             max(max(write_times_openstl), max(read_times_openstl), max(read_numpy_stl_times), max(write_numpy_stl_times)) * 2)  # Expand y-axis range

    plt.show()