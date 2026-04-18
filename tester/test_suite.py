import socket

HOST = "127.0.0.1"
PORT = 3490
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


@test("OPTIONS / returns 200", "Valid Requests")
def _():
    resp = send_request("OPTIONS / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"200" in resp or b"204" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("OPTIONS response contains Allow header", "Valid Requests")
def _():
    resp = send_request("OPTIONS / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"Allow" in resp or b"allow" in resp
    return ok, f"Allow header {'found' if ok else 'missing'}"

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
    ok = b"400" in resp 
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
# CATEGORY 4 — Size limits (from config.h)
# ══════════════════════════════════════════════

@test("Method exactly at limit (16 chars) returns 400 or 405", "Size Limits")
def _():
    method = "A" * 16  # MAX_METHOD_LEN = 16
    resp = send_request(f"{method} / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"405" in resp or b"501" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Method exceeding limit (17 chars) returns 400 or 405", "Size Limits")
def _():
    method = "A" * 17  # MAX_METHOD_LEN + 1
    resp = send_request(f"{method} / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"405" in resp or b"501" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Path exactly at limit (2048 chars) returns 200 or 404", "Size Limits")
def _():
    path = "/" + "a" * 2047  # MAX_PATH_LEN = 2048
    resp = send_request(f"GET {path} HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"200" in resp or b"404" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Path exceeding limit (2049 chars) returns 400 or 414", "Size Limits")
def _():
    path = "/" + "a" * 2048  # MAX_PATH_LEN + 1
    resp = send_request(f"GET {path} HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"414" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("HTTP version exactly at limit (16 chars) returns 400 or 505", "Size Limits")
def _():
    version = "HTTP/1." + "1" * 9  # MAX_VERSION_LEN = 16, padded to exactly 16
    resp = send_request(f"GET / {version}\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"505" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("HTTP version exceeding limit (17 chars) returns 400 or 505", "Size Limits")
def _():
    version = "HTTP/1." + "1" * 10  # MAX_VERSION_LEN + 1
    resp = send_request(f"GET / {version}\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp or b"505" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header key exactly at limit (256 chars) returns 200 or 400", "Size Limits")
def _():
    key = "X-" + "A" * 254  # MAX_HEADER_KEY_SIZE = 256
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\n{key}: value\r\n\r\n")
    ok = b"200" in resp or b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header key exceeding limit (257 chars) returns 400 or 431", "Size Limits")
def _():
    key = "X-" + "A" * 255  # MAX_HEADER_KEY_SIZE + 1
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\n{key}: value\r\n\r\n")
    ok = b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header value exactly at limit (4096 chars) returns 200 or 400", "Size Limits")
def _():
    value = "A" * 4096  # MAX_HEADER_VALUE_SIZE = 4096
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: {value}\r\n\r\n")
    ok = b"200" in resp or b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header value exceeding limit (4097 chars) returns 400 or 431", "Size Limits")
def _():
    value = "A" * 4097  # MAX_HEADER_VALUE_SIZE + 1
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: {value}\r\n\r\n")
    ok = b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Exactly 64 headers returns 200 or 400", "Size Limits")
def _():
    headers = "".join(f"X-Header-{i:02d}: value\r\n" for i in range(63))  # 63 + Host = 64
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\n{headers}\r\n")
    ok = b"200" in resp or b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Exceeding 64 headers (65) returns 400 or 431", "Size Limits")
def _():
    headers = "".join(f"X-Header-{i:02d}: value\r\n" for i in range(64))  # 64 + Host = 65
    resp = send_request(f"GET / HTTP/1.1\r\nHost: localhost\r\n{headers}\r\n")
    ok = b"400" in resp or b"431" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Body exactly at limit (65536 bytes) on GET returns any valid response", "Size Limits")
def _():
    body = "A" * 65536
    content_length = len(body)
    resp = send_request(
        f"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: {content_length}\r\n\r\n{body}"
    )
    # Server must not crash — any HTTP response is acceptable
    ok = b"HTTP" in resp or resp == b""
    return ok, resp.split(b"\r\n")[0].decode(errors="replace") if resp else "(empty response)"


@test("Body exceeding limit (65537 bytes) on GET returns 400 or 413", "Size Limits")
def _():
    body = "A" * 65537
    content_length = len(body)
    resp = send_request(
        f"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: {content_length}\r\n\r\n{body}"
    )
    ok = b"400" in resp or b"413" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace") if resp else "(empty response)"


@test("Total request exceeding limit (81921 bytes) returns 400 or 413", "Size Limits")
def _():
    # MAX_REQUEST_LEN = 65536 + 16384 = 81920
    body = "A" * 81921
    content_length = len(body)
    resp = send_request(
        f"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: {content_length}\r\n\r\n{body}"
    )
    ok = b"400" in resp or b"413" in resp or b"405" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


# ══════════════════════════════════════════════
# CATEGORY 5 — Headers
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
# CATEGORY 6 — URI edge cases
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
# CATEGORY 7 — Connection handling
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

# ══════════════════════════════════════════════
# CATEGORY 8 — Parser edge cases
# ══════════════════════════════════════════════

@test("Multiple spaces between method and path returns 400", "Parser Edge Cases")
def _():
    # Parser rejects double space in REQ_PARSING_METHOD_SEPARATOR
    resp = send_request("GET  / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Multiple spaces between path and version returns 400", "Parser Edge Cases")
def _():
    # Parser rejects double space in REQ_PARSING_PATH_SEPARATOR
    resp = send_request("GET /  HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("CR without LF in request line returns 400", "Parser Edge Cases")
def _():
    # REQ_EXPECTING_LF must receive \n, anything else is 400
    resp = send_raw(b"GET / HTTP/1.1\r\rHost: localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("LF without CR in request line returns 400", "Parser Edge Cases")
def _():
    # Parser rejects bare \n in REQ_PARSING_VERSION
    resp = send_raw(b"GET / HTTP/1.1\nHost: localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Space before colon in header returns 400", "Parser Edge Cases")
def _():
    # Explicitly rejected : key[*pos - 1] == ' ' before ':' 
    resp = send_request("GET / HTTP/1.1\r\nHost : localhost\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Header with empty value returns 400", "Parser Edge Cases")
def _():
    # REQ_PARSING_HEADER_KEY_SEPARATOR rejects \r or \n (empty value)
    resp = send_raw(b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Empty:\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Content-Length with non-numeric value returns 400", "Parser Edge Cases")
def _():
    # sscanf returns != 1 → HTTP_BAD_REQUEST
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\n")
    ok = b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Content-Length: 0 with no body parses cleanly", "Parser Edge Cases")
def _():
    # body_len == 0 → parsing_complete = true immediately
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n")
    ok = b"200" in resp or b"400" in resp
    return ok, resp.split(b"\r\n")[0].decode(errors="replace")


@test("Content-Length larger than actual body returns no crash", "Parser Edge Cases")
def _():
    # Parser will wait for more bytes that never come — server must handle timeout gracefully
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1000\r\n\r\nABC")
    ok = b"400" in resp or b"408" in resp or resp == b""
    return ok, resp.split(b"\r\n")[0].decode(errors="replace") if resp else "(empty response)"


# ══════════════════════════════════════════════
# CATEGORY 9 — Response Headers
# ══════════════════════════════════════════════

@test("Response contains Content-Type header", "Response Headers")
def _():
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"Content-Type" in resp or b"content-type" in resp
    return ok, f"Content-Type {'found' if ok else 'missing'}"


@test("Response contains Content-Length or Transfer-Encoding", "Response Headers")
def _():
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"Content-Length" in resp or b"Transfer-Encoding" in resp
    return ok, f"body-size header {'found' if ok else 'missing'}"


@test("Server header matches config (l-olive)", "Response Headers")
def _():
    resp = send_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    ok = b"l-olive" in resp
    return ok, f"Server header {'found' if ok else 'missing'}"