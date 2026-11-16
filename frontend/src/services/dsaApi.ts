import { apiRequest } from './api'
import type {
  DsaGraphInput,
  DsaGraphState,
  GraphTraversalResult,
  HashMapInput,
  HashMapSearchInput,
  HashMapSearchResult,
  HashMapState,
  IntegerInput,
  IntegerResult,
  IntegerState,
  PriorityExtractResult,
  PriorityInput,
  PriorityPeekResult,
  PriorityQueueState,
  ShortestPathInput,
  ShortestPathResult,
  TraversalInput,
} from '../types/dsa'

function post<T, B = undefined>(path: string, body?: B): Promise<T> {
  return apiRequest<T>(path, {
    method: 'POST',
    ...(body === undefined ? {} : { headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }),
  })
}

export const dsaApi = {
  getStack: () => apiRequest<IntegerState>('/dsa/stack'),
  pushStack: (input: IntegerInput) => post<IntegerState, IntegerInput>('/dsa/stack/push', input),
  popStack: () => post<IntegerState & IntegerResult>('/dsa/stack/pop'),
  peekStack: () => apiRequest<IntegerResult>('/dsa/stack/peek'),

  getQueue: () => apiRequest<IntegerState>('/dsa/queue'),
  enqueue: (input: IntegerInput) => post<IntegerState, IntegerInput>('/dsa/queue/enqueue', input),
  dequeue: () => post<IntegerState & IntegerResult>('/dsa/queue/dequeue'),
  getQueueFront: () => apiRequest<IntegerResult>('/dsa/queue/front'),

  getPriorityQueue: () => apiRequest<PriorityQueueState>('/dsa/priority-queue'),
  insertPriority: (input: PriorityInput) => post<PriorityQueueState, PriorityInput>('/dsa/priority-queue/insert', input),
  extractPriority: () => post<PriorityExtractResult>('/dsa/priority-queue/extract'),
  peekPriority: () => apiRequest<PriorityPeekResult>('/dsa/priority-queue/peek'),

  getGraph: () => apiRequest<DsaGraphState>('/dsa/graph'),
  configureGraph: (input: DsaGraphInput) => post<DsaGraphState, DsaGraphInput>('/dsa/graph', input),
  bfs: (input: TraversalInput) => post<GraphTraversalResult, TraversalInput>('/dsa/graph/bfs', input),
  dfs: (input: TraversalInput) => post<GraphTraversalResult, TraversalInput>('/dsa/graph/dfs', input),
  dijkstra: (input: ShortestPathInput) => post<ShortestPathResult, ShortestPathInput>('/dsa/graph/dijkstra', input),

  getHashMap: () => apiRequest<HashMapState>('/dsa/hash-map'),
  insertHashRecord: (input: HashMapInput) => post<HashMapState, HashMapInput>('/dsa/hash-map/insert', input),
  searchHashRecord: (input: HashMapSearchInput) => post<HashMapSearchResult, HashMapSearchInput>('/dsa/hash-map/search', input),
  removeHashRecord: (input: HashMapSearchInput) => post<HashMapState, HashMapSearchInput>('/dsa/hash-map/remove', input),
}
