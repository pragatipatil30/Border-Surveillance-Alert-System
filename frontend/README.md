# Border Command Center — Frontend

React, TypeScript and Vite dashboard frontend. Currently the Dashboard is the only implemented screen. The other sidebar entries are visual placeholders and do not navigate to unimplemented pages.

## Run

```sh
npm install
npm run dev
```

The dashboard requests `GET {VITE_API_BASE_URL}/dashboard` (default `http://localhost:8080/api/dashboard`). If no compatible C++ API is available, it falls back to clearly marked sample data. The repository currently has C++ console logic and a graph module, but no HTTP server or API contract. The graph provides `Graph::exportState()` and `GraphAlgo::shortestPath()` for a future backend adapter; route selection and Dijkstra are not simulated in React.
