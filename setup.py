import os, re, sys
import subprocess
import sysconfig
from pathlib import Path
import platform

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


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

def get_cmake_architecture_flags():
    system = platform.system()
    is_64bits = sys.maxsize > 2**32

    if system == "Linux":
        architecture_flag = "-m64" if is_64bits else "-m32"
        return [f"CMAKE_C_FLAGS={architecture_flag}", f"CMAKE_CXX_FLAGS={architecture_flag}",]
    elif system == "Darwin":  # macOS
        return [f"-DCMAKE_OSX_ARCHITECTURES={'x86_64' if is_64bits else 'i386'}"]
    elif system == "Windows":
        return ["-A x64"] if is_64bits else ["-A Win32"]
    return []

class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "", cmake: str = "cmake") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())
        self.cmake = cmake


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON3_EXECUTABLE={sys.executable}",
            f"-DPYTHON3_INCLUDE_DIR={sysconfig.get_path('include')}",
            f"-DPYTHON3_LIBRARY={sysconfig.get_config_var('LIBDIR')}",
            "-DCMAKE_BUILD_TYPE=Release",
            '-DCMAKE_INSTALL_RPATH=$ORIGIN',
            '-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON',
            '-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=OFF',
            '-DOPENSTL_BUILD_PYTHON:BOOL=ON'
        ] + get_cmake_architecture_flags()

        if "CMAKE_ARGS" in os.environ:
            cmake_args += os.environ["CMAKE_ARGS"].split()

        build_temp = Path(self.build_temp) / ext.name
        build_temp.mkdir(parents=True, exist_ok=True)


        subprocess.run([ext.cmake, ext.sourcedir] + cmake_args, cwd=build_temp, check=True)

        build_args = []
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]
        subprocess.run([ext.cmake, "--build", "."] + build_args, cwd=build_temp, check=True)

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
