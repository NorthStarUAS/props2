from setuptools import setup, Extension
import pybind11
from pathlib import Path

this_dir = Path(__file__).parent

setup(
    ext_modules=[
        Extension("PropertyTree",
                define_macros=[("HAVE_PYBIND11", "1")],
                sources=["src/props2.cpp",
                         "src/util/strutils.cpp",
                         "src/pybind11.cpp"],
                include_dirs=[pybind11.get_include(),
                              this_dir / "src"],
                depends=["src/props2.h",
                         "src/util/strutils.h"])
        ]
    # ext_modules=[
    #     Extension(
    #         "PropertyTree",
    #         include_dirs=[pybind11.get_include()],
    #     )
    # ]
)
