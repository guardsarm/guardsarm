# Attribution & Licensing

`guardsarm` (manager + analysis engine) is part of **GuardSarm**, a rebranded
downstream derivative of **Wazuh** (version 5.0.1).

## Upstream

- Upstream project: **wazuh/wazuh**
- Upstream URL: https://github.com/wazuh/wazuh
- License: **GPLv2** (GNU General Public License, version 2)

The original upstream copyright and the `LICENSE` file are retained **unmodified**
in this repository. GuardSarm does not remove, alter, or misrepresent upstream
authorship. Copyright of the upstream code remains with Wazuh Inc. and the
respective contributors.

## What the GuardSarm fork changes

- Product name and user-facing brand strings/URLs are rebranded
  `wazuh` -> `guardsarm` (guardsarm.com, documentation.guardsarm.com,
  packages.guardsarm.com, cti.guardsarm.com).
- Protocol and on-disk identifiers that affect wire/schema compatibility are
  deliberately **frozen** and NOT global-replaced.
- New GuardSarm-authored content (the local CTI/content backend under
  `guardsarm-cti`, and deployment tooling under `guardsarm-infra`) is additive.

GuardSarm claims authorship **only** of these additive/branding changes.

## License of this repository

Distributed under **GPLv2**, the same license as upstream Wazuh. See `LICENSE`.
