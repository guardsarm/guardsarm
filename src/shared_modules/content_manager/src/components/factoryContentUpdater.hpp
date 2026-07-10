/*
 * GuardSarm content manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 12, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FACTORY_CONTENT_UPDATER_HPP
#define _FACTORY_CONTENT_UPDATER_HPP

#include "IndexerDownloader.hpp"
#include "sharedDefs.hpp"
#include "updateIndexerCursor.hpp"
#include "updaterContext.hpp"
#include "utils/chainOfResponsability.hpp"
#include <memory>

/**
 * @class FactoryContentUpdater
 *
 * @brief Creates the orchestration chain for fetching CVE data from the GuardSarm Indexer
 *        and persisting it to the local RocksDB feed database.
 *
 * Pipeline:
 *   IndexerDownloader  →  UpdateIndexerCursor
 *
 * IndexerDownloader fetches CVE documents from the Indexer (initial full load via PIT
 * or incremental update via @timestamp range) and delivers them directly to the
 * fileProcessingCallback without writing intermediate files to disk.
 *
 * UpdateIndexerCursor persists the @timestamp cursor returned by the downloader so
 * that subsequent scheduler cycles perform incremental fetches only.
 */
class FactoryContentUpdater final
{
public:
    /**
     * @brief Creates the Indexer-sourced content update pipeline.
     *
     * @param config Full updater config JSON (must contain an "indexer" sub-object
     *               with at least an "index" field).
     * @return std::shared_ptr<AbstractHandler<std::shared_ptr<UpdaterContext>>>
     *         Head of the pipeline chain (IndexerDownloader).
     */
    static std::shared_ptr<AbstractHandler<std::shared_ptr<UpdaterContext>>> create(nlohmann::json& config)
    {
        logDebug1(WM_CONTENTUPDATER, "FactoryContentUpdater - Starting process");

        auto indexerDownloader = std::make_shared<IndexerDownloader>(config);
        auto cursorUpdater = std::make_shared<UpdateIndexerCursor>();

        indexerDownloader->setNext(cursorUpdater);

        return indexerDownloader;
    }
};

#endif // _FACTORY_CONTENT_UPDATER_HPP
