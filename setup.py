#!/usr/bin/python2

from distutils.core import setup, Extension

pyck_core = Extension('pyck.core_ext',
                      sources = ['pyck/core.cpp'],
                      libraries = ['boost_python'])

pyck_ugens_osc = Extension('pyck.ugens.osc_ext',
                           sources = ['pyck/ugens/osc.cpp','pyck/core.cpp'],
                           libraries = ['boost_python'])


setup(name='pyck',
      version='0.1',
      description='Pythonized ChucK',
      author='Thomas Girod',
      author_email='girodt@gmail.com',
      url='https://github.com/jiyunatori/pyck',
      ext_modules=[pyck_core, pyck_ugens_osc],
      packages=['pyck','pyck.ugens']
      )
