# ConfRemote Module

## Overview

The **confremote** module manages runtime remote configuration for the GuardSarm engine. It periodically fetches settings from `guardsarm-indexer`, applies them to registered modules via callbacks, and persists the accepted state to the internal store for crash/restart resilience.

The module follows an observer pattern: other modules register per-key callbacks (`addTrigger`), and the manager invokes them whenever the remote value changes. Callbacks can reject values by throwing an exception, in which case the previous value is preserved.

## Architecture

```
                     guardsarm-indexer
                    (.guardsarm-settings)
                          │
                 getEngineRemoteConfig()
                          │
              ┌───────────▼────────────────────────────────────┐
              │          ConfRemoteManager                      │
              │                                                 │
              │  synchronize()  ◄── scheduler (periodic task)   │
              │       │                                         │
              │       ▼                                         │
              │  Fetch remote JSON ──► Compare with lastConfig  │
              │                              │                  │
              │                    changed?  │  unchanged?      │
              │                      │              │           │
              │              invoke callback     skip           │
              │                 │         │                     │
              │            accepted    rejected (throws)        │
              │                │              │                 │
              │         update lastConfig   keep current        │
              │                │                                │
              │         saveSettingsToStore()                    │
              │                │                                │
              │      ┌────────▼─────────┐                      │
              │      │   store (IStore)  │                      │
              │      │ "remote-config/   │                      │
              │      │  engine-cnf/0"    │                      │
              │      └──────────────────┘                      │
              └─────────────────────────────────────────────────┘
                          │
                  addTrigger("key", callback, default)
                          │
              ┌───────────▼───────────┐
              │   Consumer modules    │
              │  (e.g. RawEventIndexer│
              │   hotReloadConf)      │
              └───────────────────────┘
```

## Key Concepts

### Interface (`IConfRemote`)

Defined in `interface/confremote/iconfremote.hpp`. Exposes two methods:

| Method | Description |
|--------|-------------|
| `synchronize()` | Fetches settings from guardsarm-indexer and applies changes. Non-throwing: failures are handled internally |
| `requestShutdown()` | Signals graceful shutdown; aborts in-flight or future sync operations |

### Trigger Registration (`addTrigger`)

Modules register interest in a specific configuration key via `addTrigger()`:

```cpp
json::Json addTrigger(
    std::string_view key,                              // setting key (e.g. "index_raw_events")
    std::function<void(const json::Json&)> callback,   // invoked on value change
    const json::Json& defaultValue                     // fallback when no persisted value exists
);
```

**Return value**: The last persisted value for the key (from store), or `defaultValue` if no cached state exists. This allows the consumer to initialize itself immediately.

**Rules**:
- Each key can only be registered once. Duplicate registration throws `std::invalid_argument`.
- The callback receives the candidate value. If it throws, the change is **rejected** and the previous value is kept.
- Callbacks are invoked under a unique lock (`m_mutex`), so they must not call back into the manager.

### Synchronization Cycle

Each call to `synchronize()` executes the following steps:

1. **Check shutdown flag** — abort early if shutdown was requested.
2. **Fetch remote settings** — call `IWIndexerConnector::getEngineRemoteConfig()` with retry logic (`m_attempts` retries, `m_waitSeconds` delay between attempts).
3. **Iterate remote keys** — for each key in the fetched JSON object:
   - Skip if no callback is registered for the key.
   - Skip if the value equals `lastConfig` (no change).
   - Invoke the registered callback.
   - If the callback throws, log a warning and keep the previous value.
   - If the callback succeeds, update `lastConfig`.
4. **Persist** — if any value changed, save all `lastConfig` entries to the store via `upsertDoc`.

The synchronization is **non-destructive**: network failures, invalid payloads, or rejected callbacks never corrupt existing state.

### Persistence

Settings are persisted to the store under the document path `remote-config/engine-cnf/0`. The stored document is a flat JSON object mapping keys to their last accepted values:

```json
{
  "index_raw_events": true
}
```

On construction, the manager loads this document (if it exists) to restore `lastConfig` for all previously known keys. This ensures that after a restart, `addTrigger()` returns the last known good value even before the first `synchronize()` completes.

### Shutdown

`requestShutdown()` sets an atomic flag that is checked:
- Before starting synchronization.
- Before each retry attempt during remote fetch (via `executeWithRetry`).
- Between processing individual settings.

This ensures prompt cancellation without leaving partial state.

## Dependencies

| Dependency | CMake Target | Role |
|------------|-------------|------|
| `base` | `base` | Logging, JSON, error handling, `executeWithRetry` |
| `store` | `store::istore` | Persisting/loading cached settings (`IStore`) |
| `wiconnector` | `wIndexerConnector::iwIndexerConnector` | Fetching remote config from guardsarm-indexer (`IWIndexerConnector`) |

## Configuration

Controlled by three configuration keys (defined in `conf/include/conf/keys.hpp`):

