# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from unittest.mock import AsyncMock, MagicMock, patch

import pytest

from guardsarm.core.indexer.indexer import get_indexer_client, resolve_guardsarm_path


def test_resolve_guardsarm_path_keeps_absolute_paths():
    path = "/tmp/root-ca.pem"

    assert resolve_guardsarm_path(path) == path


def test_resolve_guardsarm_path_uses_guardsarm_path_for_relative_paths():
    with patch("guardsarm.core.indexer.indexer.common.GUARDSARM_PATH", "/var/guardsarm-manager"):
        assert (
            resolve_guardsarm_path("etc/certs/root-ca.pem")
            == "/var/guardsarm-manager/etc/certs/root-ca.pem"
        )


@pytest.mark.asyncio
async def test_get_indexer_client_resolves_relative_certificate_paths():
    client = AsyncMock()
    client.close = AsyncMock()
    keystore_client = MagicMock()
    keystore_client.__enter__.return_value.get.side_effect = [
        {"value": "guardsarm-server"},
        {"value": "guardsarm-server"},
    ]

    guardsarm_config = {
        "indexer": {
            "hosts": ["https://localhost:9200"],
            "ssl": {
                "certificate_authorities": [{"ca": ["etc/certs/root-ca.pem"]}],
                "certificate": ["etc/certs/manager.pem"],
                "key": ["etc/certs/manager-key.pem"],
            },
        }
    }

    with patch("guardsarm.core.indexer.indexer.common.GUARDSARM_PATH", "/var/guardsarm-manager"), \
            patch(
                "guardsarm.core.indexer.indexer.get_ossec_conf",
                return_value=guardsarm_config,
            ), \
            patch(
                "guardsarm.core.indexer.indexer.KeystoreClient",
                return_value=keystore_client,
            ), \
            patch(
                "guardsarm.core.indexer.indexer.create_indexer",
                new_callable=AsyncMock,
                return_value=client,
            ) as create_indexer:
        async with get_indexer_client():
            pass

    create_indexer.assert_awaited_once_with(
        hosts=["localhost"],
        ports=[9200],
        user="guardsarm-server",
        password="guardsarm-server",
        use_ssl=True,
        verify_certs=True,
        client_cert_path="/var/guardsarm-manager/etc/certs/manager.pem",
        client_key_path="/var/guardsarm-manager/etc/certs/manager-key.pem",
        ca_certs_path="/var/guardsarm-manager/etc/certs/root-ca.pem",
    )
    client.close.assert_awaited_once()
