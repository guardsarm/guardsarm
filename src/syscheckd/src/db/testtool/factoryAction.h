/*
 * GuardSarm Syscheck - Test tool
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 23, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FACTORY_ACTION_H
#define _FACTORY_ACTION_H
#include <memory>
#include "action.h"

class FactoryAction
{
    public:
        static std::unique_ptr<IAction> create(const std::string& actionCode)
        {
            if (0 == actionCode.compare("RemoveFile"))
            {
                return std::make_unique<RemoveFileAction>();
            }
            else if (0 == actionCode.compare("GetFile"))
            {
                return std::make_unique<GetFileAction>();
            }
            else if (0 == actionCode.compare("CountEntries"))
            {
                return std::make_unique<CountEntriesAction>();
            }
            else if (0 == actionCode.compare("UpdateFile"))
            {
                return std::make_unique<UpdateFileAction>();
            }
            else if (0 == actionCode.compare("SearchFile"))
            {
                return std::make_unique<SearchFileAction>();
            }
            else if (0 == actionCode.compare("StartTransaction"))
            {
                return std::make_unique<StartTransactionAction>();
            }
            else if (0 == actionCode.compare("SyncTxnRows"))
            {
                return std::make_unique<SyncTxnRowsAction>();
            }
            else if (0 == actionCode.compare("GetDeletedRows"))
            {
                return std::make_unique<GetDeletedRowsAction>();
            }
            else
            {
                throw std::runtime_error { "Invalid action: " + actionCode };
            }
        }
};

#endif //_FACTORY_ACTION_H
