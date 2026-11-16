export type Threat = 'HIGH' | 'MEDIUM' | 'LOW'
export interface Metric { label: string; value: string; icon: 'users' | 'shield' | 'bell' | 'radio' }
export interface Checkpoint { id: string; x: number; y: number; state: 'normal' | 'alert' | 'selected' }
export interface Route { from: string; to: string; weight: number; blocked?: boolean }
export interface Alert { id: string; title: string; checkpoint: string; time: string; threat: Threat; status?: string; intruderId?: number; threatLevel?: number; assignedUnit?: string; route?: string[]; distanceKm?: number }
export interface Activity { id: string; name: string; checkpoint: string; threat: Threat; status: string; time: string }
export interface PatrolUnit { id: string; checkpoint: string; status: 'Available' | 'On Patrol' | 'On patrol' | 'Dispatched' | 'Offline'; assignment: string; updated: string }
export interface DashboardData { metrics: Metric[]; checkpoints: Checkpoint[]; routes: Route[]; alerts: Alert[]; threatDistribution: { name: string; value: number; count?: number; color: string }[]; activities: Activity[]; patrolUnits: PatrolUnit[]; dataset?: 'demo' | 'real' | 'sample' }
export interface IntruderRecord { id: string; name: string; location: string; threatLevel: number; status?: string }
export interface DashboardSummary { totalIntruders: number; highThreatIntruders: number; threatLevelCounts: number[]; activeAlerts: number | null; availablePatrolUnits: number | null }
export interface OperationalGraph { directed: boolean; vertices: { id: number; name: string; x: number; y: number }[]; edges: { from: number; to: number; weight: number; blocked: boolean }[] }
export interface CheckpointDetails { id: string; x: number; y: number; status: string; connectedCheckpoints: { id: string; weight: number; blocked: boolean }[]; recentIncidents: Alert[]; assignedUnits: PatrolUnit[] }
export interface AlertQueues { capacity: number; frontIndex: number; rearIndex: number; incoming: { index: number; occupied: boolean; alertId?: string | null }[]; waiting: Alert[]; policy: 'PRIORITY' | 'FCFS' }
export interface ActivityEvent { id: string; category: string; message: string; timestamp: string }
export type BackendStatus = 'connected' | 'disconnected' | 'checking'
