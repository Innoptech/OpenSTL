import openstl
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

def benchmark_rotate_numpy(num_triangles):
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


if __name__ == "__main__":
    # Benchmark parameters
    filename = "benchmark.stl"
    num_triangles_list = np.logspace(1,5,15).round().astype(int)

    # Benchmarking
    write_times_openstl = []
    read_times_openstl = []
    rotate_openstl = []
    read_numpy_stl_times = []
    write_numpy_stl_times = []
    rotate_numpy_stl = []
    read_meshio_times = []
    write_meshio_times = []
    read_stl_reader_times = []

    # Exclude 1rst operation (openstl dyn lib loading)
    write_time = benchmark_write(1, filename)
    read_time = benchmark_read_stl_reader(filename)

    for num_triangles in num_triangles_list:
        write_time = benchmark_write(num_triangles, filename)
        read_time = benchmark_read(filename)
        rotate_time = benchmark_rotate(num_triangles)
        write_times_openstl.append(write_time)
        read_times_openstl.append(read_time)
        rotate_openstl.append(rotate_time)

        read_time = benchmark_read_stl_reader(filename)
        read_stl_reader_times.append(read_time)


    # Exclude 1rst operation (openstl dyn lib loading)
    write_time = benchmark_write_numpy_stl(1, filename)

    for num_triangles in num_triangles_list:
        write_time = benchmark_write_numpy_stl(num_triangles, filename)
        read_time = benchmark_read_numpy_stl(filename)
        rotate_time = benchmark_rotate_numpy(num_triangles)
        read_numpy_stl_times.append(read_time)
        write_numpy_stl_times.append(write_time)
        rotate_numpy_stl.append(rotate_time)


    # Exclude 1rst operation (openstl dyn lib loading)
    write_time = benchmark_write_meshio(1, filename)

    for num_triangles in num_triangles_list:
        write_time = benchmark_write_meshio(num_triangles, filename)
        read_time = benchmark_read_meshio(filename)
        write_meshio_times.append(write_time)
        read_meshio_times.append(read_time)
        

    # Results
    write_diff_np = (np.array(write_numpy_stl_times) / np.array(write_times_openstl)).round(3)
    read_diff_np =  (np.array(read_numpy_stl_times) / np.array(read_times_openstl)).round(3)
    rotate_diff_np = (np.array(rotate_numpy_stl) / np.array(rotate_openstl)).round(3)
    print(f"Write:\tOpenSTL is {np.min(write_diff_np)} to {np.max(write_diff_np)} X faster than numpy-stl")
    print(f"Read:\tOpenSTL is {np.min(read_diff_np)} to {np.max(read_diff_np)} X faster than numpy-stl")
    print(f"Rotate:\tOpenSTL is {np.min(rotate_diff_np)} to {np.max(rotate_diff_np)} X faster than numpy-stl")

    write_diff_mi = (np.array(write_meshio_times) / np.array(write_times_openstl)).round(3)
    read_diff_mi =  (np.array(read_meshio_times) / np.array(read_times_openstl)).round(3)
    print(f"Write:\tOpenSTL is {np.min(write_diff_mi)} to {np.max(write_diff_mi)} X faster than meshio")
    print(f"Read:\tOpenSTL is {np.min(read_diff_mi)} to {np.max(read_diff_mi)} X faster than meshio")

    read_diff_stl_reader =  (np.array(read_stl_reader_times) / np.array(read_times_openstl)).round(3)
    print(f"Read:\tOpenSTL is {np.min(read_diff_stl_reader)} to {np.max(read_diff_stl_reader)} X faster than stl_reader")

    # Plotting
    plt.figure(figsize=(10, 6))  # Set figure size for better visualization
    plt.plot(num_triangles_list, write_times_openstl, label="Write (OpenSTL)", color="green", linestyle="-", marker="s", markersize=7, linewidth=3)
    plt.plot(num_triangles_list, read_times_openstl, label="Read (OpenSTL)", color="blue", linestyle="-", marker="s", markersize=7, linewidth=3)
    plt.plot(num_triangles_list, rotate_openstl, label="Rotate (OpenSTL)", color="red", linestyle="-", marker="s", markersize=7, linewidth=3)
    plt.plot(num_triangles_list, write_numpy_stl_times, label="Write (numpy-stl)", color="green", linestyle="--", marker="o", markersize=5, alpha=0.5)
    plt.plot(num_triangles_list, read_numpy_stl_times, label="Read (numpy-stl)", color="blue", linestyle="--", marker="o", markersize=5, alpha=0.5)
    plt.plot(num_triangles_list, rotate_numpy_stl, label="Rotate (numpy-stl)", color="red", linestyle="--", marker="o", markersize=5, alpha=0.5)
    plt.plot(num_triangles_list, write_meshio_times, label="Write (meshio)", color="green", linestyle=":", marker="^", markersize=5, alpha=0.5)
    plt.plot(num_triangles_list, read_meshio_times, label="Read (meshio)", color="blue", linestyle=":", marker="^", markersize=5, alpha=0.5)
    plt.plot(num_triangles_list, read_stl_reader_times, label="Read (stl_reader)", color="blue", linestyle="-.", marker="x", markersize=5, alpha=0.5)

    plt.xlabel("Number of Triangles", fontsize=12)
    plt.ylabel("Time (seconds)", fontsize=12)
    plt.title("Python Benchmark Results", fontsize=14)
    plt.xscale("log")
    plt.yscale("log")
    plt.legend(fontsize=10, handlelength=5, loc='upper left')
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)  # Add grid lines with dashed style
    plt.tight_layout()  # Adjust layout to prevent clipping of labels


    # Adjusting axes limits and scaling
    plt.xlim(num_triangles_list[0], num_triangles_list[-1]*1.1)  # Extend x-axis range by 10%
    plt.ylim(min(min(write_times_openstl), min(read_times_openstl), min(read_numpy_stl_times), min(write_numpy_stl_times), min(read_diff_stl_reader)) * 0.5,
             max(max(write_times_openstl), max(read_times_openstl), max(read_numpy_stl_times), max(write_numpy_stl_times), max(read_diff_stl_reader)) * 2)  # Expand y-axis range

    plt.savefig('benchmark.png')