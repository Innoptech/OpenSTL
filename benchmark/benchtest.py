import openstl
import numpy as np
import timeit
import matplotlib.pyplot as plt

def create_triangles(num_triangles):
    triangles = []
    tri = openstl.Triangle([0, 0, 1], [1, 1, 1], [2, 2, 2], [3, 3, 3], 3)
    for _ in range(num_triangles):
        triangles.append(tri)
    return triangles

def benchmark_write(num_triangles, filename):
    triangles = create_triangles(num_triangles)
    result = timeit.timeit(lambda: openstl.write(filename, triangles, openstl.StlFormat.Binary), number=5)
    return result

def benchmark_read(filename):
    result = timeit.timeit(lambda: openstl.read(filename), number=5)
    return result

def benchmark_multiply_triangles(num_triangles, matrix):
    triangles = create_triangles(num_triangles)
    vertices = openstl.get_vertices(triangles).reshape(3,-1)
    result = timeit.timeit(lambda: np.matmul(matrix, vertices), number=5)
    return result

def benchmark_multiply_numpy(num_triangles, matrix):
    array = np.random.rand(3, num_triangles*3) # 3 float per triangle
    result = timeit.timeit(lambda: np.matmul(matrix, array), number=5)
    return result

if __name__ == "__main__":
    # Benchmark parameters
    filename = "../tests/python/benchmark.stl"
    matrix = np.random.rand(3, 3)
    num_triangles_list = [10**i for i in range(1, 7)]

    # Benchmarking
    write_times = []
    read_times = []
    multiply_times = []
    multiply_numpy_times = []

    for num_triangles in num_triangles_list:
        write_time = benchmark_write(num_triangles, filename)
        read_time = benchmark_read(filename)
        multiply_time = benchmark_multiply_triangles(num_triangles, matrix)
        multiply_numpy_time = benchmark_multiply_numpy(num_triangles, matrix)

        write_times.append(write_time)
        read_times.append(read_time)
        multiply_times.append(multiply_time)
        multiply_numpy_times.append(multiply_numpy_time)

    # Plotting
    plt.figure(figsize=(10, 6))  # Set figure size for better visualization
    plt.plot(num_triangles_list, write_times, label="Write", color="green", linestyle="-", marker="o", markersize=5)
    plt.plot(num_triangles_list, read_times, label="Read", color="blue", linestyle="--", marker="s", markersize=5)
    plt.plot(num_triangles_list, multiply_times, label="Multiply (Triangles)", color="red", linestyle="-.", marker="^", markersize=5)
    plt.plot(num_triangles_list, multiply_numpy_times, label="Multiply (Numpy Array)", color="purple", linestyle=":", marker="d", markersize=5)

    plt.xlabel("Number of Triangles", fontsize=12)
    plt.ylabel("Time (seconds)", fontsize=12)
    plt.title("Python Benchmark Results", fontsize=14)
    plt.xscale("log")
    plt.yscale("log")
    plt.legend(fontsize=10)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)  # Add grid lines with dashed style
    plt.tight_layout()  # Adjust layout to prevent clipping of labels

    # Adjusting axes limits and scaling
    plt.xlim(num_triangles_list[0], num_triangles_list[-1]*1.1)  # Extend x-axis range by 10%
    plt.ylim(min(min(write_times), min(read_times), min(multiply_times), min(multiply_numpy_times)) * 0.5,
             max(max(write_times), max(read_times), max(multiply_times), max(multiply_numpy_times)) * 2)  # Expand y-axis range

    plt.show()