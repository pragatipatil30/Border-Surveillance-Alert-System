import { useCallback, useEffect, useState } from 'react'
import { AlertTriangle, Check, ChevronRight, Clock3, Crosshair, GitBranch, LocateFixed, Radio, Search, ShieldAlert, Wifi } from 'lucide-react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { DsaLab } from './DsaLab'
import { useBackendStatus } from '../services/BackendStatusContext'
import { API_BASE, blockRoute, changeUnitStatus, createRecord, deleteRecord, deployUnit, dispatchAlert, dispatchNextAlert, generateDemoEvent, getAlertQueues, getAlerts, getCheckpointDetails, getOperationalGraph, getPatrolUnits, getRecordsDetailed, getUndoState, redoRecord, releaseUnit, resetDemo, resolveAlert, setAlertPolicy, undoRecord, unblockRoute, updateRecord } from '../services/api'
import type { Alert, AlertQueues, Checkpoint, CheckpointDetails, IntruderRecord, OperationalGraph, PatrolUnit, Threat } from '../types/dashboard'

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
  return <><header className="topbar"><div className="breadcrumb"><button className="breadcrumb-button" onClick={() => onNavigate('dashboard')}>COMMAND</button><ChevronRight size={13}/>{page.toUpperCase()}</div><div className="topbar-actions"><ConnectionStatus/></div></header><div className="welcome-row"><div><div className="eyebrow welcome-kicker">{title.eyebrow}</div><h1>{title.title.split('.')[0]}<em>{title.title.includes('.') ? `.${title.title.split('.').slice(1).join('.')}` : ''}</em></h1><p>{title.description}</p></div></div></>
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
  const [alerts, setAlerts] = useState<Alert[]>([])
  const [units, setUnits] = useState<PatrolUnit[]>([])
  const [selectedName, setSelectedName] = useState('')
  const [details, setDetails] = useState<CheckpointDetails | null>(null)
  const [highlightRoute, setHighlightRoute] = useState<string[]>([])
  const [busy, setBusy] = useState(false)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const [notice, setNotice] = useState('')
  const { subscribeEvents } = useBackendStatus()
  const refresh = useCallback(async () => {
    const [nextGraph, nextAlerts, nextUnits] = await Promise.all([getOperationalGraph(), getAlerts(), getPatrolUnits()])
    setGraph(nextGraph); setAlerts(nextAlerts); setUnits(nextUnits)
    return nextGraph
  }, [])
  useEffect(() => {
    let active = true
    refresh().then((loadedGraph) => {
      if (!active) return
      const firstName = loadedGraph.vertices[0]?.name
      if (firstName) setSelectedName(firstName)
    }).catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load the checkpoint graph.') }).finally(() => { if (active) setLoading(false) })
    const unsubscribe = subscribeEvents((event) => {
      if (['graph.changed', 'alert.created', 'alert.dispatched', 'alert.resolved', 'unit.updated', 'demo.reset', 'record.created'].includes(event.event)) void refresh().catch(() => undefined)
    })
    return () => { active = false; unsubscribe() }
  }, [refresh, subscribeEvents])
  useEffect(() => {
    if (!selectedName) return
    let active = true
    getCheckpointDetails(selectedName).then((value) => { if (active) setDetails(value) }).catch(() => { if (active) setDetails(null) })
    return () => { active = false }
  }, [selectedName])
  if (loading) return <div className="loading-card">Loading checkpoint graph<span className="loading-pulse"/></div>
  if (error || !graph) return <UnavailableFeature title="Live surveillance data is unavailable" message={error || 'The C++ backend returned no graph data.'} endpoint="GET /api/graph"/>
  const xValues = graph.vertices.map((vertex) => vertex.x)
  const yValues = graph.vertices.map((vertex) => vertex.y)
  const position = (value: number, values: number[]) => {
    const min = Math.min(...values)
    const max = Math.max(...values)
    return max === min ? 50 : 5 + ((value - min) / (max - min)) * 90
  }
  const activePosts = new Set(alerts.filter((alert) => alert.status !== 'Resolved').map((alert) => alert.checkpoint))
  const points: Checkpoint[] = graph.vertices.map((vertex) => ({ id: vertex.name || String(vertex.id), x: position(vertex.x, xValues), y: position(vertex.y, yValues), state: activePosts.has(vertex.name) ? 'alert' : 'normal' }))
  const ids = new Map(graph.vertices.map((vertex) => [vertex.id, vertex.name || String(vertex.id)]))
  const routes = graph.edges.map((edge) => ({ from: ids.get(edge.from) ?? String(edge.from), to: ids.get(edge.to) ?? String(edge.to), weight: edge.weight, blocked: edge.blocked }))
  const selectedVertex = graph.vertices.find((vertex) => vertex.name === selectedName) ?? null
  const selectedAlert = alerts.find((alert) => alert.checkpoint === selectedName && alert.status === 'Queued')
  const handleRoute = async (route: { from: string; to: string; weight: number; blocked?: boolean }) => {
    if (!window.confirm(`${route.blocked ? 'Reopen' : 'Block'} route ${route.from} to ${route.to}?`)) return
    setBusy(true); setError('')
    try { if (route.blocked) await unblockRoute(route.from, route.to); else await blockRoute(route.from, route.to); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Route could not be changed.') }
    finally { setBusy(false) }
  }
  const handleDemoEvent = async () => {
    setBusy(true); setError(''); setNotice('')
    try { const event = await generateDemoEvent(); setNotice(`New intruder ${event.record.id} generated at ${event.record.location}.`); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Test event could not be generated.') }
    finally { setBusy(false) }
  }
  const handleDispatch = async () => {
    if (!selectedAlert) return
    setBusy(true); setError(''); setNotice('')
    try { const result = await dispatchAlert(selectedAlert.id); setHighlightRoute(result.route ?? []); setNotice(`${selectedAlert.id} dispatched to ${result.assignedUnit ?? 'a patrol unit'}.`); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Alert could not be dispatched.') }
    finally { setBusy(false) }
  }
  const selectCheckpoint = (name: string) => { setSelectedName(name); setDetails(null) }
  return <div className="workspace-stack"><div className="workspace-kpis"><Stat label="CHECKPOINTS" value={String(points.length)} icon={<Crosshair/>}/><Stat label="ACTIVE INCIDENTS" value={String(alerts.filter((item) => item.status !== 'Resolved').length)} icon={<ShieldAlert/>}/><Stat label="ROUTE EDGES" value={String(routes.length)} icon={<GitBranch/>}/><Stat label="UNITS ON FIELD" value={String(units.filter((unit) => unit.status !== 'Offline').length)} icon={<Radio/>}/></div><div className="surveillance-actions"><button className="dispatch-button" disabled={busy} onClick={() => void handleDemoEvent()}>{busy ? 'Working…' : 'Generate Test Intruder Event'}</button>{selectedAlert && <button className="dispatch-button" disabled={busy} onClick={() => void handleDispatch()}>Dispatch {selectedAlert.id}</button>}</div>{error && <div className="workspace-note" role="alert">{error}</div>}{notice && <div className="dispatch-feedback feedback-success" role="status">{notice}</div>}<div className="surveillance-layout"><BorderMap checkpoints={points} routes={routes} live patrolUnits={units} highlightRoute={highlightRoute} onSelectCheckpoint={selectCheckpoint} onRouteClick={(route) => void handleRoute(route)}/><section className="panel detail-panel"><div className="panel-heading"><div><div className="eyebrow">SELECTED CHECKPOINT</div><h2>{selectedName || 'Checkpoint'}</h2></div><LocateFixed size={16}/></div>{selectedVertex ? <><div className="checkpoint-status"><i className={activePosts.has(selectedName) ? 'is-alert' : ''}/>{details?.status ?? 'Operational'}</div><div className="detail-coordinate">Map position · x {selectedVertex.x}, y {selectedVertex.y}</div><div className="detail-divider"/><div className="detail-label">CONNECTED POSTS</div><div className="connected-posts">{(details?.connectedCheckpoints ?? routes.filter((route) => route.from === selectedName || route.to === selectedName).map((route) => ({ id: route.from === selectedName ? route.to : route.from, weight: route.weight, blocked: Boolean(route.blocked) }))).map((route) => <span key={route.id}>{route.id}<small>{route.blocked ? 'route blocked' : `${route.weight} km`}</small></span>)}</div><div className="detail-divider"/><div className="detail-label">RECENT INCIDENTS</div>{details?.recentIncidents.length ? details.recentIncidents.slice(0, 3).map((incident) => <div className="mini-event" key={incident.id}><ShieldAlert size={14}/><span>{incident.id} · {incident.status}<small>{incident.threat} · {incident.assignedUnit || 'Awaiting dispatch'}</small></span></div>) : <p className="muted-copy">No active incidents at this checkpoint.</p>}</> : <p className="muted-copy">Select a checkpoint on the map.</p>}</section></div></div>
}

function UnavailableFeature({ title, message, endpoint }: { title: string; message: string; endpoint: string }) {
  return <section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">C++ BACKEND RESPONSE</div><h2>{title}</h2></div><span className="demo-inline">NOT AVAILABLE</span></div><div className="workspace-note"><AlertTriangle size={15}/><span>{message}</span></div><div className="setting-row endpoint-row"><div><strong>Requested endpoint</strong><span>This page does not substitute sample data for a failed API response.</span></div><code>{endpoint}</code></div></section>
}

function Stat({ label, value, icon }: { label: string; value: string; icon: React.ReactNode }) { return <div className="panel workspace-stat"><span>{label}</span><strong>{value}</strong><i>{icon}</i></div> }

function RecordsPage() {
  const [query, setQuery] = useState(() => sessionStorage.getItem('border-command-record-search') ?? '')
  const [level, setLevel] = useState('All levels')
  const [sort, setSort] = useState('id')
  const [algorithm, setAlgorithm] = useState('merge')
  const [records, setRecords] = useState<IntruderRecord[]>([])
  const [sortStats, setSortStats] = useState({ algorithm: 'merge', comparisons: 0, passes: 0 })
  const [selected, setSelected] = useState<IntruderRecord | null>(null)
  const [editing, setEditing] = useState<IntruderRecord | null>(null)
  const [creating, setCreating] = useState(false)
  const [busy, setBusy] = useState(false)
  const [notice, setNotice] = useState('')
  const [undoState, setUndoState] = useState({ canUndo: false, canRedo: false, undoCount: 0, redoCount: 0 })
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const { subscribeEvents } = useBackendStatus()
  const refresh = useCallback(async () => {
    const result = await getRecordsDetailed({ q: query, threat: level === 'All levels' ? undefined : level.toUpperCase(), sort, algorithm })
    setRecords(result.records); setSortStats(result.sort)
    setUndoState(await getUndoState())
  }, [query, level, sort, algorithm])
  useEffect(() => {
    let disposed = false
    const timer = window.setTimeout(() => {
      refresh().catch((cause: unknown) => { if (!disposed) setError(cause instanceof Error ? cause.message : 'Could not load intruder records.') })
        .finally(() => { if (!disposed) setLoading(false) })
    }, 180)
    return () => { disposed = true; window.clearTimeout(timer) }
  }, [refresh])
  useEffect(() => subscribeEvents((event) => {
    if (['record.created', 'record.updated', 'record.deleted', 'demo.reset'].includes(event.event)) void refresh().catch(() => undefined)
  }), [refresh, subscribeEvents])

  const save = async () => {
    if (!editing) return
    setBusy(true); setError(''); setNotice('')
    try {
      const record = { ...editing, threatLevel: Number(editing.threatLevel), status: editing.status ?? 'Active' }
      if (creating) await createRecord(record); else await updateRecord(editing.id, record)
      setNotice(creating ? 'Record added.' : 'Record updated.'); setEditing(null); setCreating(false); await refresh()
    } catch (cause) { setError(cause instanceof Error ? cause.message : 'The record could not be saved.') }
    finally { setBusy(false) }
  }
  const remove = async () => {
    if (!selected || !window.confirm(`Delete record ${selected.id}? This action can be undone.`)) return
    setBusy(true); setError(''); setNotice('')
    try { await deleteRecord(selected.id); setNotice(`Record ${selected.id} deleted.`); setSelected(null); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'The record could not be deleted.') }
    finally { setBusy(false) }
  }
  const runHistory = async (redo: boolean) => {
    setBusy(true); setError(''); setNotice('')
    try { await (redo ? redoRecord() : undoRecord()); setNotice(redo ? 'Record change redone.' : 'Record change undone.'); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Record history operation failed.') }
    finally { setBusy(false) }
  }
  const exportCsv = () => {
    const rows = [['id', 'name', 'location', 'threatLevel', 'status'], ...records.map((item) => [item.id, item.name, item.location, String(item.threatLevel), item.status ?? 'Active'])]
    const csv = rows.map((row) => row.map((value) => `"${value.replace(/"/g, '""')}"`).join(',')).join('\r\n')
    const link = document.createElement('a'); link.href = URL.createObjectURL(new Blob([csv], { type: 'text/csv' })); link.download = 'intruder-records.csv'; link.click(); URL.revokeObjectURL(link.href)
  }
  const startCreate = () => { setCreating(true); setEditing({ id: '', name: '', location: '', threatLevel: 3, status: 'Active' }) }
  return <div className="workspace-stack">
    <section className="panel records-panel"><div className="records-toolbar"><div className="searchbox records-search"><Search size={15}/><input value={query} onChange={(event) => { setQuery(event.target.value); sessionStorage.removeItem('border-command-record-search') }} placeholder="Search ID, name prefix or location…"/></div><select value={level} onChange={(event) => setLevel(event.target.value)}><option>All levels</option><option>High</option><option>Medium</option><option>Low</option></select><label className="dsa-field">Sort<select value={sort} onChange={(event) => setSort(event.target.value)}><option value="id">ID</option><option value="threat">Threat</option><option value="name">Name</option><option value="location">Location</option></select></label><label className="dsa-field">Algorithm<select value={algorithm} onChange={(event) => setAlgorithm(event.target.value)}><option value="bubble">Bubble</option><option value="insertion">Insertion</option><option value="merge">Merge</option><option value="quick">Quick</option></select></label><button className="dispatch-button" disabled={busy} onClick={startCreate}>Add record</button><button className="dispatch-button" disabled={busy || !undoState.canUndo} onClick={() => void runHistory(false)}>Undo ({undoState.undoCount})</button><button className="dispatch-button" disabled={busy || !undoState.canRedo} onClick={() => void runHistory(true)}>Redo ({undoState.redoCount})</button><button className="dispatch-button" onClick={exportCsv}>Export CSV</button><span className="demo-inline">C++ FILE DATA</span></div>
      <label className="disabled-rule">Rule filter <input disabled placeholder="C++ rule evaluator not connected" title="This filter is disabled until the C++ expression evaluator is available"/></label>
      {error && <div className="workspace-note" role="alert">{error}</div>}{notice && <div className="dispatch-feedback feedback-success" role="status">{notice}</div>}
      <div className="table-wrap"><table className="records-table"><thead><tr><th>RECORD ID</th><th>SUBJECT</th><th>LOCATION</th><th>THREAT LEVEL</th><th>STATUS</th><th>DETAILS</th></tr></thead><tbody>{records.map((record) => <tr key={record.id} className={selected?.id === record.id ? 'selected-row' : ''} onClick={() => setSelected(record)}><td><strong>{record.id}</strong></td><td>{record.name}</td><td>{record.location}</td><td><ThreatLabel threat={threatCategory(record.threatLevel)}/></td><td>{record.status ?? 'Active'}</td><td><button type="button" className="text-link" onClick={(event) => { event.stopPropagation(); setSelected(record) }}>View</button></td></tr>)}</tbody></table>{loading && <div className="empty-state">Loading records from the C++ backend…</div>}{!loading && !error && records.length === 0 && <div className="empty-state">No records match those filters.</div>}</div><div className="records-foot">Showing {records.length} C++ records <span>{sortStats.algorithm} sort · {sortStats.comparisons} comparisons · {sortStats.passes} passes</span></div>
    </section>
    <div className="records-side-grid"><section className="panel detail-panel"><div className="panel-heading"><div><div className="eyebrow">RECORD DETAILS</div><h2>{selected?.id ?? 'Select a record'}</h2></div></div>{selected ? <><strong>{selected.name}</strong><p>{selected.location} · Threat {selected.threatLevel} · {selected.status ?? 'Active'}</p><div className="dsa-controls"><button className="dispatch-button" disabled={busy} onClick={() => { setCreating(false); setEditing(selected) }}>Edit</button><button className="dispatch-button" disabled={busy} onClick={() => void remove()}>Delete</button></div></> : <p className="muted-copy">Choose a row to see its record details.</p>}<div className="undo-stack"><strong>Record history</strong><span>{undoState.undoCount} undo · {undoState.redoCount} redo</span><small>Stored and replayed by the C++ API.</small></div></section>
      {editing && <section className="panel detail-panel"><div className="panel-heading"><div><div className="eyebrow">{creating ? 'NEW RECORD' : 'EDIT RECORD'}</div><h2>{creating ? 'Add intruder' : editing.id}</h2></div><button className="text-link" onClick={() => { setEditing(null); setCreating(false) }}>Cancel</button></div><div className="record-form"><label>ID<input value={editing.id} disabled={!creating} onChange={(event) => setEditing({ ...editing, id: event.target.value })}/></label><label>Name<input value={editing.name} onChange={(event) => setEditing({ ...editing, name: event.target.value })}/></label><label>Location<input value={editing.location} onChange={(event) => setEditing({ ...editing, location: event.target.value })}/></label><label>Threat level<select value={editing.threatLevel} onChange={(event) => setEditing({ ...editing, threatLevel: Number(event.target.value) })}><option value={1}>1 · Low</option><option value={2}>2 · Low</option><option value={3}>3 · Medium</option><option value={4}>4 · High</option><option value={5}>5 · High</option></select></label><label>Status<select value={editing.status ?? 'Active'} onChange={(event) => setEditing({ ...editing, status: event.target.value })}><option>Active</option><option>Dispatched</option><option>Resolved</option></select></label><button className="dispatch-button" disabled={busy || !editing.id || !editing.name || !editing.location} onClick={() => void save()}>{busy ? 'Saving…' : 'Save record'}</button></div></section>}</div>
  </div>
}

function ThreatLabel({ threat }: { threat: Threat }) { return <span className={`table-threat threat-${threat.toLowerCase()}`}><i/>{threat}</span> }
function threatCategory(level: number): Threat { return level >= 4 ? 'HIGH' : level === 3 ? 'MEDIUM' : 'LOW' }

function AlertsPage() {
  const [filter, setFilter] = useState('All')
  const [alerts, setAlerts] = useState<Alert[]>([])
  const [queues, setQueues] = useState<AlertQueues | null>(null)
  const [policy, setPolicy] = useState<'PRIORITY' | 'FCFS'>('PRIORITY')
  const [busy, setBusy] = useState<string | null>(null)
  const [notice, setNotice] = useState('')
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const { subscribeEvents } = useBackendStatus()
  const refresh = useCallback(async () => {
    const [items, queue] = await Promise.all([getAlerts(), getAlertQueues()])
    setAlerts(items); setQueues(queue); setPolicy(queue.policy)
  }, [])
  useEffect(() => { let active = true; refresh().catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load backend alerts.') }).finally(() => { if (active) setLoading(false) }); const unsubscribe = subscribeEvents((event) => { if (['alert.created', 'alert.dispatched', 'alert.resolved', 'unit.updated', 'demo.reset'].includes(event.event)) void refresh().catch(() => undefined) }); return () => { active = false; unsubscribe() } }, [refresh, subscribeEvents])
  if (loading) return <div className="loading-card">Loading alerts<span className="loading-pulse"/></div>
  if (error) return <UnavailableFeature title="Alert queue is unavailable" message={error} endpoint="GET /api/alerts"/>
  const shown = alerts.filter((alert) => filter === 'All' || (['HIGH', 'MEDIUM', 'LOW'].includes(filter) ? alert.threat === filter : alert.status === filter))
  const act = async (id: string, operation: () => Promise<unknown>) => { setBusy(id); setError(''); setNotice(''); try { await operation(); setNotice(`${id} updated.`); await refresh() } catch (cause) { setError(cause instanceof Error ? cause.message : 'Alert action failed.') } finally { setBusy(null) } }
  const changePolicy = async (next: 'PRIORITY' | 'FCFS') => { setBusy('policy'); setError(''); try { await setAlertPolicy(next); setPolicy(next); await refresh() } catch (cause) { setError(cause instanceof Error ? cause.message : 'Alert order could not be changed.') } finally { setBusy(null) } }
  const tabs = ['All', 'HIGH', 'MEDIUM', 'LOW', 'Queued', 'Dispatched', 'Resolved']
  return <div className="workspace-stack"><div className="filter-strip">{tabs.map((tab) => <button key={tab} className={filter === tab ? 'filter-active' : ''} onClick={() => setFilter(tab)}>{tab}<span>{alerts.filter((item) => tab === 'All' || (['HIGH', 'MEDIUM', 'LOW'].includes(tab) ? item.threat === tab : item.status === tab)).length}</span></button>)}<label className="policy-switch">Order<select disabled={busy === 'policy'} value={policy} onChange={(event) => void changePolicy(event.target.value as 'PRIORITY' | 'FCFS')}><option value="PRIORITY">Priority</option><option value="FCFS">First come</option></select></label><button className="dispatch-button" disabled={Boolean(busy) || !queues?.waiting.length} onClick={() => void act('Next alert', dispatchNextAlert)}>Dispatch Next</button></div>{error && <div className="workspace-note" role="alert">{error}</div>}{notice && <div className="dispatch-feedback feedback-success" role="status">{notice}</div>}<section className="panel alerts-page-panel"><div className="alerts-page-title"><div><div className="eyebrow">ORDERED BY C++ {policy === 'PRIORITY' ? 'PRIORITY QUEUE' : 'ARRIVAL TIME'}</div><h2>Alert queue</h2></div><span className="queue-state"><i/>{queues?.waiting.length ?? 0} waiting</span></div>{shown.map((alert) => <div className="alert-row alert-page-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span>{alert.id}</span><i/><span>{alert.checkpoint}</span><i/><span>{alert.time}</span>{alert.assignedUnit && <><i/><span>{alert.assignedUnit}</span></>}</div>{alert.route?.length ? <small>Route: {alert.route.join(' → ')} · {alert.distanceKm} km</small> : null}</div><ThreatLabel threat={alert.threat}/><span className="status-text">{alert.status}</span>{alert.status === 'Queued' && <button className="dispatch-button" disabled={Boolean(busy)} onClick={() => void act(alert.id, () => dispatchAlert(alert.id))}>{busy === alert.id ? 'Dispatching…' : 'Dispatch'}</button>}{alert.status === 'Dispatched' && <button className="dispatch-button" disabled={Boolean(busy)} onClick={() => void act(alert.id, () => resolveAlert(alert.id))}>{busy === alert.id ? 'Resolving…' : 'Resolve'}</button>}</div>)}{shown.length === 0 && <div className="empty-state">No alerts match this filter.</div>}</section>
    <section className="panel queue-visual"><div className="panel-heading"><div><div className="eyebrow">C++ CIRCULAR BUFFER</div><h2>Incoming queue</h2></div><span>Capacity {queues?.capacity ?? 0} · Front {queues?.frontIndex ?? -1} · Rear {queues?.rearIndex ?? -1}</span></div><div className="queue-ring">{queues?.incoming.map((slot) => <div className={`queue-slot ${slot.occupied ? 'occupied' : ''}`} key={slot.index}><small>{slot.index}</small><strong>{slot.alertId ?? '—'}</strong></div>)}</div><div className="waiting-chain"><strong>Waiting by {policy === 'PRIORITY' ? 'priority' : 'arrival'}</strong>{queues?.waiting.map((alert) => <span key={alert.id}>{alert.id} · {alert.threat} · {alert.checkpoint}</span>)}</div></section>
  </div>
}

function PatrolPage() {
  const [units, setUnits] = useState<PatrolUnit[]>([])
  const [alerts, setAlerts] = useState<Alert[]>([])
  const [checkpoints, setCheckpoints] = useState<string[]>([])
  const [deployment, setDeployment] = useState<Record<string, string>>({})
  const [busy, setBusy] = useState<string | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const { subscribeEvents } = useBackendStatus()
  const refresh = useCallback(async () => { const [items, nextAlerts, graph] = await Promise.all([getPatrolUnits(), getAlerts(), getOperationalGraph()]); setUnits(items); setAlerts(nextAlerts); setCheckpoints(graph.vertices.map((vertex) => vertex.name)) }, [])
  useEffect(() => { let active = true; refresh().catch((cause: unknown) => { if (active) setError(cause instanceof Error ? cause.message : 'Could not load patrol units.') }).finally(() => { if (active) setLoading(false) }); const unsubscribe = subscribeEvents((event) => { if (['unit.updated', 'alert.dispatched', 'alert.resolved', 'demo.reset'].includes(event.event)) void refresh().catch(() => undefined) }); return () => { active = false; unsubscribe() } }, [refresh, subscribeEvents])
  if (loading) return <div className="loading-card">Loading patrol units<span className="loading-pulse"/></div>
  if (error) return <UnavailableFeature title="Patrol unit data is unavailable" message={error} endpoint="GET /api/patrol-units"/>
  const operate = async (id: string, operation: () => Promise<unknown>) => { setBusy(id); setError(''); try { await operation(); await refresh() } catch (cause) { setError(cause instanceof Error ? cause.message : 'Patrol unit update failed.') } finally { setBusy(null) } }
  return <div className="workspace-stack">{error && <div className="workspace-note" role="alert">{error}</div>}<div className="unit-grid">{units.map((unit) => { const assigned = alerts.find((alert) => alert.id === unit.assignment); return <article className="panel unit-card" key={unit.id}><div className="unit-card-head"><div className="unit-icon"><Radio size={17}/></div><span className={`unit-status status-${unit.status.toLowerCase().replace(' ', '-')}`}><i/>{unit.status}</span></div><h2>{unit.id}</h2><div className="unit-location"><LocateFixed size={13}/>{unit.checkpoint}</div><div className="unit-assignment"><span>ASSIGNMENT</span><strong>{unit.assignment}</strong></div>{assigned?.route?.length ? <div className="unit-assignment"><span>DISPATCH ROUTE</span><strong>{assigned.route.join(' → ')} · {assigned.distanceKm} km</strong></div> : null}<div className="unit-updated"><Clock3 size={12}/>{unit.updated}</div>{unit.status !== 'Dispatched' && <div className="unit-controls"><label>Checkpoint<select value={deployment[unit.id] ?? unit.checkpoint} onChange={(event) => setDeployment((previous) => ({ ...previous, [unit.id]: event.target.value }))}>{checkpoints.map((checkpoint) => <option key={checkpoint}>{checkpoint}</option>)}</select></label><button className="dispatch-button" disabled={Boolean(busy) || unit.status === 'Offline'} onClick={() => void operate(unit.id, () => deployUnit(unit.id, deployment[unit.id] ?? unit.checkpoint))}>Deploy</button><label>Status<select value={unit.status} onChange={(event) => void operate(unit.id, () => changeUnitStatus(unit.id, event.target.value))}><option>Available</option><option>On Patrol</option><option>Offline</option></select></label></div>}{unit.status === 'Dispatched' && <button className="dispatch-button" disabled={Boolean(busy)} onClick={() => void operate(unit.id, () => releaseUnit(unit.id))}>{busy === unit.id ? 'Releasing…' : 'Release unit'}</button>}</article> })}</div>{units.length === 0 && <div className="workspace-note"><Radio size={15}/><span>The C++ backend returned no patrol units.</span></div>}</div>
}

function DsaPage() { return <DsaLab/> }

function SettingsPage() {
  const { status, health, refreshHealth } = useBackendStatus()
  const [compact, setCompact] = useState(false)
  const [notifications, setNotifications] = useState(true)
  const [resetting, setResetting] = useState(false)
  const [error, setError] = useState('')
  const [notice, setNotice] = useState('')
  const runReset = async () => {
    if (!window.confirm('Reset all demo records, alerts, patrol units, and lab state to the seeded demo dataset?')) return
    setResetting(true); setError(''); setNotice('')
    try { const result = await resetDemo(); await refreshHealth(); setNotice(`Demo data restored: ${result.records} records, ${result.alerts} alerts, ${result.units} patrol units.`) }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Demo data could not be reset.') }
    finally { setResetting(false) }
  }
  return <div className="settings-layout"><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">OPERATOR PROFILE</div><h2>Account</h2></div></div><div className="profile-row"><div className="avatar profile-avatar">A</div><div><strong>Admin</strong><span>Administrator · Demo account</span></div><span className="profile-chip">ACTIVE</span></div><div className="settings-divider"/><div className="setting-row"><div><strong>Compact dashboard</strong><span>Use tighter spacing on smaller displays.</span></div><button className={`toggle ${compact ? 'on' : ''}`} onClick={() => setCompact(!compact)} aria-pressed={compact}><i/></button></div><div className="setting-row"><div><strong>Operational notifications</strong><span>Show alert badges and event indicators.</span></div><button className={`toggle ${notifications ? 'on' : ''}`} onClick={() => setNotifications(!notifications)} aria-pressed={notifications}><i/></button></div></section><section className="panel settings-card"><div className="settings-heading"><div><div className="eyebrow">CONNECTIVITY</div><h2>Backend status</h2></div><span className="demo-inline">{health?.dataset === 'demo' ? 'DEMO DATASET' : health?.dataset === 'real' ? 'REAL DATASET' : 'UNKNOWN DATASET'}</span></div><div className={`backend-status backend-${status}`}><span className="backend-icon"><Wifi size={18}/></span><div><strong>{status === 'connected' ? 'Backend connected' : status === 'checking' ? 'Checking backend connection' : 'Backend disconnected'}</strong><span>{health ? `${health.recordCount ?? 0} records · ${health.alertCount ?? 0} alerts · ${health.unitCount ?? 0} units · uptime ${health.uptimeSeconds ?? 0}s` : 'Health data is unavailable.'}</span></div><i/></div><div className="setting-row endpoint-row"><div><strong>Health endpoint</strong><span>Configured API base + /health</span></div><code>{API_BASE}/health</code></div>{error && <div className="workspace-note" role="alert">{error}</div>}{notice && <div className="dispatch-feedback feedback-success" role="status">{notice}</div>}<button className="dispatch-button reset-demo-button" disabled={status !== 'connected' || resetting} onClick={() => void runReset()}>{resetting ? 'Resetting demo data…' : 'Reset Demo Data'}</button><div className="workspace-note"><Check size={14}/><span>Demo reset writes only to data/demo/intruders.txt and reseeds the shared C++ structures.</span></div></section></div>
}
