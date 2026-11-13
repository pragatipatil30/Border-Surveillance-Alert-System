import { useEffect, useMemo, useState } from 'react'
import { Activity, AlertTriangle, Bell, Check, ChevronRight, Clock3, Command, Crosshair, Database, GitBranch, LocateFixed, Radio, Search, Shield, ShieldAlert, Wifi } from 'lucide-react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { useBackendStatus } from '../services/BackendStatusContext'
import { API_BASE, getAlerts, getOperationalGraph, getPatrolUnits, getRecords } from '../services/api'
import type { Alert, Checkpoint, IntruderRecord, OperationalGraph, PatrolUnit, Threat } from '../types/dashboard'

type Page = 'surveillance' | 'records' | 'alerts' | 'patrol' | 'dsa' | 'settings'
const pageTitles: Record<Page, { eyebrow: string; title: string; description: string }> = {
  surveillance: { eyebrow: 'FIELD OVERVIEW · CHECKPOINT NETWORK', title: 'Live surveillance.', description: 'Checkpoint network, route status and current field events.' },
  records: { eyebrow: 'INCIDENT MANAGEMENT', title: 'Intruder records.', description: 'Review sightings and incident details from the operational feed.' },
  alerts: { eyebrow: 'RESPONSE CENTER · PRIORITY QUEUE', title: 'Alert management.', description: 'Review active incidents and their current threat priority.' },
  patrol: { eyebrow: 'FIELD OPERATIONS · UNIT STATUS', title: 'Patrol units.', description: 'Current availability and assignments across the northern sector.' },
  dsa: { eyebrow: 'TECHNICAL OVERVIEW · C++ IMPLEMENTATIONS', title: 'DSA laboratory.', description: 'Explore the structures and graph algorithms implemented in the C++ project.' },
  settings: { eyebrow: 'SYSTEM CONFIGURATION', title: 'Settings.', description: 'Application preferences and backend connection status.' },
}

function PageHeader({ page, onNavigate }: { page: Page; onNavigate: (page: string) => void }) {
  const title = pageTitles[page]
  return <><header className="topbar"><div className="breadcrumb"><button className="breadcrumb-button" onClick={() => onNavigate('dashboard')}>COMMAND</button><ChevronRight size={13}/>{page.toUpperCase()}</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input placeholder="Search this view…"/></div><button className="notification-button" aria-label="Notifications"><Bell size={18}/><i>3</i></button><span className="topbar-divider"/><ConnectionStatus/></div></header><div className="welcome-row"><div><div className="eyebrow welcome-kicker">{title.eyebrow}</div><h1>{title.title.split('.')[0]}<em>{title.title.includes('.') ? `.${title.title.split('.').slice(1).join('.')}` : ''}</em></h1><p>{title.description}</p></div></div></>
}

function DemoTag() { return <span className="demo-inline">DEMO DATA</span> }

export function WorkspacePage({ page, onNavigate }: { page: Page; onNavigate: (page: string) => void }) {
  return <main className="dashboard-content"><PageHeader page={page} onNavigate={onNavigate}/>{
    page === 'surveillance' ? <SurveillancePage/> :
    page === 'records' ? <RecordsPage/> :
    page === 'alerts' ? <AlertsPage/> :
    page === 'patrol' ? <PatrolPage/> :
    page === 'dsa' ? <DsaPage/> : <SettingsPage/>
  }<footer className="page-footer"><span>Border Command Center <i>·</i> Development view</span><span>DATA SHOWN AS LABELED</span></footer></main>
}

