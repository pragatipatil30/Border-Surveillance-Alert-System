import { mockDashboard } from './mockApi'
import type { ApiEnvelope, ApiFailure } from '../types/api'
import { ApiError } from '../types/api'
import type { ActivityEvent, Alert, AlertQueues, CheckpointDetails, DashboardData, IntruderRecord, OperationalGraph, PatrolUnit } from '../types/dashboard'

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

export interface HealthData { status: string; dataset?: 'demo' | 'real'; recordCount?: number; alertCount?: number; unitCount?: number; uptimeSeconds?: number; websocketUp?: boolean }
export async function getHealth(): Promise<HealthData> { return apiRequest<HealthData>('/health') }

export async function getDashboard(): Promise<{ data: DashboardData; demo: boolean; summaryAvailable: boolean }> {
  try {
    const data = await apiRequest<DashboardData>('/dashboard')
    return { data, demo: data.dataset === 'demo', summaryAvailable: true }
  } catch (error) {
    if (await checkHealth()) throw error
    return { data: { ...mockDashboard, dataset: 'sample' }, demo: false, summaryAvailable: false }
  }
}

export async function getRecordsDetailed(filters: { q?: string; location?: string; threat?: number | string; status?: string; sort?: string; algorithm?: string; rule?: string } = {}): Promise<{ records: IntruderRecord[]; count: number; sort: { algorithm: string; criteria: string; comparisons: number; passes: number } }> {
  const query = new URLSearchParams()
  Object.entries(filters).forEach(([key, value]) => { if (value !== undefined && value !== '') query.set(key, String(value)) })
  const queryString = query.toString()
  return apiRequest(`/records${queryString ? `?${queryString}` : ''}`)
}
export async function getRecords(filters: Parameters<typeof getRecordsDetailed>[0] = {}): Promise<IntruderRecord[]> { return (await getRecordsDetailed(filters)).records }

export async function getRecord(id: string): Promise<IntruderRecord> { return apiRequest<IntruderRecord>(`/records/${encodeURIComponent(id)}`) }
export async function searchRecordNames(prefix: string): Promise<IntruderRecord[]> {
  const response = await apiRequest<{ records: IntruderRecord[] }>(`/records/search?prefix=${encodeURIComponent(prefix)}`)
  return response.records
}

function post<T>(path: string, body?: unknown): Promise<T> {
  return apiRequest<T>(path, { method: 'POST', ...(body === undefined ? {} : { headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }) })
}

export const createRecord = (record: Omit<IntruderRecord, 'status'> & { status?: string }) => post<IntruderRecord>('/records', { ...record, id: Number(record.id) })
export const updateRecord = (id: string, record: Omit<IntruderRecord, 'status'> & { status?: string }) => apiRequest<IntruderRecord>(`/records/${encodeURIComponent(id)}`, { method: 'PUT', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(record) })
export const deleteRecord = (id: string) => apiRequest<unknown>(`/records/${encodeURIComponent(id)}`, { method: 'DELETE' })
export const undoRecord = () => post<{ records: number; canUndo: boolean; canRedo: boolean }>('/records/undo')
export const redoRecord = () => post<{ records: number; canUndo: boolean; canRedo: boolean }>('/records/redo')
export const getUndoState = () => apiRequest<{ canUndo: boolean; canRedo: boolean; undoCount: number; redoCount: number }>('/records/undo-state')

export async function getOperationalGraph(): Promise<OperationalGraph> {
  return apiRequest<OperationalGraph>('/graph')
}

export async function getAlerts(): Promise<Alert[]> {
  const response = await apiRequest<{ alerts: Alert[] }>('/alerts')
  return response.alerts
}

export async function getPatrolUnits(): Promise<PatrolUnit[]> {
  const response = await apiRequest<{ patrolUnits: PatrolUnit[] }>('/patrol-units')
  return response.patrolUnits
}

export async function dispatchAlert(alertId: string): Promise<Alert> { return post<Alert>(`/alerts/${encodeURIComponent(alertId)}/dispatch`) }
export async function dispatchNextAlert(): Promise<Alert> { return post<Alert>('/alerts/dispatch-next') }
export async function resolveAlert(alertId: string): Promise<Alert> { return post<Alert>(`/alerts/${encodeURIComponent(alertId)}/resolve`) }
export async function createAlert(intruderId: number): Promise<Alert> { return post<Alert>('/alerts', { intruderId }) }
export async function setAlertPolicy(policy: 'PRIORITY' | 'FCFS'): Promise<{ policy: string; waiting: Alert[] }> { return post('/alerts/policy', { policy }) }
export async function getAlertQueues(): Promise<AlertQueues> { return apiRequest<AlertQueues>('/alerts/queues') }
export async function deployUnit(id: string, checkpoint: string): Promise<PatrolUnit> { return post(`/patrol-units/${encodeURIComponent(id)}/deploy`, { checkpoint }) }
export async function releaseUnit(id: string): Promise<PatrolUnit> { return post(`/patrol-units/${encodeURIComponent(id)}/release`) }
export async function changeUnitStatus(id: string, status: string): Promise<PatrolUnit> {
  return apiRequest<PatrolUnit>(`/patrol-units/${encodeURIComponent(id)}`, { method: 'PATCH', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ status }) })
}
export async function getCheckpointDetails(id: string): Promise<CheckpointDetails> { return apiRequest<CheckpointDetails>(`/checkpoints/${encodeURIComponent(id)}`) }
export async function getActivity(category?: string): Promise<ActivityEvent[]> {
  const response = await apiRequest<{ events: ActivityEvent[] }>(`/activity${category ? `?category=${encodeURIComponent(category)}` : ''}`)
  return response.events
}
export async function blockRoute(from: string, to: string): Promise<OperationalGraph> { return post<OperationalGraph>('/graph/routes/block', { from, to }) }
export async function unblockRoute(from: string, to: string): Promise<OperationalGraph> { return post<OperationalGraph>('/graph/routes/unblock', { from, to }) }
export async function generateDemoEvent(): Promise<{ record: IntruderRecord; alert: Alert }> { return post('/demo/event') }
export async function resetDemo(): Promise<{ dataset: string; records: number; alerts: number; units: number }> { return post('/demo/reset') }
