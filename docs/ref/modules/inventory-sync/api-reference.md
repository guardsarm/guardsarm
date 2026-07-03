# API Reference

Inventory Sync exposes two API surfaces:

- The **module start/stop interface** used by the GuardSarm module loader.
- The **indexed state data surface** exposed through the GuardSarm Indexer.

## Module interface

### C entry points

The shared library exports these symbols:

```c
EXPORTED void inventory_sync_start(full_log_fnc_t callbackLog, const cJSON* configuration);
EXPORTED void inventory_sync_stop();
```

The manager-side wrapper in `wm_inventory_sync.c` loads the shared library dynamically and invokes those functions at module start and stop.

### C++ interface

The public C++ facade exposes:

```cpp
class InventorySync final : public Singleton<InventorySync>
{
public:
    void start(const std::function<void(...)>& logFunction,
               const nlohmann::json& configuration) const;
    void stop() const;
};
```

## Runtime protocol surface

The FlatBuffer protocol accepted by Inventory Sync includes:

- `Start`
- `DataValue`
- `DataBatch`
- `DataContext`
- `DataClean`
- `ChecksumModule`
- `End`
- `ReqRet`

Responses produced by the manager include:

- `StartAck`
- `EndAck`
- `ReqRet`

See the FlatBuffers page for the full schema details.

## Indexed state indices

Inventory Sync works against the `guardsarm-states-*` family. The currently supported inventory families are:

### Syscollector indices

- `guardsarm-states-inventory-system`
- `guardsarm-states-inventory-hardware`
- `guardsarm-states-inventory-hotfixes`
- `guardsarm-states-inventory-packages`
- `guardsarm-states-inventory-processes`
- `guardsarm-states-inventory-ports`
- `guardsarm-states-inventory-interfaces`
- `guardsarm-states-inventory-protocols`
- `guardsarm-states-inventory-networks`
- `guardsarm-states-inventory-users`
- `guardsarm-states-inventory-groups`
- `guardsarm-states-inventory-services`
- `guardsarm-states-inventory-browser-extensions`

### FIM indices

- `guardsarm-states-fim-files`
- `guardsarm-states-fim-registry-keys`
- `guardsarm-states-fim-registry-values`

### SCA index

- `guardsarm-states-sca`

### Related downstream index

- `guardsarm-states-vulnerabilities`

`guardsarm-states-vulnerabilities` is not written directly by Inventory Sync, but it is part of the current manager-side flow because Inventory Sync can trigger vulnerability scans from the same synchronization session.

## Querying synchronized data

State data can be queried through the GuardSarm Indexer search API.

### Search all state indices

```http
GET /guardsarm-states-*/_search
```

### Search one agent across all state indices

```json
GET /guardsarm-states-*/_search
{
  "query": {
    "term": {
      "guardsarm.agent.id": "001"
    }
  }
}
```

### Search SCA documents for one agent

```json
GET /guardsarm-states-sca/_search
{
  "query": {
    "term": {
      "guardsarm.agent.id": "001"
    }
  }
}
```

### Search inventory packages for one agent

```json
GET /guardsarm-states-inventory-packages/_search
{
  "query": {
    "bool": {
      "filter": [
        { "term": { "guardsarm.agent.id": "001" } },
        { "exists": { "field": "package.name" } }
      ]
    }
  }
}
```

## Indexed document shape

Inventory Sync enriches indexed state documents with manager-side metadata before sending them to the indexer. In practice, upserted documents include at least:

- `guardsarm.agent.id`
- `guardsarm.agent.name`
- `guardsarm.agent.version`
- `guardsarm.agent.groups`
- `guardsarm.agent.host.architecture`
- `guardsarm.agent.host.hostname`
- `guardsarm.agent.host.os.*`
- `guardsarm.cluster.name`

The domain-specific payload from the agent is then appended to that metadata. A package document, for example, can look like this:

```json
{
  "guardsarm": {
    "agent": {
      "id": "001",
      "name": "ubuntu22",
      "version": "v5.0.0",
      "groups": ["default"],
      "host": {
        "architecture": "x86_64",
        "hostname": "ubuntu22",
        "os": {
          "name": "Ubuntu",
          "platform": "ubuntu",
          "type": "linux",
          "version": "22.04.5 LTS"
        }
      }
    },
    "cluster": {
      "name": "cluster"
    }
  },
  "package": {
    "name": "openssl",
    "version": "3.0.2",
    "type": "deb"
  },
  "checksum": {
    "hash": {
      "sha1": "..."
    }
  },
  "state": {
    "modified_at": "2026-04-20T10:00:00.000Z"
  }
}
```
