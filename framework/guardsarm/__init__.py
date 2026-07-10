

# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from time import strftime

from guardsarm.core import common
from guardsarm.core.exception import GuardSarmException, GuardSarmError, GuardSarmInternalError

"""
GuardSarm HIDS Python package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GuardSarm is a python package to manage OSSEC.

"""

__version__ = '2.0.0'


msg = "\n\nPython 2.7 or newer not found."
msg += "\nUpdate it or set the path to a valid version. Example:"
msg += "\n  export PATH=$PATH:/opt/rh/python27/root/usr/bin"
msg += "\n  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/rh/python27/root/usr/lib64"

try:
    from sys import version_info as python_version
    if python_version.major < 2 or (python_version.major == 2 and python_version.minor < 7):
        raise GuardSarmInternalError(999, msg)
except Exception:
    raise GuardSarmInternalError(999, msg)


class GuardSarm:
    """
    Basic class to set up OSSEC directories
    """

    def __init__(self):
        """
        Initialize basic information and directories.
        :return:
        """

        self.version = f'v{__version__}'
        self.type = 'server'
        self.path = common.GUARDSARM_PATH
        self.max_agents = 'unlimited'
        self.tz_offset = None
        self.tz_name = None

        self._initialize()

    def __str__(self):
        return str(self.to_dict())

    def __eq__(self, other):
        if isinstance(other, GuardSarm):
            return self.to_dict() == other.to_dict()
        return False

    def to_dict(self):

        return {'path': self.path,
                'version': self.version,
                'type': self.type,
                'max_agents': self.max_agents,
                'tz_offset': self.tz_offset,
                'tz_name': self.tz_name,
                }

    def _initialize(self):
        """
        Calculates all GuardSarm installation metadata
        """
        # Timezone info
        try:
            self.tz_offset = strftime("%z")
            self.tz_name = strftime("%Z")
        except Exception:
            self.tz_offset = None
            self.tz_name = None

        return self.to_dict()
