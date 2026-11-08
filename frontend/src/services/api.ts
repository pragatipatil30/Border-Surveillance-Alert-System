import { mockDashboard } from './mockApi'
import type { DashboardData } from '../types/dashboard'

const API_BASE = import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:8080/api'

export async function getDashboard(): Promise<{ data: DashboardData; demo: boolean }> {
  try {
    const response = await fetch(`${API_BASE}/dashboard`, { signal: AbortSignal.timeout(1200) })
    if (!response.ok) throw new Error(`Dashboard request failed (${response.status})`)
    return { data: await response.json() as DashboardData, demo: false }
  } catch {
    return { data: mockDashboard, demo: true }
  }
}
