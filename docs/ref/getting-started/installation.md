# Installation

This guide provides instructions for installing GuardSarm server and agent components. Before proceeding, verify that your system meets the requirements listed in the [Packages](packages.md) page.

## Server

This section covers single-node and multi-node server installation.

### Download package

Download the GuardSarm manager package for your platform and version. See the [Package Download](packages.md#package-download) section for available repositories and download instructions.

### Installation

Install the downloaded GuardSarm manager package for your platform:

**Debian-based platforms:**

```bash
sudo dpkg -i guardsarm-manager_*.deb
```

**Red Hat-based platforms:**

```bash
sudo rpm -ivh guardsarm-manager-*.rpm
```

### Configuration

#### Deploy certificates

Deploy the SSL certificates for secure communication between the GuardSarm server and indexer. These certificates should be extracted from the `guardsarm-certificates.tar` file generated during the certificate creation process.

```bash
NODE_NAME=node-1

# Create certificates directory
sudo mkdir -p /var/guardsarm-manager/etc/certs

# Extract and deploy certificates
sudo tar -xf guardsarm-certificates.tar -C /var/guardsarm-manager/etc/certs/ ./$NODE_NAME.pem ./$NODE_NAME-key.pem ./root-ca.pem
sudo mv /var/guardsarm-manager/etc/certs/$NODE_NAME.pem /var/guardsarm-manager/etc/certs/manager.pem
sudo mv /var/guardsarm-manager/etc/certs/$NODE_NAME-key.pem /var/guardsarm-manager/etc/certs/manager-key.pem

# Set proper permissions
sudo chmod 500 /var/guardsarm-manager/etc/certs
sudo chmod 400 /var/guardsarm-manager/etc/certs/*
sudo chown -R guardsarm-manager:guardsarm-manager /var/guardsarm-manager/etc/certs
```

**Note:** Replace `node-1` with the name you used when generating the certificates.

#### Configure indexer connection

Configure the GuardSarm server to connect to the GuardSarm indexer using the secure keystore:

```bash
# Set indexer credentials (default: guardsarm-server/guardsarm-server)
sudo /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k username -v guardsarm-server
sudo /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k password -v guardsarm-server
```

Update the indexer configuration in `/var/guardsarm-manager/etc/guardsarm-manager.conf` to specify the indexer IP address:

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

Replace `127.0.0.1` with your indexer IP address if it's running on a different host.

### Start the manager

Start and enable the server service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable guardsarm-manager
sudo systemctl start guardsarm-manager
```

Verify the server is running:

```bash
sudo systemctl status guardsarm-manager
```

### Cluster configuration

The GuardSarm server cluster allows you to scale horizontally by distributing the load across multiple nodes. The cluster comes enabled by default with the following configuration in `/var/guardsarm-manager/etc/guardsarm-manager.conf`:

```xml
<cluster>
  <name>guardsarm</name>
  <node_name>node01</node_name>
  <node_type>master</node_type>
  <key>fd3350b86d239654e34866ab3c4988a8</key>
  <port>1516</port>
  <bind_addr>127.0.0.1</bind_addr>
  <nodes>
      <node>127.0.0.1</node>
  </nodes>
  <hidden>no</hidden>
</cluster>
```

#### Multi-node deployment

For a multi-node cluster deployment, you need to configure one master node and one or more worker nodes. Follow these steps on each node:

1. **On the master node**, edit `/var/guardsarm-manager/etc/guardsarm-manager.conf`:

```xml
<cluster>
  <name>guardsarm</name>
  <node_name>master-node</node_name>
  <node_type>master</node_type>
  <key>fd3350b86d239654e34866ab3c4988a8</key>
  <port>1516</port>
  <bind_addr>0.0.0.0</bind_addr>
  <nodes>
      <node>MASTER_NODE_IP</node>
  </nodes>
  <hidden>no</hidden>
</cluster>
```

Replace `MASTER_NODE_IP` with the actual IP address of the master node.

2. **On each worker node**, edit `/var/guardsarm-manager/etc/guardsarm-manager.conf`:

```xml
<cluster>
  <name>guardsarm</name>
  <node_name>worker-node-01</node_name>
  <node_type>worker</node_type>
  <key>fd3350b86d239654e34866ab3c4988a8</key>
  <port>1516</port>
  <bind_addr>0.0.0.0</bind_addr>
  <nodes>
      <node>MASTER_NODE_IP</node>
  </nodes>
  <hidden>no</hidden>
</cluster>
```

Replace `MASTER_NODE_IP` with the actual IP address of the master node, and use a unique `node_name` for each worker.

3. **Restart the GuardSarm manager service** on all nodes after making configuration changes:

```bash
sudo systemctl restart guardsarm-manager
```

4. **Verify the cluster status** from any node:

```bash
sudo /var/guardsarm-manager/bin/cluster_control -l
```

### Configuration parameters

**`name`**\
Name of the cluster. All nodes must use the same cluster name.

**`node_name`**\
Unique name for each node in the cluster.

**`node_type`**\
Node role, either `master` or `worker`. Only one master node is allowed per cluster.

**`key`**\
Pre-shared key for cluster authentication. All nodes must use the same key.

**`port`**\
Port for cluster communication. Default: `1516`.

**`bind_addr`**\
IP address to bind the cluster listener. Use `0.0.0.0` to listen on all interfaces.

**`nodes`**\
List of master node IP addresses for worker nodes to connect to.

**`hidden`**\
Whether the node is hidden from the cluster. Default: `no`.

## Agent

### Download package

Download the GuardSarm agent package for your platform and version. See the [Package Download](packages.md#package-download) section for available repositories and download instructions.

### Linux

#### Debian-based platforms

```bash
sudo dpkg -i guardsarm-agent_*.deb
```

You can optionally specify configuration parameters:

```bash
sudo GUARDSARM_MANAGER='10.0.0.2' GUARDSARM_AGENT_NAME='web-server-01' dpkg -i guardsarm-agent_*.deb
```

#### Red Hat-based platforms

```bash
sudo rpm -ivh guardsarm-agent-*.rpm
```

You can optionally specify configuration parameters:

```bash
sudo GUARDSARM_MANAGER='10.0.0.2' GUARDSARM_AGENT_NAME='web-server-01' rpm -ivh guardsarm-agent-*.rpm
```

#### SUSE-based platforms

```bash
sudo rpm -ivh guardsarm-agent-*.rpm
```

You can optionally specify configuration parameters:

```bash
sudo GUARDSARM_MANAGER='10.0.0.2' GUARDSARM_AGENT_NAME='web-server-01' rpm -ivh guardsarm-agent-*.rpm
```

#### Starting the agent

After installation, start and enable the agent service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable guardsarm-agent
sudo systemctl start guardsarm-agent
```

Verify the agent is running:

```bash
sudo systemctl status guardsarm-agent
```

### macOS

Install the agent:

```bash
sudo installer -pkg guardsarm-agent-*.pkg -target /
```

You can optionally specify configuration parameters by writing them to `/tmp/guardsarm_envs` before running the installer:

```bash
echo "GUARDSARM_MANAGER='10.0.0.2'" > /tmp/guardsarm_envs && echo "GUARDSARM_AGENT_NAME='macbook-01'" >> /tmp/guardsarm_envs && sudo installer -pkg guardsarm-agent-*.pkg -target /
```

Start the agent service:

```bash
sudo launchctl bootstrap system /Library/LaunchDaemons/com.guardsarm.agent.plist
```

Verify the agent is running:

```bash
sudo /Library/Ossec/bin/guardsarm-control status
```

### Windows

Install the agent silently:

```powershell
guardsarm-agent-*.msi /q
```

You can optionally specify configuration parameters:

```powershell
guardsarm-agent-*.msi /q GUARDSARM_MANAGER="10.0.0.2" GUARDSARM_AGENT_NAME="windows-server-01"
```

For interactive installation, double-click the MSI file and follow the installation wizard.

Start the GuardSarm Agent service:

```powershell
Start-Service -Name guardsarm
```

Verify the agent is running:

```powershell
Get-Service -Name guardsarm
```

### Options

#### Server connection

**`GUARDSARM_MANAGER`**\
Specifies the IP address or hostname of the GuardSarm server. The agent uses this to establish communication with the server.

**`GUARDSARM_MANAGER_PORT`**\
Defines the port used to communicate with the GuardSarm server. Default: `1514`.

#### Enrollment configuration

**`GUARDSARM_REGISTRATION_SERVER`**\
Specifies the IP address or hostname of the enrollment server. When not specified, the value of `GUARDSARM_MANAGER` is used.

**`GUARDSARM_REGISTRATION_PORT`**\
Defines the port used for agent enrollment. Default: `1515`.

**`GUARDSARM_REGISTRATION_PASSWORD`**\
Sets the password required for agent enrollment. This password must match the one configured on the server.

**`GUARDSARM_REGISTRATION_CA`**\
Specifies the path to the CA certificate used to verify the manager's identity during enrollment.

**`GUARDSARM_REGISTRATION_CERTIFICATE`**\
Specifies the path to the agent's certificate for enrollment authentication.

**`GUARDSARM_REGISTRATION_KEY`**\
Specifies the path to the agent's private key for enrollment authentication.

#### Agent identity

**`GUARDSARM_AGENT_NAME`**\
Sets the agent's name for identification in the GuardSarm server. Default: system hostname.

**`GUARDSARM_AGENT_GROUP`**\
Assigns the agent to a specific group upon enrollment. Default: `default`.

#### Advanced options

**`GUARDSARM_KEEP_ALIVE_INTERVAL`**\
Defines the interval in seconds between keep-alive messages sent to the server. When not specified, system defaults apply.

**`GUARDSARM_TIME_RECONNECT`**\
Forces the agent to reconnect to the server every N seconds. Default: disabled.

**`ENROLLMENT_DELAY`**\
Sets a delay in seconds between agent enrollment and the first connection attempt. When not specified, system defaults apply.
