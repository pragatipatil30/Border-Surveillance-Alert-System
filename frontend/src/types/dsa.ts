import type { IntruderRecord } from './dashboard'

export interface IntegerInput { value: number }
export interface IntegerState { items: number[]; size: number }
export interface IntegerResult { value: number }
export interface PriorityInput { value: number; priority: number }
export interface PriorityItem { value: number; priority: number; sequence: number }
export interface PriorityQueueState { heap: PriorityItem[]; size: number; top: PriorityItem | null }
export interface PriorityExtractResult extends PriorityQueueState { extracted: PriorityItem }
export interface PriorityPeekResult { item: PriorityItem }
export interface DsaGraphVertex { id: number; name: string; x: number; y: number }
export interface DsaGraphEdge { from: number; to: number; weight: number; blocked: boolean }
export interface DsaGraphState { configured: boolean; directed: boolean; vertices: DsaGraphVertex[]; edges: DsaGraphEdge[] }
export interface DsaGraphInput { vertices: Omit<DsaGraphVertex, 'id'>[]; edges: Omit<DsaGraphEdge, 'blocked'>[] }
export interface TraversalInput { start: number }
export interface GraphTraversalResult { algorithm: 'BFS' | 'DFS'; order: number[]; names: string[] }
export interface ShortestPathInput { source: number; target: number }
export interface ShortestPathResult { found: boolean; distance: number | null; path: number[]; names: string[] }
export interface HashMapState { size: number; capacity: number; buckets: HashMapInput[][] }
export type HashMapInput = Omit<IntruderRecord, 'id'> & { id: number }
export interface HashMapSearchInput { id: number }
export interface HashMapSearchResult { record: HashMapInput }
