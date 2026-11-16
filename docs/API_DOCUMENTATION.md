# Border Command Center API — Phase 4

## Scope

Phase 1 added a Crow HTTP server, a CORS policy for the local Vite app, a JSON response envelope, and the React request client. Phase 2 added read endpoints for the current intruder record file and dashboard statistics. Phase 3 connected operational pages and reports unavailable runtime data honestly. Phase 4 exposes real C++ data-structure and graph-algorithm operations to the DSA Lab.

Crow's JSON writer is used for responses, so this phase does not add a second JSON dependency. The Crow project describes its framework as supporting HTTP/WebSocket services and built-in JSON; its CORS middleware supports origin, method and header rules. [Crow project](https://github.com/CrowCpp/Crow), [Crow CORS middleware](https://crowcpp.org/1.3.0/guides/included-middleware/).

## Implemented route

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api` | Reports implemented endpoints and known unavailable modules. |
| `GET` | `/api/health` | Returns a successful health response when the C++ HTTP process is serving requests. |
| `GET` | `/api/dashboard` | Returns total intruder records, high threat count, and counts for threat levels 1–5. Active alerts and patrol units are `null` because no runtime data for them exists in the current C++ application. |
| `GET` | `/api/records` | Returns records loaded from the same `intruders.txt` file used by the console application. |
| `GET` | `/api/graph` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: the graph class exists but no checkpoint graph is initialized by the application. |
| `GET` | `/api/alerts` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: no alert queue is initialized by the application. |
| `GET` | `/api/patrol-units` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: no patrol-unit store is initialized by the application. |
| `GET` | `/api/dsa/stack` | Current stack values, bottom to top. |
| `POST` | `/api/dsa/stack/push` | Push `{"value": 7}`. |
| `POST` | `/api/dsa/stack/pop` | Pop and return the top value; `409 STACK_EMPTY` if empty. |
| `GET` | `/api/dsa/stack/peek` | Read the top value without removing it. |
| `GET` | `/api/dsa/queue` | Current queue values, front to back. |
| `POST` | `/api/dsa/queue/enqueue` | Enqueue `{"value": 7}`. |
| `POST` | `/api/dsa/queue/dequeue` | Remove the front value; `409 QUEUE_EMPTY` if empty. |
| `GET` | `/api/dsa/queue/front` | Read the front value without removing it. |
| `GET` | `/api/dsa/priority-queue` | Current binary max-heap state and highest-priority item. |
| `POST` | `/api/dsa/priority-queue/insert` | Insert `{"value": 7, "priority": 3}`. Higher number means higher priority; equal priorities are FIFO. |
| `POST` | `/api/dsa/priority-queue/extract` | Remove and return the highest-priority item. |
| `GET` | `/api/dsa/priority-queue/peek` | Read the highest-priority item without removing it. |
| `GET` | `/api/dsa/graph` | Current DSA graph, or `configured: false` if none was supplied. |
| `POST` | `/api/dsa/graph` | Configure vertices and weighted edges for DSA algorithm operations. |
| `POST` | `/api/dsa/graph/bfs` | Breadth-first traversal from `{"start": 0}`. |
| `POST` | `/api/dsa/graph/dfs` | Depth-first traversal from `{"start": 0}`. |
| `POST` | `/api/dsa/graph/dijkstra` | Shortest path for `{"source": 0, "target": 1}`. |
| `GET` | `/api/dsa/hash-map` | Current separate-chaining hash table with buckets. |
| `POST` | `/api/dsa/hash-map/insert` | Insert an intruder record (`id`, `name`, `location`, `threatLevel`). |
| `POST` | `/api/dsa/hash-map/search` | Search by `{"id": 7}`. |
| `POST` | `/api/dsa/hash-map/remove` | Remove by `{"id": 7}`. |

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

Example `GET /api/records` data:

```json
{
  "records": [
    { "id": "102", "name": "Ali", "location": "Punjab", "threatLevel": 3 }
  ],
  "count": 1
}
```

Example `GET /api/dashboard` data:

```json
{
  "totalIntruders": 2,
  "highThreatIntruders": 1,
  "threatLevelCounts": [0, 0, 1, 1, 0],
  "activeAlerts": null,
  "availablePatrolUnits": null
}
```

Unreadable or malformed record files return HTTP `503` with `DATA_UNAVAILABLE`. A missing file is treated as an empty record collection, matching the console application's existing behavior. Records are re-read on each request so saves made by the console app appear without restarting the API.

The graph algorithms and graph class exist in `Graph_Map`, but the production app does not initialize a shared checkpoint graph. The DSA Lab accepts graph input and invokes the existing C++ `Graph`, BFS, DFS, and Dijkstra implementations; this is a lab graph, separate from Live Surveillance. The alert queue and patrol runtime are absent, so those operational routes continue to return HTTP `501`. Records remains connected to the existing file-backed records in read-only mode.

The DSA runtime uses the existing `SimpleStack<int>`, `SimpleQueue<int>`, and `HashChaining` implementations. State is held in the API process, guarded by a mutex, and clears when the server restarts. The repository did not contain an application priority-queue class, so Phase 4 adds a C++ binary max-heap for DSA Lab operations; it extracts the largest numeric priority and preserves insertion order for ties. It does not change alert handling because no alert queue exists.

## Architecture

`api/server/main.cpp` reads the listener and CORS settings, configures Crow, registers route modules and starts the server. `api/server/routes/health_routes.cpp` owns the health controller. `api/server/include/api/responses.hpp` centralizes success/error envelopes. The C++ DSA modules remain separate and are not yet attached to HTTP state.

The React client in `frontend/src/services/api.ts` centralizes the base URL, timeout, envelope validation and network errors. `BackendStatusProvider` polls `/health`. The dashboard reads total/high-threat counts from `/dashboard`; its preview panels remain marked sample data. Records reads actual records from `/records`. Live Surveillance, Alerts, and Patrol show explicit API capability errors where runtime data is absent. The DSA Lab calls the C++ APIs for stack, queue, priority-queue, graph, and hash-map operations and renders the returned state/results.

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

The API reads `intruders.txt` relative to its current working directory by default. Start the binary from the repository root, or set `INTRUDER_DATA_FILE` to an explicit file path. Configure React in `frontend/.env`:

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
curl -i http://localhost:18080/api/dashboard
curl -i http://localhost:18080/api/records
curl -i http://localhost:18080/api/graph
curl -i http://localhost:18080/api/alerts
curl -i http://localhost:18080/api/patrol-units
curl -i http://localhost:18080/api/dsa/stack
curl -i -X POST -H 'Content-Type: application/json' -d '{"value":7}' http://localhost:18080/api/dsa/stack/push
curl -i http://localhost:18080/api/dsa/priority-queue
curl -i http://localhost:18080/api/dsa/hash-map
curl -i http://localhost:18080/api/dsa/graph
curl -i -H 'Origin: http://localhost:5173' http://localhost:18080/api/health
curl -i -X OPTIONS \
  -H 'Origin: http://localhost:5173' \
  -H 'Access-Control-Request-Method: GET' \
  http://localhost:18080/api/health
curl -i http://localhost:18080/api/not-implemented
```

