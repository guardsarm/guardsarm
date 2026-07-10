/*
 * GuardSarm Module for routing messages.
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

 #include "wm_inventory_sync.h"
 #include "indexer-config.h"
 #include <cJSON.h>
 #include "sym_load.h"
 #include "inventory_sync.h"

 #ifndef HOST_NAME_MAX
 #define HOST_NAME_MAX 255
 #endif

 static void* gm_inventory_sync_main(gm_inventory_sync_t* data);
 static void gm_inventory_sync_destroy(gm_inventory_sync_t* data);
 static void gm_inventory_sync_stop(gm_inventory_sync_t* data);
 cJSON* gm_inventory_sync_dump(gm_inventory_sync_t* data);

 void* inventory_sync_module = NULL;
 inventory_sync_start_func inventory_sync_start_ptr = NULL;
 inventory_sync_stop_func inventory_sync_stop_ptr = NULL;

 const gm_context GM_INVENTORY_SYNC_CONTEXT = {
     .name = "inventory_sync",
     .start = (gm_routine)gm_inventory_sync_main,
     .destroy = (void (*)(void*))gm_inventory_sync_destroy,
     .dump = (cJSON * (*)(const void*)) gm_inventory_sync_dump,
     .sync = NULL,
     .stop = (void (*)(void*))gm_inventory_sync_stop,
     .query = NULL,
 };

 static void gm_inventory_sync_log_config(cJSON* config_json)
 {
     if (config_json)
     {
         char* config_str = cJSON_PrintUnformatted(config_json);
         if (config_str)
         {
             mtdebug1(GM_INVENTORY_SYNC_LOGTAG, "%s", config_str);
             cJSON_free(config_str);
         }
     }
 }

 void* gm_inventory_sync_main(__attribute__((unused))gm_inventory_sync_t* data)
 {
     mtinfo(GM_INVENTORY_SYNC_LOGTAG, STARTUP_MSG, (int)getpid());
     if (inventory_sync_module = so_get_module_handle("inventory_sync"), inventory_sync_module)
     {
        inventory_sync_start_ptr = so_get_function_sym(inventory_sync_module, "inventory_sync_start");
        inventory_sync_stop_ptr = so_get_function_sym(inventory_sync_module, "inventory_sync_stop");

         if (inventory_sync_start_ptr)
         {
             cJSON* config_json = cJSON_CreateObject();

             if (indexer_config == NULL)
             {
                 cJSON_AddItemToObject(config_json, "indexer", cJSON_CreateObject());
             }
             else
             {
                 cJSON_AddItemToObject(config_json, "indexer", cJSON_Duplicate(indexer_config, TRUE));
             }

             // Add cluster name to inventory sync configurations. From 5.x.x onwards, the cluster is enabled by default.
             char* cluster_name = get_cluster_name();
             cJSON_AddStringToObject(config_json, "clusterName", cluster_name);
             os_free(cluster_name);

             char* manager_node_name = get_node_name();
             cJSON_AddStringToObject(config_json, "clusterNodeName", manager_node_name);
             os_free(manager_node_name);

             // Add max sessions from internal_options (inventory_sync specific)
             int max_sessions = getDefine_Int_default("guardsarm_modules", "max_sessions", 1, 100000, 1000);
             cJSON_AddNumberToObject(config_json, "maxSessions", max_sessions);

             // Add input worker queue size from internal_options
             int queue_size = getDefine_Int_default("guardsarm_modules", "inventory_sync_queue_size", 100, 1000000, 10000);
             cJSON_AddNumberToObject(config_json, "queueSize", queue_size);

             // Add global DataValue quota from internal_options
             int data_value_quota = getDefine_Int_default("guardsarm_modules", "inventory_sync_data_value_quota", 1, 1000000000, 500000);
             cJSON_AddNumberToObject(config_json, "dataValueQuota", data_value_quota);

             gm_inventory_sync_log_config(config_json);
             inventory_sync_start_ptr(mtLoggingFunctionsWrapper, config_json);
             cJSON_Delete(config_json);
         }
         else
         {
             mtwarn(GM_INVENTORY_SYNC_LOGTAG, "Unable to start inventory_sync module.");
             return NULL;
         }
     }
     else
     {
         mtwarn(GM_INVENTORY_SYNC_LOGTAG, "Unable to load inventory_sync module.");
         return NULL;
     }

     return NULL;
 }

 void gm_inventory_sync_destroy(gm_inventory_sync_t* data)
 {
     free(data);
 }

 void gm_inventory_sync_stop(__attribute__((unused)) gm_inventory_sync_t* data)
 {
     mtinfo(GM_INVENTORY_SYNC_LOGTAG, "Module finished.");
     if (inventory_sync_stop_ptr)
     {
         inventory_sync_stop_ptr();
     }
     else
     {
         mtwarn(GM_INVENTORY_SYNC_LOGTAG, "Unable to stop inventory_sync module.");
     }
 }

 gmodule* gm_inventory_sync_read()
 {
     gmodule* module;

     os_calloc(1, sizeof(gmodule), module);
     module->context = &GM_INVENTORY_SYNC_CONTEXT;
     module->tag = strdup(module->context->name);
     mtdebug1(GM_INVENTORY_SYNC_LOGTAG, "Loaded Inventory sync module.");
     return module;
 }

 cJSON* gm_inventory_sync_dump(__attribute__((unused)) gm_inventory_sync_t* data)
 {
     cJSON* root = cJSON_CreateObject();

     return root;
 }
