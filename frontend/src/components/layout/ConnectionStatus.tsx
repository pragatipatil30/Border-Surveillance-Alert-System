import { Wifi } from 'lucide-react'
import { useBackendStatus } from '../../services/BackendStatusContext'

const copy = { connected: 'SYSTEM ONLINE', disconnected: 'BACKEND DISCONNECTED', checking: 'CHECKING CONNECTION' }
export function ConnectionStatus() {
  const { status } = useBackendStatus()
  return <div className={`connection connection-${status}`} role="status"><Wifi size={15}/><span>{copy[status]}</span><i/></div>
}
