Here’s a **minimal, clean README** you can use for your project — it keeps things short but still covers the essentials:

---

# Multithreaded HTTP Server (C++17)

A lightweight HTTP/1.1 static file server for Linux.  
One thread accepts connections; a pool of workers handles requests in parallel.

```
client -> accept loop -> worker threads -> parse -> route -> respond
```

## Build & Run

Requires C++17 and CMake 3.16+.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/MultithreadedHTTPServer            # default config/server.conf
./build/MultithreadedHTTPServer my.conf    # custom config
```

Visit [http://localhost:9090](http://localhost:8080) or test with curl:

```bash
curl -i http://localhost:9090/about
curl -I http://localhost:9090/
curl http://localhost:9090/health
```

## Config (`config/server.conf`)

- `host` → default `0.0.0.0`
- `port` → default `9090` (overridden by `PORT` env)
- `workers` → default `4`
- `document_root` → default `public`

## Deploy

```bash
docker build -t my-http-server .
docker run -p 9090:9090 my-http-server
```

For production: run behind **Caddy** or **Nginx** to add HTTPS.

---
