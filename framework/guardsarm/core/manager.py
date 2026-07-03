# Copyright (C) 2015, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

import copy
import json
import re
from datetime import timezone
from enum import Enum
from os.path import exists
from typing import Union

from api import configuration
from guardsarm import GuardSarmError, GuardSarmInternalError
from guardsarm.core import common
from guardsarm.core.cluster.utils import get_manager_status
from guardsarm.core.configuration import get_ossec_conf
from guardsarm.core.utils import get_utc_strptime, tail, load_guardsarm_xml

GUARDSARM_LOG_FIELDS = ['timestamp', 'tag', 'level', 'description']

class LoggingFormat(Enum):
    plain = "plain"
    json = "json"


def status() -> dict:
    """Return the Manager processes that are running."""

    return get_manager_status()


def get_guardsarm_log_fields(log: str, log_format: LoggingFormat = LoggingFormat.plain) -> Union[tuple, None]:
    """Get guardsarm-manager.log log fields.

    Parameters
    ----------
    log : str
        Log example.
    log_format : LoggingFormat
        GuardSarm log format.

    Returns
    -------
    tuple or None
        Log fields: timestamp, tag, level, and description.
    """
    if log_format == LoggingFormat.plain:
        regex_category = re.compile(
            r"^(\d\d\d\d/\d\d/\d\d\s\d\d:\d\d:\d\d)\s(\S+)(?:\[.*)?:\s(DEBUG|INFO|CRITICAL|ERROR|WARNING):(.*)$")

        match = re.search(regex_category, log)
        if not match:
            return None

        date = match.group(1)
        tag = match.group(2)
        level = match.group(3)
        description = match.group(4)

    elif log_format == LoggingFormat.json:
        try:
            match = json.loads(log)
        except json.decoder.JSONDecodeError:
            return None

        try:
            date = match['timestamp']
            tag = match['tag']
            level = match['level']
            description = match['description']
        except KeyError:
            return None
    else:
        return None

    if "rootcheck" in tag:  # Unify rootcheck category
        tag = "guardsarm-rootcheck"

    return get_utc_strptime(date, '%Y/%m/%d %H:%M:%S'), tag, level.lower(), description


def get_guardsarm_active_logging_format() -> LoggingFormat:
    """Obtain the GuardSarm active logging format.

    Returns
    -------
    LoggingFormat
        GuardSarm active log format. Can either be `plain` or `json`. If it has both types, `plain` will be returned.
    """
    logging_config = get_ossec_conf(section='logging')['logging']
    return LoggingFormat.plain if 'plain' in logging_config.get('log_format') else LoggingFormat.json

def get_guardsarm_logs(limit: int = 2000) -> list:
    """Return last <limit> lines of guardsarm-manager.log file.

    Parameters
    ----------
    limit : int
        Number of lines to return. Default: 2000

    Returns
    -------
    list
        List of dictionaries with requested logs.
    """
    logs = []

    log_format = get_guardsarm_active_logging_format()
    if log_format == LoggingFormat.plain and exists(common.GUARDSARM_LOG):
        guardsarm_log_content = tail(common.GUARDSARM_LOG, limit)
    elif log_format == LoggingFormat.json and exists(common.GUARDSARM_LOG_JSON):
        guardsarm_log_content = tail(common.GUARDSARM_LOG_JSON, limit)
    else:
        raise GuardSarmInternalError(1020)

    for line in guardsarm_log_content:
        log_fields = get_guardsarm_log_fields(line, log_format=log_format)
        if log_fields:
            date, tag, level, description = log_fields

            # We transform local time (guardsarm log file) to UTC with ISO8601 maintaining time integrity
            timestamp = date.astimezone(timezone.utc).strftime(common.DATE_FORMAT)
            log_line = {'timestamp': timestamp, 'tag': tag, 'level': level, 'description': description}
            logs.append(log_line)

    return logs


def get_logs_summary(limit: int = 2000) -> dict:
    """Get the number of alerts of each tag.

    Parameters
    ----------
    limit : int
        Number of lines to return. Default: 2000

    Returns
    -------
    dict
        Number of logs for every tag.
    """
    tags = dict()
    logs = get_guardsarm_logs(limit)

    for log in logs:
        if log['tag'] in tags:
            tags[log['tag']]['all'] += 1
        else:
            tags[log['tag']] = {'all': 1, 'info': 0, 'error': 0, 'critical': 0, 'warning': 0, 'debug': 0}
        tags[log['tag']][log['level']] += 1

    return tags


def validate_ossec_conf() -> dict:
    """Check if GuardSarm configuration is OK.

    Validates the ossec.conf file by reading and parsing the XML structure.
    This replaces the previous socket-based validation after execd removal.

    Raises
    ------
    GuardSarmInternalError(1020)
        If the configuration file doesn't exist.
    GuardSarmError(1113)
        If there are XML syntax errors.
    GuardSarmError(1908)
        If there are validation errors in the configuration.

    Returns
    -------
    dict
        Status of the configuration with 'status' key set to 'OK' if valid.
    """
    # Check if configuration file exists
    if not exists(common.OSSEC_CONF):
        raise GuardSarmInternalError(1020)

    # Load and validate XML structure
    # This will raise GuardSarmError(1113) if there are syntax errors
    try:
        load_guardsarm_xml(xml_path=common.OSSEC_CONF)
        return {'status': 'OK'}

    except GuardSarmError as e:
        # Re-raise GuardSarmError (includes validation errors)
        raise
    except Exception as e:
        # Wrap other exceptions as validation errors
        raise GuardSarmError(1908, extra_message=str(e))


def get_api_conf() -> dict:
    """Return current API configuration.

    Returns
    -------
    dict
        API configuration.
    """
    return copy.deepcopy(configuration.api_conf)
