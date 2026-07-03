# Description

The **GuardSarm Manager** is the central server component of the GuardSarm platform. It
receives data from deployed GuardSarm Agents, processes it, and forwards the results
to the GuardSarm Indexer, where they are stored and later explored through the GuardSarm
Dashboard.

Its main responsibilities are:

- **Agent management**: enrollment, connection handling, agent groups, and
  centralized configuration distribution (`guardsarm-manager-authd`,
  `guardsarm-manager-remoted`).
- **Event processing**: decoding, enrichment, and detection through the
  **Engine** (`guardsarm-manager-analysisd`), which in 5.0 replaces the legacy
  rules pipeline with YAML decoders and rules, KVDB lookups, and structured
  findings indexed as `guardsarm-findings-v5-*`.
- **Security modules**: orchestration of inventory (Syscollector / IT Hygiene),
  configuration assessment (SCA), file integrity monitoring (FIM), and
  vulnerability detection fed by the GuardSarm CTI service.
- **Management plane**: a RESTful Server API (`guardsarm-manager-apid`) with RBAC,
  and a cluster mode (`guardsarm-manager-clusterd`) for horizontal scaling and high
  availability.

The manager is a multi-daemon system installed under `/var/guardsarm-manager` and
configured through `etc/guardsarm-manager.conf`. See the
[Architecture](architecture.md) page for the full component breakdown and the
[Modules](modules/README.md) section for per-module documentation.
