import { Sidebar } from './components/layout/Sidebar'
import { Dashboard } from './pages/Dashboard'

export default function App() { return <div className="app-shell"><div className="landscape-backdrop"><div className="backdrop-moon"/><div className="backdrop-mountain back-mountain"/><div className="backdrop-mountain fore-mountain"/><div className="pagoda"><i/><i/><i/><b/></div><div className="blossom blossom-one">✿</div><div className="blossom blossom-two">✿</div><div className="backdrop-river"/><div className="koi koi-one">◢</div><div className="koi koi-two">◣</div></div><div className="app-frame"><Sidebar/><Dashboard/></div></div> }
