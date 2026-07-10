# Logging Configuration

The `<logging>` section controls the format of manager internal log output (`/var/guardsarm-manager/logs/gsmsec.log`).

Configuration file: `/var/guardsarm-manager/etc/guardsarm-manager.conf`

Parser: `src/shared/src/debug_op.c` (`os_logging_config`)

## Configuration Options

### log_format

Format for manager internal logs. Accepts a single value or a comma-separated pair to enable both formats simultaneously.

- **Default value**: `plain`
- **Allowed values**: `plain`, `json`, `plain,json` (or `json,plain` — order does not matter)

When set to `plain,json`, the manager writes every log line in both formats. Plain output goes to `gsmsec.log`; JSON output goes to `gsmsec.json`.

## Configuration Example

```xml
<!-- Plain text only (default) -->
<logging>
  <log_format>plain</log_format>
</logging>
```

```xml
<!-- JSON only -->
<logging>
  <log_format>json</log_format>
</logging>
```

```xml
<!-- Both formats simultaneously -->
<logging>
  <log_format>plain,json</log_format>
</logging>
```
