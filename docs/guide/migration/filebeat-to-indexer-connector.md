# Migrating from Filebeat to the Indexer Connector

In GuardSarm 4.x, Filebeat was a separate package on the manager host that shipped alerts and archives to the GuardSarm Indexer. In GuardSarm 5.x this is done by the built-in **Indexer Connector**, part of the manager process. Filebeat is removed.

There is no Filebeat configuration to reproduce. The manager connects to the indexer directly, so the only values you need to carry over are the ones you already set in `filebeat.yml`: the indexer hosts, the credentials, and the TLS certificates.

Filebeat is not uninstalled automatically during the manager upgrade. Removing it is part of the procedure below.

## Configuration mapping

| `filebeat.yml` (4.x) | GuardSarm 5.x | Where |
|---|---|---|
| `output.elasticsearch.hosts` | `<indexer><hosts><host>` (include `https://` and port) | `guardsarm-manager.conf` |
| `output.elasticsearch.username` | `guardsarm-manager-keystore -f indexer -k username -v <value>` | Keystore |
| `output.elasticsearch.password` | `guardsarm-manager-keystore -f indexer -k password -v <value>` | Keystore |
| `output.elasticsearch.ssl.certificate_authorities` | `<indexer><ssl><certificate_authorities><ca>` | `guardsarm-manager.conf` |
| `output.elasticsearch.ssl.certificate` | `<indexer><ssl><certificate>` | `guardsarm-manager.conf` |
| `output.elasticsearch.ssl.key` | `<indexer><ssl><key>` | `guardsarm-manager.conf` |

Every other `filebeat.yml` setting is gone and requires no action:

- **Output tuning** (`bulk_max_size`, `worker`, `timeout`, `compression_level`): bulk sizing and retries are handled automatically by the connector and are not configurable.
- **Queue/buffering** (`queue.mem.*`): managed internally by the connector.
- **TLS options** (`ssl.verification_mode`, `ssl.supported_protocols`, `ssl.cipher_suites`): the connector always verifies the server certificate. If you used `verification_mode: none`, you must now provide a valid CA.
- **Processors** (`add_host_metadata`, `add_fields`, `drop_fields`, etc.): not supported. If you relied on them to enrich events, move that logic to the Engine decoders/integrations.
- **Templates and ILM** (`setup.template.*`, `setup.ilm.*`): managed on the GuardSarm Indexer.
- **Logging and sandboxing** (`logging.*`, `seccomp`): not applicable; use the manager logs.

## Migration steps

### 1. Record your current Filebeat settings

From `/etc/filebeat/filebeat.yml`, note:

- Indexer host(s) and port
- Username and password
- Paths to the CA, certificate, and key files

### 2. Deploy the certificates

Place the certificates Filebeat used (previously under `/etc/filebeat/certs/`) at `/var/guardsarm-manager/etc/certs/`:

```bash
NODE_NAME=manager  # Replace with your manager node name

sudo mkdir -p /var/guardsarm-manager/etc/certs

sudo tar -xf guardsarm-certificates.tar -C /var/guardsarm-manager/etc/certs/ \
  ./$NODE_NAME.pem ./$NODE_NAME-key.pem ./root-ca.pem

sudo mv /var/guardsarm-manager/etc/certs/$NODE_NAME.pem \
        /var/guardsarm-manager/etc/certs/manager.pem
sudo mv /var/guardsarm-manager/etc/certs/$NODE_NAME-key.pem \
        /var/guardsarm-manager/etc/certs/manager-key.pem

sudo chmod 500 /var/guardsarm-manager/etc/certs
sudo chmod 400 /var/guardsarm-manager/etc/certs/*
sudo chown -R guardsarm-manager:guardsarm-manager /var/guardsarm-manager/etc/certs
```

### 3. Store the credentials in the keystore

```bash
sudo /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k username -v <your_username>
sudo /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k password -v <your_password>
```

### 4. Configure the `<indexer>` block

Add the block to `/var/guardsarm-manager/etc/guardsarm-manager.conf` using your hosts and certificate paths:

```xml
<indexer>
  <hosts>
    <host>https://127.0.0.1:9200</host>
  </hosts>
  <ssl>
    <certificate_authorities>
      <ca>/var/guardsarm-manager/etc/certs/root-ca.pem</ca>
    </certificate_authorities>
    <certificate>/var/guardsarm-manager/etc/certs/manager.pem</certificate>
    <key>/var/guardsarm-manager/etc/certs/manager-key.pem</key>
  </ssl>
</indexer>
```

For a multi-node indexer cluster, list each node as a separate `<host>`:

```xml
<hosts>
  <host>https://10.0.0.1:9200</host>
  <host>https://10.0.0.2:9200</host>
  <host>https://10.0.0.3:9200</host>
</hosts>
```

Credentials are not set here; they come from the keystore (step 3).

### 5. Stop Filebeat and restart the manager

```bash
sudo systemctl stop filebeat
sudo systemctl disable filebeat
sudo systemctl restart guardsarm-manager
sudo systemctl status guardsarm-manager
```

### 6. Remove Filebeat

Once indexing is confirmed, uninstall the package. This deletes `filebeat.yml`.

**Debian-based:**

```bash
sudo apt-get remove --purge filebeat
sudo rm -rf /etc/filebeat /var/log/filebeat
```

**Red Hat-based:**

```bash
sudo rpm -e filebeat
sudo rm -rf /etc/filebeat /var/log/filebeat
```
