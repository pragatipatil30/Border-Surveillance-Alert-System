import { LocateFixed, Minus, Plus, Radio, ShieldAlert } from 'lucide-react'
import type { Checkpoint, Route } from '../../types/dashboard'

interface Props { checkpoints: Checkpoint[]; routes: Route[]; live?: boolean }
const patrols = [{ id: 'P-04', x: 34, y: 49 }, { id: 'P-08', x: 73, y: 51 }]

export function BorderMap({ checkpoints, routes, live = false }: Props) {
  const byId = Object.fromEntries(checkpoints.map((point) => [point.id, point]))
  return <section className="panel map-panel">
    <div className="panel-heading"><div><div className="eyebrow">FIELD OVERVIEW · {live ? 'C++ GRAPH' : 'SAMPLE GRAPH'}</div><h2>Border network</h2></div><div className="map-heading-right"><span className="demo-inline">{live ? 'BACKEND DATA' : 'DEMO GRAPH'}</span></div></div>
    <div className="map-canvas">
      <div className="map-art"><div className="moon"/><div className="mountain mountain-back"/><div className="mountain mountain-front"/><div className="mist mist-one"/><div className="mist mist-two"/><div className="map-grid"/>
      <svg className="routes" viewBox="0 0 1000 440" preserveAspectRatio="none" aria-label="Border checkpoint network map">
        <defs><filter id="routeGlow"><feGaussianBlur stdDeviation="3" result="blur"/><feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge></filter></defs>
        {routes.map((route) => { const a = byId[route.from]; const b = byId[route.to]; if (!a || !b) return null; const selected = !live && ((route.from === 'CP-04' && route.to === 'CP-06') || (route.from === 'CP-06' && route.to === 'CP-07')); const mx = (a.x + b.x) * 5; const my = (a.y + b.y) * 2.2 - 5; return <g key={`${route.from}-${route.to}`}><line x1={a.x * 10} y1={a.y * 4.4} x2={b.x * 10} y2={b.y * 4.4} className={`route-line ${selected ? 'route-active' : ''} ${route.blocked ? 'route-blocked' : ''}`} filter={selected ? 'url(#routeGlow)' : undefined}/><rect className="weight-label-bg" x={mx - 15} y={my - 9} width="30" height="16" rx="4"/><text className="route-distance" x={mx} y={my + 3}>{route.weight}</text></g> })}
      </svg>
      <div className="terrain-river"/>
      {checkpoints.map((point) => <div className={`checkpoint ${point.state}`} key={point.id} style={{ left: `${point.x}%`, top: `${point.y}%` }}><span className="node-core"/><span className="node-label">{point.id}</span>{point.state === 'alert' && <span className="node-alert"><ShieldAlert size={12}/></span>}</div>)}
      {!live && patrols.map((unit) => <div className="patrol-marker" key={unit.id} style={{ left: `${unit.x}%`, top: `${unit.y}%` }}><Radio size={12}/><span>{unit.id}</span></div>)}
      <div className="map-controls"><button aria-label="Zoom in"><Plus size={15}/></button><button aria-label="Zoom out"><Minus size={15}/></button><button aria-label="Center map"><LocateFixed size={15}/></button></div>
      </div></div>
    <div className="map-footer"><div className="map-legend"><span><i className="legend-dot normal-dot"/>Checkpoint</span>{!live && <><span><i className="legend-dot alert-dot"/>Alert</span><span><i className="legend-diamond"/>Patrol unit</span><span><i className="legend-route"/>Highlighted route</span></>}</div><div className="map-status">{checkpoints.length} CHECKPOINTS <span>·</span> {routes.length} ROUTE EDGES <span>·</span> WEIGHTS: {live ? 'C++ DATA' : 'SAMPLE'}</div></div>
  </section>
}
