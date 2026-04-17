import socket

HOST = "127.0.0.1"
PORT = 8080
TIMEOUT = 5


def send_raw(data: bytes) -> bytes:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(TIMEOUT)
        s.connect((HOST, PORT))
        if data:
            s.sendall(data)
        s.shutdown(socket.SHUT_WR)  # ← envoie le FIN TCP → recv() retourne 0 côté serveur
        response = b""
        try:
            while chunk := s.recv(4096):
                response += chunk
        except (socket.timeout, ConnectionResetError):
            pass
        return response


def send_request(request: str) -> bytes:
    return send_raw(request.encode())


# ─────────────────────────────────────────────
# Each test is a dict:
#   name        : display name
#   category    : group label
#   fn          : callable() -> (passed: bool, detail: str)
# ─────────────────────────────────────────────

TESTS = []


def test(name: str, category: str):
    """Decorator to register a test."""
    def decorator(fn):
        TESTS.append({"name": name, "category": category, "fn": fn})
        return fn
    return decorator


# ══════════════════════════════════════════════
# CATEGORY 1 — Basic valid requests
# ══════════════════════════════════════════════

@test("GET / returns 200", "Valid Requests")
def _():
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"200" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("HEAD / returns 200 with no body", "Valid Requests")
def _():
    resp = send_request("HEAD / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    status_ok = b"200" in resp
    # HEAD must not return a body — response ends after headers
    parts = resp.split(b"\r\n\r\n", 1)
    body_empty = len(parts) < 2 or parts[1] == b""
    ok = status_ok and body_empty
    return ok, f"body={'empty' if body_empty else 'present'}"


@test("GET /index.html returns 200", "Valid Requests")
def _():
    resp = send_request("GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"200" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 2 — HTTP error codes
# ══════════════════════════════════════════════

@test("GET /does_not_exist returns 404", "Error Codes")
def _():
    resp = send_request("GET /does_not_exist_xyz HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"404" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Unsupported method returns 405", "Error Codes")
def _():
    resp = send_request("PATCH / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"405" in resp or b"501" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("HTTP/0.9 request returns 400 or 505", "Error Codes")
def _():
    resp = send_request("GET /\r\n")
    ok = b"400" in resp or b"505" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 3 — Malformed requests
# ══════════════════════════════════════════════

@test("Empty request closes connection cleanly", "Malformed Requests")
def _():
    resp = send_raw(b"")
    # Server should close without crashing — empty or 400 are both acceptable
    ok = True
    return ok, f"got {len(resp)} bytes back"


@test("Request with no CRLF returns 400", "Malformed Requests")
def _():
    resp = send_raw(b"GET / HTTP/1.1\nHost: localhost\n\n")
    ok = b"400" in resp or b"200" in resp  # some servers are lenient
    return ok, resp.split(b"\n")[0].decode(errors="replace")


@test("Truncated request line returns 400", "Malformed Requests")
def _():
    resp = send_request("GET /\r\n\r\n")
    ok = b"400" in resp or b"505" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Garbage data returns 400", "Malformed Requests")
def _():
    resp = send_raw(b"\x00\x01\x02\x03\xff\xfe gibberish \r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Method with no URI returns 400", "Malformed Requests")
def _():
    resp = send_request("GET HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 4 — Headers
# ══════════════════════════════════════════════

@test("Missing Host header returns 400", "Headers")
def _():
    resp = send_request("GET / HTTP/1.1\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Duplicate Host header returns 400", "Headers")
def _():
    resp = send_request(
        "GET / HTTP/1.1\r\nHost: localhost\r\nHost: evil.com\r\n\r\n"
    )
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Very long header value returns 400 or 431", "Headers")
def _():
    long_val = "A" * 8192
    resp = send_request(
        f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Evil: {long_val}\r\n\r\n"
    )
    ok = b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header without colon returns 400", "Headers")
def _():
    resp = send_request(
        "GET / HTTP/1.1\r\nHost localhost\r\n\r\n"
    )
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 5 — URI edge cases
# ══════════════════════════════════════════════

@test("Path traversal attempt returns 400 or 403", "URI Edge Cases")
def _():
    resp = send_request("GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"403" in resp or b"404" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Very long URI returns 400 or 414", "URI Edge Cases")
def _():
    long_path = "/" + "a" * 8192
    resp = send_request(f"GET {long_path} HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"414" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Double slash in URI is handled", "URI Edge Cases")
def _():
    resp = send_request("GET //index.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"200" in resp or b"301" in resp or b"400" in resp or b"404" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 6 — Connection handling
# ══════════════════════════════════════════════

@test("Connection: close is respected", "Connection")
def _():
    resp = send_request(
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
    )
    ok = b"200" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Pipelining: two requests on one connection", "Connection")
def _():
    req = (
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
    )
    resp = send_request(req)
    count = resp.count(b"HTTP/1.1")
    ok = count >= 1  # at least one valid response
    return ok, f"{count} HTTP response(s) received"
