#!/usr/bin/env python3

from setuptools import setup, Extension

setup(name="PropertyTree",
      version="2.0",
      description="C++/Rapidjson-based Property Tree",
      author="Curtis L. Olson",
      author_email="curtolson@flightgear.org",
      url="https://github.com/RiceCreekUAS",
      packages = ["v2"],
      ext_modules=[
          Extension("PropertyTree",
                    define_macros=[("HAVE_PYBIND11", "1")],
                    sources=["v2/props2.cpp",
                             "v2/strutils.cpp",
                             "v2/pybind11.cpp"],
                    depends=["v2/props2.h",
                             "v2/strutils.h"])
          ]
      )
