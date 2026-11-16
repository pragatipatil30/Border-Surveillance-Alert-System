# Demo Audit

**Repository inspected:** `C:\Users\Pragati\Border-Surveillance-Alert-System`  
**Audit scope:** checked-in C++ modules, Crow API, React pages/services and visible interactive controls. The repository now includes `api/` and `docs/API_DOCUMENTATION.md`.

## 1. Repository and implementation inventory

- `main.cpp`: original interactive console program; directly owns its menu flow and file persistence.
- `common/Intruder.h`: shared record `{id, name, location, threatlevel}`.
- `Array_Search_Sort/`: sparse matrix, bubble/insertion/merge/quick sorting, linear/binary search, singly/doubly/circular lists, separate-chaining and linear-probing hash tables, Josephus.
- `Graph_Map/`: adjacency-list graph with an adjacency-matrix export; BFS, recursive/iterative DFS, connectivity/components, bipartite and cycle checks, Dijkstra, Prim, Kruskal and Union-Find. Graph edges have weight and blocked state; graph algorithms ignore blocked edges. BST is keyed case-insensitively by `(name,id)` and has prefix search and recursive/iterative traversals.
- Graph helper `SimpleStack<T>` is a growable array stack; `SimpleQueue<T>` is a growable circular-array queue. These are traversal helpers, not operational alert/undo structures.
- `api/`: Crow C++17 server. `DsaRuntime` mutex-protects process-local stack, queue, priority queue, hash map and editable DSA graph. Routes expose these operations. Record and dashboard routes reread the existing whitespace-delimited intruder file and are read-only. Operational graph, alerts and patrol routes return 501.
- `frontend/src/`: React dashboard, activity log, surveillance/records/alerts/patrol/DSA/settings pages, status provider, API clients and CSS illustration. There is a DSA Lab for Stack, Queue, Priority Queue, Graph and Hash Map only.
- No `Auth`, `AppState`, undo/redo manager, alert scheduler, operational patrol store, rule expression parser, record mutation API, demo seed/reset route, demo event route, checkpoint-details endpoint or application-level activity-log API exists in this tree at audit time. No `api` unit/integration test target or frontend test script is configured.

## 2. Truthful structure facts for UI

| Subject | Checked-in implementation |
|---|---|
| DSA priority queue | Binary max-heap in `api/server/include/api/dsa/priority_queue.hpp`; ties use insertion sequence (FIFO). It is a DSA Lab-only structure, not the operational alert scheduler. |
| DSA hash map | `HashChaining`, separate chaining, 17 buckets in `DsaRuntime`; hash is `id % capacity` (negative results normalized). |
| Other hash implementation | `HashLinearProbing` exists in `Array_Search_Sort/`, but is not built into `border_api` or exposed by the API. |
| Production graph | None initialized. The DSA graph is caller configured and separate. `Graph` stores adjacency lists; matrix is generated on request. |
| Stack and queue | DSA Lab `SimpleStack<int>` and `SimpleQueue<int>` are growable arrays; queue is circular. API queue state currently exports logical front-to-back values only, not slots or indices. |
| Records | GET-only file repository using `INTRUDER_DATA_FILE` or `intruders.txt`; no shared in-memory indexes. |
| Seed/demo mode | Not implemented. Existing `intruders.txt` is read as the actual dataset. |

## 3. API endpoint audit

| Route | Current behavior | Work needed for requested live demo |
|---|---|---|
| `GET /api`, `GET /api/health` | Real service metadata/health; health only reports status/service. | Add dataset identity, counts, uptime and WebSocket status. |
| `GET /api/dashboard` | Computes record and threat counts from file; operational values are null. | Back all dashboard values and activity/alert/unit previews from shared live state. |
| `GET /api/records` | Read-only records from file; no query/filter/sort parameters. | Shared state, CRUD, search indexes, sorting/filtering, safe persistence, undo/redo. |
| `GET /api/graph`, `/api/alerts`, `/api/patrol-units` | Explicit 501. | Initialize production graph and operational alert/unit services. |
| `GET /api/dsa/stack`, `POST push/pop`, `GET peek` | Real C++ operations; empty pop/peek return 409. | Clear, stack mode switch, structured steps/envelope and state animation. |
| `GET /api/dsa/queue`, `POST enqueue/dequeue`, `GET front` | Real growable circular queue; empty dequeue/front return 409. | Clear, expose capacity/indices/slots, structured steps. |
| `GET /api/dsa/priority-queue`, `POST insert/extract`, `GET peek` | Real binary max-heap; higher numeric priority first, FIFO ties. Empty extract/peek return 409. | Reset, structured steps and heap visualization. |
| `GET /api/dsa/hash-map`, `POST insert/search/remove` | Real separate chaining table, 17 buckets; duplicate insert gives 409. | Reset/configuration, step traces and chain visualization. |
| `GET/POST /api/dsa/graph`, `POST bfs/dfs/dijkstra` | Configures a separate graph and invokes C++ algorithms. No production graph. | Structured step output; add listed graph algorithms, production graph read-only bridge. |
| `GET /ws/events` | WebSocket connects, heartbeat ping/pong; DSA mutations publish structure-specific events. | Add application event types after operational state exists; frontend refresh only affected pages. |

