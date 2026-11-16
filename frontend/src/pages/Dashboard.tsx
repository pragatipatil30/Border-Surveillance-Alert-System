import { ChevronRight, CircleHelp, Clock3, Search } from 'lucide-react'
import { useEffect, useState } from 'react'
import { BorderMap } from '../components/dashboard/BorderMap'
import { ThreatChart } from '../components/dashboard/ThreatChart'
import { getDashboard } from '../services/api'
import { ConnectionStatus } from '../components/layout/ConnectionStatus'
import { useBackendStatus } from '../services/BackendStatusContext'
import { useClock } from '../hooks/useClock'
import type { DashboardData } from '../types/dashboard'

interface DashboardProps { onNavigate: (page: string) => void }

function formatDate(date: Date) { return new Intl.DateTimeFormat('en-IN', { weekday: 'long', day: '2-digit', month: 'long', year: 'numeric', timeZone: 'Asia/Kolkata' }).format(date) }

export function Dashboard({ onNavigate }: DashboardProps) {
  const [dashboard, setDashboard] = useState<DashboardData | null>(null)
  const [demo, setDemo] = useState(true)
  const [summaryAvailable, setSummaryAvailable] = useState(false)
  const now = useClock()
  const { status: backendStatus } = useBackendStatus()
  useEffect(() => { getDashboard().then(({ data, demo: isDemo, summaryAvailable: hasSummary }) => { setDashboard(data); setDemo(isDemo); setSummaryAvailable(hasSummary) }) }, [])
  if (!dashboard) return <main className="dashboard-content"><div className="loading-card">Establishing command link<span className="loading-pulse"/></div></main>
  return <main className="dashboard-content">
    <header className="topbar"><div className="breadcrumb">COMMAND <ChevronRight size={13}/> OVERVIEW</div><div className="topbar-actions"><div className="searchbox"><Search size={15}/><input placeholder="Search records, alerts…"/><kbd>⌘ K</kbd></div><span className="topbar-divider"/><ConnectionStatus/></div></header>
    <div className="welcome-row"><div><div className="eyebrow welcome-kicker">{formatDate(now).split(',')[0].toUpperCase()} · COMMAND OVERVIEW</div><h1>Border operations, <em>at a glance.</em></h1><p>Live view of your monitored border network.</p></div><div className="clock-card"><Clock3 size={15}/><div><strong>{now.toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false, timeZone: 'Asia/Kolkata' })}</strong><span>{formatDate(now)}</span></div></div></div>
    <div className={`demo-banner backend-${backendStatus}`}><span className="demo-pulse"/><strong>{backendStatus === 'checking' ? 'CHECKING CONNECTION' : backendStatus === 'connected' ? 'SYSTEM ONLINE' : 'BACKEND DISCONNECTED'}</strong><span>{summaryAvailable ? 'Record statistics are live; map, alerts and activity remain sample data' : 'Showing development sample data'}</span><CircleHelp size={14}/></div>
    <div className="overview-grid"><BorderMap checkpoints={dashboard.checkpoints} routes={dashboard.routes}/><ThreatChart data={dashboard.threatDistribution} demo={!summaryAvailable}/></div>
    <div className="dashboard-map-hint"><span>Network view</span><span>Select a section in the sidebar to explore alerts, patrols, and records <ChevronRight size={13}/></span></div>
    <footer className="page-footer"><span>Border Command Center <i>·</i> {demo ? 'Demo operational view' : 'Operational view'}</span><span className={`footer-status footer-${backendStatus}`}><i/> {backendStatus === 'connected' ? 'BACKEND HEALTHY' : backendStatus === 'checking' ? 'CHECKING BACKEND' : 'BACKEND UNAVAILABLE'}</span></footer>
  </main>
}
