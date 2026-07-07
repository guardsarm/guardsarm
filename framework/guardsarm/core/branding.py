# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm, Inc.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

"""GuardSarm branding layer for the Python framework and API.

This module exposes USER-VISIBLE branding values (product name, vendor, contact,
website, display version). Values are sourced from the repository-root
``branding.json`` (the single source of truth, shared with the C build and the
init scripts). Resolution order:

    1. ``$GUARDSARM_BRANDING_JSON`` environment variable (explicit path).
    2. ``branding.json`` found by walking up from this file (source checkout).
    3. ``<GUARDSARM_PATH>/branding.json`` (installed location).

If none is found, the ``_FALLBACK`` values reproduce the original GuardSarm branding,
so removing ``branding.json`` is a complete rollback of the user-visible rebrand.

IMPORTANT -- COMPATIBILITY CONTRACT: never use these display values for
machine-readable fields that existing clients depend on (e.g. the API ``version``
field, ``JWT_ISSUER``, logger names, RBAC identifiers, index names). Those are
frozen. This module is strictly for human-facing text.
"""

import json
import os

# Original GuardSarm values -- used when branding.json is absent (clean rollback).
_FALLBACK = {
    "product_name": "GuardSarm",
    "product_short_name": "GuardSarm",
    "product_id": "guardsarm",
    "product_upper": "GUARDSARM",
    "product_version": "",  # empty -> callers fall back to the real GuardSarm version
    "vendor_name": "Wazuh Inc.",
    "legal_entity": "Wazuh Inc.",
    "author": "Wazuh Inc.",
    "contact": "info@guardsarmsiem.com",
    "security_contact": "security@guardsarmsiem.com",
    "site": "https://guardsarmsiem.com",
    "doc_url": "https://documentation.guardsarmsiem.com",
    "github_org": "https://github.com/guardsarm",
}


def _find_branding_file() -> str:
    """Return the path to branding.json, or an empty string if not found."""
    env_path = os.environ.get("GUARDSARM_BRANDING_JSON")
    if env_path and os.path.isfile(env_path):
        return env_path

    # Walk up from this file looking for a repository-root branding.json.
    current = os.path.abspath(os.path.dirname(__file__))
    while True:
        candidate = os.path.join(current, "branding.json")
        if os.path.isfile(candidate):
            return candidate
        parent = os.path.dirname(current)
        if parent == current:
            break
        current = parent

    # Installed location.
    try:
        from guardsarm.core import common

        candidate = os.path.join(common.GUARDSARM_PATH, "branding.json")
        if os.path.isfile(candidate):
            return candidate
    except Exception:
        pass

    return ""


def _load() -> dict:
    values = dict(_FALLBACK)
    path = _find_branding_file()
    if path:
        try:
            with open(path, "r", encoding="utf-8") as handle:
                data = json.load(handle)
            for key in _FALLBACK:
                if data.get(key):
                    values[key] = data[key]
        except Exception:
            # Any parse/read error falls back to GuardSarm defaults rather than failing.
            pass
    return values


_VALUES = _load()

PRODUCT_NAME = _VALUES["product_name"]
PRODUCT_SHORT_NAME = _VALUES["product_short_name"]
PRODUCT_ID = _VALUES["product_id"]
PRODUCT_UPPER = _VALUES["product_upper"]
PRODUCT_VERSION = _VALUES["product_version"]
VENDOR_NAME = _VALUES["vendor_name"]
LEGAL_ENTITY = _VALUES["legal_entity"]
AUTHOR = _VALUES["author"]
CONTACT = _VALUES["contact"]
SECURITY_CONTACT = _VALUES["security_contact"]
SITE = _VALUES["site"]
DOC_URL = _VALUES["doc_url"]
GITHUB_ORG = _VALUES["github_org"]