function SurveillancePage() {
  const [graph, setGraph] = useState<OperationalGraph | null>(null)
  const [selectedId, setSelectedId] = useState<number | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  useEffect(() => { let active = true; getOperationalGraph().then((value) => { if (active) { setGraph(value); setSelectedId(value.vertices[0]?.id ?? null) } }).catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load the checkpoint graph.') }).finally(() => { if (active) setLoading(false) }); return () => { active = false } }, [])
  if (loading) return <div className="loading-card">Loading checkpoint graph<span className="loading-pulse"/></div>
  if (error || !graph) return <UnavailableFeature title="Live surveillance data is unavailable" message={error || 'The C++ backend returned no graph data.'} endpoint="GET /api/graph"/>
  const xValues = graph.vertices.map((vertex) => vertex.x)
  const yValues = graph.vertices.map((vertex) => vertex.y)
  const position = (value: number, values: number[]) => {
    const min = Math.min(...values)
    const max = Math.max(...values)
    return max === min ? 50 : 5 + ((value - min) / (max - min)) * 90
  }
  const points: Checkpoint[] = graph.vertices.map((vertex) => ({ id: vertex.name || String(vertex.id), x: position(vertex.x, xValues), y: position(vertex.y, yValues), state: 'normal' }))
  const ids = new Map(graph.vertices.map((vertex) => [vertex.id, vertex.name || String(vertex.id)]))
  const routes = graph.edges.map((edge) => ({ from: ids.get(edge.from) ?? String(edge.from), to: ids.get(edge.to) ?? String(edge.to), weight: edge.weight, blocked: edge.blocked }))
  const selectedVertex = graph.vertices.find((vertex) => vertex.id === selectedId) ?? null
  const selectedName = selectedVertex?.name || (selectedVertex ? String(selectedVertex.id) : '')
  return <div className="workspace-stack"><div className="workspace-kpis"><Stat label="CHECKPOINTS" value={String(points.length)} icon={<Crosshair/>}/><Stat label="ACTIVE INCIDENTS" value="—" icon={<ShieldAlert/>}/><Stat label="ROUTE EDGES" value={String(routes.length)} icon={<GitBranch/>}/><Stat label="UNITS ON FIELD" value="—" icon={<Radio/>}/></div><div className="surveillance-layout"><div onClick={(event) => { const target = (event.target as HTMLElement).closest('.checkpoint'); if (!target) return; const pointName = target.querySelector('.node-label')?.textContent; const vertex = graph.vertices.find((item) => (item.name || String(item.id)) === pointName); if (vertex) setSelectedId(vertex.id) }}><BorderMap checkpoints={points} routes={routes} live/></div><section className="panel detail-panel"><div className="panel-heading"><div><div className="eyebrow">SELECTED CHECKPOINT</div><h2>{selectedName || 'Checkpoint'}</h2></div><LocateFixed size={16}/></div>{selectedVertex ? <><div className="checkpoint-status"><i/>Graph node</div><div className="detail-coordinate">Map position · x {selectedVertex.x}, y {selectedVertex.y}</div><div className="detail-divider"/><div className="detail-label">CONNECTED POSTS</div><div className="connected-posts">{routes.filter((route) => route.from === selectedName || route.to === selectedName).map((route) => { const name = route.from === selectedName ? route.to : route.from; return <span key={name}>{name}<small>{route.blocked ? 'route blocked' : `${route.weight} km`}</small></span> })}</div></> : <p className="muted-copy">Select a checkpoint on the graph.</p>}</section></div>{points.length === 0 && <div className="workspace-note"><Radio size={15}/><span>The graph endpoint is connected, but the C++ application currently has no checkpoint graph initialized.</span></div>}</div>
}

function UnavailableFeature({ title, message, endpoint }: { title: string; message: string; endpoint: string }) {
  return <section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">C++ BACKEND RESPONSE</div><h2>{title}</h2></div><span className="demo-inline">NOT AVAILABLE</span></div><div className="workspace-note"><AlertTriangle size={15}/><span>{message}</span></div><div className="setting-row endpoint-row"><div><strong>Requested endpoint</strong><span>This page does not substitute sample data for a failed API response.</span></div><code>{endpoint}</code></div></section>
}

function Stat({ label, value, icon }: { label: string; value: string; icon: React.ReactNode }) { return <div className="panel workspace-stat"><span>{label}</span><strong>{value}</strong><i>{icon}</i></div> }

function RecordsPage() {
  const [query, setQuery] = useState('')
  const [level, setLevel] = useState('All levels')
  const [records, setRecords] = useState<IntruderRecord[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  useEffect(() => {
    let disposed = false
    getRecords().then((items) => { if (!disposed) setRecords(items) }).catch((cause: unknown) => {
      if (!disposed) setError(cause instanceof Error ? cause.message : 'Could not load intruder records.')
    }).finally(() => { if (!disposed) setLoading(false) })
    return () => { disposed = true }
  }, [])
  const rows = useMemo(() => records.filter((item) => `${item.id} ${item.name} ${item.location}`.toLowerCase().includes(query.toLowerCase()) && (level === 'All levels' || threatCategory(item.threatLevel) === level.toUpperCase())), [records, query, level])
  return <div className="panel records-panel"><div className="records-toolbar"><div className="searchbox records-search"><Search size={15}/><input value={query} onChange={(e) => setQuery(e.target.value)} placeholder="Search ID, subject or location…"/></div><select value={level} onChange={(e) => setLevel(e.target.value)}><option>All levels</option><option>High</option><option>Medium</option><option>Low</option></select><span className="demo-inline">C++ FILE DATA</span></div><div className="table-wrap"><table className="records-table"><thead><tr><th>RECORD ID</th><th>SUBJECT</th><th>LOCATION</th><th>THREAT LEVEL</th></tr></thead><tbody>{rows.map((record) => <tr key={record.id}><td><strong>{record.id}</strong></td><td>{record.name}</td><td>{record.location}</td><td><ThreatLabel threat={threatCategory(record.threatLevel)}/></td></tr>)}</tbody></table>{loading && <div className="empty-state">Loading records from the C++ backend…</div>}{error && <div className="empty-state" role="alert">Backend error: {error}</div>}{!loading && !error && rows.length === 0 && <div className="empty-state">{records.length === 0 ? 'No C++ intruder records are stored yet.' : 'No records match those filters.'}</div>}</div><div className="records-foot">Showing {rows.length} of {records.length} backend records <span>Read from intruders.txt · id, name, location, threat level</span></div></div>
}

function ThreatLabel({ threat }: { threat: Threat }) { return <span className={`table-threat threat-${threat.toLowerCase()}`}><i/>{threat}</span> }
function threatCategory(level: number): Threat { return level >= 4 ? 'HIGH' : level === 3 ? 'MEDIUM' : 'LOW' }

function AlertsPage() {
  const [filter, setFilter] = useState('ALL')
  const [alerts, setAlerts] = useState<Alert[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  useEffect(() => { let active = true; getAlerts().then((items) => { if (active) setAlerts(items) }).catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load backend alerts.') }).finally(() => { if (active) setLoading(false) }); return () => { active = false } }, [])
  if (loading) return <div className="loading-card">Loading alerts<span className="loading-pulse"/></div>
  if (error) return <UnavailableFeature title="Alert queue is unavailable" message={error} endpoint="GET /api/alerts"/>
  const shown = alerts.filter((alert) => filter === 'ALL' || alert.threat === filter)
  return <div className="workspace-stack"><div className="filter-strip">{['ALL', 'HIGH', 'MEDIUM', 'LOW'].map((f) => <button key={f} className={filter === f ? 'filter-active' : ''} onClick={() => setFilter(f)}>{f}<span>{f === 'ALL' ? alerts.length : alerts.filter((a) => a.threat === f).length}</span></button>)}<span className="demo-inline">C++ ALERT QUEUE</span></div><section className="panel alerts-page-panel"><div className="alerts-page-title"><div><div className="eyebrow">BACKEND PRIORITY ORDER</div><h2>Open alert queue</h2></div><span className="queue-state"><i/> Backend</span></div>{shown.map((alert) => <div className="alert-row alert-page-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span>{alert.id}</span><i/><span>{alert.checkpoint}</span><i/><span>{alert.time}</span></div></div><ThreatLabel threat={alert.threat}/><span className="status-text">{alert.status ?? 'Pending'}</span></div>)}{shown.length === 0 && <div className="empty-state">The C++ backend returned no alerts.</div>}</section></div>
}

function PatrolPage() {
  const [units, setUnits] = useState<PatrolUnit[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  useEffect(() => { let active = true; getPatrolUnits().then((items) => { if (active) setUnits(items) }).catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load patrol units.') }).finally(() => { if (active) setLoading(false) }); return () => { active = false } }, [])
  if (loading) return <div className="loading-card">Loading patrol units<span className="loading-pulse"/></div>
  if (error) return <UnavailableFeature title="Patrol unit data is unavailable" message={error} endpoint="GET /api/patrol-units"/>
  return <div className="workspace-stack"><div className="unit-grid">{units.map((unit) => <article className="panel unit-card" key={unit.id}><div className="unit-card-head"><div className="unit-icon"><Radio size={17}/></div><span className={`unit-status status-${unit.status.toLowerCase().replace(' ', '-')}`}><i/>{unit.status}</span></div><h2>{unit.id}</h2><div className="unit-location"><LocateFixed size={13}/>{unit.checkpoint}</div><div className="unit-assignment"><span>ASSIGNMENT</span><strong>{unit.assignment}</strong></div><div className="unit-updated"><Clock3 size={12}/>Updated {unit.updated}</div></article>)}</div>{units.length === 0 && <div className="workspace-note"><Radio size={15}/><span>The C++ backend returned no patrol units.</span></div>}</div>
}

function DsaPage() {
  const [selected, setSelected] = useState('Graph')
  const cards = [
    { name: 'Graph', icon: <GitBranch/>, status: 'IMPLEMENTED', description: 'Adjacency-list graph with checkpoint coordinates, weighted routes, blocked edges and exported visual state.', cpp: 'Graph::exportState()\nGraphAlgo::shortestPath(g, source, target)' },
    { name: 'Graph traversal', icon: <Activity/>, status: 'IMPLEMENTED', description: 'Breadth-first and depth-first traversal ignore blocked routes and can emit step messages through callbacks.', cpp: 'GraphAlgo::bfs(g, start, order, log)\nGraphAlgo::dfsIterative(g, start, order, log)' },
    { name: 'Shortest path', icon: <Crosshair/>, status: 'IMPLEMENTED', description: 'Dijkstra routing and nearest-candidate helper support patrol dispatch calculations in C++.', cpp: 'GraphAlgo::dijkstra(g, source, dist, prev)\nGraphAlgo::nearestCandidate(...)' },
    { name: 'Binary search tree', icon: <GitBranch/>, status: 'IMPLEMENTED', description: 'Intruder BST supports name search, prefix suggestions, traversals and an exported node view.', cpp: 'BST::prefixSearch(prefix, results)\nBST::exportState()' },
    { name: 'Stack & queue', icon: <Database/>, status: 'GRAPH INTERNALS', description: 'The graph algorithms use small C++ stack and circular queue helpers for traversals. App-level alert/undo structures are not present in this repository yet.', cpp: 'SimpleStack<T>\nSimpleQueue<T>' },
    { name: 'Hash table', icon: <Database/>, status: 'NOT IN CURRENT REPO', description: 'The README describes hashing as planned project scope, but a hash-table implementation is not present in the checked-in folders.', cpp: 'No hash-table source found' },
  ]
  const active = cards.find((card) => card.name === selected) ?? cards[0]
  return <div className="dsa-layout"><section className="panel dsa-menu"><div className="dsa-menu-heading"><div className="eyebrow">STRUCTURES &amp; ALGORITHMS</div><h2>Implementation map</h2></div>{cards.map((card) => <button className={`dsa-option ${selected === card.name ? 'selected' : ''}`} onClick={() => setSelected(card.name)} key={card.name}>{card.icon}<span>{card.name}<small>{card.status}</small></span><ChevronRight size={15}/></button>)}</section><section className="panel dsa-detail"><div className="dsa-detail-heading"><div className="dsa-feature-icon">{active.icon}</div><div><div className="eyebrow">C++ SOURCE OVERVIEW</div><h2>{active.name}</h2></div><span className="implementation-badge">{active.status}</span></div><p className="dsa-description">{active.description}</p><div className="dsa-code-label">PROJECT API / SOURCE SYMBOLS</div><pre className="code-block"><code>{active.cpp}</code></pre><div className="workspace-note"><Command size={14}/><span>This panel documents the checked-in C++ surface. Interactive operation visualizations need an API adapter to call the real implementations.</span><DemoTag/></div></section><div className="dsa-footnote"><Shield size={15}/><span>Algorithm execution belongs to the C++ backend. This UI does not recreate graph traversal, Dijkstra, or other DSA behavior in JavaScript.</span></div></div>
}

function SettingsPage() {
  const { status } = useBackendStatus()
  const [compact, setCompact] = useState(false)
  const [notifications, setNotifications] = useState(true)
  return <div className="settings-layout"><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">OPERATOR PROFILE</div><h2>Account</h2></div></div><div className="profile-row"><div className="avatar profile-avatar">A</div><div><strong>Admin</strong><span>Administrator · Local demo session</span></div><span className="profile-chip">ACTIVE</span></div><div className="settings-divider"/><div className="setting-row"><div><strong>Compact dashboard</strong><span>Use tighter spacing on smaller displays.</span></div><button className={`toggle ${compact ? 'on' : ''}`} onClick={() => setCompact(!compact)} aria-pressed={compact}><i/></button></div><div className="setting-row"><div><strong>Operational notifications</strong><span>Show alert badges and event indicators.</span></div><button className={`toggle ${notifications ? 'on' : ''}`} onClick={() => setNotifications(!notifications)} aria-pressed={notifications}><i/></button></div></section><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">CONNECTIVITY</div><h2>Backend status</h2></div><span className="demo-inline">DEVELOPMENT</span></div><div className={`backend-status backend-${status}`}><span className="backend-icon"><Wifi size={18}/></span><div><strong>{status === 'connected' ? 'Backend connected' : status === 'checking' ? 'Checking backend connection' : 'Backend disconnected'}</strong><span>Records and dashboard statistics use the C++ API; unsupported operational modules stay unavailable.</span></div><i/></div><div className="setting-row endpoint-row"><div><strong>Health endpoint</strong><span>Configured API base + /health</span></div><code>{API_BASE}/health</code></div><div className="workspace-note"><Check size={14}/><span>Health, dashboard and record endpoints are implemented. Graph, alert and patrol routes return an explicit not implemented response until their C++ runtime data is added.</span></div></section></div>
}
