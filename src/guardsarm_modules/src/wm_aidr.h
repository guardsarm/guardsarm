/*
 * GuardSarm Module for native AIDR (AI Detection & Response) telemetry
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Native on-agent AI-surface telemetry collector — the endpoint half of the
 * AIDR module. Sweeps the host for:
 *   * ai_runtime   — local AI/LLM runtime processes (ollama, lm-studio,
 *                    llama.cpp, vllm, text-generation-webui, jan, gpt4all, …)
 *                    with pid, executable, listen port and loaded models;
 *   * llm_api_call — outbound connections from host processes to known AI
 *                    SaaS API endpoints (endpoint->provider map resolved to
 *                    IPs at runtime);
 *   * mcp_server   — MCP server processes and MCP servers registered in
 *                    well-known client configs (Claude Desktop, Cursor, …);
 *   * model_file   — local model artifacts (gguf/safetensors/pt/onnx) with
 *                    size, sha256 and derived source registry.
 * Each observation streams as one JSON event to the manager through the
 * standard agent queue (location "aidr"), mirroring the native EDR module.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized
 * copying, distribution, modification, or use is prohibited except under a
 * written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AIDR_H
#define GM_AIDR_H

#include "wmodules_def.h"
#include "os_xml.h"

#define GM_AIDR_CONTEXT_NAME     "aidr"
#define GM_AIDR_LOGTAG           ARGV0 ":aidr"
#define GM_AIDR_DEFAULT_INTERVAL 300               // seconds between sweeps
#define GM_AIDR_DEFAULT_HASH_MB  256               // max model size to sha256
#define GM_AIDR_LOG_PATH         "logs/aidr-telemetry.log"

typedef struct gm_aidr_flags_t {
    unsigned int enabled:1;                        // main switch
    unsigned int runtimes:1;                       // ai_runtime telemetry
    unsigned int api_calls:1;                      // llm_api_call telemetry
    unsigned int mcp:1;                            // mcp_server telemetry
    unsigned int models:1;                         // model_file telemetry
    unsigned int running:1;                        // module is running
} gm_aidr_flags_t;

typedef struct gm_aidr_t {
    unsigned int interval;                         // sweep interval (seconds)
    gm_aidr_flags_t flags;                         // feature switches
    long max_eps;                                  // max events per second
    long max_hash_mb;                              // only sha256 models <= this size
} gm_aidr_t;

extern const gm_context GM_AIDR_CONTEXT;           // module context

// Parse the <wodle name="aidr"> configuration block.
int gm_aidr_read(const OS_XML *xml, XML_NODE nodes, gmodule *module);

#endif // GM_AIDR_H
