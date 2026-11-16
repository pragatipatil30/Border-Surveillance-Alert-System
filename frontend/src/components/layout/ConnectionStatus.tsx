import { Wifi } from 'lucide-react'
import { useBackendStatus } from '../../services/BackendStatusContext'

const copy = { connected: 'SYSTEM ONLINE', disconnected: 'BACKEND DISCONNECTED', checking: 'CHECKING CONNECTION' }
export function ConnectionStatus() {
  const { status, realtimeStatus } = useBackendStatus()
  return <div className={`connection connection-${status}`} role="status" title={`API: ${status}; live events: ${realtimeStatus}`}><Wifi size={15}/><span>{copy[status]} · {realtimeStatus === 'connected' ? 'LIVE' : realtimeStatus === 'checking' ? 'CONNECTING' : 'RECONNECTING'}</span><i/></div>
}
