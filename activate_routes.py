import json, socket, sys

SOCK = "/var/ossec/queue/sockets/analysis"

def post(path, body):
    data = json.dumps(body)
    req = (
        f"POST {path} HTTP/1.1\r\n"
        f"Host: localhost\r\n"
        f"Content-Type: text/plain\r\n"
        f"Content-Length: {len(data)}\r\n"
        f"Connection: close\r\n"
        f"\r\n"
        f"{data}"
    )
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(SOCK)
    s.sendall(req.encode())
    resp = b""
    while True:
        chunk = s.recv(65536)
        if not chunk:
            break
        resp += chunk
    s.close()
    txt = resp.decode(errors="replace")
    # split headers/body
    parts = txt.split("\r\n\r\n", 1)
    body_txt = parts[1] if len(parts) > 1 else txt
    return txt.split("\r\n",1)[0], body_txt.strip()

DEC = "11111111-1111-4111-8111-111111111111"
OUT = "22222222-2222-4222-8222-222222222222"
INT = "33333333-3333-4333-8333-333333333333"
POL = "44444444-4444-4444-8444-444444444444"

decoder = {"name":"decoder/local-root/0","id":DEC,"enabled":True}
output = {
  "name":"output/indexer/0","id":OUT,"enabled":True,
  "outputs":[{"guardsarm-indexer":{"index":"guardsarm-events-v5-unclassified"}}]
}
integration = {"id":INT,"metadata":{"title":"local-core"},"enabled":True,"category":"unclassified","default_parent":DEC,"decoders":[DEC],"kvdbs":[]}
policy = {
  "type":"policy","metadata":{"title":"local default policy"},"enabled":True,
  "root_decoder":DEC,"origin_space":"standard","index_unclassified_events":True,
  "index_discarded_events":False,"cleanup_decoder_variables":True,"filters":[],"enrichments":[],"integrations":[INT],"outputs":[],"id":POL
}
bundle = {"policy":policy,"resources":{"kvdbs":[],"decoders":[decoder],"filters":[],"outputs":[],"integrations":[integration]}}

print("== namespace/list (before) ==")
print(post("/_internal/content/namespace/list", {}))

print("== namespace/delete (cleanup) ==")
print(post("/_internal/content/namespace/delete", {"space":"standard"}))
print("== namespace/import ==")
line, body = post("/_internal/content/namespace/import", {"space":"standard","force":True,"jsonContent":json.dumps(bundle)})
print(line); print(body)

print("== router/route/post ==")
line, body = post("/_internal/router/route/post", {"route":{"name":"local_default","namespaceId":"standard","priority":1,"description":"local default route"}})
print(line); print(body)

print("== router/table/get (after) ==")
print(post("/_internal/router/table/get", {}))
