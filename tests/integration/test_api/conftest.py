"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""

from time import sleep

import pytest

from guardsarm_testing.constants.paths.logs import (
    GUARDSARM_API_LOG_FILE_PATH,
    GUARDSARM_API_JSON_LOG_FILE_PATH,
)
from guardsarm_testing.constants.api import GUARDSARM_API_PORT, CONFIGURATION_TYPES
from guardsarm_testing.modules.api.configuration import (
    get_configuration,
    append_configuration,
    delete_configuration_file,
)
from guardsarm_testing.modules.api.utils import login
from guardsarm_testing.modules.api.patterns import API_STARTED_MSG
from guardsarm_testing.tools.monitors import file_monitor
from guardsarm_testing.utils.callbacks import generate_callback


@pytest.fixture
def add_configuration(
    test_configuration: list[dict], request: pytest.FixtureRequest
) -> None:
    """Add configuration to the GuardSarm API configuration files.

    Args:
        test_configuration (dict): Configuration data to be added to the configuration files.
        request (pytest.FixtureRequest): Gives access to the requesting test context and has an optional `param`
                                         attribute in case the fixture is parametrized indirectly.
    """
    # Configuration file that will be used to apply the test configuration
    test_target_type = request.module.configuration_type
    # Save current configuration
    backup = get_configuration(configuration_type=test_target_type)
    # Set new configuration at the end of the configuration file
    append_configuration(test_configuration["blocks"], test_target_type)

    yield

    # Restore base configuration file or delete security configuration file
    if test_target_type != CONFIGURATION_TYPES[1]:
        append_configuration(backup, test_target_type)
    else:
        delete_configuration_file(test_target_type)


@pytest.fixture
def wait_for_api_start(test_configuration: dict) -> None:
    """Monitor the API log file to detect whether it has been started or not.

    Args:
        test_configuration (dict): Configuration data.

    Raises:
        RuntimeError: When the log was not found.
    """
    # Set the default values
    logs_format = "plain"
    host = ["0.0.0.0", "::"]
    port = GUARDSARM_API_PORT
    protocol = "https"
    skip_login_probe = False

    # Check if specific values were set or set the defaults
    if test_configuration is not None:
        if test_configuration.get("blocks") is not None:
            logs_configuration = test_configuration["blocks"].get("logs")
            # Set the default value if `format`` is not set
            logs_format = (
                "plain"
                if logs_configuration is None
                else logs_configuration.get("format", "plain")
            )
            host = test_configuration["blocks"].get("host", ["0.0.0.0", "::"])
            port = test_configuration["blocks"].get("port", GUARDSARM_API_PORT)
            https_configuration = test_configuration["blocks"].get("https")
            if (
                https_configuration is not None
                and https_configuration.get("enabled") is False
            ):
                protocol = "http"
            intervals_configuration = test_configuration["blocks"].get("intervals")
            if intervals_configuration is not None:
                request_timeout = intervals_configuration.get("request_timeout")
                skip_login_probe = request_timeout in (0, "0")

    file_to_monitor = (
        GUARDSARM_API_JSON_LOG_FILE_PATH
        if logs_format == "json"
        else GUARDSARM_API_LOG_FILE_PATH
    )
    monitor_start_message = file_monitor.FileMonitor(file_to_monitor)
    monitor_start_message.start(
        callback=generate_callback(
            API_STARTED_MSG, {"host": str(host), "port": str(port)}
        )
    )

    if monitor_start_message.callback_result is None:
        raise RuntimeError("The API was not started as expected.")

    if skip_login_probe:
        return

    configured_hosts = host if isinstance(host, list) else [host]
    local_hosts = {"0.0.0.0", "::", "127.0.0.1", "::1", "localhost"}
    if not any(configured_host in local_hosts for configured_host in configured_hosts):
        return

    last_exception = None
    for _ in range(15):
        try:
            login(
                host="localhost",
                port=str(port),
                protocol=protocol,
                timeout=2,
                login_attempts=1,
                backoff_factor=0,
            )
            return
        except Exception as exception:
            last_exception = exception
            sleep(1)

    if last_exception is not None:
        raise last_exception

    raise RuntimeError("The API was not ready to accept logins.")
