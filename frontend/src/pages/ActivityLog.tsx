import { Activity, Bell, ChevronRight, Clock3, Search, ShieldAlert } from 'lucide-react'
import { useEffect, useState } from 'react'
import { dispatchAlert, getDashboard } from '../services/api'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import type { DashboardData } from '../types/dashboard'

interface ActivityLogProps { dashboard: DashboardData | null; onNavigate: (page: string) => void }
export function ActivityLog({ onNavigate }: Pick<ActivityLogProps, 'onNavigate'>) {
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  const [dispatching, setDispatching] = useState<string | null>(null)
  const [feedback, setFeedback] = useState('')
  useEffect(() => { getDashboard().then(({ data }) => setDashboard(data)) }, [])
  const onDispatch = async (id: string) => {
    setDispatching(id); setFeedback('')
    const result = await dispatchAlert(id)
    setDispatching(null)
    if (result.dispatched) setDashboard((current) => current ? { ...current, alerts: current.alerts.map((alert) => alert.id === id ? { ...alert, status: 'Dispatched' } : alert) } : current)
    setFeedback(result.dispatched ? `${id} dispatch confirmed by backend.` : `Demo only: no dispatch sent for ${id}; no C++ dispatch endpoint is available.`)
  }

  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb"><button className="breadcrumb-button" onClick={() => onNavigate('dashboard')}>COMMAND</button><ChevronRight size={13}/> ACTIVITY LOG</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input placeholder="Search activity…"/></div><button className="notification-button" aria-label="Notifications"><Bell size={18}/><i>3</i></button><span className="topbar-divider"/><ConnectionStatus/></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">OPERATIONS · EVENT HISTORY</div><h1>Activity <em>log.</em></h1><p>Priority alerts and recent field activity in one place.</p></div><div className="activity-summary"><span><i/> SAMPLE FEED</span><strong>{(dashboard?.alerts.length ?? 0) + (dashboard?.activities.length ?? 0)}<small> events shown</small></strong></div></div>
    {!dashboard ? <div className="loading-card">Loading activity<span className="loading-pulse"/></div> : <div className="activity-page-grid">
      <section className="panel alerts-panel activity-page-alerts"><div className="panel-heading"><div><div className="eyebrow">NEEDS ATTENTION</div><h2>Priority alerts <span className="heading-count">{String(dashboard.alerts.length).padStart(2, '0')}</span></h2></div><span className="demo-inline">SAMPLE QUEUE</span></div><div className="alert-list">{dashboard.alerts.map((alert) => <div className="alert-row" key={alert.id}><div className={`alert-icon alert-${alert.threat.toLowerCase()}`}><ShieldAlert size={15}/></div><div className="alert-main"><div className="alert-title">{alert.title}</div><div className="alert-meta"><span>{alert.id}</span><i/> <span>{alert.checkpoint}</span><i/> <span>{alert.time}</span></div></div><span className={`threat-pill pill-${alert.threat.toLowerCase()}`}>{alert.threat}</span>{alert.status === 'Dispatched' ? <span className="status-text resolved"><i/>Dispatched</span> : <button className="dispatch-button" disabled={dispatching === alert.id} onClick={() => void onDispatch(alert.id)}>{dispatching === alert.id ? 'Sending…' : 'Dispatch'}</button>}</div>)}</div>{feedback && <div className={`dispatch-feedback ${feedback.startsWith('Demo') ? 'feedback-demo' : 'feedback-success'}`} role="status">{feedback}</div>}<div className="alerts-footer"><span><i/> Sample alert data</span><span>Backend priority order preserved when returned</span></div></section>
      <section className="panel activity-panel activity-page-recent"><div className="panel-heading"><div><div className="eyebrow">FIELD UPDATES</div><h2>Recent activity</h2></div><span className="activity-total"><Activity size={14}/>{dashboard.activities.length} records</span></div><div className="table-wrap"><table><thead><tr><th>RECORD</th><th>CHECKPOINT</th><th>LEVEL</th><th>STATUS</th><th>TIME</th></tr></thead><tbody>{dashboard.activities.map((item) => <tr key={item.id}><td><strong>{item.id}</strong><span>{item.name}</span></td><td>{item.checkpoint}</td><td><span className={`table-threat threat-${item.threat.toLowerCase()}`}><i/>{item.threat}</span></td><td><span className={`status-text ${item.status === 'Resolved' ? 'resolved' : ''}`}><i/>{item.status}</span></td><td>{item.time}</td></tr>)}</tbody></table></div></section>
      <section className="panel activity-note"><div className="activity-note-icon"><Clock3 size={17}/></div><div><strong>Timeline view</strong><p>Activity is displayed using the current dashboard feed. Full historical filtering will be available when the C++ activity API is connected.</p></div><span className="demo-inline">DEMO FEED</span></section>
    </div>}
    <footer className="page-footer"><span>Border Command Center <i>·</i> Development view</span><span>DATA SHOWN AS LABELED</span></footer>
  </main>

}
