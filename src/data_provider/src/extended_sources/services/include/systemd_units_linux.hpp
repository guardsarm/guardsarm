/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <string>
#include <vector>
#include "idbus_wrapper.hpp"
#include "json.hpp"

/// @brief Structure to hold information about a systemd unit.
struct SystemdUnit
{
    std::string id;
    std::string description;
    std::string loadState;
    std::string activeState;
    std::string subState;
    std::string following;
    std::string objectPath;
    uint32_t jobId;
    std::string jobType;
    std::string jobPath;

    // Extra properties
    std::string fragmentPath;
    std::string sourcePath;
    std::string user;
    std::string unitFileState;
};

/// @brief Structure to describe a property query for D-Bus.
struct PropertyQueryDesc
{
    std::string propertyName;
    std::string columnName;
    std::string interface;
};

/// @brief Class to collect systemd units information from the systemd service manager.
/// This class uses D-Bus to query systemd for unit information and formats it into a
/// JSON object for easy consumption.
class SystemdUnitsProvider
{
    public:
        /// @brief Constructor that initializes the SystemdUnitsProvider with D-Bus wrapper.
        /// @param dbusWrapper Pointer to the D-Bus wrapper for system operations.
        explicit SystemdUnitsProvider(std::shared_ptr<IDBusWrapper> dbusWrapper);

        /// @brief Default constructor.
        SystemdUnitsProvider();

        /// @brief Collects systemd units information.
        /// @return A JSON object containing the collected systemd units information.
        nlohmann::json collect();

    private:
        /// @brief Pointer to the D-Bus wrapper for system operations.
        std::shared_ptr<IDBusWrapper> m_dbusWrapper;

        /// @brief List of property queries to be executed on D-Bus.
        /// Each entry specifies the property name, the column name to use in the output,
        /// and the D-Bus interface to query.
        const std::vector<PropertyQueryDesc> m_propertyQueryList =
        {
            {"FragmentPath", "fragment_path", "org.freedesktop.systemd1.Unit"},
            {"SourcePath", "source_path", "org.freedesktop.systemd1.Unit"},
            {"User", "user", "org.freedesktop.systemd1.Service"},
            {"UnitFileState", "unit_file_state", "org.freedesktop.systemd1.Unit"},
        };

        /// @brief Retrieves systemd units from the system using D-Bus.
        /// @param output A vector to store the retrieved systemd units.
        /// @return True if the units were successfully retrieved, false otherwise.
        bool getSystemdUnits(std::vector<SystemdUnit>& output);
};
