# Border Command Center — Frontend

React, TypeScript and Vite frontend for Border Command Center. The sidebar routes to the dashboard, surveillance map, records, alerts, patrol units, DSA implementation notes, activity log and settings.

## Run

```sh
npm install
npm run dev
```

The frontend checks `GET {VITE_API_BASE_URL}/health` every 15 seconds (default `http://localhost:18080/api/health`) and requests `GET {VITE_API_BASE_URL}/dashboard`. Dispatch attempts use `POST {VITE_API_BASE_URL}/alerts/{alertId}/dispatch`. If the endpoints are unavailable, dashboard content falls back to clearly marked sample data and dispatch is reported as demo-only without changing the alert state.

Phase 1 provides a Crow API server with `GET /api/health`; dashboard and dispatch handlers remain future work. The checked-in console application and graph module are not yet owned by a shared backend service. The graph provides `Graph::exportState()` and `GraphAlgo::shortestPath()` for a future adapter; React does not implement Dijkstra or reorder the backend's alert response. See [API documentation](../docs/API_DOCUMENTATION.md) for setup and the current contract.