The health, dashboard and records requests should return HTTP 200 and JSON when the record file is readable. Graph, alerts, and patrol return HTTP 501 with `FEATURE_NOT_IMPLEMENTED` until those C++ application modules have initialized runtime state. The allowed-origin request should include `Access-Control-Allow-Origin: http://localhost:5173`. The preflight should include the configured CORS headers. An unknown route should return HTTP 404 and JSON.

## Request flow

The React status provider calls the configured base URL plus `/health`. Records and dashboard requests read the existing console file. DSA Lab requests call the C++ structure or graph algorithm directly; the API serializes the resulting state in the standard JSON envelope. Runtime structure calls are serialized with a mutex. Crow adds CORS headers, and the React API client validates HTTP status and response envelopes. A failed health request is reported as disconnected. API requests use a five-second timeout.

## Remaining work after Phase 4

- Connect records POST/PUT/DELETE to shared application state; current record integration is read-only through the existing file format.
- Initialize and own the production checkpoint graph before connecting DSA graph input to Live Surveillance.
- Integrate actual alert queue and patrol-unit structures before implementing their reads or operations.
- Add persistence if DSA Lab state must survive API restarts.
- Add endpoint-level C++ integration tests and connect each frontend view to its real route.
- `GET /ws/events` upgrades to a WebSocket connection for realtime application events. The server sends `system.connected` on connect and answers JSON `{"type":"ping","timestamp":"..."}` messages with `system.pong` (echoing `clientTimestamp`). DSA state mutations broadcast `dsa.stack.changed`, `dsa.queue.changed`, `dsa.priority-queue.changed`, `dsa.graph.changed`, or `dsa.hash-map.changed`; each envelope has `event`, UTC `timestamp`, and `data` fields. The React client retries with exponential backoff and jitter and keeps HTTP health polling separate.
- Live alert events are not available: the application still has no alert queue (`GET /api/alerts` returns HTTP 501 `FEATURE_NOT_IMPLEMENTED`). The event channel does not synthesize alerts. Records are read-only, so there are no record mutation events to publish.
