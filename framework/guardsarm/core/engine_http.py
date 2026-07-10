# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import httpx

from guardsarm.core import common
from guardsarm.core.exception import GuardSarmError, GuardSarmInternalError


class EngineHTTPClient:
    """Synchronous HTTP client for the Engine API unix socket (analysisd)."""

    API_URL = 'http://localhost'

    def __init__(self, timeout: float = 10):
        self.socket_path = str(common.ANALYSISD_SOCKET)
        try:
            transport = httpx.HTTPTransport(uds=self.socket_path)
            self._client = httpx.Client(transport=transport, timeout=timeout)
        except Exception as exc:
            raise GuardSarmInternalError(2018, extra_message=str(exc)) from exc

    def close(self) -> None:
        """Close the Engine HTTP client."""
        self._client.close()

    def get_metrics_dump(self) -> dict:
        try:
            response = self._client.post(
                url=f'{self.API_URL}/metrics/dump',
                content='{}',
                headers={'Content-Type': 'application/json'},
            )
        except httpx.TimeoutException as exc:
            raise GuardSarmInternalError(2020, extra_message=str(exc))
        except httpx.ConnectError as exc:
            raise GuardSarmInternalError(2021, extra_message=str(exc))
        except httpx.RequestError as exc:
            raise GuardSarmError(2013, extra_message=str(exc))

        if response.is_error:
            raise GuardSarmError(2019, extra_message=response.text)

        try:
            return response.json()
        except ValueError as exc:
            raise GuardSarmInternalError(2022, extra_message=f'Invalid JSON in Engine API response: {exc}')
