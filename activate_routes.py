#!/usr/bin/env python3
# Rebuild the engine 'standard' namespace from the on-disk ruleset files and import it,
# restoring all decoders (incl the Windows event decoders) into the live engine, then
# (re)create the default route. Runs INSIDE the manager container.
# Copyright (C) 2026 GuardSarm, Inc.
import json, os, socket

SOCK = "/var/guardsarm-manager/queue/sockets/analysis"
RS = "/var/guardsarm-manager/data/ruleset/standard"

def post(path, body):
    data = json.dumps(body)
    req = (f"POST {path} HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\n"
           f"Content-Length: {len(data)}\r\nConnection: close\r\n\r\n{data}")
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(SOCK); s.sendall(req.encode())
    resp = b""
    while True:
        c = s.recv(65536)
        if not c:
            break
        resp += c
    s.close()
    t = resp.decode(errors="replace")
    p = t.split("\r\n\r\n", 1)
    return t.split("\r\n", 1)[0], (p[1] if len(p) > 1 else t).strip()

decoders = [json.load(open(f"{RS}/decoders/{f}")) for f in os.listdir(f"{RS}/decoders") if f.endswith(".json")]
integrations = [json.load(open(f"{RS}/integrations/{f}")) for f in os.listdir(f"{RS}/integrations") if f.endswith(".json")]
policy = json.load(open(f"{RS}/policy.json"))
policy["integrations"] = [i["id"] for i in integrations]
policy.setdefault("type", "policy")
policy.setdefault("outputs", [])
policy.setdefault("filters", [])
policy.setdefault("enrichments", [])
bundle = {"policy": policy, "resources": {"kvdbs": [], "decoders": decoders, "filters": [], "outputs": [], "integrations": integrations}}

print("decoders=%d integrations=%d" % (len(decoders), len(integrations)))
print("delete:", post("/_internal/content/namespace/delete", {"space": "standard"}))
print("import:", post("/_internal/content/namespace/import", {"space": "standard", "force": True, "jsonContent": json.dumps(bundle)}))
post("/_internal/router/route/delete", {"name": "local_default"})
print("route :", post("/_internal/router/route/post", {"route": {"name": "local_default", "namespaceId": "standard", "priority": 1, "description": "local default route"}}))
print("table :", post("/_internal/router/table/get", {})[1][:220])
print("ondisk decoders after import:", len(os.listdir(f"{RS}/decoders")))
