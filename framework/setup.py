#!/usr/bin/env python

# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from guardsarm import __version__

from setuptools import setup, find_namespace_packages

setup(name='guardsarm',
      version=__version__,
      description='GuardSarm control with Python',
      url='https://github.com/guardsarm',
      author='GuardSarm',
      author_email='info@guardsarmsiem.com',
      license='GPLv2',
      packages=find_namespace_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
      package_data={'guardsarm': ['core/guardsarm.json',
                              'core/cluster/cluster.json', 'rbac/default/*.yaml']},
      include_package_data=True,
      install_requires=[],
      zip_safe=False,
      )
