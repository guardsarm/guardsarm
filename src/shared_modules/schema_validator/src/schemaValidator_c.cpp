#include "schemaValidator_c.h"
#include "schemaValidator.hpp"
#include <cstring>
#include <string>

extern "C" {

    bool schema_validator_initialize(void)
    {
        try
        {
            auto& factory = SchemaValidator::SchemaValidatorFactory::getInstance();
            return factory.initialize();
        }
        // LCOV_EXCL_START
        catch (...)
        {
            return false;
        }

        // LCOV_EXCL_STOP
    }

    bool schema_validator_is_initialized(void)
    {
        try
        {
            auto& factory = SchemaValidator::SchemaValidatorFactory::getInstance();
            return factory.isInitialized();
        }
        // LCOV_EXCL_START
        catch (...)
        {
            return false;
        }

        // LCOV_EXCL_STOP
    }

    bool schema_validator_validate(const char* indexPattern,
                                   const char* message,
                                   char** errorMessage)
    {
        if (!indexPattern || !message)
        {
            return false;
        }

        // Initialize output parameter
        if (errorMessage)
        {
            *errorMessage = nullptr;
        }

        try
        {
            auto& factory = SchemaValidator::SchemaValidatorFactory::getInstance();

            if (!factory.isInitialized())
            {
                return true; // If not initialized, consider it valid (backward compatibility)
            }

            auto validator = factory.getValidator(std::string(indexPattern));

            if (!validator)
            {
                // GuardSarm: no validator registered for this index -> fail OPEN (treat as
                // valid) rather than discarding real telemetry. Mirrors the
                // factory-not-initialized case above and the syscollector
                // validateSchemaAndLog fail-open. Without this, FIM (registry/file) and
                // SCA events are silently dropped whenever their index has no registered
                // schema validator, which is exactly what happened on the Windows agent.
                return true;
            }

            auto result = validator->validate(std::string(message));

            if (!result.isValid)
            {
                // Build error message
                if (errorMessage)
                {
                    std::string errors;

                    for (const auto& error : result.errors)
                    {
                        if (!errors.empty())
                        {
                            errors += "\n";
                        }

                        errors += error;
                    }

                    *errorMessage = strdup(errors.c_str());
                }

                return false;
            }

            // Message is valid
            return true;
        }
        // LCOV_EXCL_START
        catch (...)
        {
            return false;
        }

        // LCOV_EXCL_STOP
    }

} // extern "C"
