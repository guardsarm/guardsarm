# Output Reference

The output stage is responsible for sending alerts to different destinations. This stage is only supported by
`outputs assets` and can have multiple outputs. Each output can have its own configuration.


## First of

To choose between the different output methods the `first_of` stage can be used.
This acts as an if else block. The first `check` that returns true will execute what's inside the `then` block, this block can be filled with `File` or `Indexer` blocks.

### Signature

```yaml
outputs:
  - first_of:
    - check: A
      then:
        - guardsarm-indexer:
            index: "A"

    - check: B
      then:
        - guardsarm-indexer:
            index: "B"

    - check: true
      then:
        - guardsarm-indexer:
            file: "C"
```

### Parameters

Accepts any array of `check` `then` blocks in each item the order is mandatory and will be respected as an order of execution. Ideally the last option should act as a fallback case.

### Asset example

```yaml
name: output/indexer/0

metadata:
  module: guardsarm
  title: Indexer data stream outputs
  description: Output integrations events to guardsarm-indexer

outputs:
  - first_of:
    - check: >-
        $guardsarm.integration.category != "cloud-services" OR
        (NOT starts_with($guardsarm.integration.name, "aws")
        AND NOT starts_with($guardsarm.integration.name, "azure")
        AND NOT starts_with($guardsarm.integration.name, "gcp"))

      then:
        - guardsarm-indexer:
            index: "guardsarm-events-v5-${guardsarm.integration.category}"

    - check: starts_with($guardsarm.integration.name, "gcp")
      then:
        - guardsarm-indexer:
            index: "guardsarm-events-v5-${guardsarm.integration.category}-gcp"

    - check: starts_with($guardsarm.integration.name, "azure")
      then:
        - guardsarm-indexer:
            index: "guardsarm-events-v5-${guardsarm.integration.category}-azure"

    - check: starts_with($guardsarm.integration.name, "aws")
      then:
        - guardsarm-indexer:
            index: "guardsarm-events-v5-${guardsarm.integration.category}-aws"

```


## File

The `file` output sends events to a file. This output supports compression and rotation.
Each policy's `originSpace` is prepended to the channel name so that different spaces
write to isolated streamlog channels (e.g., `standard-guardsarm-events-v5`).

### Signature

```yaml
file: "guardsarm-events-v5"
```

### Parameters

The parameter is a base channel name. The effective streamlog channel is derived as
`{originSpace}-{channelName}`, where `originSpace` comes from the policy definition.

### Asset example

```yaml
name: output/file-output-integrations/0

metadata:
  module: guardsarm
  title: file output event
  description: Output integrations events to a file
  compatibility: >
    This decoder has been tested on GuardSarm version 5.x
  versions:
    - 5.x
  author:
    name: Wazuh, Inc.
    date: 2022/11/08
  references:
    - ""

outputs:
  - file: "guardsarm-events-v5"
```

## Indexer

The `indexer` output sends alerts to `guardsarm-indexer` for indexing.

### Signature

```yaml
guardsarm-indexer:
    index: ${INDEX}
```

### Parameters

| Name | type | required | Description |
|------|------|----------|-------------|
| index | string | yes | Data Stream name where the alerts will be indexed. Should be a valid guardsarm-indexer data stream name and start with `guardsarm-events-v5-`. |

Index name can be expanded with placeholders like `index-name-${PH1}-${PH2}`. If PHX results in an existing string reference to the event it will be replaced in runtime if not will be fail and the alert will not be sent to the indexer.
The replacement text is not sanitized, so referenced field values must already contain only valid index characters.

### Asset example

```yaml
name: output/indexer/0

metadata:
  module: guardsarm
  title: Indexer output event
  description: Output integrations events to guardsarm-indexer
  compatibility: >
    This decoder has been tested on GuardSarm version 5.0
  versions:
    - ""
  author:
    name: Wazuh, Inc.
    date: 2025/12/01
  references:
    - ""

outputs:
  - guardsarm-indexer:
      index: "guardsarm-events-v5-${guardsarm.integration.category}"
```

> [!TIP]
> The `$(DATE)` is special placeholder and replaced by the current date in the format `YYYY.MM.DD` when the alert is indexed.