No endpoint in the current source returns HTTP 501 except the three operational reads above. No route handles POST/PUT/DELETE record operations.

## 4. UI control audit

| Page / control | Current behavior | Needed for requested demo |
|---|---|---|
| Sidebar: Dashboard, Live Surveillance, Records, Alerts, Patrol Units, DSA Laboratory, Activity Log | Client-side page navigation works. | Keep; counts must be live rather than hard-coded. |
| Sidebar Settings | Button has no click handler. | Navigate to Settings. |
| Sidebar user menu, Log out | No behavior. | Real account menu/auth logout or visibly disable with reason. |
| Dashboard global search, notification button | Input is uncontrolled; notification button has no action. | Wire search/navigation and notification view or disable honestly. |
| Dashboard date/clock | Clock updates locally; date is local browser time. | Use one shared clock; replace static weekday text. |
| Dashboard metrics/chart/map | Intruder counts partly from API, but remaining cards/map/alerts/activities/distribution are copied from mock data. | Compute from shared backend; remove fake changes and fixed values. |
| Dashboard “Open priority alerts & activity log” | Navigates to Activity Log. | Keep, with real alert preview and backend event data. |
| Map sector selector, zoom in/out/center, patrol markers | Decorative buttons; no map state changes. Some coordinates/route labels are fixed/mock. | Connect to production graph, zoom state and operations; remove unsupported location claims. |
| Surveillance checkpoint selection | Selects an SVG/map node locally. | Fetch checkpoint details from API. |
| Surveillance route click, test-event, dispatch controls | Not present. | Add only when backed by API. |
| Records search and threat select | Filter current API-loaded file records client-side. | Backend hash/BST lookup, filters, sort choices, rule filter and CRUD controls. |
| Records table | Read-only; no detail, edit, delete, export, undo or redo controls. | Implement API-backed actions and state. |
| Alerts threat filter | Filters loaded alert list; current endpoint returns 501, so page shows explicit unavailable state. | Seed/runtime API, status tabs, policy, queue visualization, dispatch/resolve. |
| Alert Dispatch | No action in current version. Older `dispatchAlert` helper swallowed errors but is not used. | Real dispatch endpoint and loading/error/success state. |
| Patrol cards | Reads `/api/patrol-units`; endpoint returns 501, page shows explicit unavailable state. No controls. | Add real unit operations and controls. |
| DSA Lab tab selector | Switches among Stack, Queue, Priority Queue, Graph and Hash Map; load calls real C++ API. | Add remaining syllabus tabs and live-system view. |
| DSA Stack Push/Pop/Peek/Refresh | Calls C++ API; busy/error states exist. | Clear/toggle, operation history, trace, complexity/snippet and visualization. |
| DSA Queue Enqueue/Dequeue/Front/Refresh | Calls C++ API; busy/error states exist. | Clear and ring slot/index display, history and steps. |
| DSA Priority Insert/Extract/Peek/Refresh | Calls C++ heap API; currently output rendered as JSON. | Heap visualization, reset and structured operation steps. |
| DSA Graph Configure/BFS/DFS/Dijkstra | Parses inputs in browser then invokes C++ graph endpoints; output is JSON. | Validate input, add graph algorithms, animation from backend-produced structured steps. |
| DSA Hash Insert/Search/Delete/Refresh | Calls C++ chaining hash map; output is JSON. | Bucket visualization and collision steps. |
| Settings compact/notifications toggles | Local UI state only; behavior beyond the toggle is not connected. | Persist or clearly label as local preferences. |
| Settings profile/backend status/API URL | Profile is hard-coded; connection status uses shared health context; URL is shown. | Auth-derived profile and dataset identity. |
| Activity Log search/notifications | Search is uncontrolled; notification button inert. | Real activity API/filter and live event subscription. |
| Activity Log alert Dispatch | Button has no handler. | Wire to dispatch when operational API exists. |

## 5. Stage 1 conclusion and implementation order

The supplied implementation brief describes substantially more runtime functionality than exists. The current tree is a working API foundation and a partial read-only dashboard plus five interactive C++ DSA controls. Implement the missing operational runtime before presenting the sample dashboard as live. Preserve existing C++ structures and their actual representations. Do not route demo mode through the user's real `intruders.txt`; use a distinct path and only claim demo status when the backend reports it.
