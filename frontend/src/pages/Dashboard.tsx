import { Bell, ChevronRight, CircleHelp, Clock3, Radio, Search, ShieldAlert, Users } from 'lucide-react'
import { useEffect, useState } from 'react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ThreatChart } from '../components/dashboard/ThreatChart'
import { dispatchAlert, getDashboard } from '../services/api'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { useBackendStatus } from '../services/BackendStatusContext'
import { useClock } from '../hooks/useClock'
import type { DashboardData } from '../types/dashboard'

interface DashboardProps { onNavigate: (page: string) => void }

function MetricIcon({ type }: { type: DashboardData['metrics'][number]['icon'] }) { const props = { size: 18, strokeWidth: 1.7 }; if (type === 'users') return <Users {...props}/>; if (type === 'shield') return <ShieldAlert {...props}/>; if (type === 'bell') return <Bell {...props}/>; return <Radio {...props}/> }

function formatDate(date: Date) { return new Intl.DateTimeFormat('en-IN', { weekday: 'long', day: '2-digit', month: 'long', year: 'numeric', timeZone: 'Asia/Kolkata' }).format(date) }

export function Dashboard({ onNavigate }: DashboardProps) {
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  const [demo, setDemo] = useState(true)
  const [summaryAvailable, setSummaryAvailable] = useState(false)
  const [dispatching, setDispatching] = useState<string | null>(null)
  const [feedback, setFeedback] = useState('')
  const [alerts, setAlerts] = useState<DashboardData['alerts']>([])
  const now = useClock()
  const { status: backendStatus } = useBackendStatus()
  useEffect(() => { getDashboard().then(({ data, demo: isDemo, summaryAvailable: hasSummary }) => { setDashboard(data); setAlerts(data.alerts); setDemo(isDemo); setSummaryAvailable(hasSummary) }) }, [])
  const handleDispatch = async (alertId: string) => {
    setDispatching(alertId); setFeedback('')
    const result = await dispatchAlert(alertId)
    setDispatching(null)
    if (result.dispatched) {
      setAlerts((current) => current.map((alert) => alert.id === alertId ? { ...alert, status: 'Dispatched' } : alert))
      setFeedback(`${alertId} dispatch confirmed by backend.`)
    } else {
      setFeedback(`Demo only: no dispatch sent for ${alertId}; the C++ dispatch endpoint is unavailable.`)
    }
  }
  if (!dashboard) return <main className="dashboard-content"><div className="loading-card">Establishing command link<span className="loading-pulse"/></div></main>
  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb">COMMAND <ChevronRight size={13}/> OVERVIEW</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input placeholder="Search records, alerts…"/><kbd>⌘ K</kbd></div><button className="notification-button" aria-label="Notifications"><Bell size={18}/><i>3</i></button><span className="topbar-divider"/><ConnectionStatus/></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">{formatDate(now).split(',')[0].toUpperCase()} · COMMAND OVERVIEW</div><h1>Welcome back, <em>Admin.</em></h1><p>Here’s the latest across your border operations.</p></div><div className="clock-card"><Clock3 size={15}/><div><strong>{now.toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false, timeZone: 'Asia/Kolkata' })}</strong><span>{formatDate(now)}</span></div></div></div>
    <div className={`demo-banner backend-${backendStatus}`}><span className="demo-pulse"/><strong>{backendStatus === 'checking' ? 'CHECKING CONNECTION' : backendStatus === 'connected' ? 'SYSTEM ONLINE' : 'BACKEND DISCONNECTED'}</strong><span>{summaryAvailable ? 'Record statistics are live; map, alerts and activity remain sample data' : 'Showing development sample data'}</span><CircleHelp size={14}/></div>
    <section className="metric-grid">{dashboard.metrics.map((metric, index) => <article className="metric-card" key={metric.label}><div className="metric-top"><span>{metric.label}</span><span className={`metric-icon icon-${index}`}><MetricIcon type={metric.icon}/></span></div><div className="metric-value numeric">{metric.value}</div><div className="metric-foot"><span>{index === 0 ? summaryAvailable ? 'C++ intruder records' : 'Sample records' : index === 1 ? summaryAvailable ? 'Threat level 4–5 · C++ data' : 'Threat level 4–5 · sample' : 'Not exposed by current C++ backend'}</span></div><div className={`metric-accent accent-${index}`}/></article>)}</section>
    <div className="overview-grid"><BorderMap checkpoints={dashboard.checkpoints} routes={dashboard.routes}/><ThreatChart data={dashboard.threatDistribution} demo={!summaryAvailable}/></div>
    <div className="dashboard-data-grid"><section className="panel dashboard-alert-queue"><div className="panel-heading"><div><div className="eyebrow">RESPONSE PRIORITY · {demo ? 'DEMO ORDER' : 'BACKEND ORDER'}</div><h2>Alert queue <span className="heading-count">{String(alerts.length).padStart(2, '0')}</span></h2></div><button className="text-link" onClick={() => onNavigate('alerts')}>View all <ChevronRight size={14}/></button></div><div className="alert-list">{alerts.slice(0, 5).map((alert) => <div className="alert-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span className="numeric">{alert.id}</span><i/><span className="numeric">{alert.checkpoint}</span><i/><span>{alert.time}</span></div></div><span className={`threat-pill pill-${alert.threat.toLowerCase()}`}>{alert.threat}</span>{alert.status === 'Dispatched' ? <span className="status-text resolved"><i/>Dispatched</span> : <button className="dispatch-button" disabled={dispatching === alert.id} onClick={() => void handleDispatch(alert.id)}>{dispatching === alert.id ? 'Sending…' : 'Dispatch'}</button>}</div>)}</div>{feedback && <div className={`dispatch-feedback ${feedback.startsWith('Demo') ? 'feedback-demo' : 'feedback-success'}`} role="status">{feedback}</div>}<div className="alerts-footer"><span><i/> {demo ? 'Sample queue data' : 'Queue data from backend'}</span><span>Backend priority ordering preserved</span></div></section>
      <section className="panel dashboard-recent-activity"><div className="panel-heading"><div><div className="eyebrow">LATEST SYSTEM EVENTS · {demo ? 'DEMO FEED' : 'BACKEND FEED'}</div><h2>Recent activity</h2></div><button className="text-link" onClick={() => onNavigate('activity')}>View all activity <ChevronRight size={14}/></button></div><div className="table-wrap"><table><thead><tr><th>EVENT ID</th><th>ACTIVITY</th><th>LOCATION</th><th>TIME</th><th>STATUS</th></tr></thead><tbody>{dashboard.activities.map((item) => <tr key={item.id}><td className="numeric">{item.id}</td><td>{item.name}</td><td className="numeric">{item.checkpoint}</td><td className="numeric">{item.time}</td><td><span className={`status-text ${item.status === 'Resolved' ? 'resolved' : ''}`}><i/>{item.status}</span></td></tr>)}</tbody></table></div><div className="activity-foot"><span>{demo ? 'Sample events; no activity endpoint is connected.' : 'Events returned by the dashboard endpoint.'}</span><button onClick={() => onNavigate('activity')}>View All Activity <ChevronRight size={13}/></button></div></section></div>
    <footer className="page-footer"><span>Border Command Center <i>·</i> Demo operational view</span><span className={`footer-status footer-${backendStatus}`}><i/> {backendStatus === 'connected' ? 'BACKEND HEALTHY' : backendStatus === 'checking' ? 'CHECKING BACKEND' : 'BACKEND UNAVAILABLE'}</span></footer>
  </main>
}
