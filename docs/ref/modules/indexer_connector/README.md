# Indexer Connector

The Indexer Connector is a shared library (`libindexer_connector`) that handles all data indexing operations between the GuardSarm Manager and the GuardSarm Indexer (OpenSearch). It is the Filebeat replacement introduced in GuardSarm 5.0.

Source: `src/shared_modules/indexer_connector/`

For configuration options see [Indexer Configuration](../../configuration/indexer.md).

## Overview

The Indexer Connector is not a standalone daemon. It is linked into the processes that need to write to or query the Indexer:

- **Vulnerability Scanner** — indexes CVE detections into `guardsarm-states-vulnerabilities`
- **Inventory Sync** — indexes agent state into `guardsarm-states-inventory-*` and `guardsarm-states-fim-*`
- **Engine** — indexes SCA results and other engine-generated events

The library provides two classes depending on the use case:

| Class | Mode | Queue | Use case |
|-------|------|-------|----------|
| `IndexerConnectorSync` | Synchronous | In-memory (up to 10 MB) | Low-latency, bounded writes |
| `IndexerConnectorAsync` | Asynchronous | RocksDB (`queue/indexer/<id>/`) | Write-ahead queue, survives restarts |

## How it works

1. The caller instantiates a connector with a JSON configuration (derived from the `<indexer>` XML block).
2. Credentials (`username`/`password`) are read from the RocksDB keystore (`queue/keystore/`).
3. A background health-monitor thread polls `/_cat/health` on all configured hosts every 60 seconds and marks nodes available or unavailable.
4. A server-selector performs round-robin load balancing across available nodes.
5. Documents are accumulated (sync: in memory; async: in RocksDB) and flushed as OpenSearch Bulk API requests.

### Sync flush behavior

- Buffer up to 10 MB of events before flushing.
- Flush automatically after 20 seconds of inactivity.
- If the indexer returns HTTP 413 (payload too large), the batch is split and retried.
- Version conflicts at the document level are handled per-document.

### Async flush behavior

- Events are queued to RocksDB immediately and flushed by a background thread.
- Up to 25,000 documents per flush batch.
- If `max_queue_size` is set, events that exceed the limit are dropped and counted.
- The queue survives manager restarts.

## Indices

| Index | Written by |
|-------|------------|
| `guardsarm-states-vulnerabilities` | Vulnerability Scanner |
| `guardsarm-states-inventory-system` | Inventory Sync |
| `guardsarm-states-inventory-hardware` | Inventory Sync |
| `guardsarm-states-inventory-packages` | Inventory Sync |
| `guardsarm-states-inventory-hotfixes` | Inventory Sync (Windows) |
| `guardsarm-states-inventory-processes` | Inventory Sync |
| `guardsarm-states-inventory-ports` | Inventory Sync |
| `guardsarm-states-inventory-interfaces` | Inventory Sync |
| `guardsarm-states-inventory-protocols` | Inventory Sync |
| `guardsarm-states-inventory-networks` | Inventory Sync |
| `guardsarm-states-inventory-users` | Inventory Sync |
| `guardsarm-states-inventory-groups` | Inventory Sync |
| `guardsarm-states-inventory-services` | Inventory Sync |
| `guardsarm-states-inventory-browser-extensions` | Inventory Sync |
| `guardsarm-states-fim-files` | Inventory Sync (FIM) |
| `guardsarm-states-fim-registry-keys` | Inventory Sync (FIM, Windows) |
| `guardsarm-states-fim-registry-values` | Inventory Sync (FIM, Windows) |
| `guardsarm-states-sca` | Engine (SCA) |
| `guardsarm-threatintel-*` | Read-only (Content Manager) |

## Key source files

| File | Purpose |
|------|---------|
| `include/indexerConnector.hpp` | Public API: `IndexerConnectorSync`, `IndexerConnectorAsync` |
| `src/indexerConnectorSyncImpl.hpp` | Sync implementation: in-memory buffer, bulk flush, 413 splitting |
| `src/indexerConnectorAsyncImpl.hpp` | Async implementation: RocksDB queue, background flusher |
| `src/serverSelector.hpp` | Round-robin load balancer with health tracking |
| `src/monitoring.hpp` | Background health-monitor thread (60s interval) |
| `testtool/` | CLI test tool: `push-events`, `export-policy`, `generate-full-policy` |

## Test tool

```bash
# Build
make indexer_connector_tool -j$(nproc)

# Push events to an index (sync)
./indexer_connector_tool push-events -c config.json -e events.json

# Push events (async)
./indexer_connector_tool push-events -c config.json -e events.json -m async -w 5
```

See `testtool/README.md` for the full reference.
