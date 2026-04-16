#!/usr/bin/env python3
import sys
import os
import subprocess

ALLOWED_COMMANDS = {
    "date": ["date"],
    "uptime": ["uptime"],
    "whoami": ["whoami"],
}

def parse_query_string(qs):
    params = {}
    for pair in qs.split("&"):
        if "=" in pair:
            k, v = pair.split("=", 1)
            params[k.strip()] = v.strip()
    return params

def run_command(command_key):
    if command_key not in ALLOWED_COMMANDS:
        return None, f"Commande inconnue : {command_key}"
    result = subprocess.run(ALLOWED_COMMANDS[command_key], capture_output=True, text=True)
    return result.stdout.strip(), None

def respond(status, body):
    print(f"Status: {status}")
    print("Content-Type: text/plain")
    print(f"Content-Length: {len(body.encode())}")
    print()
    print(body)
    sys.stdout.flush()

def main():
    method = os.environ.get("REQUEST_METHOD", "GET")

    if method == "POST":
        body = sys.stdin.read()
        params = parse_query_string(body)

    elif method == "GET":
        qs = os.environ.get("QUERY_STRING", "")
        params = parse_query_string(qs)

    else:
        respond("405 Method Not Allowed", "Méthode non supportée.")
        return

    command_key = params.get("command", "").strip()
    if not command_key:
        respond("400 Bad Request", "Paramètre 'command' manquant.")
        return

    output, error = run_command(command_key)
    if error:
        respond("400 Bad Request", error)
        return

    respond("200 OK", output)

main()