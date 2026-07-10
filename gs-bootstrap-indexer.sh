#!/usr/bin/env bash
# =============================================================================
# gs-bootstrap-indexer.sh
#
# Idempotent post-start bootstrap for the GuardSarm state/inventory pipeline.
# Given a RUNNING manager container + a reachable OpenSearch, it:
#
#   1. Installs the 20 index templates (renamed wazuh-* -> guardsarm-*), pulled
#      live from the manager container's /build/guardsarm/src/external/indexer-plugins.
#   2. Pre-creates every guardsarm-states-* index with its EXACT base name
#      (no @timestamp seeding) so the agent's schema-validator gate passes and
#      does NOT log "No schema validator found for index ... Discarding message".
#   3. Creates the two engine bootstrap indices:
#        - guardsarm-threatintel-policies  (empty; stops the CMSync 404 storm)
#        - .guardsarm-settings             (+ exactly ONE {"engine":{}} seed doc)
#   4. (optional, --with-index-patterns) creates the OpenSearch-Dashboards
#      index-patterns via the saved_objects API.
#
# Everything is idempotent: existing templates are overwritten, existing indices
# are ignored (resource_already_exists_exception is treated as success), and the
# .guardsarm-settings seed doc is only inserted when the index is empty.
#
# IMPORTANT (dynamic:strict): state index templates reject any field not in their
# schema (including @timestamp). NEVER hand-seed docs into a guardsarm-states-*
# index -- only the agent writes valid data. The sole exception is
# .guardsarm-settings, which REQUIRES exactly one {"engine":{}} doc.
#
# Usage:
#   ./gs-bootstrap-indexer.sh [--with-index-patterns]
#
# Environment overrides (defaults shown):
#   MGR_CONTAINER=gsmgr                 manager container name
#   OS_URL=http://localhost:9200        OpenSearch base URL (from the host)
#   OSD_URL=http://localhost:5601       OpenSearch-Dashboards base URL (host)
#   PLUGINS_DIR=/build/guardsarm/src/external/indexer-plugins
#
# Requires on the host: docker, curl, sed, grep.
# =============================================================================
set -u

# Git-Bash on Windows mangles absolute container paths in `docker exec` args.
# MSYS_NO_PATHCONV=1 disables that rewriting. Harmless on Linux/macOS.
export MSYS_NO_PATHCONV=1

MGR_CONTAINER="${MGR_CONTAINER:-gsmgr}"
OS_URL="${OS_URL:-http://localhost:9200}"
OSD_URL="${OSD_URL:-http://localhost:5601}"
PLUGINS_DIR="${PLUGINS_DIR:-/build/guardsarm/src/external/indexer-plugins}"

WITH_PATTERNS=0
[ "${1:-}" = "--with-index-patterns" ] && WITH_PATTERNS=1

ok()   { printf '  [ ok ] %s\n' "$*"; }
skip() { printf '  [skip] %s\n' "$*"; }
info() { printf '\n== %s\n' "$*"; }
err()  { printf '  [ERR] %s\n' "$*" >&2; }

# curl wrapper that prints the HTTP status; treats "already exists" as success.
os_put_index() {
  local idx="$1" body
  body="$(curl -s -o /dev/null -w '%{http_code}' -X PUT "$OS_URL/$idx")"
  # re-run capturing body only if we need the error text
  if [ "$body" = "200" ] || [ "$body" = "201" ]; then
    ok "index $idx created"
  else
    local resp
    resp="$(curl -s -X PUT "$OS_URL/$idx")"
    if printf '%s' "$resp" | grep -q 'resource_already_exists_exception'; then
      skip "index $idx already exists"
    else
      err "index $idx -> HTTP $body : $(printf '%s' "$resp" | head -c 200)"
    fi
  fi
}

# -----------------------------------------------------------------------------
info "Pre-flight: manager container '$MGR_CONTAINER' and OpenSearch reachability"
if ! docker inspect "$MGR_CONTAINER" >/dev/null 2>&1; then
  err "manager container '$MGR_CONTAINER' not found"; exit 1
fi
if ! curl -sf "$OS_URL" >/dev/null 2>&1; then
  err "OpenSearch not reachable at $OS_URL"; exit 1
fi
ok "manager container present"
ok "OpenSearch reachable at $OS_URL"

# -----------------------------------------------------------------------------
info "1/4  Installing index templates (wazuh-* -> guardsarm-*), pulled from the container"

