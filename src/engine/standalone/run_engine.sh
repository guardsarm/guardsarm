#!/usr/bin/env bash
OLD_PWD="$(pwd)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

DATA_PATH="$SCRIPT_DIR/data"
SOCKET_PATH="$SCRIPT_DIR/sockets"
LOG_PATH="$SCRIPT_DIR/logs"

export GUARDSARM_TZDB_PATH="${DATA_PATH}/tzdb"
export GUARDSARM_GEO_DB_PATH="${DATA_PATH}/mmdb"
export GUARDSARM_ENGINE_STANDALONE="true"
export GUARDSARM_STANDALONE_LOG_LEVEL="info"
export GUARDSARM_STORE_PATH="${DATA_PATH}/store"
export GUARDSARM_OUTPUTS_PATH="${DATA_PATH}/outputs"
export GUARDSARM_KVDB_IOC_PATH="${DATA_PATH}/kvdb-ioc"
export GUARDSARM_CM_RULESET_PATH="${DATA_PATH}/content"
export GUARDSARM_SERVER_API_SOCKET="${SOCKET_PATH}/engine-api.sock"
export GUARDSARM_STREAMLOG_BASE_PATH="${LOG_PATH}"
export GUARDSARM_SERVER_ENABLE_EVENT_PROCESSING="false"
export GUARDSARM_SERVER_API_MAX_RESOURCE_PAYLOAD_SIZE="50000"
export GUARDSARM_SERVER_API_MAX_RESOURCE_KVDB_PAYLOAD_SIZE="100000"

# See README.md for a full description of configuration variables.

# Auto-detect log file path based on environment
if [ -d "/var/log/guardsarm-indexer" ]; then
    # Production environment: guardsarm-indexer installed
    export GUARDSARM_STANDALONE_LOG_FILE_PATH="/var/log/guardsarm-indexer/guardsarm-engine.log"
else
    # Development/CI environment: use local logs directory
    export GUARDSARM_STANDALONE_LOG_FILE_PATH="${LOG_PATH}/guardsarm-engine.log"
fi

export GUARDSARM_STANDALONE_LOG_ROTATION_ENABLED="true"
export GUARDSARM_STANDALONE_LOG_MAX_FILE_SIZE="134217728"        # 128 MB
export GUARDSARM_STANDALONE_LOG_ROTATION_HOUR="0"                # midnight
export GUARDSARM_STANDALONE_LOG_ROTATION_MINUTE="0"
export GUARDSARM_STANDALONE_LOG_MAX_FILES="7"
export GUARDSARM_STANDALONE_LOG_MAX_ACCUMULATED_SIZE="2147483648" # 2 GB
export GUARDSARM_STANDALONE_LOG_COMPRESSION_ENABLED="true"
export GUARDSARM_STANDALONE_LOG_COMPRESSION_LEVEL="5"

# If not exist create directories
mkdir -p "$SOCKET_PATH" "$LOG_PATH"
mkdir -p "${GUARDSARM_OUTPUTS_PATH}/default" # Base outputs path with default/ directory
mkdir -p "${GUARDSARM_CM_RULESET_PATH}" # For Ruleset store

exec "${SCRIPT_DIR}/bin/guardsarm-engine" -f
