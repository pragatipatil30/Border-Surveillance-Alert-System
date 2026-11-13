import { useEffect, useMemo, useState } from 'react'
import { Activity, AlertTriangle, Bell, Check, ChevronRight, Clock3, Command, Crosshair, Database, GitBranch, LocateFixed, Radio, Search, Shield, ShieldAlert, Wifi } from 'lucide-react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { useBackendStatus } from '../services/BackendStatusContext'
import { API_BASE, getDashboard, getRecords } from '../services/api'
import type { Checkpoint, DashboardData, IntruderRecord, Threat } from '../types/dashboard'

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
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  useEffect(() => { getDashboard().then(({ data }) => setDashboard(data)) }, [])
  return <main className="dashboard-content"><PageHeader page={page} onNavigate={onNavigate}/>{!dashboard ? <div className="loading-card">Establishing command link<span className="loading-pulse"/></div> : <>
    {page === 'surveillance' && <SurveillancePage data={dashboard}/>}
    {page === 'records' && <RecordsPage/>}
    {page === 'alerts' && <AlertsPage data={dashboard}/>}
    {page === 'patrol' && <PatrolPage data={dashboard}/>}
    {page === 'dsa' && <DsaPage/>}
    {page === 'settings' && <SettingsPage/>}
    </>}<footer className="page-footer"><span>Border Command Center <i>·</i> Development view</span><span>DATA SHOWN AS LABELED</span></footer></main>
}

