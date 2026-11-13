import { useEffect, useState } from 'react'
import { Activity, AlertTriangle, ChevronRight, Command, Database, GitBranch, Shield } from 'lucide-react'
import { dsaApi } from '../services/dsaApi'

type Tool = 'Stack' | 'Queue' | 'Priority Queue' | 'Graph' | 'Hash Map'

const loaders: Record<Tool, () => Promise<unknown>> = {
  Stack: dsaApi.getStack,
  Queue: dsaApi.getQueue,
  'Priority Queue': dsaApi.getPriorityQueue,
  Graph: dsaApi.getGraph,
  'Hash Map': dsaApi.getHashMap,
}

const tools: { name: Tool; icon: React.ReactNode }[] = [
  { name: 'Stack', icon: <Database/> },
  { name: 'Queue', icon: <Activity/> },
  { name: 'Priority Queue', icon: <AlertTriangle/> },
  { name: 'Graph', icon: <GitBranch/> },
  { name: 'Hash Map', icon: <Command/> },
]

export function DsaLab() {
  const [selected, setSelected] = useState<Tool>('Stack')
  const [result, setResult] = useState<unknown>(null)
  const [error, setError] = useState('')
  const [busy, setBusy] = useState(false)
  const [value, setValue] = useState('')
  const [priority, setPriority] = useState('')
  const [recordId, setRecordId] = useState('')
  const [recordName, setRecordName] = useState('')
  const [location, setLocation] = useState('')
  const [threatLevel, setThreatLevel] = useState('3')
  const [start, setStart] = useState('0')
  const [source, setSource] = useState('0')
  const [target, setTarget] = useState('1')
  const [verticesJson, setVerticesJson] = useState('[]')
  const [edgesJson, setEdgesJson] = useState('[]')

  async function run(operation: () => Promise<unknown>) {
    setBusy(true)
    setError('')
    try {
      setResult(await operation())
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : 'The C++ operation failed.')
      setResult(null)
    } finally { setBusy(false) }
  }

  useEffect(() => {
    let active = true
    setError('')
    setResult(null)
    setBusy(true)
    loaders[selected]().then((data) => { if (active) setResult(data) }).catch((cause: unknown) => {
      if (active) setError(cause instanceof Error ? cause.message : 'Could not read the C++ data structure.')
    }).finally(() => { if (active) setBusy(false) })
    return () => { active = false }
  }, [selected])

  const invoke = (operation: () => Promise<unknown>) => { void run(operation) }
  const integer = (raw: string) => {
    const parsed = Number(raw)
    if (!Number.isInteger(parsed) || parsed < -2147483648 || parsed > 2147483647) throw new Error('Enter a valid 32-bit integer.')
    return parsed
  }
  const safeInvoke = (action: () => void) => {
    try { action() } catch (cause) { setError(cause instanceof Error ? cause.message : 'Check the operation inputs.') }
  }

  return <div className="dsa-layout">
    <section className="panel dsa-menu"><div className="dsa-menu-heading"><div className="eyebrow">C++ DATA STRUCTURES</div><h2>DSA Lab</h2></div>{tools.map((tool) => <button className={`dsa-option ${selected === tool.name ? 'selected' : ''}`} onClick={() => setSelected(tool.name)} key={tool.name}>{tool.icon}<span>{tool.name}<small>C++ API</small></span><ChevronRight size={15}/></button>)}</section>
    <section className="panel dsa-detail"><div className="dsa-detail-heading"><div className="dsa-feature-icon">{tools.find((tool) => tool.name === selected)?.icon}</div><div><div className="eyebrow">C++ OPERATION CONSOLE</div><h2>{selected}</h2></div><span className="implementation-badge">LIVE API</span></div>
      {selected === 'Stack' && <div className="dsa-controls"><NumberField label="Integer value" value={value} onChange={setValue}/><Action disabled={!value || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.pushStack({ value: integer(value) })))}>Push</Action><Action disabled={busy} onClick={() => invoke(dsaApi.popStack)}>Pop</Action><Action disabled={busy} onClick={() => invoke(dsaApi.peekStack)}>Peek</Action><Action disabled={busy} onClick={() => invoke(dsaApi.getStack)}>Refresh</Action></div>}
      {selected === 'Queue' && <div className="dsa-controls"><NumberField label="Integer value" value={value} onChange={setValue}/><Action disabled={!value || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.enqueue({ value: integer(value) })))}>Enqueue</Action><Action disabled={busy} onClick={() => invoke(dsaApi.dequeue)}>Dequeue</Action><Action disabled={busy} onClick={() => invoke(dsaApi.getQueueFront)}>Front</Action><Action disabled={busy} onClick={() => invoke(dsaApi.getQueue)}>Refresh</Action></div>}
      {selected === 'Priority Queue' && <div className="dsa-controls"><NumberField label="Value" value={value} onChange={setValue}/><NumberField label="Priority" value={priority} onChange={setPriority}/><Action disabled={!value || !priority || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.insertPriority({ value: integer(value), priority: integer(priority) })))}>Insert</Action><Action disabled={busy} onClick={() => invoke(dsaApi.extractPriority)}>Extract highest</Action><Action disabled={busy} onClick={() => invoke(dsaApi.peekPriority)}>Peek</Action><Action disabled={busy} onClick={() => invoke(dsaApi.getPriorityQueue)}>Refresh</Action></div>}
      {selected === 'Graph' && <div className="dsa-controls dsa-graph-controls"><label className="dsa-field">Vertices JSON<textarea value={verticesJson} onChange={(event) => setVerticesJson(event.target.value)} placeholder='[{"name":"A","x":10,"y":20}]'/></label><label className="dsa-field">Edges JSON<textarea value={edgesJson} onChange={(event) => setEdgesJson(event.target.value)} placeholder='[{"from":0,"to":1,"weight":5}]'/></label><Action disabled={busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.configureGraph({ vertices: JSON.parse(verticesJson), edges: JSON.parse(edgesJson) })))}>Configure graph</Action><NumberField label="Start vertex" value={start} onChange={setStart}/><Action disabled={busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.bfs({ start: integer(start) })))}>Run BFS</Action><Action disabled={busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.dfs({ start: integer(start) })))}>Run DFS</Action><NumberField label="Source" value={source} onChange={setSource}/><NumberField label="Target" value={target} onChange={setTarget}/><Action disabled={busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.dijkstra({ source: integer(source), target: integer(target) })))}>Dijkstra</Action></div>}
      {selected === 'Hash Map' && <div className="dsa-controls"><NumberField label="Record ID" value={recordId} onChange={setRecordId}/><TextField label="Name" value={recordName} onChange={setRecordName}/><TextField label="Location" value={location} onChange={setLocation}/><NumberField label="Threat level (1–5)" value={threatLevel} onChange={setThreatLevel}/><Action disabled={!recordId || !recordName || !location || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.insertHashRecord({ id: integer(recordId), name: recordName, location, threatLevel: integer(threatLevel) })))}>Insert</Action><Action disabled={!recordId || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.searchHashRecord({ id: integer(recordId) })))}>Search</Action><Action disabled={!recordId || busy} onClick={() => safeInvoke(() => invoke(() => dsaApi.removeHashRecord({ id: integer(recordId) })))}>Delete</Action><Action disabled={busy} onClick={() => invoke(dsaApi.getHashMap)}>Refresh</Action></div>}
      <div className="dsa-code-label">C++ RESPONSE · STATE CLEARS WHEN API RESTARTS</div>{error && <div className="workspace-note" role="alert"><AlertTriangle size={14}/><span>{error}</span></div>}{busy && result === null ? <div className="loading-card">Waiting for C++ response<span className="loading-pulse"/></div> : <pre className="code-block dsa-result"><code>{result === null ? 'No operation result yet.' : JSON.stringify(result, null, 2)}</code></pre>}
    </section>
    <div className="dsa-footnote"><Shield size={15}/><span>All operations run in C++. Stack, queue, priority queue and hash state is in memory. Graph input is supplied in this lab and processed by the existing C++ Graph algorithms.</span></div>
  </div>
}

function Action({ children, onClick, disabled }: { children: React.ReactNode; onClick: () => void; disabled?: boolean }) {
  return <button className="dispatch-button" type="button" onClick={onClick} disabled={disabled}>{children}</button>
}

function NumberField({ label, value, onChange }: { label: string; value: string; onChange: (value: string) => void }) {
  return <label className="dsa-field">{label}<input type="number" value={value} onChange={(event) => onChange(event.target.value)}/></label>
}

function TextField({ label, value, onChange }: { label: string; value: string; onChange: (value: string) => void }) {
  return <label className="dsa-field">{label}<input type="text" value={value} onChange={(event) => onChange(event.target.value)}/></label>
}
