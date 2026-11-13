import { mockDashboard } from './mockApi'
import type { ApiEnvelope, ApiFailure } from '../types/api'
import { ApiError } from '../types/api'
import type { DashboardData } from '../types/dashboard'

export const API_BASE = (import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:18080/api').replace(/\/$/, '')

export async function apiRequest<T>(path: string, init: RequestInit = {}): Promise<T> {
  let response: Response
  try {
    response = await fetch(`${API_BASE}${path.startsWith('/') ? path : `/${path}`}`, {
      ...init,
      headers: { Accept: 'application/json', ...init.headers },
      signal: init.signal ?? AbortSignal.timeout(5000),
    })
  } catch (error) {
    const message = error instanceof Error && error.name === 'TimeoutError'
      ? 'The API request timed out.'
      : 'The backend could not be reached.'
    throw new ApiError(message, 0, 'NETWORK_ERROR')
  }

  let payload: ApiEnvelope<T> | undefined
  try { payload = await response.json() as ApiEnvelope<T> } catch { /* handled below as an invalid API response */ }

  if (!response.ok) {
    const failure = payload && !payload.success ? payload as ApiFailure : undefined
    throw new ApiError(failure?.error.message ?? `API request failed (${response.status}).`, response.status, failure?.error.code)
  }
  if (!payload || payload.success !== true || !('data' in payload)) {
    const failure = payload && !payload.success ? payload as ApiFailure : undefined
    throw new ApiError(failure?.error.message ?? 'The backend returned an invalid response.', response.status, failure?.error.code ?? 'INVALID_RESPONSE')
  }
  return payload.data
}

export async function checkHealth(): Promise<boolean> {
  try {
    const data = await apiRequest<{ status: string }>('/health')
    return data.status === 'ok'
  } catch { return false }
}

export async function getDashboard(): Promise<{ data: DashboardData; demo: boolean }> {
  try {
    return { data: await apiRequest<DashboardData>('/dashboard'), demo: false }
  } catch {
    return { data: mockDashboard, demo: true }
  }
}

export async function dispatchAlert(alertId: string): Promise<{ dispatched: boolean; demo: boolean }> {
  try {
    await apiRequest<{ status: string }>(`/alerts/${encodeURIComponent(alertId)}/dispatch`, { method: 'POST' })
    return { dispatched: true, demo: false }
  } catch {
    return { dispatched: false, demo: true }
  }
}