JSON_LIST="$(docker exec "$MGR_CONTAINER" bash -c "ls $PLUGINS_DIR/*.json" 2>/dev/null)"
if [ -z "$JSON_LIST" ]; then
  err "no template JSONs found under $PLUGINS_DIR in $MGR_CONTAINER"; exit 1
fi

# base index names of every guardsarm-states-* template, collected for step 2.
STATE_INDICES=""
TPL_OK=0; TPL_FAIL=0        # aggregate template-install result (gap M8)

while IFS= read -r jf; do
  [ -z "$jf" ] && continue
  # Pull the JSON out of the container and rebrand wazuh -> guardsarm.
  # These template files are pure schema (index_patterns + guardsarm.* fields);
  # a full rename is exactly what the live OpenSearch templates use. Idempotent:
  # re-running on an already-guardsarm file is a no-op.
  body="$(docker exec "$MGR_CONTAINER" cat "$jf" 2>/dev/null | sed -e 's/wazuh/guardsarm/g')"
  [ -z "$body" ] && { err "empty JSON: $jf"; continue; }

  # Template name = index_patterns[0] with the trailing '*' stripped.
  # (Deriving from the filename collides at priority 1 -> illegal_argument_exception.)
  pattern="$(printf '%s' "$body" | grep -o '"guardsarm-[a-z0-9-]*\*"' | head -1 | tr -d '"')"
  if [ -z "$pattern" ]; then
    err "could not derive index_pattern from $jf"; continue
  fi
  name="${pattern%\*}"           # e.g. guardsarm-states-fim-files
  code="$(printf '%s' "$body" | curl -s -o /dev/null -w '%{http_code}' \
            -X PUT "$OS_URL/_index_template/$name" \
            -H 'Content-Type: application/json' --data-binary @-)"
  if [ "$code" = "200" ]; then
    ok "template $name"; TPL_OK=$((TPL_OK+1))
  else
    err "template $name -> HTTP $code"; TPL_FAIL=$((TPL_FAIL+1))
  fi

  case "$name" in
    guardsarm-states-*) STATE_INDICES="$STATE_INDICES $name" ;;
  esac
done <<EOF
$JSON_LIST
EOF

# Aggregate result (gap M8): a partial install must not look like success.
printf '  [INFO] templates: %d installed, %d failed
' "$TPL_OK" "$TPL_FAIL" >&2
BOOTSTRAP_FAILED=0
if [ "$TPL_FAIL" -gt 0 ]; then
  err "$TPL_FAIL template(s) failed to install -- bootstrap INCOMPLETE"
  BOOTSTRAP_FAILED=1
fi

# -----------------------------------------------------------------------------
info "2/4  Pre-creating guardsarm-states-* indices (exact base names, no docs)"
for idx in $STATE_INDICES; do
  os_put_index "$idx"
done

# -----------------------------------------------------------------------------
info "3/4  Engine bootstrap indices"

# 3a. threatintel policies: empty index is sufficient (existsPolicy only checks
#     totalHits > 0; an empty existing index returns false gracefully, no 404).
os_put_index "guardsarm-threatintel-policies"

# 3b. .guardsarm-settings: needs the index PLUS exactly ONE {"engine":{}} doc.
#     getEngineRemoteConfig() throws on 0 docs and on >1 docs, so seed exactly one.
os_put_index ".guardsarm-settings"
settings_count="$(curl -s "$OS_URL/.guardsarm-settings/_count" | grep -o '"count":[0-9]*' | head -1 | cut -d: -f2)"
settings_count="${settings_count:-0}"
if [ "$settings_count" -eq 0 ] 2>/dev/null; then
  curl -s -X POST "$OS_URL/.guardsarm-settings/_doc?refresh=true" \
       -H 'Content-Type: application/json' -d '{"engine":{}}' >/dev/null \
    && ok ".guardsarm-settings seeded with one {\"engine\":{}} doc"
else
  skip ".guardsarm-settings already has $settings_count doc(s) (must stay exactly 1)"
fi

# -----------------------------------------------------------------------------
info "4/4  Dashboard index-patterns (OSD saved_objects API)"
if [ "$WITH_PATTERNS" -ne 1 ]; then
  skip "not requested (re-run with --with-index-patterns; needs the dashboard up at $OSD_URL)"
else
  if ! curl -sf "$OSD_URL/api/status" >/dev/null 2>&1; then
    err "dashboard not reachable at $OSD_URL -- skipping index-pattern creation"
  else
    # "pattern|timeField"  -- state patterns have NO time field; findings use @timestamp.
    PATTERNS='
guardsarm-states-inventory*|
guardsarm-states-inventory-hardware*|
guardsarm-states-inventory-hotfixes*|
guardsarm-states-inventory-interfaces*|
guardsarm-states-inventory-networks*|
guardsarm-states-inventory-packages*|
guardsarm-states-inventory-ports*|
guardsarm-states-inventory-processes*|
guardsarm-states-inventory-protocols*|
guardsarm-states-inventory-system*|
guardsarm-states-inventory-users*|
guardsarm-states-inventory-groups*|
guardsarm-states-inventory-services*|
guardsarm-states-inventory-browser-extensions*|
guardsarm-states-fim*|
guardsarm-states-fim-files*|
guardsarm-states-fim-registry-keys*|
guardsarm-states-fim-registry-values*|
guardsarm-states-sca*|
guardsarm-states-vulnerabilities*|
guardsarm-findings-v5*|@timestamp
guardsarm-findings-v5-security*|@timestamp
'
    printf '%s\n' "$PATTERNS" | while IFS='|' read -r title tf; do
      [ -z "$title" ] && continue
      # saved-object id = URL-encoded title (only '*' needs encoding -> %2A).
      id="$(printf '%s' "$title" | sed 's/\*/%2A/g')"
      if [ -n "$tf" ]; then
        payload="{\"attributes\":{\"title\":\"$title\",\"timeFieldName\":\"$tf\"}}"
      else
        payload="{\"attributes\":{\"title\":\"$title\"}}"
      fi
      code="$(curl -s -o /dev/null -w '%{http_code}' \
                -X POST "$OSD_URL/api/saved_objects/index-pattern/$id?overwrite=true" \
                -H 'osd-xsrf: true' -H 'Content-Type: application/json' \
                -d "$payload")"
      case "$code" in
        200) ok "index-pattern $title" ;;
        409) skip "index-pattern $title already exists" ;;
        *)   err "index-pattern $title -> HTTP $code" ;;
      esac
    done
  fi
fi

info "Bootstrap complete."
echo "Verify state data with:  curl -s '$OS_URL/_cat/indices/guardsarm-states-*?v&h=index,docs.count'"

# Exit non-zero if any template failed, so callers/CI can detect a partial bootstrap (gap M8).
exit "${BOOTSTRAP_FAILED:-0}"
