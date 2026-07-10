#!/usr/bin/env python

###
#  Copyright (C) 2026 GuardSarm, Inc.All rights reserved.
#  GuardSarmsiem.com
#
#  Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
###

# Instructions:
#  - Use the embedded interpreter to run the script: {guardsarm_path}/framework/python/bin/python3 get_agents.py

import json

import guardsarm.agent as agent

if __name__ == "__main__":

    result = agent.get_agents()
    print(json.dumps(result.render(), indent=4, sort_keys=True, default=str))