function SurveillancePage({ data }: { data: DashboardData }) {
  const [selected, setSelected] = useState<Checkpoint | null>(data.checkpoints.find((p) => p.state === 'selected') ?? null)
  const activeCount = data.checkpoints.filter((p) => p.state === 'alert').length
  return <div className="workspace-stack"><div className="workspace-kpis"><Stat label="CHECKPOINTS" value={String(data.checkpoints.length)} icon={<Crosshair/>}/><Stat label="ACTIVE INCIDENTS" value={String(activeCount)} icon={<ShieldAlert/>}/><Stat label="ROUTE EDGES" value={String(data.routes.length)} icon={<GitBranch/>}/><Stat label="UNITS ON FIELD" value={String(data.patrolUnits.filter((p) => p.status !== 'Offline').length)} icon={<Radio/>}/></div><div className="surveillance-layout"><div onClick={(event) => { const target = (event.target as HTMLElement).closest('.checkpoint'); if (!target) return; setSelected(data.checkpoints.find((p) => p.id === target.querySelector('.node-label')?.textContent) ?? selected) }}><BorderMap checkpoints={data.checkpoints} routes={data.routes}/></div><section className="panel detail-panel"><div className="panel-heading"><div><div className="eyebrow">SELECTED POST</div><h2>{selected?.id ?? 'Checkpoint'}</h2></div><LocateFixed size={16}/></div>{selected ? <><div className="checkpoint-status"><i className={selected.state === 'alert' ? 'is-alert' : ''}/>{selected.state === 'alert' ? 'Incident detected' : 'Operational'}</div><div className="detail-coordinate">Map position · x {selected.x.toFixed(0)}, y {selected.y.toFixed(0)}</div><div className="detail-divider"/><div className="detail-label">CONNECTED POSTS</div><div className="connected-posts">{data.routes.filter((r) => r.from === selected.id || r.to === selected.id).map((r) => { const id = r.from === selected.id ? r.to : r.from; return <span key={id}>{id}<small>weight {r.weight}</small></span> })}</div><div className="detail-divider"/><div className="detail-label">RECENT INCIDENTS</div>{data.alerts.filter((a) => a.checkpoint.includes(selected.id)).slice(0, 2).map((a) => <div className="mini-event" key={a.id}><ShieldAlert size={14}/><span>{a.title}<small>{a.time}</small></span></div>)}{!data.alerts.some((a) => a.checkpoint.includes(selected.id)) && <p className="muted-copy">No recent incidents at this post.</p>}</> : <p className="muted-copy">Select a checkpoint on the map.</p>}</section></div><div className="workspace-note"><Radio size={15}/><span>Map points and routes are sample values. The C++ graph exports checkpoint coordinates and blocked route state; a backend endpoint is not connected.</span><DemoTag/></div></div>
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

function AlertsPage({ data }: { data: DashboardData }) {
  const [filter, setFilter] = useState('ALL')
  const [alerts, setAlerts] = useState(data.alerts)
  const shown = alerts.filter((alert) => filter === 'ALL' || alert.threat === filter)
  return <div className="workspace-stack"><div className="filter-strip">{['ALL', 'HIGH', 'MEDIUM', 'LOW'].map((f) => <button key={f} className={filter === f ? 'filter-active' : ''} onClick={() => setFilter(f)}>{f}<span>{f === 'ALL' ? alerts.length : alerts.filter((a) => a.threat === f).length}</span></button>)}<DemoTag/></div><section className="panel alerts-page-panel"><div className="alerts-page-title"><div><div className="eyebrow">SAMPLE QUEUE ORDER</div><h2>Open alert queue</h2></div><span className="queue-state"><i/> Demo data</span></div>{shown.map((alert) => <div className="alert-row alert-page-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span>{alert.id}</span><i/><span>{alert.checkpoint}</span><i/><span>{alert.time}</span></div></div><ThreatLabel threat={alert.threat}/><span className="status-text">{alert.status ?? 'Pending'}</span></div>)}{shown.length === 0 && <div className="empty-state">No alerts at this level.</div>}<div className="workspace-note"><AlertTriangle size={14}/><span>Alert data is a development sample. The checked-in C++ code does not yet expose an alert dispatch endpoint.</span><DemoTag/></div></section></div>
}

function PatrolPage({ data }: { data: DashboardData }) {
  return <div className="workspace-stack"><div className="unit-grid">{data.patrolUnits.map((unit) => <article className="panel unit-card" key={unit.id}><div className="unit-card-head"><div className="unit-icon"><Radio size={17}/></div><span className={`unit-status status-${unit.status.toLowerCase().replace(' ', '-')}`}><i/>{unit.status}</span></div><h2>{unit.id}</h2><div className="unit-location"><LocateFixed size={13}/>{unit.checkpoint}</div><div className="unit-assignment"><span>ASSIGNMENT</span><strong>{unit.assignment}</strong></div><div className="unit-updated"><Clock3 size={12}/>Updated {unit.updated}</div></article>)}</div><div className="workspace-note"><Radio size={15}/><span>Unit list is development sample data. Patrol unit operations are not currently exposed by the C++ console program.</span><DemoTag/></div></div>
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
  return <div className="settings-layout"><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">OPERATOR PROFILE</div><h2>Account</h2></div></div><div className="profile-row"><div className="avatar profile-avatar">A</div><div><strong>Admin</strong><span>Administrator · Local demo session</span></div><span className="profile-chip">ACTIVE</span></div><div className="settings-divider"/><div className="setting-row"><div><strong>Compact dashboard</strong><span>Use tighter spacing on smaller displays.</span></div><button className={`toggle ${compact ? 'on' : ''}`} onClick={() => setCompact(!compact)} aria-pressed={compact}><i/></button></div><div className="setting-row"><div><strong>Operational notifications</strong><span>Show alert badges and event indicators.</span></div><button className={`toggle ${notifications ? 'on' : ''}`} onClick={() => setNotifications(!notifications)} aria-pressed={notifications}><i/></button></div></section><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">CONNECTIVITY</div><h2>Backend status</h2></div><span className="demo-inline">DEVELOPMENT</span></div><div className={`backend-status backend-${status}`}><span className="backend-icon"><Wifi size={18}/></span><div><strong>{status === 'connected' ? 'Backend connected' : status === 'checking' ? 'Checking backend connection' : 'Backend disconnected'}</strong><span>Dashboard sample data remains separate from connection status.</span></div><i/></div><div className="setting-row endpoint-row"><div><strong>Health endpoint</strong><span>Configured API base + /health</span></div><code>{API_BASE}/health</code></div><div className="workspace-note"><Check size={14}/><span>Health check is implemented; dashboard and operational data routes are not yet available.</span></div></section></div>
}
