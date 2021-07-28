#!/usr/bin/env python3

from setuptools import setup, Extension

setup(name="PropertyTree",
      version="2.0",
      description="C++/Rapidjson-based Property Tree",
      author="Curtis L. Olson",
      author_email="curtolson@flightgear.org",
      url="https://github.com/RiceCreekUAS",
      packages = ["src"],
      ext_modules=[
          Extension("PropertyTree",
                    define_macros=[("HAVE_PYBIND11", "1")],
                    sources=["src/props2.cpp",
                             "src/strutils.cpp",
                             "src/pybind11.cpp"],
                    depends=["src/props2.h",
                             "src/strutils.h"])
          ]
      )
