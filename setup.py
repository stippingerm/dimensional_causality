#!/usr/bin/env python

import os
from setuptools.command.install import install
import glob
from distutils.core import setup

extension = ""
if os.name == "posix":
	extension = ".so"
else:
	extension = ".dll"


def compile_CPU_library():
	""" Compile the CPU-only version of the source. """
	os.system("g++ -g -O3 -fPIC -fopenmp -IC++/CPU/include -IC++/CPU/lib/alglib -c C++/CPU/lib/alglib/alglibinternal.cpp C++/CPU/lib/alglib/alglibmisc.cpp C++/CPU/lib/alglib/ap.cpp C++/CPU/src/causality.cpp C++/CPU/src/dimensions.cpp C++/CPU/src/embedding.cpp C++/CPU/src/probabilities.cpp C++/CPU/src/statistics.cpp C++/CPU/src/trimming.cpp")
	os.system("g++ -shared -fopenmp -o Python/dimensional_causality/dimensional_causality_cpu" + extension + " alglibinternal.o alglibmisc.o ap.o causality.o dimensions.o embedding.o probabilities.o statistics.o trimming.o")
	tmp_files = glob.glob("*.o")
	for file in tmp_files:
		os.remove(file)


class CompileLibraries(install):
	"""Customized setuptools install command that compiles the C++ source into a shared library."""
	def run(self):
		compile_CPU_library()
		install.run(self)


setup(name='Dimensional Causality',
      version='1.0',
      description='Python version of the Dimensional Causality method',
      author='Adam Zlatniczki',
      author_email='adam.zlatniczki@cs.bme.hu',
      url='https://github.com/adam-zlatniczki/dimensional_causality',
	  cmdclass={
         'install': CompileLibraries
      },
	  packages=['dimensional_causality'],
	  package_dir={'dimensional_causality': 'Python/dimensional_causality'},
	  package_data={'dimensional_causality': ['dimensional_causality_cpu'+extension]}
)