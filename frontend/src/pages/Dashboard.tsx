import { ArrowDownRight, ArrowUpRight, Bell, ChevronRight, CircleHelp, Clock3, Radio, Search, ShieldAlert, Users, Wifi } from 'lucide-react'
import { useEffect, useState } from 'react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ThreatChart } from '../components/dashboard/ThreatChart'
import { getDashboard } from '../services/api'
import type { DashboardData } from '../types/dashboard'

interface DashboardProps { onNavigate: (page: string) => void }

function MetricIcon({ type }: { type: DashboardData['metrics'][number]['icon'] }) { const props = { size: 18, strokeWidth: 1.7 }; if (type === 'users') return <Users {...props}/>; if (type === 'shield') return <ShieldAlert {...props}/>; if (type === 'bell') return <Bell {...props}/>; return <Radio {...props}/> }

function formatDate(date: Date) { return new Intl.DateTimeFormat('en-IN', { weekday: 'long', day: '2-digit', month: 'long', year: 'numeric' }).format(date) }

export function Dashboard({ onNavigate }: DashboardProps) {
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  const [demo, setDemo] = useState(true)
  const [now, setNow] = useState(new Date())
  useEffect(() => { getDashboard().then(({ data, demo: isDemo }) => { setDashboard(data); setDemo(isDemo) }); const timer = window.setInterval(() => setNow(new Date()), 1000); return () => window.clearInterval(timer) }, [])
  if (!dashboard) return <main className="dashboard-content"><div className="loading-card">Establishing command link<span className="loading-pulse"/></div></main>
  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb">COMMAND <ChevronRight size={13}/> OVERVIEW</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input placeholder="Search records, alerts…"/><kbd>⌘ K</kbd></div><button className="notification-button" aria-label="Notifications"><Bell size={18}/><i>3</i></button><span className="topbar-divider"/><div className="connection"><Wifi size={15}/><span>System online</span><i/></div></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">SUNDAY · NORTHERN COMMAND</div><h1>Welcome back, <em>Admin.</em></h1><p>Here’s the latest across your border operations.</p></div><div className="clock-card"><Clock3 size={15}/><div><strong>{now.toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false })}</strong><span>{formatDate(now)}</span></div></div></div>
    {demo && <div className="demo-banner"><span className="demo-pulse"/><strong>DEMO MODE</strong><span>Backend not connected</span><span className="demo-separator">·</span><span>Showing sample operational data</span><CircleHelp size={14}/></div>}
    <section className="metric-grid">{dashboard.metrics.map((metric, index) => <article className="metric-card" key={metric.label}><div className="metric-top"><span>{metric.label}</span><span className={`metric-icon icon-${index}`}><MetricIcon type={metric.icon}/></span></div><div className="metric-value">{metric.value}</div><div className="metric-foot"><span className={index === 1 ? 'trend-warn' : 'trend-up'}>{index === 1 ? <ArrowDownRight size={13}/> : <ArrowUpRight size={13}/>} {metric.change.split(' ').slice(0, index === 3 ? 2 : 1).join(' ')}</span><span>{metric.change.split(' ').slice(index === 3 ? 2 : 1).join(' ')}</span></div><div className={`metric-accent accent-${index}`}/></article>)}</section>
    <div className="overview-grid"><BorderMap checkpoints={dashboard.checkpoints} routes={dashboard.routes}/><ThreatChart data={dashboard.threatDistribution}/></div>
    <div className="dashboard-activity-link"><button className="text-link" onClick={() => onNavigate('activity')}>Open priority alerts &amp; activity log <ChevronRight size={14}/></button></div>
    <footer className="page-footer"><span>Border Command Center <i>·</i> Northern Sector</span><span>SECURE CONNECTION <i className="footer-dot"/> ENCRYPTED</span></footer>
  </main>
}
