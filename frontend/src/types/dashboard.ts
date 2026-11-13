export type Threat = 'HIGH' | 'MEDIUM' | 'LOW'
export interface Metric { label: string; value: string; icon: 'users' | 'shield' | 'bell' | 'radio' }
export interface Checkpoint { id: string; x: number; y: number; state: 'normal' | 'alert' | 'selected' }
export interface Route { from: string; to: string; weight: number; blocked?: boolean }
export interface Alert { id: string; title: string; checkpoint: string; time: string; threat: Threat; status?: string }
export interface Activity { id: string; name: string; checkpoint: string; threat: Threat; status: string; time: string }
export interface PatrolUnit { id: string; checkpoint: string; status: 'Available' | 'On patrol' | 'Dispatched' | 'Offline'; assignment: string; updated: string }
export interface DashboardData { metrics: Metric[]; checkpoints: Checkpoint[]; routes: Route[]; alerts: Alert[]; threatDistribution: { name: string; value: number; color: string }[]; activities: Activity[]; patrolUnits: PatrolUnit[] }
export type BackendStatus = 'connected' | 'disconnected' | 'checking'
