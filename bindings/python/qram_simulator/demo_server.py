"""Loopback-only GUI; each experiment runs in an isolated, bounded child process."""
from __future__ import annotations

import dataclasses
import hmac
import json
import secrets
import subprocess
import sys
import threading
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from importlib.resources import files

from .demo import Experiment, information


def make_server(port: int = 0) -> ThreadingHTTPServer:
    if type(port) is not int or not 0 <= port <= 65535:
        raise ValueError("port must be an integer in [0,65535]")
    token = secrets.token_urlsafe(32)
    slot = threading.Lock()

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def handle(self):
            try:
                super().handle()
            except ConnectionError:
                pass  # The browser may close while its worker is running.

        def send(self, code, body, mime="application/json; charset=utf-8"):
            data = body if isinstance(body, bytes) else json.dumps(body).encode()
            self.send_response(code)
            self.send_header("Content-Type", mime)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("Content-Security-Policy", "default-src 'none'; connect-src 'self'; "
                             f"script-src 'nonce-{token}'; style-src 'unsafe-inline'; "
                             "frame-ancestors 'none'; base-uri 'none'")
            self.end_headers()
            self.wfile.write(data)

        def local(self):
            expected = f"127.0.0.1:{self.server.server_port}"
            return (self.headers.get("Host") == expected and
                    self.headers.get("Origin", f"http://{expected}") == f"http://{expected}")

        def do_GET(self):
            if not self.local():
                return self.send(403, {"error": "local origin required"})
            if self.path == "/":
                html = files("qram_simulator").joinpath("demo.html").read_text(encoding="utf-8")
                return self.send(200, html.replace("__SESSION_TOKEN__", token).encode(),
                                 "text/html; charset=utf-8")
            if self.path == "/api/info":
                return self.send(200, information())
            self.send(404, {"error": "not found"})

        def do_POST(self):
            if (not self.local() or not hmac.compare_digest(
                    self.headers.get("X-Session", ""), token)):
                return self.send(403, {"error": "local session required"})
            if self.path != "/api/run":
                return self.send(404, {"error": "not found"})
            try:
                length = int(self.headers.get("Content-Length", "0"))
                if not 0 < length <= 2048 or self.headers.get("Content-Type") != "application/json":
                    raise ValueError("a bounded JSON request is required")
                config = Experiment(**json.loads(self.rfile.read(length)))
                config.validate()
                if (config.depth > 12 or config.data_bits > 2 or config.inputs > 32 or
                        config.trajectories > 64 or config.probability > 0.1):
                    raise ValueError("GUI limit: depth12, bus2, inputs32, trajectories64, noise0.1; use CLI for larger runs")
            except (ValueError, TypeError, KeyError) as exc:
                return self.send(400, {"error": str(exc)})
            if not slot.acquire(blocking=False):
                return self.send(409, {"error": "an experiment is already running"})
            try:
                command = [sys.executable, "-m", "qram_simulator.demo", "run"]
                for field in dataclasses.fields(config):
                    command.extend(["--"+field.name.replace("_", "-"), str(getattr(config, field.name))])
                child = subprocess.run(command, capture_output=True, text=True, timeout=60)
                if child.returncode:
                    return self.send(422, {"error": child.stderr.strip()[-1024:]})
                self.send(200, json.loads(child.stdout))
            except ConnectionError:
                pass  # Do not try to write another response to a closed socket.
            except subprocess.TimeoutExpired:
                self.send(504, {"error": "experiment exceeded 60 seconds"})
            except (OSError, ValueError) as exc:
                self.send(500, {"error": str(exc)})
            finally:
                slot.release()

    return ThreadingHTTPServer(("127.0.0.1", port), Handler)


def serve(port=0, open_browser=True):
    with make_server(port) as server:
        url = f"http://127.0.0.1:{server.server_port}/"
        print(url, flush=True)
        if open_browser:
            webbrowser.open(url)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
