/*
 * GuardSarm AIDR (AI Detection & Response) — pure classification helpers.
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Dependency-free static helpers shared by the Linux and Windows AIDR
 * collectors (wm_aidr.c) and unit-testable standalone
 * (guardsarm_modules/tests/test_wm_aidr_classify.c): AI/LLM runtime
 * process classification, AI SaaS endpoint -> provider mapping, and model
 * artifact format classification. Kept header-only (static) so the test
 * harness compiles it without the agent build tree.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized
 * copying, distribution, modification, or use is prohibited except under a
 * written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AIDR_CLASSIFY_H
#define GM_AIDR_CLASSIFY_H

#include <string.h>
#include <ctype.h>

/* ---------------------------------------------------------------- helpers */

// Case-insensitive substring search (portable strcasestr).
static const char *aidr_stristr(const char *hay, const char *needle) {
    if (!hay || !needle || !needle[0]) {
        return hay;
    }
    size_t nlen = strlen(needle);
    for (const char *p = hay; *p; p++) {
        size_t i = 0;
        while (i < nlen && p[i] &&
               tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) {
            return p;
        }
    }
    return NULL;
}

/* -------------------------------------------- AI/LLM runtime classification */

// A known local LLM runtime, matched against the process basename (exact,
// case-insensitive, ".exe" stripped) or — when basename_only == 0 — against
// the full command line / executable path as a substring.
typedef struct aidr_runtime_sig {
    const char *match;         // token to match
    const char *name;          // canonical runtime name emitted in telemetry
    int basename_only;         // 1 = exact basename match; 0 = substring of cmdline/path
} aidr_runtime_sig;

static const aidr_runtime_sig AIDR_RUNTIMES[] = {
    {"ollama",                  "ollama",                1},
    {"ollama app",              "ollama",                0},
    {"llama-server",            "llama.cpp",             1},
    {"llama-cli",               "llama.cpp",             1},
    {"llamafile",               "llamafile",             1},
    {"lm-studio",               "lm-studio",             0},
    {"lm studio",               "lm-studio",             0},
    {"lms",                     "lm-studio",             1},
    {"vllm",                    "vllm",                  0},
    {"text-generation-webui",   "text-generation-webui", 0},
    {"text-generation-launcher","text-generation-inference", 1},
    {"jan",                     "jan",                   1},
    {"gpt4all",                 "gpt4all",               0},
    {"koboldcpp",               "koboldcpp",             0},
    {"local-ai",                "localai",               1},
    {"localai",                 "localai",               1},
    {NULL, NULL, 0},
};

// Classify a process as a local AI/LLM runtime. `basename` is the executable
// basename (comm on Linux, szExeFile on Windows); `context` is the command
// line or full executable path (may be NULL/empty). Returns the canonical
// runtime name or NULL.
static const char *aidr_classify_runtime(const char *basename_in, const char *context) {
    char base[256] = "";
    if (basename_in) {
        size_t n = strlen(basename_in);
        if (n >= sizeof(base)) n = sizeof(base) - 1;
        memcpy(base, basename_in, n);
        base[n] = '\0';
        // strip a trailing ".exe" (Windows)
        size_t bl = strlen(base);
        if (bl > 4 && base[bl - 4] == '.' &&
            tolower((unsigned char)base[bl - 3]) == 'e' &&
            tolower((unsigned char)base[bl - 2]) == 'x' &&
            tolower((unsigned char)base[bl - 1]) == 'e') {
            base[bl - 4] = '\0';
        }
    }
    for (int i = 0; AIDR_RUNTIMES[i].match; i++) {
        const aidr_runtime_sig *s = &AIDR_RUNTIMES[i];
        if (s->basename_only) {
            // exact, case-insensitive basename match
            if (base[0] && strlen(base) == strlen(s->match) && aidr_stristr(base, s->match) == base) {
                return s->name;
            }
        } else {
            if ((base[0] && aidr_stristr(base, s->match)) ||
                (context && context[0] && aidr_stristr(context, s->match))) {
                return s->name;
            }
        }
    }
    return NULL;
}

/* ----------------------------------------- AI SaaS endpoint -> provider map */

// Known public AI/LLM API endpoints. Kept aligned with the server-side AIDR
// KNOWN_AI_ENDPOINTS map (guardsarm-ai-soc aidr_normalization.py) — resolvable
// API hosts only (the collector resolves these to IPs and matches egress).
typedef struct aidr_provider_host {
    const char *host;
    const char *provider;
} aidr_provider_host;

