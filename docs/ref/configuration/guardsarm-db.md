# GuardSarm-DB Configuration

The `<wdb>` section configures automatic backups of the global agent database managed by `guardsarm-manager-db`.

Configuration file: `/var/guardsarm-manager/etc/guardsarm-manager.conf`

Parser: `src/config/src/guardsarm_db-config.c`

For the full guardsarm_db module reference (daemon, socket protocol, schemas) see [GuardSarm DB](../modules/guardsarm_db/README.md).

## Configuration Options

The only currently configurable feature is the `<backup>` sub-element. The `database` attribute selects which database to configure; `global` is the only supported value.

### backup / enabled

Enable automatic backups of the selected database.

- **Default value**: `yes`
- **Allowed values**: `yes`, `no`

### backup / interval

How often a backup is created.

- **Default value**: `1d` (86400 seconds)
- **Allowed values**: Positive time value with optional suffix — `s`, `m`, `h`, `d`

### backup / max_files

Maximum number of backup files to keep. Older backups are removed when this limit is exceeded.

- **Default value**: `3`
- **Allowed values**: Positive integer

## Configuration Example

```xml
<wdb>
  <backup database="global">
    <enabled>yes</enabled>
    <interval>1d</interval>
    <max_files>3</max_files>
  </backup>
</wdb>
```

Backup files are written to `/var/guardsarm-manager/backup/db/`.
