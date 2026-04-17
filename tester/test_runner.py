#!/usr/bin/env python3
"""
test_runner.py — HTTP server test runner
Usage: python test_runner.py [--host 127.0.0.1] [--port 8080]
"""

import argparse
import time
from collections import defaultdict

from rich import box
from rich.columns import Columns
from rich.console import Console
from rich.live import Live
from rich.panel import Panel
from rich.rule import Rule
from rich.table import Table
from rich.text import Text

import test_suite

console = Console()


def parse_args():
    parser = argparse.ArgumentParser(description="HTTP server test runner")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    return parser.parse_args()


def build_results_table(results: list[dict]) -> Table:
    table = Table(
        box=box.SIMPLE_HEAD,
        show_header=True,
        header_style="bold dim",
        show_edge=False,
        padding=(0, 1),
        expand=True,
    )
    table.add_column("", width=3)           # icon
    table.add_column("Test", ratio=3)
    table.add_column("Detail", ratio=2, style="dim")
    table.add_column("Time", justify="right", width=8, style="dim")

    for r in results:
        icon = "[green]✓[/]" if r["passed"] else "[red]✗[/]"
        name_style = "white" if r["passed"] else "red"
        table.add_row(
            icon,
            Text(r["name"], style=name_style),
            r["detail"],
            f"{r['elapsed']*1000:.0f} ms",
        )
    return table


def build_summary_panel(results: list[dict], total_time: float) -> Panel:
    passed = sum(1 for r in results if r["passed"])
    failed = len(results) - passed
    rate = int(passed / len(results) * 100) if results else 0

    summary = Text()
    summary.append(f"  {passed} passed", style="bold green")
    summary.append("  ·  ")
    summary.append(f"{failed} failed", style="bold red" if failed else "dim")
    summary.append("  ·  ")
    summary.append(f"{len(results)} total", style="dim")
    summary.append(f"  ·  {total_time*1000:.0f} ms total", style="dim")
    summary.append(f"  ·  {rate}% success rate", style="bold")

    style = "green" if failed == 0 else "red"
    return Panel(summary, style=style, box=box.ROUNDED, padding=(0, 1))


def run_all(host: str, port: int):
    # Patch test_suite connection info
    test_suite.HOST = host
    test_suite.PORT = port

    tests = test_suite.TESTS
    categories = defaultdict(list)
    for t in tests:
        categories[t["category"]].append(t)

    all_results = []

    console.print()
    console.print(
        Panel(
            f"[bold]HTTP Server Test Runner[/]\n"
            f"[dim]Target → [cyan]{host}:{port}[/cyan]  ·  {len(tests)} tests in {len(categories)} categories[/]",
            box=box.ROUNDED,
            padding=(0, 2),
        )
    )
    console.print()

    for category, cat_tests in categories.items():
        console.print(Rule(f"[bold]{category}[/]", style="dim"))
        console.print()

        cat_results = []

        with Live(console=console, refresh_per_second=20, transient=True) as live:
            for t in cat_tests:
                # Show "running" state
                spinner_table = Table.grid(padding=(0, 1))
                spinner_table.add_row("[yellow]●[/]", Text(t["name"], style="dim"))
                live.update(spinner_table)

                start = time.perf_counter()
                try:
                    passed, detail = t["fn"]()
                except Exception as exc:
                    passed = False
                    detail = f"Exception: {exc}"
                elapsed = time.perf_counter() - start

                result = {
                    "name": t["name"],
                    "passed": passed,
                    "detail": detail,
                    "elapsed": elapsed,
                }
                cat_results.append(result)
                all_results.append(result)

                # Redraw with all results so far
                live.update(build_results_table(cat_results))

        # Final static table for this category
        console.print(build_results_table(cat_results))

        cat_passed = sum(1 for r in cat_results if r["passed"])
        cat_total = len(cat_results)
        badge_style = "green" if cat_passed == cat_total else "red"
        console.print(
            f"  [{badge_style}]{cat_passed}/{cat_total} passed[/]\n"
        )

    # ── Global summary ──────────────────────────────────────────
    total_time = sum(r["elapsed"] for r in all_results)
    console.print(Rule(style="dim"))
    console.print()
    console.print(build_summary_panel(all_results, total_time))
    console.print()

    failed_tests = [r for r in all_results if not r["passed"]]
    if failed_tests:
        console.print(Panel(
            "\n".join(f"  [red]✗[/] {r['name']}  [dim]{r['detail']}[/]"
                      for r in failed_tests),
            title="[bold red]Failed tests[/]",
            box=box.ROUNDED,
            border_style="red",
            padding=(0, 1),
        ))
        console.print()

    return 0 if not failed_tests else 1


if __name__ == "__main__":
    args = parse_args()
    exit(run_all(args.host, args.port))
