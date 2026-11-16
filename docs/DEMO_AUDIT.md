# Live Demo Audit

**Repository:** `C:\Users\Pragati\Border-Surveillance-Alert-System`

**Updated:** 2026-11-16. This audit was rechecked after the `api/` and `docs/` directories appeared in the project.

## Implementation inventory and actual structure facts

- `Array_Search_Sort/` contains sorting, linear/binary search, sparse matrix, singly/doubly/circular lists, Josephus, and separate chaining/linear probing hash tables.
- `Graph_Map/` contains the adjacency-list graph (with matrix export), graph algorithms and BST. `SimpleStack` and `SimpleQueue` are growable array structures; queue is circular.
- `api/` is a Crow C++17 API. `AppState` owns the operational records, indexes, graph, alerts, patrol units, activity and undo/redo under a mutex. `DsaRuntime` separately owns process-local lab structures under a mutex.
- Production record index: `HashChaining`, 10 buckets, `id % capacity`; demo IDs 1047 and 1057 collide. The name index is the existing case-insensitive BST.
- DSA Lab hash index: `HashChaining`, 17 buckets. DSA Lab priority queue is a binary max-heap with FIFO tie order. Operational incoming alerts use the circular queue.
- Production graph uses the existing `Graph` adjacency-list representation and the specified ten checkpoints/fifteen weighted routes.
- Demo mode defaults on (`API_DEMO`); its record file is `data/demo/intruders.txt`, separate from `intruders.txt`. `/api/health` reports the active dataset and counts.
- The DSA Lab currently exposes stack, queue, priority queue, hash map and configurable graph BFS/DFS/Dijkstra. It does not yet cover all syllabus tabs or the requested structured `state`/`steps`/`trace`/`complexity` response envelope.
- Authentication routes, login sessions and role-based action enforcement are not implemented.

## UI control checklist

| Page / control | Current behavior | Remaining work |
|---|---|---|
| Sidebar page navigation | Dashboard, surveillance, records, alerts, patrol, DSA Lab, activity and settings navigate locally. | None. |
| Sidebar profile and Log out | Profile is static; logout is not connected to authentication. | Implement Auth/session and role-based access. |
| Dashboard search | Stores the query and opens Records. | None. |
| Dashboard notification action | No notification workflow is available. | Connect a notification view or mark unavailable. |
| Dashboard metrics/chart/activity/map | Data comes from backend; sample fallback is distinguished when API is down. | Build and run full UI/API verification. |
| Dashboard alert Dispatch | Calls operational API and refreshes on success. | Authentication and end-to-end verification. |
| Map zoom and center | Changes local map zoom. | None. |
| Map checkpoint selection | Fetches checkpoint details. | Verify against running server. |
| Map route selection | Calls route block/unblock API. | Verify against running server. |
| Surveillance test-event | Calls `/api/demo/event`, creates real record and alert. | Verify repeated use and user feedback. |
| Records query/filter/sort | Backend query and C++ sort APIs are used. | Add real expression rule support. |
| Records create/edit/delete | Calls record API; delete uses confirmation. | Add role checks and end-to-end persistence verification. |
| Records Undo/Redo | Calls C++ API history endpoints. | Verify after restart and failures. |
| Records CSV export | Exports currently loaded backend rows. | None. |
| Records selection/detail | Displays selected row details. | None. |
| Alerts tabs/policy/queue | Loads ordered alert state and switches priority/FCFS policy. | Verify queue ordering with live API. |
| Alerts Dispatch/Resolve | Calls operational API and refreshes on events. | Add role checks and end-to-end verification. |
| Patrol deploy/release/status | Calls operational unit API. | Add role checks and end-to-end verification. |
| DSA Lab selector | Switches among Stack, Queue, Priority Queue, Graph, Hash Map. | Add BST, lists/Josephus, sorting, searching, sparse matrix and expression tabs. |
| DSA Lab operation controls | Invoke existing C++ structures/algorithms and show returned result. | Clear/reset/toggles, structure-specific visualization, structured steps/trace/complexity. |
| Activity search/category/dispatch | Filters live activity/alerts and dispatches queued alerts. | Authentication and integration verification. |
| Settings demo reset | Confirms then calls reset endpoint. | Reset should also clear/reset all lab instances when implemented. |
| Settings backend status/dataset | Reads health endpoint. | Add authenticated profile and role display. |
| Settings compact/notification toggles | Local preference state only. | Persist preferences or label them as local. |

## API status

The old API document described the graph, alerts and patrol endpoints as HTTP 501. Those operational routes have since been implemented; see [API_DOCUMENTATION.md](API_DOCUMENTATION.md) for current routes and remaining limitations. Records CRUD, production graph/alerts/patrol, demo seed/reset/event, health, dashboard, activity and WebSocket operational events are now present.

The remaining requested work is substantial: authentication with lockout and roles; complete DSA lab route/tab coverage, clear/reset and implementation toggles; structured operation envelopes and truthful visualizations; reset isolation for all lab instances; and successful end-to-end build/run verification.

## Verification status

- Direct syntax compilation passed for the added app-state/index and repository translation units.
- C++ state test source compiles and links, but Windows Application Control blocked launching the test executable in this environment.
- `cmake` configure could not complete while fetching Crow from the network.
- Frontend production build could not run because the existing `node_modules` has a platform-specific TypeScript install that does not match this Windows host (`tsc` unavailable).
- Do not report the live demo as fully verified until those builds and the application workflows are run successfully.
