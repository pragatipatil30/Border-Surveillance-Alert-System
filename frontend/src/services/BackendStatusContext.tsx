import { createContext, useContext, useEffect, useMemo, useState } from 'react'
import type { BackendStatus } from '../types/dashboard'
import { checkHealth } from './api'

interface BackendStatusValue { status: BackendStatus; lastChecked: Date | null }
const BackendStatusContext = createContext<BackendStatusValue>({ status: 'checking', lastChecked: null })

export function BackendStatusProvider({ children }: { children: React.ReactNode }) {
  const [status, setStatus] = useState<BackendStatus>('checking')
  const [lastChecked, setLastChecked] = useState<Date | null>(null)
  useEffect(() => {
    let disposed = false
    const poll = async () => {
      setStatus((current) => current === 'checking' ? 'checking' : current)
      const connected = await checkHealth()
      if (!disposed) { setStatus(connected ? 'connected' : 'disconnected'); setLastChecked(new Date()) }
    }
    void poll()
    const timer = window.setInterval(() => { setStatus('checking'); void poll() }, 15000)
    return () => { disposed = true; window.clearInterval(timer) }
  }, [])
  const value = useMemo(() => ({ status, lastChecked }), [status, lastChecked])
  return <BackendStatusContext.Provider value={value}>{children}</BackendStatusContext.Provider>
}

export function useBackendStatus() { return useContext(BackendStatusContext) }
