#!/usr/bin/env python

import os
from subprocess import call
from setuptools.command.install import install
from wheel.bdist_wheel import bdist_wheel
from setuptools.command.test import test
import glob
from distutils.core import setup


extension = ''
if os.name == 'posix':
    extension = '.so'
else:
    extension = '.dll'


def compile_OpenMP_library():
    ''' Compile the OpenMP-only version of the source. '''
    print "Compiling and linking shared library..."
    call('g++ -g -O3 -fopenmp -m64 -std=c++11 -I../C++/OpenMP/include -I../C++/OpenMP/lib/alglib -c ../C++/OpenMP/lib/alglib/alglibinternal.cpp ../C++/OpenMP/lib/alglib/alglibmisc.cpp ../C++/OpenMP/lib/alglib/ap.cpp ../C++/OpenMP/src/causality.cpp ../C++/OpenMP/src/dimensions.cpp ../C++/OpenMP/src/embedding.cpp ../C++/OpenMP/src/probabilities.cpp ../C++/OpenMP/src/statistics.cpp ../C++/OpenMP/src/trimming.cpp')
    call('g++ -shared -fopenmp -o dimensional_causality/dimensional_causality_openmp' + extension + ' alglibinternal.o alglibmisc.o ap.o causality.o dimensions.o embedding.o probabilities.o statistics.o trimming.o')
    tmp_files = glob.glob('*.o')
    for file in tmp_files:
        os.remove(file)


class CustomInstall(install):
    ''' Customized setuptools install command that compiles the C++ source into a shared library. '''

    def run(self):
        compile_OpenMP_library()
        install.run(self)


class CustomWheel(bdist_wheel):
    ''' Customized setuptools install command that compiles the C++ source into a shared library. '''

    def run(self):
        compile_OpenMP_library()
        bdist_wheel.run(self)


class CustomTest(test):
    ''' Customized setuptools install command that compiles the C++ source into a shared library. '''

    def run(self):
        compile_OpenMP_library()
        test.run(self)

setup(name='Dimensional Causality',
      version='1.0',
      description='Python version of the Dimensional Causality method',
      long_description='Python version of the Dimensional Causality method developed in Benko, Zlatniczki, Fabo, Solyom, Eross, Telcs & Somogyvari (2018) - Inference of causal relations via dimensions.',
      author='Adam Zlatniczki',
      author_email='adam.zlatniczki@cs.bme.hu',
      url='https://github.com/adam-zlatniczki/dimensional_causality',
      cmdclass={
          'install': CustomInstall,
          'bdist_wheel': CustomWheel,
          'test': CustomTest
      },
      packages=['dimensional_causality'],
      package_dir={'dimensional_causality': 'dimensional_causality'},
      package_data={'dimensional_causality': ['dimensional_causality_openmp' + extension]},
      test_suite='nose.collector',
      tests_require=['nose'],
      classifiers=['TODO'],
      license='TODO'
      )
