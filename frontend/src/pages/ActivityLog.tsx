import { Activity, ChevronRight, Clock3, Search, ShieldAlert } from 'lucide-react'
import { useCallback, useEffect, useMemo, useState } from 'react'
import { dispatchAlert, getActivity, getAlerts } from '../services/api'
import { useBackendStatus } from '../services/BackendStatusContext'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import type { ActivityEvent, Alert } from '../types/dashboard'

interface ActivityLogProps { onNavigate: (page: string) => void }
export function ActivityLog({ onNavigate }: ActivityLogProps) {
  const { subscribeEvents } = useBackendStatus()
  const [events, setEvents] = useState<ActivityEvent[]>([])
  const [alerts, setAlerts] = useState<Alert[]>([])
  const [category, setCategory] = useState('all')
  const [query, setQuery] = useState('')
  const [busy, setBusy] = useState<string | null>(null)
  const [notice, setNotice] = useState('')
  const [error, setError] = useState('')

  const refresh = useCallback(async () => {
    try {
      const [nextEvents, nextAlerts] = await Promise.all([getActivity(category === 'all' ? undefined : category), getAlerts()])
      setEvents(nextEvents); setAlerts(nextAlerts); setError('')
    } catch (cause) { setError(cause instanceof Error ? cause.message : 'Activity could not be loaded.') }
  }, [category])

  useEffect(() => { void refresh() }, [refresh])
  useEffect(() => subscribeEvents((event) => {
    if (['activity.added', 'alert.created', 'alert.dispatched', 'alert.resolved', 'record.created', 'record.updated', 'record.deleted', 'unit.updated', 'demo.reset'].includes(event.event)) void refresh()
  }), [refresh, subscribeEvents])

  const visibleEvents = useMemo(() => events.filter((item) => `${item.category} ${item.message}`.toLowerCase().includes(query.toLowerCase())), [events, query])
  const onDispatch = async (alert: Alert) => {
    setBusy(alert.id); setNotice(''); setError('')
    try { await dispatchAlert(alert.id); setNotice(`${alert.id} dispatched.`); await refresh() }
    catch (cause) { setError(cause instanceof Error ? cause.message : 'Alert dispatch failed.') }
    finally { setBusy(null) }
  }

  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb"><button className="breadcrumb-button" onClick={() => onNavigate('dashboard')}>COMMAND</button><ChevronRight size={13}/> ACTIVITY LOG</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input value={query} onChange={(event) => setQuery(event.target.value)} placeholder="Search activity…"/></div><span className="topbar-divider"/><ConnectionStatus/></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">OPERATIONS · EVENT HISTORY</div><h1>Activity <em>log.</em></h1><p>Recent events and alerts from the C++ application state.</p></div><div className="activity-summary"><span><i/> LIVE FEED</span><strong>{visibleEvents.length}<small> events shown</small></strong></div></div>
    {error && <div className="workspace-note" role="alert">{error}</div>}{notice && <div className="dispatch-feedback feedback-success" role="status">{notice}</div>}
    <div className="activity-page-grid">
      <section className="panel alerts-panel activity-page-alerts"><div className="panel-heading"><div><div className="eyebrow">NEEDS ATTENTION</div><h2>Queued alerts <span className="heading-count">{alerts.filter((alert) => alert.status === 'Queued').length}</span></h2></div><span className="live-chip"><i/> C++ QUEUE</span></div>
        <div className="alert-list">{alerts.filter((alert) => alert.status === 'Queued').slice(0, 5).map((alert) => <div className="alert-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span>{alert.id}</span><i/> <span>{alert.checkpoint}</span></div></div><span className={`threat-pill pill-${alert.threat.toLowerCase()}`}>{alert.threat}</span><button className="dispatch-button" disabled={busy === alert.id} onClick={() => void onDispatch(alert)}>{busy === alert.id ? 'Dispatching…' : 'Dispatch'}</button></div>)}</div>
        {alerts.filter((alert) => alert.status === 'Queued').length === 0 && <div className="empty-state">No queued alerts.</div>}
      </section>
      <section className="panel activity-panel activity-page-recent"><div className="panel-heading"><div><div className="eyebrow">FIELD UPDATES</div><h2>Recent activity</h2></div><label className="activity-total"><Activity size={14}/><select aria-label="Filter activity category" value={category} onChange={(event) => setCategory(event.target.value)}><option value="all">All categories</option><option value="intruder">Intruder</option><option value="alert">Alert</option><option value="dispatch">Dispatch</option><option value="resolution">Resolution</option><option value="unit">Unit</option><option value="graph">Graph</option><option value="system">System</option></select></label></div>
        <div className="table-wrap"><table><thead><tr><th>TIME</th><th>CATEGORY</th><th>EVENT</th></tr></thead><tbody>{visibleEvents.map((item) => <tr key={item.id}><td>{new Date(item.timestamp).toLocaleString('en-IN', { timeZone: 'Asia/Kolkata' })}</td><td><span className="status-text">{item.category}</span></td><td>{item.message}</td></tr>)}</tbody></table>{visibleEvents.length === 0 && !error && <div className="empty-state">No activity matches this filter.</div>}</div>
      </section>
      <section className="panel activity-note"><div className="activity-note-icon"><Clock3 size={17}/></div><div><strong>Live C++ activity log</strong><p>New operational events are refreshed from the API when their WebSocket event arrives.</p></div><span className="demo-inline">BACKEND DATA</span></section>
    </div>
    <footer className="page-footer"><span>Border Command Center <i>·</i> Operations history</span><span>DATA FROM C++ STATE</span></footer>
  </main>
}
