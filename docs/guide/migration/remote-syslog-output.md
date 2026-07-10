# Migrating Remote Syslog Output to Dashboard Notifications

In previous GuardSarm versions (4.x), forwarding alerts to remote log servers via Syslog was managed directly in the GuardSarm manager's `gsmsec.conf` file using `<syslog_output>` configuration blocks handled by the internal `csyslogd` daemon.

Starting with GuardSarm 5.0, these legacy backend Syslog forwarding capabilities have been removed from the manager. Alert forwarding logic must now be configured directly through the GuardSarm dashboard using the **Notifications** and **Alerting** dashboard plugins via webhooks.

> **Note:** There is no automatic upgrade tooling to migrate your existing GuardSarm 4.x `<syslog_output>` configurations. You must manually recreate your data filtering and forwarding logic in the GuardSarm dashboard. Use the mapping table below to identify which GuardSarm 5.x feature corresponds to each element in your `gsmsec.conf`.

## Configuration mapping (4.x -> 5.x)

The following table maps each `gsmsec.conf` element from GuardSarm 4.x to the corresponding feature in the GuardSarm 5.x dashboard. Entries use the form `section.element` - for example, `syslog_output.server` refers to `<syslog_output><server>` in your `gsmsec.conf`.

> See the [gsmsec.conf reference](#guardsarm-4x-ossecconf-reference) below for the full XML context of this section.

### Syslog output mapping

| 4.x `gsmsec.conf`         | 5.x dashboard                                            | Guide                                                                 |
| ------------------------ | -------------------------------------------------------- | --------------------------------------------------------------------- |
| `syslog_output.server`   | Notifications > Channels > Custom Webhook (target host) | [Step 1](#1-setting-up-a-custom-webhook-notification-channel)         |
| `syslog_output.port`     | Notifications > Channels > Custom Webhook (target port) | [Step 1](#1-setting-up-a-custom-webhook-notification-channel)         |
| `syslog_output.format`   | Alerting > Monitor > Action > Message (Mustache Template)  | [Step 2.2](#22-configuring-triggers-and-actions)                      |
| `<syslog_output>` filter | Alerting > Monitor > Query (data filter)                | [Step 2.1](#21-creating-a-monitor)                                    |
| `syslog_output.level`    | Alerting > Monitor > Trigger / data threshold filter    | [Step 2.2](#22-configuring-triggers-and-actions)                      |
| `syslog_output.group`    | Alerting > Monitor > Query / data filter (`guardsarm.rule.tags`) | [Step 2.1](#21-creating-a-monitor)                                |
| `syslog_output.rule_id`  | Alerting > Monitor > Query / data filter (`guardsarm.rule.id`) | [Step 2.1](#21-creating-a-monitor)                                  |
> **Option scope note:** `protocol` is not a valid `<syslog_output>` option in GuardSarm 4.x `gsmsec.conf` (the documented options are `server`, `port`, `level`, `group`, `rule_id`, `location`, `use_fqdn`, and `format`).

> **GuardSarm 4.x protocol note:** In GuardSarm 4.x, `syslog_output` forwarding uses Syslog transport (UDP by default on port `514`). In GuardSarm 5.x, Notifications uses HTTP/HTTPS POST webhooks for outbound routing. Target endpoints must support HTTP webhook ingestion, or you must deploy a webhook-to-syslog translation layer on the receiving side.

> **Note on Output Formats**: In GuardSarm 4.x, the <format> tag automatically converted the data layout into predefined profiles (json, cef, or splunk). In GuardSarm 5.x, the Notifications plugin does not include these pre-configured encoding profiles. To migrate specific formats (such as ArcSight CEF, Splunk key-value pairs, or custom JSON payloads), the structure must be manually designed inside the Message text block using Mustache syntax variables during the Action configuration phase.

## GuardSarm 4.x gsmsec.conf reference

Below is a typical GuardSarm 4.x configuration block you may have in your `gsmsec.conf`. Use it as a reference when following the migration steps.

> **Port context:** In this legacy 4.x example, port `514` is the standard Syslog transport port (UDP/TCP). In GuardSarm 5.x webhook routing, use the HTTP/HTTPS port exposed by your receiver (for example, `10515`).

```xml
<syslog_output>
  <server>192.168.1.50</server>
  <port>514</port>
  <format>json</format>
  <level>10</level>
  <group>authentication_failed,</group>
</syslog_output>
```

## Migration steps

### Prerequisites

Before proceeding, make sure you have:

- GuardSarm 5.0 or later fully deployed (indexer, manager, dashboard).
- Access to the GuardSarm dashboard as an administrator.
- The target endpoint URL and port prepared to receive incoming webhook traffic.

## 1. Setting up a Custom Webhook Notification Channel

Instead of defining raw server endpoints inside XML blocks, destinations are now managed as reusable notification channels in the dashboard UI.

1. Open the GuardSarm dashboard and go to the **Notifications** plugin.
2. Go to **Channels** and click **Create channel**.
3. Enter a **Name** for the channel (for example, `remote-syslog-webhook-channel`).
4. (Optional) Provide a **Description** clarifying the purpose of this channel.
5. Under **Configurations**, select **Custom webhook** as the **Channel type**.
6. Set the **Method** to `POST`.
7. Under **Define endpoints by**, select **Webhook URL** and enter your endpoint address matching your legacy `syslog_output.server` configuration (for example, `http://192.168.1.50:10515`).
  Use the HTTP/HTTPS listening port of your webhook endpoint here (for example, `10515`), not the legacy Syslog transport port `514`. If your destination only accepts Syslog (UDP/TCP 514), place a webhook-to-syslog bridge in front of it and configure this URL to point to the bridge HTTP endpoint.
8. (Optional) Click **Send test message** to verify network connectivity.
9. Click **Create** to save the channel.

![Notification Channel](../../images/remote-syslog-output/create-notification-channel.png)


## 2. Recreating Data Filters with Alerts and Monitors

In GuardSarm 4.x, you used `<syslog_output>` blocks with tags such as `<level>`, `<group>`, and `<rule_id>` to determine which events were forwarded. In GuardSarm 5.0, the Alerting plugin executes periodic queries directly against your selected indexes.

### 2.1. Creating a monitor

1. Navigate to the **Alerting** plugin in the GuardSarm dashboard and select **Monitors**.
2. Click **Create monitor**.
3. Configure the monitor:
   - **Monitor name:** Enter a name (for example, `Syslog-Forwarding-Monitor`).
  - **Monitor type:** Select **Per query monitor** or **Per document monitor** depending on your forwarding behavior.
  - **Indexes:** Use `guardsarm-findings-v5*` when your filters depend on `guardsarm.rule.*` fields (for example, `guardsarm.rule.level`, `guardsarm.rule.tags`, or `guardsarm.rule.id`).
4. Under **Query / Data filter**, translate your old XML rules into dashboard filter conditions.
  For example, to replicate a legacy `<group>` filter, add a condition where `guardsarm.rule.tags` contains the expected value.

![Notification Monitor](../../images/remote-syslog-output/create-notification-monitor.png)


### 2.2. Configuring triggers and actions

Triggers act as threshold selectors (similar to legacy `<level>` intent), while actions define the outbound payload format.

1. Add a trigger and set its condition.
2. Configure the trigger so it activates when your query matches events, and add a severity filter for the desired keywords in `guardsarm.rule.level` (for example, `low`, `medium`, `high`, or `critical`). If you use a **Per document monitor**, use a document-level trigger condition.
3. Under **Actions**, click **Add notification** and configure:
   - **Action name:** Provide a label (for example, `Send-Syslog-Payload`).
   - **Channel:** Select the custom webhook channel created in [Step 1](#1-setting-up-a-custom-webhook-notification-channel).
  - **Message:** Replace the default monitor/trigger text with a finding-based payload. Use Mustache templates from `ctx.results` to map alert fields to your destination JSON structure. For example:

```json
{
  "alert_id": "{{ctx.results.0.hits.hits.0._id}}",
  "rule_id": "{{ctx.results.0.hits.hits.0._source.guardsarm.rule.id}}",
  "rule_title": "{{ctx.results.0.hits.hits.0._source.guardsarm.rule.title}}",
  "severity_level": "{{ctx.results.0.hits.hits.0._source.guardsarm.rule.level}}",
  "agent_name": "{{ctx.results.0.hits.hits.0._source.guardsarm.agent.name}}",
  "event_dataset": "{{ctx.results.0.hits.hits.0._source.event.dataset}}"
}
```
4. Click **Create** to activate the monitor pipeline.

![Notification Triggers and Actions](../../images/remote-syslog-output/create-notification-trigger-and-action.png)


### 2.3. Testing the forwarder

1. Generate an event on a monitored endpoint that matches your filter conditions.
2. Verify the matching activity appears in **Alerting** under **Alerts** history.
3. Confirm that your ingestion endpoint receives the HTTP POST payload with the expected metadata.

![Listed Alerts](../../images/remote-syslog-output/listed-alerts.png)


<details>
<summary>Example: recreating a legacy syslog forwarding filter</summary>

If your GuardSarm 4.x `gsmsec.conf` contained:

```xml
<syslog_output>
  <server>192.168.1.50</server>
  <port>514</port>
  <level>10</level>
  <group>authentication_failed,</group>
</syslog_output>
```

You can replicate this behavior with the following dashboard workflow:

**Monitor setup:**

- **Type:** Per query monitor
- **Index pattern:** `guardsarm-findings-v5*`
- **Data filter:** `guardsarm.rule.tags` contains `authentication_failed`

**Trigger setup:**

- **Condition:** Trigger when the query returns at least one matching document
- **Data threshold filter:** `guardsarm.rule.level` is one of `informational`, `low`, `medium`, `high`, or `critical` (adjust to the severity values present in your environment)

**Action setup:**

- **Channel:** Your custom webhook endpoint channel
- **Message payload:**

```json
{
  "alert_id": "{{ctx.results.0.hits.hits.0._id}}",
  "rule_description": "{{ctx.results.0.hits.hits.0._source.guardsarm.rule.title}}",
  "severity_level": "{{ctx.results.0.hits.hits.0._source.guardsarm.rule.level}}",
  "agent_host": "{{ctx.results.0.hits.hits.0._source.guardsarm.agent.name}}"
}
```

</details>