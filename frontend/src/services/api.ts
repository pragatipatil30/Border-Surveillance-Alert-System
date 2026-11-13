import { mockDashboard } from './mockApi'
import type { DashboardData } from '../types/dashboard'

const API_BASE = import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:8080/api'

export async function checkHealth(): Promise<boolean> {
  try {
    const response = await fetch(`${API_BASE}/health`, { signal: AbortSignal.timeout(2500) })
    return response.ok
  } catch { return false }
}

export async function getDashboard(): Promise<{ data: DashboardData; demo: boolean }> {
  try {
    const response = await fetch(`${API_BASE}/dashboard`, { signal: AbortSignal.timeout(1200) })
    if (!response.ok) throw new Error(`Dashboard request failed (${response.status})`)
    return { data: await response.json() as DashboardData, demo: false }
  } catch {
    return { data: mockDashboard, demo: true }
  }
}

export async function dispatchAlert(alertId: string): Promise<{ dispatched: boolean; demo: boolean }> {
  try {
    const response = await fetch(`${API_BASE}/alerts/${encodeURIComponent(alertId)}/dispatch`, {
      method: 'POST', headers: { 'Content-Type': 'application/json' }, signal: AbortSignal.timeout(3000),
    })
    if (!response.ok) throw new Error(`Dispatch request failed (${response.status})`)
    return { dispatched: true, demo: false }
  } catch {
    return { dispatched: false, demo: true }
  }
}

export { API_BASE }
