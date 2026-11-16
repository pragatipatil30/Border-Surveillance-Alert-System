import { createContext, useCallback, useContext, useEffect, useMemo, useRef, useState } from 'react'
import type { BackendStatus } from '../types/dashboard'
import { API_BASE, getHealth, type HealthData } from './api'

export interface RealtimeEvent<T = unknown> {
  event: string
  timestamp: string
  data: T
}

type RealtimeStatus = 'checking' | 'connected' | 'disconnected'
interface BackendStatusValue {
  status: BackendStatus
  realtimeStatus: RealtimeStatus
  lastChecked: Date | null
  health: HealthData | null
  refreshHealth: () => Promise<void>
  subscribeEvents: (listener: (event: RealtimeEvent) => void) => () => void
}

const BackendStatusContext = createContext<BackendStatusValue>({
  status: 'checking', realtimeStatus: 'checking', lastChecked: null, health: null, refreshHealth: async () => undefined, subscribeEvents: () => () => undefined,
})

export function BackendStatusProvider({ children }: { children: React.ReactNode }) {
  const [status, setStatus] = useState<BackendStatus>('checking')
  const [realtimeStatus, setRealtimeStatus] = useState<RealtimeStatus>('checking')
  const [lastChecked, setLastChecked] = useState<Date | null>(null)
  const [health, setHealth] = useState<HealthData | null>(null)
  const listeners = useRef(new Set<(event: RealtimeEvent) => void>())

  const subscribeEvents = useCallback((listener: (event: RealtimeEvent) => void) => {
    listeners.current.add(listener)
    return () => { listeners.current.delete(listener) }
  }, [])

  const refreshHealth = useCallback(async () => {
    try {
      const current = await getHealth()
      setHealth(current); setStatus(current.status === 'ok' ? 'connected' : 'disconnected'); setLastChecked(new Date())
    } catch { setHealth(null); setStatus('disconnected'); setLastChecked(new Date()) }
  }, [])

  useEffect(() => {
    let disposed = false
    const poll = () => { if (!disposed) void refreshHealth() }
    poll()
    const timer = window.setInterval(poll, 15000)
    return () => { disposed = true; window.clearInterval(timer) }
  }, [refreshHealth])

  useEffect(() => {
    let disposed = false
    let socket: WebSocket | null = null
    let retryTimer = 0
    let heartbeatTimer = 0
    let heartbeatTimeout = 0
    let retryAttempt = 0
    let lastPing = ''

    const clearHeartbeat = () => {
      window.clearInterval(heartbeatTimer)
      window.clearTimeout(heartbeatTimeout)
    }
    const scheduleReconnect = () => {
      if (disposed || retryTimer !== 0) return
      setRealtimeStatus('disconnected')
      const base = Math.min(1000 * (2 ** retryAttempt), 30000)
      retryAttempt = Math.min(retryAttempt + 1, 6)
      retryTimer = window.setTimeout(() => { retryTimer = 0; connect() }, base * (0.8 + Math.random() * 0.4))
    }
    const connect = () => {
      if (disposed) return
      const base = new URL(API_BASE, window.location.href)
      base.protocol = base.protocol === 'https:' ? 'wss:' : 'ws:'
      base.pathname = '/ws/events'
      base.search = ''
      base.hash = ''
      try { socket = new WebSocket(base) } catch { scheduleReconnect(); return }
      socket.onopen = () => {
        if (disposed) { socket?.close(); return }
        retryAttempt = 0
        setRealtimeStatus('connected')
        void refreshHealth()
        heartbeatTimer = window.setInterval(() => {
          if (socket?.readyState !== WebSocket.OPEN) return
          lastPing = new Date().toISOString()
          socket.send(JSON.stringify({ type: 'ping', timestamp: lastPing }))
          heartbeatTimeout = window.setTimeout(() => socket?.close(4000, 'Heartbeat timeout'), 10000)
        }, 25000)
      }
      socket.onmessage = ({ data }) => {
        try {
          const event = JSON.parse(String(data)) as RealtimeEvent<{ clientTimestamp?: string }>
          if (!event || typeof event.event !== 'string' || typeof event.timestamp !== 'string') return
          if (event.event === 'system.pong' && event.data?.clientTimestamp === lastPing) {
            window.clearTimeout(heartbeatTimeout)
            return
          }
          listeners.current.forEach((listener) => listener(event))
        } catch { /* Ignore malformed event frames and keep the connection alive. */ }
      }
      socket.onclose = () => { clearHeartbeat(); scheduleReconnect() }
      socket.onerror = () => socket?.close()
    }

    connect()
    return () => {
      disposed = true
      window.clearTimeout(retryTimer)
      clearHeartbeat()
      socket?.close(1000, 'Provider unmounted')
    }
  }, [refreshHealth])

  const value = useMemo(() => ({ status, realtimeStatus, lastChecked, health, refreshHealth, subscribeEvents }), [status, realtimeStatus, lastChecked, health, refreshHealth, subscribeEvents])
  return <BackendStatusContext.Provider value={value}>{children}</BackendStatusContext.Provider>
}

export function useBackendStatus() { return useContext(BackendStatusContext) }
