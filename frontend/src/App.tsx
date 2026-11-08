import { useState } from 'react'
import { Sidebar } from './components/layout/Sidebar'
import { ActivityLog } from './pages/ActivityLog'
import { Dashboard } from './pages/Dashboard'
import { WorkspacePage } from './pages/WorkspacePages'

export default function App() {
  const [activePage, setActivePage] = useState('dashboard')
  const navigate = (page: string) => setActivePage(page)
  const workspacePages = ['surveillance', 'records', 'alerts', 'patrol', 'dsa', 'settings'] as const
  return <div className="app-shell"><div className="landscape-backdrop"><div className="backdrop-moon"/><div className="backdrop-mountain back-mountain"/><div className="backdrop-mountain fore-mountain"/><div className="pagoda"><i/><i/><i/><b/></div><div className="blossom blossom-one">✿</div><div className="blossom blossom-two">✿</div><div className="backdrop-river"/><div className="koi koi-one">◢</div><div className="koi koi-two">◣</div></div><div className="app-frame"><Sidebar activePage={activePage} onNavigate={navigate}/>{activePage === 'activity' ? <ActivityLog onNavigate={navigate}/> : activePage === 'dashboard' ? <Dashboard onNavigate={navigate}/> : workspacePages.includes(activePage as typeof workspacePages[number]) ? <WorkspacePage page={activePage as typeof workspacePages[number]} onNavigate={navigate}/> : <Dashboard onNavigate={navigate}/>}</div></div>
}
