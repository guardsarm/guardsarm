# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

import os
import subprocess
from functools import lru_cache
from sys import exit


@lru_cache(maxsize=None)
def find_guardsarm_path() -> str:
    """
    Get the GuardSarm installation path.

    Returns
    -------
    str
        Path where GuardSarm is installed or empty string if there is no framework in the environment.
    """
    abs_path = os.path.abspath(os.path.dirname(__file__))
    allparts = []
    while 1:
        parts = os.path.split(abs_path)
        if parts[0] == abs_path:  # sentinel for absolute paths
            allparts.insert(0, parts[0])
            break
        elif parts[1] == abs_path:  # sentinel for relative paths
            allparts.insert(0, parts[1])
            break
        else:
            abs_path = parts[0]
            allparts.insert(0, parts[1])

    guardsarm_path = ''
    try:
        for i in range(0, allparts.index('wodles')):
            guardsarm_path = os.path.join(guardsarm_path, allparts[i])
    except ValueError:
        pass

    return guardsarm_path


def call_guardsarm_control(option: str) -> str:
    """
    Execute the guardsarm-control script with the parameters specified.

    Parameters
    ----------
    option : str
        The option that will be passed to the script.

    Returns
    -------
    str
        The output of the call to guardsarm-control.
    """
    guardsarm_control = os.path.join(find_guardsarm_path(), "bin", "guardsarm-control")
    try:
        proc = subprocess.Popen([guardsarm_control, option], stdout=subprocess.PIPE)
        (stdout, stderr) = proc.communicate()
        return stdout.decode()
    except (OSError, ChildProcessError):
        print(f'ERROR: a problem occurred while executing {guardsarm_control}')
        exit(1)


def get_guardsarm_info(field: str) -> str:
    """
    Execute the guardsarm-control script with the 'info' argument, filtering by field if specified.

    Parameters
    ----------
    field : str
        The field of the output that's being requested. Its value can be 'GUARDSARM_VERSION', 'GUARDSARM_REVISION' or
        'GUARDSARM_TYPE'.

    Returns
    -------
    str
        The output of the guardsarm-control script.
    """
    guardsarm_info = call_guardsarm_control("info")
    if not guardsarm_info:
        return "ERROR"

    if not field:
        return guardsarm_info

    env_variables = guardsarm_info.rsplit("\n")
    env_variables.remove("")
    guardsarm_env_vars = dict()
    for env_variable in env_variables:
        key, value = env_variable.split("=")
        guardsarm_env_vars[key] = value.replace("\"", "")

    return guardsarm_env_vars[field]


@lru_cache(maxsize=None)
def get_guardsarm_version() -> str:
    """
    Return the version of GuardSarm installed.

    Returns
    -------
    str
        The version of GuardSarm installed.
    """
    return get_guardsarm_info("GUARDSARM_VERSION")


@lru_cache(maxsize=None)
def get_guardsarm_revision() -> str:
    """
    Return the revision of the GuardSarm instance installed.

    Returns
    -------
    str
        The revision of the GuardSarm instance installed.
    """
    return get_guardsarm_info("GUARDSARM_REVISION")


@lru_cache(maxsize=None)
def get_guardsarm_type() -> str:
    """
    Return the type of GuardSarm instance installed.

    Returns
    -------
    str
        The type of GuardSarm instance installed.
    """
    return get_guardsarm_info("GUARDSARM_TYPE")


ANALYSISD = os.path.join(find_guardsarm_path(), 'queue', 'sockets', 'queue')
# Max size of the event that ANALYSISID can handle
MAX_EVENT_SIZE = 65535