static const aidr_provider_host AIDR_PROVIDER_HOSTS[] = {
    {"api.openai.com",                     "openai"},
    {"chatgpt.com",                        "openai"},
    {"api.anthropic.com",                  "anthropic"},
    {"claude.ai",                          "anthropic"},
    {"generativelanguage.googleapis.com",  "google"},
    {"aistudio.google.com",                "google"},
    {"api.mistral.ai",                     "mistral"},
    {"api.cohere.com",                     "cohere"},
    {"api.together.xyz",                   "together"},
    {"api.groq.com",                       "groq"},
    {"api.perplexity.ai",                  "perplexity"},
    {"openrouter.ai",                      "openrouter"},
    {"api.deepseek.com",                   "deepseek"},
    {"huggingface.co",                     "huggingface"},
    {"api-inference.huggingface.co",       "huggingface"},
    {"api.githubcopilot.com",              "github"},
    {"copilot.microsoft.com",              "microsoft"},
    {"api.x.ai",                           "xai"},
    {NULL, NULL},
};

// Provider name for a known AI endpoint host (exact or suffix match), or NULL.
static const char *aidr_provider_for_host(const char *host) {
    if (!host || !host[0]) {
        return NULL;
    }
    for (int i = 0; AIDR_PROVIDER_HOSTS[i].host; i++) {
        const char *h = AIDR_PROVIDER_HOSTS[i].host;
        size_t hl = strlen(h), gl = strlen(host);
        if (gl == hl && aidr_stristr(host, h) == host) {
            return AIDR_PROVIDER_HOSTS[i].provider;
        }
        // suffix match with a dot boundary (e.g. eu.api.openai.com)
        if (gl > hl && host[gl - hl - 1] == '.' && aidr_stristr(host + (gl - hl), h)) {
            return AIDR_PROVIDER_HOSTS[i].provider;
        }
    }
    return NULL;
}

/* -------------------------------------------- model artifact classification */

// Model artifact format from a file name; returns NULL for non-model files.
// ".bin" is only a model when it matches the HF pytorch naming convention
// (pytorch_model*.bin) — bare .bin is far too generic.
static const char *aidr_model_format(const char *fname) {
    if (!fname) {
        return NULL;
    }
    const char *dot = strrchr(fname, '.');
    if (!dot) {
        return NULL;
    }
    if (strlen(dot) == 5 && aidr_stristr(dot, ".gguf") == dot) return "gguf";
    if (strlen(dot) == 12 && aidr_stristr(dot, ".safetensors") == dot) return "safetensors";
    if (strlen(dot) == 3 && aidr_stristr(dot, ".pt") == dot) return "pt";
    if (strlen(dot) == 4 && aidr_stristr(dot, ".pth") == dot) return "pt";
    if (strlen(dot) == 5 && aidr_stristr(dot, ".onnx") == dot) return "onnx";
    if (strlen(dot) == 4 && aidr_stristr(dot, ".bin") == dot &&
        aidr_stristr(fname, "pytorch_model")) {
        return "pt";
    }
    return NULL;
}

// Ollama blob file names carry their own digest: "sha256-<64 hex>". Returns a
// pointer to the 64-char hex digest inside fname, or NULL.
static const char *aidr_ollama_blob_sha256(const char *fname) {
    if (!fname || strncmp(fname, "sha256-", 7) != 0) {
        return NULL;
    }
    const char *hex = fname + 7;
    for (int i = 0; i < 64; i++) {
        if (!isxdigit((unsigned char)hex[i])) {
            return NULL;
        }
    }
    return hex[64] == '\0' ? hex : NULL;
}

/* ------------------------------------------------ MCP process classification */

// Heuristic: is this process an MCP server? Matches the executable basename
// (mcp-* / *-mcp / *-mcp-server) or tell-tale command-line markers.
static int aidr_is_mcp_process(const char *basename_in, const char *cmdline) {
    if (basename_in && basename_in[0]) {
        if (aidr_stristr(basename_in, "mcp-server") ||
            (strncmp(basename_in, "mcp-", 4) == 0)) {
            return 1;
        }
        size_t bl = strlen(basename_in);
        if (bl > 4 && aidr_stristr(basename_in + bl - 4, "-mcp") == basename_in + bl - 4) {
            return 1;
        }
    }
    if (cmdline && cmdline[0]) {
        if (aidr_stristr(cmdline, "@modelcontextprotocol/") ||
            aidr_stristr(cmdline, "mcp-server") ||
            aidr_stristr(cmdline, "mcp_server") ||
            aidr_stristr(cmdline, "fastmcp")) {
            return 1;
        }
    }
    return 0;
}

#endif // GM_AIDR_CLASSIFY_H
