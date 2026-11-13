# Border Command Center — Frontend

React, TypeScript and Vite frontend for Border Command Center. The sidebar routes to the dashboard, surveillance map, records, alerts, patrol units, DSA implementation notes, activity log and settings.

## Run

```sh
npm install
npm run dev
```

The frontend checks `GET {VITE_API_BASE_URL}/health` every 15 seconds (default `http://localhost:8080/api/health`) and requests `GET {VITE_API_BASE_URL}/dashboard`. Dispatch attempts use `POST {VITE_API_BASE_URL}/alerts/{alertId}/dispatch`. If the endpoints are unavailable, dashboard content falls back to clearly marked sample data and dispatch is reported as demo-only without changing the alert state.

The repository currently has C++ console logic and a graph module, but no HTTP server or API handlers for these routes. These contracts must be implemented by a future C++ API adapter. The graph provides `Graph::exportState()` and `GraphAlgo::shortestPath()` for that adapter; React does not implement Dijkstra or reorder the backend's alert response.
