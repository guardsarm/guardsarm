# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import os

from guardsarm.core import common

API_PATH = os.path.join(common.GUARDSARM_PATH, 'api')
CONFIG_PATH = os.path.join(API_PATH, 'configuration')
CONFIG_FILE_PATH = os.path.join(CONFIG_PATH, 'api.yaml')
RELATIVE_CONFIG_FILE_PATH = os.path.relpath(CONFIG_FILE_PATH, common.GUARDSARM_PATH)
SECURITY_PATH = os.path.join(CONFIG_PATH, 'security')
SECURITY_CONFIG_PATH = os.path.join(SECURITY_PATH, 'security.yaml')
API_LOG_PATH = os.path.join(common.GUARDSARM_PATH, 'logs', 'api')
API_SSL_PATH = os.path.join(CONFIG_PATH, 'ssl')
