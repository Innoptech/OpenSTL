import os, re, sys
import subprocess
import sysconfig
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

#from single_source import get_version
#__version__ = get_version("cz_conventional_commits", '.')

# A simple function to read the repo version
def read_version_from_pyproject(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    pattern = r'\bversion\s*=\s*"([^"]+)"'
    match = re.search(pattern, content)

    if match:
        version = match.group(1)
        return version
    else:
        return None


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "", cmake: str = "cmake") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())
        self.cmake = cmake


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)  # type: ignore[no-untyped-call]
        extdir = ext_fullpath.parent.resolve()

        # Print the list of files in the source directory
        print(f"Files in source directory:")
        for filename in os.listdir(ext.sourcedir):
            print(filename)

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
        # from Python.
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DPYTHON_INCLUDE_DIR={sysconfig.get_path('include')}",
            f"-DPYTHON_LIBRARY={sysconfig.get_config_var('LIBDIR')}",
            f"-DCMAKE_BUILD_TYPE=Release",
            '-DCMAKE_INSTALL_RPATH=$ORIGIN',
            '-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON',
            '-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=OFF',
            '-DOPENSTL_BUILD_PYTHON:BOOL=ON'
        ]
        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]


        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        subprocess.run(
            [ext.cmake, ext.sourcedir] + cmake_args, cwd=build_temp, check=True
        )
        subprocess.run(
            [ext.cmake, "--build", "."] + build_args, cwd=build_temp, check=True
        )

test_deps = [
    'coverage',
    'pytest',
    'numpy'
]

setup(
    name                    ="openstl",
    version                 =read_version_from_pyproject("pyproject.toml"),
    description             ="A simple STL serializer and deserializer",
    long_description        =open('README.md').read(),
    long_description_content_type='text/markdown',
    url                     ='https://github.com/Innoptech/OpenSTL',
    author                  ='Jean-Christophe Ruel',
    author_email            ='info@innoptech.com',
    python_requires         =">=3.4",
    ext_modules             =[CMakeExtension("openstl",
                                             sourcedir=os.environ.get('OPENSTL_SOURCE_DIR', '.'),
                                             cmake=os.environ.get('OPENSTL_CMAKE_PATH', 'cmake'))],
    cmdclass                ={'build_ext': CMakeBuild},
    test_suite              ="tests/python",
    tests_require           =test_deps,
    extras_require          ={'test': test_deps},
    include_package_data    =False,
    zip_safe                =True,
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Multimedia :: Graphics :: 3D Modeling",
        "Topic :: Scientific/Engineering :: Mathematics",
        "Topic :: Scientific/Engineering :: Visualization",
        "License :: OSI Approved :: BSD License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12"
    ],
)
