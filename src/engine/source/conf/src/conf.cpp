#include <conf/conf.hpp>

#include <filesystem>
#include <unistd.h>

#include <base/process.hpp>
#include <fmt/format.h>

#include <conf/keys.hpp>

namespace conf
{

using namespace internal;

Conf::Conf(std::shared_ptr<IFileLoader> fileLoader)
    : m_fileLoader(fileLoader)
    , m_loaded(false)
{
    if (!m_fileLoader)
    {
        throw std::invalid_argument("The file loader cannot be null.");
    }

    // fs path
    const std::filesystem::path guardsarmRoot = base::process::getGuardSarmHome();

    // Register available configuration units with Default Settings

    // Logging module
    addUnit<int>(key::LOGGING_LEVEL, "GUARDSARM_LOG_LEVEL", 0);

    // Standalone Logging module
    addUnit<std::string>(key::STANDALONE_LOGGING_LEVEL, "GUARDSARM_STANDALONE_LOG_LEVEL", "info");

    // Store module
    addUnit<std::string>(key::STORE_PATH, "GUARDSARM_STORE_PATH", (guardsarmRoot / "data/store").c_str());

    // Default outputs
    addUnit<std::string>(key::OUTPUTS_PATH, "GUARDSARM_OUTPUTS_PATH", (guardsarmRoot / "etc/outputs/").c_str());

    // Default kvdb ioc
    addUnit<std::string>(key::KVDB_IOC_PATH, "GUARDSARM_KVDB_IOC_PATH", (guardsarmRoot / "data/kvdb-ioc").c_str());

    // Content Manager
    addUnit<std::string>(key::CM_RULESET_PATH, "GUARDSARM_CM_RULESET_PATH", (guardsarmRoot / "data/ruleset").c_str());
    addUnit<size_t>(key::CM_SYNC_INTERVAL, "GUARDSARM_CM_SYNC_INTERVAL", 120);
    addUnit<size_t>(key::IOC_SYNC_INTERVAL, "GUARDSARM_IOC_SYNC_INTERVAL", 360);

    // Geo module
    addUnit<size_t>(key::GEO_SYNC_INTERVAL, "GUARDSARM_GEO_SYNC_INTERVAL", 360);
    addUnit<std::string>(key::GEO_DB_PATH, "GUARDSARM_GEO_DB_PATH", (guardsarmRoot / "data/mmdb").c_str());
    addUnit<std::string>(
        key::GEO_MANIFEST_URL,
        "GUARDSARM_GEO_MANIFEST_URL",
        "https://guardsarm-cloud-cti-web-components-dev.s3.us-east-2.amazonaws.com/maxmind_geoip/manifest.json");
    addUnit<size_t>(key::GEO_DOWNLOAD_TIMEOUT, "GUARDSARM_GEO_DOWNLOAD_TIMEOUT", 60000);

    // Indexer connector
    addUnit<std::vector<std::string>>(key::INDEXER_HOST, "GUARDSARM_INDEXER_HOSTS", {"http://localhost:9200"});
    addUnit<std::string>(key::INDEXER_USER, "GUARDSARM_INDEXER_USER", "guardsarm-server");
    addUnit<std::string>(key::INDEXER_PASSWORD, "GUARDSARM_INDEXER_PASSWORD", "guardsarm-server");
    addUnit<std::vector<std::string>>(key::INDEXER_SSL_CA_BUNDLE, "GUARDSARM_INDEXER_SSL_CA_BUNDLE", {});
    addUnit<std::string>(key::INDEXER_SSL_CERTIFICATE, "GUARDSARM_INDEXER_SSL_CERTIFICATE", "");
    addUnit<std::string>(key::INDEXER_SSL_KEY, "GUARDSARM_INDEXER_SSL_KEY", "");
    addUnit<size_t>(key::INDEXER_QUEUE_MAX_EVENTS, "GUARDSARM_INDEXER_QUEUE_MAX_EVENTS", 0x1 << 17);
    addUnit<size_t>(
        key::CMSYNC_INDEXER_CONNECTOR_SYNC_BATCH_SIZE, "GUARDSARM_CMSYNC_INDEXER_CONNECTOR_SYNC_BATCH_SIZE", 100);
    // IOC Sync
    addUnit<size_t>(key::IOC_INDEXER_CONNECTOR_MAX_RETRIES, "GUARDSARM_IOC_INDEXER_CONNECTOR_MAX_RETRIES", 3);
    addUnit<size_t>(key::IOC_INDEXER_CONNECTOR_RETRY_INTERVAL, "GUARDSARM_IOC_INDEXER_CONNECTOR_RETRY_INTERVAL", 5);
    addUnit<size_t>(key::IOC_INDEXER_CONNECTOR_SYNC_BATCH_SIZE, "GUARDSARM_IOC_INDEXER_CONNECTOR_SYNC_BATCH_SIZE", 1000);
    // CM Sync
    addUnit<size_t>(key::CMSYNC_INDEXER_CONNECTOR_MAX_RETRIES, "GUARDSARM_CMSYNC_INDEXER_CONNECTOR_MAX_RETRIES", 3);
    addUnit<size_t>(key::CMSYNC_INDEXER_CONNECTOR_RETRY_INTERVAL, "GUARDSARM_CMSYNC_INDEXER_CONNECTOR_RETRY_INTERVAL", 5);
    // Remote Configuration Sync
    addUnit<size_t>(
        key::REMOTE_CONF_INDEXER_CONNECTOR_MAX_RETRIES, "GUARDSARM_REMOTE_CONF_INDEXER_CONNECTOR_MAX_RETRIES", 3);
    addUnit<size_t>(
        key::REMOTE_CONF_INDEXER_CONNECTOR_RETRY_INTERVAL, "GUARDSARM_REMOTE_CONF_INDEXER_CONNECTOR_RETRY_INTERVAL", 5);

    // RemoteConfig Indexer
    addUnit<size_t>(key::REMOTE_CONF_SYNC_INTERVAL, "GUARDSARM_REMOTE_CONF_SYNC_INTERVAL", 120);

    // Queue event module
    addUnit<size_t>(key::EVENT_QUEUE_SIZE, "GUARDSARM_EVENT_QUEUE_SIZE", 0x1 << 17);
    addUnit<size_t>(key::EVENT_QUEUE_EPS, "GUARDSARM_EVENT_QUEUE_EPS", 0);

    // Orchestrator module
    addUnit<int>(key::ORCHESTRATOR_THREADS, "GUARDSARM_ORCHESTRATOR_THREADS", 0);

    // Http server module
    addUnit<std::string>(
        key::SERVER_API_SOCKET, "GUARDSARM_SERVER_API_SOCKET", (guardsarmRoot / "queue/sockets/analysis").c_str());
    addUnit<int>(key::SERVER_API_TIMEOUT, "GUARDSARM_SERVER_API_TIMEOUT", 5000);
    addUnit<int64_t>(key::SERVER_API_PAYLOAD_MAX_BYTES, "GUARDSARM_SERVER_API_PAYLOAD_MAX_BYTES", 0);

    // Event server - enriched (http)
    addUnit<std::string>(key::SERVER_ENRICHED_EVENTS_SOCKET,
                         "GUARDSARM_SERVER_ENRICHED_EVENTS_SOCKET",
                         (guardsarmRoot / "queue/sockets/queue-http.sock").c_str());

    // Enable or disable server event processing
    addUnit<bool>(key::SERVER_ENABLE_EVENT_PROCESSING, "GUARDSARM_SERVER_ENABLE_EVENT_PROCESSING", true);

    // TZDB module
    addUnit<std::string>(key::TZDB_PATH, "GUARDSARM_TZDB_PATH", (guardsarmRoot / "data/tzdb").c_str());
    addUnit<bool>(key::TZDB_AUTO_UPDATE, "GUARDSARM_TZDB_AUTO_UPDATE", false);
    addUnit<std::string>(key::TZDB_FORCE_VERSION_UPDATE, "GUARDSARM_TZDB_FORCE_VERSION_UPDATE", "");

    // Streamlog module
    addUnit<std::string>(key::STREAMLOG_BASE_PATH, "GUARDSARM_STREAMLOG_BASE_PATH", (guardsarmRoot / "logs/").c_str());
    addUnit<bool>(key::STREAMLOG_SHOULD_COMPRESS, "GUARDSARM_STREAMLOG_SHOULD_COMPRESS", true);
    addUnit<size_t>(key::STREAMLOG_COMPRESSION_LEVEL, "GUARDSARM_STREAMLOG_COMPRESSION_LEVEL", 5);
    addUnit<std::string>(
        key::STREAMLOG_EVENTS_PATTERN, "GUARDSARM_STREAMLOG_EVENTS_PATTERN", "${YYYY}/${MMM}/guardsarm-${name}-${DD}");
    addUnit<size_t>(key::STREAMLOG_EVENTS_MAX_SIZE, "GUARDSARM_STREAMLOG_EVENTS_MAX_SIZE", 0);
    addUnit<size_t>(key::STREAMLOG_EVENTS_BUFFER_SIZE, "GUARDSARM_STREAMLOG_EVENTS_BUFFER_SIZE", 0x1 << 20);
    addUnit<std::string>(
        key::STREAMLOG_DUMPER_PATTERN, "GUARDSARM_STREAMLOG_DUMPER_PATTERN", "${YYYY}/${MMM}/guardsarm-${name}-${DD}");
    addUnit<size_t>(key::STREAMLOG_DUMPER_MAX_SIZE, "GUARDSARM_STREAMLOG_DUMPER_MAX_SIZE", 0);
    addUnit<size_t>(key::STREAMLOG_DUMPER_BUFFER_SIZE, "GUARDSARM_STREAMLOG_DUMPER_BUFFER_SIZE", 0x1 << 20);

    addUnit<std::string>(key::STREAMLOG_METRICS_PATTERN, "GUARDSARM_STREAMLOG_METRICS_PATTERN", "${YYYY}-${MM}-${DD}");
    addUnit<size_t>(key::STREAMLOG_METRICS_MAX_SIZE, "GUARDSARM_STREAMLOG_METRICS_MAX_SIZE", 10 * 1024 * 1024);
    addUnit<size_t>(key::STREAMLOG_METRICS_BUFFER_SIZE, "GUARDSARM_STREAMLOG_METRICS_BUFFER_SIZE", 0x1 << 20);
    addUnit<size_t>(key::STREAMLOG_MAX_FILES, "GUARDSARM_STREAMLOG_MAX_FILES", 90);
    addUnit<size_t>(
        key::STREAMLOG_MAX_ACCUMULATED_SIZE, "GUARDSARM_STREAMLOG_MAX_ACCUMULATED_SIZE", 20ULL * 1024 * 1024 * 1024);

    // Metrics module
    addUnit<bool>(key::METRICS_LOG_ENABLED, "GUARDSARM_METRICS_LOG_ENABLED", false);
    addUnit<size_t>(key::METRICS_LOG_INTERVAL, "GUARDSARM_METRICS_LOG_INTERVAL", 1);

    // Dumper module
    addUnit<bool>(key::DUMPER_ENABLED, "GUARDSARM_DUMPER_ENABLED", false);

    // Process module
    addUnit<std::string>(key::PID_FILE_PATH, "GUARDSARM_ENGINE_PID_FILE_PATH", (guardsarmRoot / "var/run/").c_str());
    addUnit<std::string>(key::GROUP, "GUARDSARM_ENGINE_GROUP", "guardsarm-manager");
    addUnit<bool>(key::SKIP_GROUP_CHANGE, "GUARDSARM_SKIP_GROUP_CHANGE", false);

    // API modules
    addUnit<int64_t>(key::API_RESOURCE_PAYLOAD_MAX_BYTES, "GUARDSARM_SERVER_API_MAX_RESOURCE_PAYLOAD_SIZE", 50'000);
    addUnit<int64_t>(
        key::API_RESOURCE_KVDB_PAYLOAD_MAX_BYTES, "GUARDSARM_SERVER_API_MAX_RESOURCE_KVDB_PAYLOAD_SIZE", 100'000);
};

void Conf::validate(const OptionMap& config) const
{
    for (const auto& [key, unit] : m_units)
    {
        auto it = config.find(key);
        if (it == config.end())
        {
            continue; // The configuration is not set for this key, ignore it
        }

        const auto& valueStr = it->second;
        const auto unitType = unit->getType();
        switch (unitType)
        {
            case UnitConfType::INTEGER:
            {
                std::size_t pos = 0;
                try
                {
                    auto v = std::stoll(valueStr, &pos);

                    if (pos != valueStr.size())
                    {
                        throw std::runtime_error(fmt::format(
                            "Invalid configuration type for key '{}'. Extra characters found in integer: '{}'.",
                            key,
                            valueStr.substr(pos)));
                    }
                }
                catch (const std::invalid_argument& e)
                {
                    throw std::runtime_error(
                        fmt::format("Invalid configuration type for key '{}'. Could not parse '{}'.", key, valueStr));
                }
                catch (const std::out_of_range& e)
                {
                    throw std::runtime_error(
                        fmt::format("Invalid configuration type for key '{}'. Value out of range for integer: '{}'.",
                                    key,
                                    valueStr));
                }

                break;
            }

            case UnitConfType::STRING:
            {
                break;
            }

            case UnitConfType::STRING_LIST:
            {
                // Detect list-style formatting with brackets: [a,b]
                if (valueStr.front() == '[' && valueStr.back() == ']')
                {
                    throw std::runtime_error(fmt::format(
                        "Invalid configuration type for key '{}'. Bracket notation '[...]' is not allowed: '{}'.",
                        key,
                        valueStr));
                }

                break;
            }

            case UnitConfType::BOOL:
            {
                std::string lowerVal = valueStr;
                std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), ::tolower);
                if (lowerVal != "true" && lowerVal != "false")
                {
                    throw std::runtime_error(fmt::format(
                        "Invalid configuration type for key '{}'. Expected boolean, got '{}'.", key, valueStr));
                }
                break;
            }

            default: throw std::logic_error(fmt::format("Invalid configuration type for key '{}'.", key));
        }
    }
}

void Conf::load()
{
    if (m_loaded)
    {
        throw std::logic_error("The configuration is already loaded.");
    }
    m_loaded = true;

    // Only load the internal configuration if we are not in standalone mode
    if (!base::process::isStandaloneModeEnable())
    {
        auto fileConf = (*m_fileLoader)();
        validate(fileConf);
        m_fileConfig = std::move(fileConf);
    }
}

} // namespace conf
