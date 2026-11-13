# Border Command Center API — Phase 3

## Scope

Phase 1 added a Crow HTTP server, a CORS policy for the local Vite app, a JSON response envelope, and the React request client. Phase 2 added read endpoints for the current intruder record file and dashboard statistics derived from those records. Phase 3 connects the operational pages to their API routes and reports unavailable C++ runtime data explicitly instead of displaying the dashboard demo records as live data.

Crow's JSON writer is used for responses, so this phase does not add a second JSON dependency. The Crow project describes its framework as supporting HTTP/WebSocket services and built-in JSON; its CORS middleware supports origin, method and header rules. [Crow project](https://github.com/CrowCpp/Crow), [Crow CORS middleware](https://crowcpp.org/1.3.0/guides/included-middleware/).

## Implemented route

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api` | Reports the API foundation and implemented route list. |
| `GET` | `/api/health` | Returns a successful health response when the C++ HTTP process is serving requests. |
| `GET` | `/api/dashboard` | Returns total intruder records, high threat count, and counts for threat levels 1–5. Active alerts and patrol units are `null` because no runtime data for them exists in the current C++ application. |
| `GET` | `/api/records` | Returns records loaded from the same `intruders.txt` file used by the console application. |
| `GET` | `/api/graph` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: the graph class exists but no checkpoint graph is initialized by the application. |
| `GET` | `/api/alerts` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: no alert queue is initialized by the application. |
| `GET` | `/api/patrol-units` | Returns HTTP `501 FEATURE_NOT_IMPLEMENTED`: no patrol-unit store is initialized by the application. |
| `GET` | `/api` | Reports implemented and not implemented endpoints. |

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

The graph algorithms and graph class exist in `Graph_Map`, but this repository does not initialize a shared checkpoint graph for the API to expose. The alert queue and patrol runtime are absent. Those GET routes return HTTP `501` with a JSON error instead of returning empty or sample live data. The Live Surveillance, Alerts, and Patrol pages call these routes and show the backend response. Records remains connected to the existing file-backed records in read-only mode.

## Architecture

`api/server/main.cpp` reads the listener and CORS settings, configures Crow, registers route modules and starts the server. `api/server/routes/health_routes.cpp` owns the health controller. `api/server/include/api/responses.hpp` centralizes success/error envelopes. The C++ DSA modules remain separate and are not yet attached to HTTP state.

The React client in `frontend/src/services/api.ts` centralizes the base URL, timeout, envelope validation and network errors. `BackendStatusProvider` polls `/health` and makes that result available to every view. The dashboard reads total/high-threat counts from `/dashboard`; dashboard preview panels remain clearly marked sample data. The Records tab reads actual records from `/records`. Live Surveillance, Alerts, and Patrol request their respective endpoints and display loading, error, and empty states without replacing backend errors with sample rows.

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
curl -i -H 'Origin: http://localhost:5173' http://localhost:18080/api/health
curl -i -X OPTIONS \
  -H 'Origin: http://localhost:5173' \
  -H 'Access-Control-Request-Method: GET' \
  http://localhost:18080/api/health
curl -i http://localhost:18080/api/not-implemented
```

The health, dashboard and records requests should return HTTP 200 and JSON when the record file is readable. Graph, alerts, and patrol return HTTP 501 with `FEATURE_NOT_IMPLEMENTED` until those C++ application modules have initialized runtime state. The allowed-origin request should include `Access-Control-Allow-Origin: http://localhost:5173`. The preflight should include the configured CORS headers. An unknown route should return HTTP 404 and JSON.

## Request flow

The React status provider calls the configured base URL plus `/health`. For records and dashboard stats, the client calls `/records` or `/dashboard`; the C++ repository reads the console app's file and the dashboard route calculates counts from those records. The operational pages call `/graph`, `/alerts`, and `/patrol-units`; the API responds with an explicit not-implemented error while those runtimes are absent. Crow adds CORS headers, and the React API client validates HTTP status and response envelopes. A failed health request is reported as disconnected. API requests use a five-second timeout.

## Remaining work after Phase 3

- Connect records POST/PUT/DELETE to shared application state; current record integration is read-only through the existing file format.
- Initialize and own the production checkpoint graph before replacing the `501` response with graph queries or algorithms.
- Integrate actual alert queue and patrol-unit structures before implementing their reads or operations.
- Add synchronization and persistence rules before adding concurrent API mutations.
- Add endpoint-level C++ integration tests and connect each frontend view to its real route.
- Consider WebSockets after REST reads and mutations exist; current updates use health polling only.