| Key | Env Override | Default | Description |
|-----|-------------|---------|-------------|
| `analysisd.remote_conf_indexer_connector_max_retries` | `GUARDSARM_REMOTE_CONF_INDEXER_CONNECTOR_MAX_RETRIES` | `3` | Max retry attempts when fetching remote config |
| `analysisd.remote_conf_indexer_connector_retry_interval` | `GUARDSARM_REMOTE_CONF_INDEXER_CONNECTOR_RETRY_INTERVAL` | `5` | Seconds between retry attempts |
| `analysisd.remote_conf_sync_interval` | `GUARDSARM_REMOTE_CONF_SYNC_INTERVAL` | `120` | Seconds between periodic synchronization calls |

## Integration in `main.cpp`

The module is wired in the engine entry point with the following lifecycle:

```cpp
// 1. Construction — injected with indexer connector, store, and retry config
remoteConf = std::make_shared<confremote::ConfRemoteManager>(
    indexerConnector, store, maxRetries, retryInterval);

// 2. Shutdown hook
exitHandler.add([remoteConf]() { remoteConf->requestShutdown(); });

// 3. Register consumers
const auto initialValue = remoteConf->addTrigger(
    "index_raw_events",
    [rawEventIndexer](const json::Json& v) { rawEventIndexer->hotReloadConf(v); },
    json::Json("false"));
rawEventIndexer->hotReloadConf(initialValue);

// 4. Schedule periodic sync
scheduler->scheduleTask("remote-conf-sync",
    {.interval = remoteConfSyncInterval, .runImmediately = true, ...},
    [remoteConf]() { remoteConf->synchronize(); });
```

Currently, the only registered setting is `"index_raw_events"`, which controls whether the `RawEventIndexer` module forwards raw events to the indexer.

## Thread Safety

- `m_mutex` (`std::shared_mutex`) protects `m_settings`. Both `synchronize()` and `addTrigger()` acquire a unique lock.
- `m_shutdownRequested` is an `std::atomic<bool>` with relaxed ordering, sufficient for signaling intent.
- `m_indexerConnector` and `m_store` are held as `std::weak_ptr` to avoid preventing destruction of shared resources.

## File Structure

```
confremote/
├── CMakeLists.txt                                    # Build targets: iconfremote (INTERFACE), confremote (STATIC)
├── interface/confremote/
│   └── iconfremote.hpp                               # IConfRemote interface (synchronize, requestShutdown)
├── include/confremote/
│   └── confremotemanager.hpp                         # ConfRemoteManager class declaration
├── src/
│   └── confremotemanager.cpp                         # Full implementation
└── test/src/
    ├── unit/
    │   └── confremotemanager_test.cpp                # Unit tests (mocked store + connector)
    └── component/
        └── confremote_refresh_test.cpp               # Component tests (with real RawEventIndexer)
```

## Testing

### Unit Tests (`confremote_utest`)

Test the manager in isolation with mocked `IStore` and `IWIndexerConnector`:

| Test | Verifies |
|------|----------|
| `CanConstructWithStoreAndNullConnector` | Null connector is accepted (sync will fail gracefully) |
| `AddTriggerReturnsDefaultWhenStoreIsEmpty` | Default value returned when no cache exists |
| `AddTriggerReturnsPersistedValueWhenStoreHasCache` | Cached value returned over default |
| `AddTriggerThrowsWhenKeyIsAlreadyRegistered` | Duplicate key throws `std::invalid_argument` |
| `SynchronizeSkipsCallbackWhenValueDoesNotChange` | No callback invoked when remote == lastConfig |
| `SynchronizeNotifiesWhenValueChanges` | Callback invoked and value persisted on change |
| `RejectedCallbackDoesNotCommitValue` | Throwing callback keeps previous state |
| `SynchronizeCallbackRejectsWrongTypeAndPreservesCurrentState` | Type validation via callback works |
| `SynchronizeWithFetchFailureKeepsCurrentState` | Network failure leaves state unchanged |
| `SynchronizeIgnoresUnregisteredKeys` | Keys without callbacks are skipped |
| `SynchronizeWithNullConnectorDoesNotThrow` | Graceful handling of expired connector |

### Component Tests (`confremote_ctest`)

Test integration with the real `RawEventIndexer` module:

| Test | Verifies |
|------|----------|
| `SynchronizePropagatesChangedValue` | End-to-end value change propagation |
| `PersistedValueIsReturnedByAddTriggerOnRecreation` | Restart resilience via store persistence |
| `FreshInstallKeepsRawEventIndexerDisabled` | Default behavior when indexer is unreachable |
| `RestartWithCachedSettingsWhenRemoteUnavailableUsesLastValidValue` | Cached state survives outage |
| `SynchronizeTogglesRawEventIndexer` | Toggle from disabled to enabled |

Build tests with `ENGINE_TEST=y`:

```bash
make --directory=$GUARDSARM_REPO/src -j TARGET=manager ENGINE_TEST=y DEBUG=yes
```

Run:

```bash
$ENGINE_BUILD/source/confremote/confremote_utest
$ENGINE_BUILD/source/confremote/confremote_ctest
```
