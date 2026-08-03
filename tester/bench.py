#!/usr/bin/env python3
"""Benchmark a server via wrk: throughput, error rate, CPU/RAM vs concurrency.
Deps: pip install psutil matplotlib ; wrk must be in PATH.
Usage: python3 bench.py ./server --port 3490 --concurrency 1 10 100 500 1000 5000 10000 15000 20000 25000
"""

import argparse, os, re, resource, shutil, socket, subprocess, sys, threading, time
import psutil
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HOST = "127.0.0.1"


def wait_ready(port, timeout=5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((HOST, port), timeout=0.5):
                return True
        except OSError:
            time.sleep(0.1)
    return False


def kill_tree(pid):
    try:
        root = psutil.Process(pid)
        procs = root.children(recursive=True) + [root]
    except psutil.NoSuchProcess:
        return
    for p in procs:
        try:
            p.terminate()
        except psutil.NoSuchProcess:
            pass
    _, alive = psutil.wait_procs(procs, timeout=3)
    for p in alive:
        try:
            p.kill()
        except psutil.NoSuchProcess:
            pass


def monitor(pid, stop, samples):
    """Samples CPU% + RSS of pid and all its children every 0.2s (fork-friendly)."""
    tracked = {}
    while not stop.is_set():
        try:
            procs = [psutil.Process(pid)] + psutil.Process(pid).children(recursive=True)
        except psutil.NoSuchProcess:
            break
        cpu, mem, alive = 0.0, 0, set()
        for p in procs:
            alive.add(p.pid)
            if p.pid not in tracked:
                try:
                    p.cpu_percent(None)  # priming call, discarded
                    tracked[p.pid] = p
                except psutil.NoSuchProcess:
                    pass
                continue
            try:
                cpu += tracked[p.pid].cpu_percent(None)
                mem += tracked[p.pid].memory_info().rss
            except psutil.NoSuchProcess:
                tracked.pop(p.pid, None)
        for pid_ in list(tracked):
            if pid_ not in alive:
                tracked.pop(pid_, None)
        samples.append((cpu, mem))
        time.sleep(0.2)


def parse_wrk(out):
    rps = re.search(r"Requests/sec:\s+([\d.]+)", out)
    total = re.search(r"(\d+) requests in", out)
    err = re.search(r"Socket errors: connect (\d+), read (\d+), write (\d+), timeout (\d+)", out)
    non2xx = re.search(r"Non-2xx or 3xx responses: (\d+)", out)
    if not rps or not total:
        raise RuntimeError("wrk parse failed:\n" + out)
    total_req = int(total.group(1))
    errors = (sum(int(g) for g in err.groups()) if err else 0) + (int(non2xx.group(1)) if non2xx else 0)
    return float(rps.group(1)), (errors / total_req * 100 if total_req else 0.0)


def run_level(binary, port, path, concurrency, duration):
    threads = max(1, min(4, concurrency))
    proc = subprocess.Popen([binary], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        if not wait_ready(port):
            raise RuntimeError("server did not start in time")

        stop, samples = threading.Event(), []
        t = threading.Thread(target=monitor, args=(proc.pid, stop, samples), daemon=True)
        t.start()

        cmd = ["wrk", f"-t{threads}", f"-c{concurrency}", f"-d{duration}s", f"http://{HOST}:{port}{path}"]
        print(" ", " ".join(cmd))
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=duration + 20)

        stop.set()
        t.join(timeout=2)

        rps, err_pct = parse_wrk(result.stdout)
        cpu = sum(s[0] for s in samples) / len(samples) if samples else 0.0
        mem = max((s[1] for s in samples), default=0) / (1024 * 1024)
        print(f"    {rps:.0f} req/s | errors {err_pct:.2f}% | CPU {cpu:.1f}% | RAM {mem:.1f} MB")
        return {"concurrency": concurrency, "rps": rps, "err_pct": err_pct, "cpu": cpu, "mem": mem}
    finally:
        kill_tree(proc.pid)


def plot(results, out, label):
    c = [r["concurrency"] for r in results]
    fig, (a1, a2, a3) = plt.subplots(1, 3, figsize=(16, 4.8))

    a1.plot(c, [r["rps"] for r in results], "o-", color="tab:blue")
    a1.set(title="Throughput", xlabel="Concurrency", ylabel="req/s")

    a2.plot(c, [r["err_pct"] for r in results], "o-", color="tab:red")
    a2.set(title="Errors", xlabel="Concurrency", ylabel="Error rate (%)")

    a3.plot(c, [r["cpu"] for r in results], "o-", color="tab:green")
    a3.set(title="Resources", xlabel="Concurrency", ylabel="CPU (%)")
    a3.set_ylim(bottom=0)
    a3b = a3.twinx()
    a3b.plot(c, [r["mem"] for r in results], "s-", color="tab:orange")
    a3b.set_ylabel("RAM (MB)")

    for a in (a1, a2, a3):
        a.grid(alpha=0.3)
    fig.suptitle(f"Benchmark -- {label}")
    fig.tight_layout()
    fig.savefig(out, dpi=150)
    print("saved:", out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", help="path to the server binary")
    ap.add_argument("--port", type=int, default=3490)
    ap.add_argument("--path", default="/")
    ap.add_argument("--concurrency", type=int, nargs="+", default=[1, 10, 100, 500, 1000, 5000, 10000, 15000, 20000, 25000])
    ap.add_argument("--duration", type=int, default=5)
    ap.add_argument("--output", default=None)
    args = ap.parse_args()

    if shutil.which("wrk") is None:
        sys.exit("wrk not found in PATH")
    binary = os.path.abspath(args.binary)
    if not os.access(binary, os.X_OK):
        sys.exit(f"{binary} is not executable")

    soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
    resource.setrlimit(resource.RLIMIT_NOFILE, (min(max(args.concurrency) + 1024, hard), hard))

    results = []
    for c in args.concurrency:
        print(f"--- concurrency={c} ---")
        results.append(run_level(binary, args.port, args.path, c, args.duration))
        time.sleep(1)

    out = args.output or f"bench_{os.path.basename(args.binary)}_{int(time.time())}.png"
    plot(results, out, os.path.basename(args.binary))


if __name__ == "__main__":
    main()