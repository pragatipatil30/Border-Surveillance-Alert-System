# Border Command Center API

This document describes the routes currently implemented in `api/server`. JSON routes use the envelope `{ "success": true, "data": ..., "message": ... }`; errors use `{ "success": false, "error": { "code": ..., "message": ... } }`. The server uses Crow and listens on loopback.

## Runtime setup

- `API_PORT` defaults to `18080`.
- `API_CORS_ORIGIN` defaults to `http://localhost:5173`.
- `API_DEMO` defaults to enabled. Set it to `0` to load real records.
- `INTRUDER_DATA_FILE` selects the real records file when demo mode is disabled. Demo writes are isolated at `data/demo/intruders.txt`.
- `VITE_API_BASE_URL` defaults to `http://localhost:18080/api` for the frontend.

Configure and run from the repository root:

```powershell
cmake -S api -B api/build
cmake --build api/build --config Release
api/build/Release/border_api.exe
```

Crow is fetched by CMake at its pinned `v1.3.1` tag, so first configure needs Git and network access. In another terminal run `cd frontend; npm install; npm run dev`.

## Application routes

| Method | Path | Behavior |
|---|---|---|
| `GET` | `/api` | API endpoint inventory. |
| `GET` | `/api/health` | Status, dataset label, record/alert/unit counts, uptime, and WebSocket connection state. |
| `GET` | `/api/dashboard` | Derived operational metrics, threat distribution, alert preview, activity, units, and checkpoint graph. |
| `GET` | `/api/records` | Search/filter/sort records; accepts `q`, `location`, `threat`, `status`, `sort`, `algorithm`, and `rule`. |
| `GET` | `/api/records/{id}` | ID lookup using the hash index. |
| `GET` | `/api/records/search?prefix=ka` | Name prefix lookup using the BST. |
| `POST` | `/api/records` | Create a record. |
| `PUT` | `/api/records/{id}` | Update a record. |
| `DELETE` | `/api/records/{id}` | Delete a record. |
| `GET` | `/api/records/undo-state` | Undo/redo availability and history counts. |
| `POST` | `/api/records/undo`, `/api/records/redo` | Apply the previous or next record mutation. |
| `GET` | `/api/graph` | Production checkpoint graph. |
| `POST` | `/api/graph/checkpoints` | Add a checkpoint. |
| `POST` | `/api/graph/routes` | Add a weighted route. |
| `DELETE` | `/api/graph/routes` | Remove a route. |
| `POST` | `/api/graph/routes/block`, `/api/graph/routes/unblock` | Change route availability. |
| `GET`, `POST` | `/api/alerts` | Read the ordered alert list or create an alert for an intruder. |
| `GET` | `/api/alerts/queues` | Incoming circular queue and waiting alert order. |
| `POST` | `/api/alerts/policy` | Switch between `PRIORITY` and `FCFS`. |
| `POST` | `/api/alerts/dispatch-next` | Dispatch the next waiting alert. |
| `POST` | `/api/alerts/{id}/dispatch`, `/api/alerts/{id}/resolve` | Dispatch a selected alert or resolve it. |
| `GET` | `/api/patrol-units` | Current unit state. |
| `POST` | `/api/patrol-units/{id}/deploy` | Deploy a unit to a checkpoint. |
| `POST` | `/api/patrol-units/{id}/release` | Release a unit. |
| `PATCH` | `/api/patrol-units/{id}` | Update unit status. |
| `GET` | `/api/activity` | Activity history; optional category filter. |
| `GET` | `/api/checkpoints/{id}` | Checkpoint details, neighbors, incidents, and assigned unit. |
| `POST` | `/api/demo/reset` | Reset the live demo dataset. |
| `POST` | `/api/demo/seed` | Seed only when the record collection is empty. |
| `POST` | `/api/demo/event` | Generate a plausible intruder and alert in demo mode. |
| `GET` | `/ws/events` | WebSocket for system and operational events. |

Mutating record/alert/unit/graph/demo routes broadcast corresponding events. The event envelope includes an event name, timestamp, and data. Clients should refresh only the view affected by the event.

## DSA Lab routes

Implemented lab routes are scoped to the API process and do not mutate live surveillance state:

| Method | Path | Behavior |
|---|---|---|
| `GET`, `POST` | `/api/dsa/stack`, `/api/dsa/stack/push`, `/api/dsa/stack/pop`, `/api/dsa/stack/peek` | Operate on the C++ integer stack. |
| `GET`, `POST` | `/api/dsa/queue`, `/api/dsa/queue/enqueue`, `/api/dsa/queue/dequeue`, `/api/dsa/queue/front` | Operate on the C++ queue. |
| `GET`, `POST` | `/api/dsa/priority-queue`, `/api/dsa/priority-queue/insert`, `/api/dsa/priority-queue/extract`, `/api/dsa/priority-queue/peek` | Operate on the C++ binary max-heap; priority ties preserve insertion order. |
| `GET`, `POST` | `/api/dsa/hash-map`, `/api/dsa/hash-map/insert`, `/api/dsa/hash-map/search`, `/api/dsa/hash-map/remove` | Operate on the C++ separate-chaining hash table. |
| `GET`, `POST` | `/api/dsa/graph`, `/api/dsa/graph/bfs`, `/api/dsa/graph/dfs`, `/api/dsa/graph/dijkstra` | Configure a lab graph and run BFS, iterative DFS, or shortest path. |

Every DSA route currently exposed by the API uses the standard success/error envelope, but does **not yet** return the full `state`/`steps`/`trace`/`complexity` animation envelope required for all syllabus operations. The current DSA Lab covers stack, queue, priority queue, hash map, and graph BFS/DFS/Dijkstra only. BST traversal variants, linked-list/Josephus, sorting/searching step APIs, sparse matrix, and expression conversion/evaluation are not implemented as lab endpoints yet; the interface must not imply those operations are live.

## Actual structures and seeded data

The production ID hash is `HashChaining` with 10 buckets; the existing hash function makes IDs `1047` and `1057` collide. The name index is the repository BST. The production checkpoint map is the existing `Graph` adjacency-list implementation. The DSA Lab priority queue is a binary max-heap with FIFO tie breaking. The production alert waiting queue uses the C++ priority queue adapter; the incoming alerts use the existing circular queue.

Demo mode seeds 15 predictable records, 12 alerts, 8 patrol units, 10 checkpoints and 15 routes. Dashboard counts are derived from this state. Demo record persistence is isolated from the real `intruders.txt`; use `POST /api/demo/reset` to restore the known seed. The activity feed and UI label the dataset as demo.

## Current limitations

Authentication routes, user sessions, role-based authorization, full DSA lab syllabus coverage, and the complete structured animation envelope are not implemented. API compilation and frontend production build have not been verified in the current Windows environment: CMake's Crow fetch requires network access, and the existing frontend dependencies are installed for a different platform. The direct C++ state/index translation units and the added C++ state test source have passed syntax compilation; the test executable was blocked by Windows Application Control before it could run.

## Quick checks

```powershell
curl.exe http://localhost:18080/api/health
curl.exe http://localhost:18080/api/dashboard
curl.exe http://localhost:18080/api/records
curl.exe http://localhost:18080/api/alerts/queues
curl.exe http://localhost:18080/api/graph
curl.exe -X POST http://localhost:18080/api/demo/reset
```
