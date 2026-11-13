import { Activity, Bell, ChevronDown, Command, LayoutDashboard, LogOut, Map, Radio, Settings, Shield, Users } from 'lucide-react'

const nav = [
  { label: 'Dashboard', icon: LayoutDashboard, page: 'dashboard' },
  { label: 'Live Surveillance', icon: Map, page: 'surveillance' }, { label: 'Records', icon: Users, page: 'records' }, { label: 'Alerts', icon: Bell, count: '14', page: 'alerts' },
  { label: 'Patrol Units', icon: Radio, page: 'patrol' }, { label: 'DSA Laboratory', icon: Command, page: 'dsa' }, { label: 'Activity Log', icon: Activity, page: 'activity' },
]

interface SidebarProps { activePage: string; onNavigate: (page: string) => void }

export function Sidebar({ activePage, onNavigate }: SidebarProps) {
  return <aside className="sidebar">
    <div className="brand"><div className="brand-mark"><Shield size={20} strokeWidth={1.5} /></div><div><strong>Border Command</strong><span>COMMAND CENTER</span></div></div>
    <div className="nav-label">OPERATIONS</div>
    <nav>{nav.map(({ label, icon: Icon, page, count }) => <button className={`nav-item ${page === activePage ? 'active' : ''}`} key={label} onClick={() => page && onNavigate(page)} aria-current={page === activePage ? 'page' : undefined}><Icon size={17} strokeWidth={1.7} /><span>{label}</span>{count && <b>{count}</b>}</button>)}</nav>
    <div className="nav-label system-label">SYSTEM</div>
    <button className={`nav-item ${activePage === 'settings' ? 'active' : ''}`} onClick={() => onNavigate('settings')} aria-current={activePage === 'settings' ? 'page' : undefined}><Settings size={17} strokeWidth={1.7} /><span>Settings</span></button>
    <div className="sidebar-bottom"><div className="user-card"><div className="avatar">A</div><div className="user-meta"><strong>Admin</strong><span>Administrator</span></div><button className="icon-button" aria-label="User menu"><ChevronDown size={16}/></button></div><button className="logout"><LogOut size={15}/>Log out</button><div className="version">BORDER COMMAND <span>v1.0.0</span></div></div>
  </aside>
}
