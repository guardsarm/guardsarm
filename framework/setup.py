#!/usr/bin/env python

# Copyright (C) 2015, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

from guardsarm import __version__

from setuptools import setup, find_namespace_packages

setup(name='guardsarm',
      version=__version__,
      description='GuardSarm control with Python',
      url='https://github.com/guardsarm',
      author='GuardSarm',
      author_email='info@guardsarm.com',
      license='GPLv2',
      packages=find_namespace_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
      package_data={'guardsarm': ['core/guardsarm.json',
                              'core/cluster/cluster.json', 'rbac/default/*.yaml']},
      include_package_data=True,
      install_requires=[],
      zip_safe=False,
      )
