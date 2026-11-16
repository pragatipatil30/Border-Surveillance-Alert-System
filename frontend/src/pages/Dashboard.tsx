import { Bell, ChevronRight, CircleHelp, Clock3, Radio, Search, ShieldAlert, Users } from 'lucide-react'
import { useCallback, useEffect, useState } from 'react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ThreatChart } from '../components/dashboard/ThreatChart'
import { dispatchAlert, getDashboard } from '../services/api'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { useBackendStatus } from '../services/BackendStatusContext'
import { useClock } from '../hooks/useClock'
import type { DashboardData } from '../types/dashboard'

interface DashboardProps { onNavigate: (page: string) => void }

function formatDate(date: Date) { return new Intl.DateTimeFormat('en-IN', { weekday: 'long', day: '2-digit', month: 'long', year: 'numeric', timeZone: 'Asia/Kolkata' }).format(date) }

export function Dashboard({ onNavigate }: DashboardProps) {
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  const [demo, setDemo] = useState(false)
  const [summaryAvailable, setSummaryAvailable] = useState(false)
  const [loadError, setLoadError] = useState('')
  const [search, setSearch] = useState('')
  const [dispatching, setDispatching] = useState<string | null>(null)
  const [feedback, setFeedback] = useState('')
  const now = useClock()
  const { status: backendStatus, subscribeEvents } = useBackendStatus()
  const refresh = useCallback(async () => {
    const result = await getDashboard()
    setDashboard(result.data); setDemo(result.demo); setSummaryAvailable(result.summaryAvailable)
  }, [])
  useEffect(() => {
    let active = true
    const load = () => refresh().then(() => { if (active) setLoadError('') }).catch((error: unknown) => {
      if (active) setLoadError(error instanceof Error ? error.message : 'Dashboard data could not be loaded.')
    })
    void load()
    const unsubscribe = subscribeEvents((event) => {
      if (['record.created', 'record.updated', 'record.deleted', 'alert.created', 'alert.dispatched', 'alert.resolved', 'unit.updated', 'graph.changed', 'activity.added', 'demo.reset'].includes(event.event)) void load()
    })
    return () => { active = false; unsubscribe() }
  }, [refresh, subscribeEvents])
  if (!dashboard) return <main className="dashboard-content"><div className="loading-card">Establishing command link<span className="loading-pulse"/></div></main>
  const iconFor = (icon: string) => icon === 'users' ? <Users size={18}/> : icon === 'shield' ? <ShieldAlert size={18}/> : icon === 'bell' ? <Bell size={18}/> : <Radio size={18}/>
  const runDispatch = async (id: string) => {
    setDispatching(id); setFeedback('')
    try { await dispatchAlert(id); setFeedback(`${id} dispatched through the C++ scheduler.`); await refresh() }
    catch (error) { setFeedback(error instanceof Error ? error.message : 'Dispatch failed.') }
    finally { setDispatching(null) }
  }
  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb">COMMAND <ChevronRight size={13}/> OVERVIEW</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input value={search} onChange={(event) => setSearch(event.target.value)} onKeyDown={(event) => { if (event.key === 'Enter') { sessionStorage.setItem('border-command-record-search', search); onNavigate('records') } }} placeholder="Search records, alerts…"/><kbd>Enter</kbd></div><span className="topbar-divider"/><ConnectionStatus/></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">{formatDate(now).split(',')[0].toUpperCase()} · COMMAND OVERVIEW</div><h1>Border operations, <em>at a glance.</em></h1><p>Live view of your monitored border network.</p></div><div className="clock-card"><Clock3 size={15}/><div><strong>{now.toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false, timeZone: 'Asia/Kolkata' })}</strong><span>{formatDate(now)}</span></div></div></div>
    {loadError && <div className="workspace-note" role="alert"><CircleHelp size={14}/><span>{loadError}</span></div>}
    {backendStatus === 'disconnected' ? <div className="demo-banner backend-disconnected"><span className="demo-pulse"/><strong>BACKEND DISCONNECTED</strong><span>Showing the labelled sample fallback</span><CircleHelp size={14}/></div> : demo && <div className="demo-inline" aria-label="Demo dataset">DEMO DATASET</div>}
    <section className="metric-grid">{dashboard.metrics.map((metric, index) => <article className="metric-card" key={metric.label}><div className="metric-top"><span>{metric.label}</span><span className={`metric-icon icon-${index}`}>{iconFor(metric.icon)}</span></div><div className="metric-value">{metric.value}</div><div className="metric-foot"><span>{demo ? 'Seeded demo state' : summaryAvailable ? 'Computed from backend' : 'Sample fallback'}</span></div><div className={`metric-accent accent-${index}`}/></article>)}</section>
    <div className="overview-grid"><BorderMap checkpoints={dashboard.checkpoints} routes={dashboard.routes} patrolUnits={dashboard.patrolUnits} live={summaryAvailable}/><ThreatChart data={dashboard.threatDistribution} demo={demo || !summaryAvailable}/></div>
    <div className="dashboard-feed-grid"><section className="panel alerts-panel"><div className="panel-heading"><div><div className="eyebrow">C++ PRIORITY QUEUE</div><h2>Top alerts</h2></div><button className="text-link" onClick={() => onNavigate('alerts')}>All alerts <ChevronRight size={13}/></button></div>{dashboard.alerts.map((alert) => <div className="alert-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><strong>{alert.id}</strong><div className="alert-title">{alert.title}</div><div className="alert-meta">{alert.checkpoint} · {alert.threat}</div></div><button className="dispatch-button" disabled={!summaryAvailable || Boolean(dispatching)} onClick={() => void runDispatch(alert.id)}>{dispatching === alert.id ? 'Dispatching…' : 'Dispatch'}</button></div>)}{dashboard.alerts.length === 0 && <div className="empty-state">No queued alerts.</div>}</section>
      <section className="panel activity-panel"><div className="panel-heading"><div><div className="eyebrow">RECENT EVENTS</div><h2>Activity</h2></div><button className="text-link" onClick={() => onNavigate('activity')}>Full log <ChevronRight size={13}/></button></div><div className="dashboard-event-list">{dashboard.activities.slice(0, 5).map((event) => <div className="dashboard-event" key={event.id}><span className="event-dot"/><div><strong>{event.name}</strong><small>{event.status} · {event.time}</small></div></div>)}</div>{dashboard.activities.length === 0 && <div className="empty-state">No recent activity.</div>}</section></div>
    {feedback && <div className="dispatch-feedback" role="status">{feedback}</div>}
    <div className="dashboard-map-hint"><span>Network view</span><span>Select a section in the sidebar to explore alerts, patrols, and records <ChevronRight size={13}/></span></div>
    <footer className="page-footer"><span>Border Command Center <i>·</i> {demo ? 'Demo dataset' : summaryAvailable ? 'Live operational view' : 'Sample fallback'}</span><span className={`footer-status footer-${backendStatus}`}><i/> {backendStatus === 'connected' ? 'BACKEND HEALTHY' : backendStatus === 'checking' ? 'CHECKING BACKEND' : 'BACKEND UNAVAILABLE'}</span></footer>
  </main>
}
