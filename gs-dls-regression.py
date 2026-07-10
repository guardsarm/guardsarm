#!/usr/bin/env python3
"""Post-bootstrap DLS / tenant-isolation regression assertion (gap L11).

Proves the indexer's document-level security actually filters by tenantId — a misconfig
(wrong field name, missing dls) would otherwise pass unnoticed until a customer saw
another tenant's data. Self-contained: creates a throwaway DLS role + user, seeds docs
with three different tenantIds, then asserts:
  * the tenant user sees ONLY its own tenantId (DLS filtering works)
  * admin sees all docs (no filtering for the provider)
and tears everything down. Exit 0 = isolation holds; non-zero = FAIL (wire into CI).

Env: GS_ES_ADMIN_URL (default https://localhost:7500), GS_ADMIN_USER (admin), GS_ADMIN_PASS.
"""
import base64
import json
import os
import ssl
import sys
import urllib.request

ES = os.environ.get("GS_ES_ADMIN_URL", "https://localhost:7500")
AU = os.environ.get("GS_ADMIN_USER", "admin")
AP = os.environ.get("GS_ADMIN_PASS", "")
IDX = "guardsarm-dls-regression-test"
ROLE = "dls_regression_role"
USER = "dls_regression_user"
UPW = "Dls_Regression_Pw_28charslong!Aa1"
TENANT = "dlstest-alpha"
_CTX = ssl.create_default_context()
_CTX.check_hostname = False
_CTX.verify_mode = ssl.CERT_NONE


def call(method, path, body=None, user=AU, pw=AP):
    data = json.dumps(body).encode() if body is not None else None
    r = urllib.request.Request(ES + path, data=data, method=method, headers={
        "Content-Type": "application/json",
        "Authorization": "Basic " + base64.b64encode(f"{user}:{pw}".encode()).decode(),
    })
    try:
        with urllib.request.urlopen(r, context=_CTX, timeout=20) as resp:
            return resp.status, json.loads(resp.read() or b"{}")
    except urllib.error.HTTPError as e:
        return e.code, json.loads(e.read() or b"{}")


def cleanup():
    call("DELETE", f"/{IDX}")
    call("DELETE", f"/_plugins/_security/api/internalusers/{USER}")
    call("DELETE", f"/_plugins/_security/api/roles/{ROLE}")


def main():
    if not AP:
        sys.exit("set GS_ADMIN_PASS (indexer admin password)")
    cleanup()  # idempotent pre-clean
    ok = True
    try:
        # 1) DLS role: read the test index, filtered to one tenantId
        call("PUT", f"/_plugins/_security/api/roles/{ROLE}", {
            "index_permissions": [{
                "index_patterns": [IDX],
                "dls": json.dumps({"term": {"tenantId": TENANT}}),
                "allowed_actions": ["read"],
            }],
        })
        call("PUT", f"/_plugins/_security/api/internalusers/{USER}",
             {"password": UPW, "opendistro_security_roles": [ROLE]})
        # 2) seed 3 docs across tenants (+ refresh)
        call("PUT", f"/{IDX}", {"mappings": {"properties": {"tenantId": {"type": "keyword"}}}})
        for i, t in enumerate([TENANT, "dlstest-beta", ""]):
            call("PUT", f"/{IDX}/_doc/{i}?refresh=true", {"tenantId": t, "n": i})
        # 3) assert tenant user sees ONLY its tenant; admin sees all
        _, ures = call("POST", f"/{IDX}/_search", {"query": {"match_all": {}}}, user=USER, pw=UPW)
        _, ares = call("POST", f"/{IDX}/_search", {"query": {"match_all": {}}})
        u_tenants = sorted({h["_source"]["tenantId"] for h in ures.get("hits", {}).get("hits", [])})
        a_total = ares.get("hits", {}).get("total", {}).get("value", 0)
        print(f"  tenant user sees tenantIds: {u_tenants}  (expect ['{TENANT}'])")
        print(f"  admin sees {a_total} docs  (expect 3)")
        if u_tenants != [TENANT]:
            print("  FAIL: DLS did not isolate — tenant user saw other/all tenants"); ok = False
        if a_total != 3:
            print("  FAIL: admin did not see all docs"); ok = False
    finally:
        cleanup()
    print("DLS regression:", "PASS ✅" if ok else "FAIL ❌")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    import urllib.error  # noqa: E402
    main()
