/*
 * Unit tests for the AIDR collector's pure classification helpers
 * (wm_aidr_classify.h): AI runtime classification, endpoint->provider
 * mapping, model-format classification, ollama blob digest parsing and MCP
 * process heuristics. Standalone — no agent build tree needed:
 *
 *   gcc -I../src -o test_wm_aidr_classify test_wm_aidr_classify.c && ./test_wm_aidr_classify
 *
 * Copyright (C) 2026 GuardSarm, Inc.
 */
#include <stdio.h>
#include <string.h>
#include "wm_aidr_classify.h"

static int failures = 0;

#define CHECK(cond, desc) do { \
    if (cond) { printf("  ok  %s\n", desc); } \
    else { printf("FAIL  %s\n", desc); failures++; } \
} while (0)

#define STREQ(a, b) ((a) && (b) && strcmp((a), (b)) == 0)

int main(void) {
    // ---- runtime classification -------------------------------------------
    CHECK(STREQ(aidr_classify_runtime("ollama", NULL), "ollama"),
          "ollama basename -> ollama");
    CHECK(STREQ(aidr_classify_runtime("ollama.exe", NULL), "ollama"),
          "ollama.exe basename -> ollama (exe stripped)");
    CHECK(STREQ(aidr_classify_runtime("llama-server", "./llama-server -m model.gguf"), "llama.cpp"),
          "llama-server -> llama.cpp");
    CHECK(STREQ(aidr_classify_runtime("python3", "python3 -m vllm.entrypoints.openai.api_server"), "vllm"),
          "python -m vllm... cmdline -> vllm");
    CHECK(STREQ(aidr_classify_runtime("python", "/opt/text-generation-webui/server.py --listen"), "text-generation-webui"),
          "text-generation-webui path -> text-generation-webui");
    CHECK(STREQ(aidr_classify_runtime("jan", NULL), "jan"), "jan -> jan");
    CHECK(STREQ(aidr_classify_runtime("Jan.exe", NULL), "jan"), "Jan.exe -> jan");
    CHECK(STREQ(aidr_classify_runtime("gpt4all-chat", NULL), "gpt4all"),
          "gpt4all-chat -> gpt4all");
    CHECK(STREQ(aidr_classify_runtime("LM Studio.exe", "C:\\Program Files\\LM Studio\\LM Studio.exe"), "lm-studio"),
          "LM Studio.exe -> lm-studio");
    CHECK(STREQ(aidr_classify_runtime("lms", NULL), "lm-studio"), "lms -> lm-studio");
    CHECK(STREQ(aidr_classify_runtime("koboldcpp.exe", NULL), "koboldcpp"),
          "koboldcpp.exe -> koboldcpp");
    CHECK(aidr_classify_runtime("janitor", NULL) == NULL,
          "janitor is NOT jan (exact basename match)");
    CHECK(aidr_classify_runtime("bash", "grep ollama-not /var/log") == NULL,
          "grep mentioning 'ollama' does NOT classify (ollama is basename-only)");
    CHECK(aidr_classify_runtime("systemd", NULL) == NULL, "systemd -> none");
    CHECK(aidr_classify_runtime("nginx", "nginx: worker process") == NULL,
          "nginx -> none");

    // ---- provider mapping --------------------------------------------------
    CHECK(STREQ(aidr_provider_for_host("api.openai.com"), "openai"),
          "api.openai.com -> openai");
    CHECK(STREQ(aidr_provider_for_host("api.anthropic.com"), "anthropic"),
          "api.anthropic.com -> anthropic");
    CHECK(STREQ(aidr_provider_for_host("eu.api.openai.com"), "openai"),
          "eu.api.openai.com suffix -> openai");
    CHECK(STREQ(aidr_provider_for_host("generativelanguage.googleapis.com"), "google"),
          "generativelanguage -> google");
    CHECK(STREQ(aidr_provider_for_host("api.mistral.ai"), "mistral"),
          "api.mistral.ai -> mistral");
    CHECK(STREQ(aidr_provider_for_host("openrouter.ai"), "openrouter"),
          "openrouter.ai -> openrouter");
    CHECK(aidr_provider_for_host("api.example.com") == NULL,
          "api.example.com -> none");
    CHECK(aidr_provider_for_host("notapi.openai.com.evil.com") == NULL,
          "embedded-host lookalike -> none (dot-boundary suffix only)");
    CHECK(aidr_provider_for_host("") == NULL, "empty host -> none");
    CHECK(aidr_provider_for_host(NULL) == NULL, "NULL host -> none");

    // ---- model format ------------------------------------------------------
    CHECK(STREQ(aidr_model_format("llama-3.1-8b.Q4_K_M.gguf"), "gguf"),
          ".gguf -> gguf");
    CHECK(STREQ(aidr_model_format("model.safetensors"), "safetensors"),
          ".safetensors -> safetensors");
    CHECK(STREQ(aidr_model_format("weights.pt"), "pt"), ".pt -> pt");
    CHECK(STREQ(aidr_model_format("weights.pth"), "pt"), ".pth -> pt");
    CHECK(STREQ(aidr_model_format("encoder.onnx"), "onnx"), ".onnx -> onnx");
    CHECK(STREQ(aidr_model_format("pytorch_model-00001-of-00002.bin"), "pt"),
          "pytorch_model*.bin -> pt");
    CHECK(aidr_model_format("random.bin") == NULL, "bare .bin -> none");
    CHECK(aidr_model_format("notes.txt") == NULL, ".txt -> none");
    CHECK(aidr_model_format("no_extension") == NULL, "no extension -> none");

    // ---- ollama blob digest ------------------------------------------------
    CHECK(aidr_ollama_blob_sha256(
              "sha256-8eeb52dfb3bb9aefdf9d1ef24b3bdbcf27fb6a20ae9d3dbf5e35ef1d78b3c72e") != NULL,
          "well-formed ollama blob name -> digest extracted");
    CHECK(aidr_ollama_blob_sha256("sha256-shortdigest") == NULL,
          "short digest -> rejected");
    CHECK(aidr_ollama_blob_sha256("model.gguf") == NULL,
          "non-blob name -> rejected");

    // ---- MCP process heuristics -------------------------------------------
    CHECK(aidr_is_mcp_process("mcp-server-filesystem", NULL),
          "mcp-server-* basename -> MCP");
    CHECK(aidr_is_mcp_process("github-mcp", NULL), "*-mcp basename -> MCP");
    CHECK(aidr_is_mcp_process("node", "npx -y @modelcontextprotocol/server-github"),
          "@modelcontextprotocol/ cmdline -> MCP");
    CHECK(aidr_is_mcp_process("python3", "python3 -m fastmcp run server.py"),
          "fastmcp cmdline -> MCP");
    CHECK(!aidr_is_mcp_process("nginx", "nginx -g daemon off;"),
          "nginx -> not MCP");
    CHECK(!aidr_is_mcp_process("bash", "ls -la"), "bash ls -> not MCP");

    printf("\n%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
