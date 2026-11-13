# Border Command Center API — Phase 1

## Scope

Phase 1 adds a Crow HTTP server, a CORS policy for the local Vite app, a JSON response envelope, and the React request client. The implemented routes are `GET /api` and `GET /api/health`. The repository has no existing HTTP service or C++ API build system; this API is a separate executable and does not change or duplicate existing DSA classes.

Crow's JSON writer is used for responses, so this phase does not add a second JSON dependency. The Crow project describes its framework as supporting HTTP/WebSocket services and built-in JSON; its CORS middleware supports origin, method and header rules. [Crow project](https://github.com/CrowCpp/Crow), [Crow CORS middleware](https://crowcpp.org/1.3.0/guides/included-middleware/).

## Implemented route

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api` | Reports the API foundation and implemented route list. |
| `GET` | `/api/health` | Returns a successful health response when the C++ HTTP process is serving requests. |

Successful response:

```json
{
  "success": true,
  "data": {
    "status": "ok",
    "service": "border-command-center-api"
  },
  "message": "API is healthy"
}
```

Unknown `/api/*` paths return HTTP `404` and a JSON error envelope:

```json
{
  "success": false,
  "error": {
    "code": "NOT_FOUND",
    "message": "API route not found"
  }
}
```

The dashboard, records, graph, alert, patrol and DSA routes are not implemented in Phase 1. Their future handlers must call the existing C++ structures and preserve backend state and ordering.

## Architecture

`api/server/main.cpp` reads the listener and CORS settings, configures Crow, registers route modules and starts the server. `api/server/routes/health_routes.cpp` owns the health controller. `api/server/include/api/responses.hpp` centralizes success/error envelopes. The C++ DSA modules remain separate and are not yet attached to HTTP state.

The React client in `frontend/src/services/api.ts` centralizes the base URL, timeout, envelope validation and network errors. `BackendStatusProvider` polls `/health` and makes that result available to every view. The dashboard's demo data fallback remains clearly marked; successful rendering does not count as backend health.

## Build and run

Requirements: C++17 compiler, CMake 3.16+, standalone Asio, and Git/network access on first CMake configure (Crow is fetched at the pinned `v1.3.1` tag).

On macOS with Homebrew:

```sh
brew install cmake asio
cmake -S api -B api/build
cmake --build api/build
API_PORT=18080 API_CORS_ORIGIN=http://localhost:5173 ./api/build/border_api
```

`API_PORT` defaults to `18080`; `API_CORS_ORIGIN` defaults to `http://localhost:5173`. The server binds to loopback (`127.0.0.1`) for local development. CORS allows GET, POST, PUT, DELETE and OPTIONS methods and the `Content-Type` and `Authorization` request headers. Credentials are not enabled.

Configure React in `frontend/.env`:

```dotenv
VITE_API_BASE_URL=http://localhost:18080/api
```

Then run:

```sh
cd frontend
npm install
npm run dev
```

## Verification

With the server running:

```sh
curl -i http://localhost:18080/api
curl -i http://localhost:18080/api/health
curl -i -H 'Origin: http://localhost:5173' http://localhost:18080/api/health
curl -i -X OPTIONS \
  -H 'Origin: http://localhost:5173' \
  -H 'Access-Control-Request-Method: GET' \
  http://localhost:18080/api/health
curl -i http://localhost:18080/api/not-implemented
```

The health request should return HTTP 200 and JSON. The allowed-origin request should include `Access-Control-Allow-Origin: http://localhost:5173`. The preflight should include the configured CORS headers. The unknown route should return HTTP 404 and JSON.

## Request flow

The React status provider calls the configured base URL plus `/health`. Crow matches the route, creates the JSON envelope, and adds CORS headers before sending the response. The React API client validates the HTTP status and envelope; `checkHealth()` returns connected only when the response is successful and `data.status` is `ok`. A failed request is reported as disconnected. API requests use a five-second timeout.

## Remaining work after Phase 1

- Add an application service object that owns the actual runtime C++ records, graph, alert queue, and other supported structures.
- Implement read endpoints only after defining how current console globals and graph instances are initialized and shared.
- Implement mutations only against those C++ structures with synchronization for concurrent requests.
- Add endpoint-level C++ integration tests and connect each frontend view to its real route.
- Consider WebSockets after REST reads and mutations exist; Phase 1 uses health polling only.
