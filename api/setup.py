#!/usr/bin/env python

# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from setuptools import setup, find_namespace_packages

# To install the library, run the following
#
# python setup.py install
#
# prerequisite: setuptools
# http://pypi.python.org/pypi/setuptools

setup(
    name='api',
    version='5.0.1',
    description="GuardSarm API",
    author_email="info@guardsarmsiem.com",
    author="GuardSarm",
    url="https://github.com/guardsarm",
    keywords=["GuardSarm API"],
    install_requires=[],
    packages=find_namespace_packages(exclude=["*.test", "*.test.*", "test.*", "test"]),
    package_data={'': ['spec/spec.yaml']},
    include_package_data=True,
    zip_safe=False,
    license='GPLv2',
    long_description="""\
    The GuardSarm API is an open source RESTful API that allows for interaction with the GuardSarm manager from a web browser, command line tool like cURL or any script or program that can make web requests. The GuardSarm app relies on this heavily and GuardSarm’s goal is to accommodate complete remote management of the GuardSarm infrastructure via the GuardSarm app. Use the API to easily perform everyday actions like adding an agent or restarting the manager(s) or agent(s).
    """
)
