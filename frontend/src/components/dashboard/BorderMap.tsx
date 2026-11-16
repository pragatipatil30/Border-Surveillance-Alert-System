import { useState } from 'react'
import { LocateFixed, Minus, Plus, Radio, ShieldAlert } from 'lucide-react'
import type { Checkpoint, PatrolUnit, Route } from '../../types/dashboard'

interface Props { checkpoints: Checkpoint[]; routes: Route[]; live?: boolean; patrolUnits?: PatrolUnit[]; highlightRoute?: string[]; onSelectCheckpoint?: (id: string) => void; onRouteClick?: (route: Route) => void }

export function BorderMap({ checkpoints, routes, live = false, patrolUnits = [], highlightRoute = [], onSelectCheckpoint, onRouteClick }: Props) {
  const [zoom, setZoom] = useState(1)
  const byId = Object.fromEntries(checkpoints.map((point) => [point.id, point]))
  return <section className="panel map-panel">
    <div className="panel-heading"><div><div className="eyebrow">FIELD OVERVIEW · {live ? 'C++ GRAPH' : 'SAMPLE GRAPH'}</div><h2>Border network</h2></div><div className="map-heading-right"><span className="map-live-dot"/><span className="map-live-label">{live ? 'CONNECTED' : 'SIMULATION'}</span></div></div>
    <div className="map-canvas">
      <div className="map-art" style={{ transform: `scale(${zoom})` }}><div className="moon"/><div className="mountain mountain-back"/><div className="mountain mountain-front"/><div className="mist mist-one"/><div className="mist mist-two"/><div className="map-grid"/>
      <svg className="routes" viewBox="0 0 1000 440" preserveAspectRatio="none" aria-label="Border checkpoint network map">
        <defs><filter id="routeGlow"><feGaussianBlur stdDeviation="3" result="blur"/><feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge></filter></defs>
        {routes.map((route) => { const a = byId[route.from]; const b = byId[route.to]; if (!a || !b) return null; const mx = (a.x + b.x) * 5; const my = (a.y + b.y) * 2.2 - 5; const highlighted = highlightRoute.some((node, index) => index < highlightRoute.length - 1 && ((node === route.from && highlightRoute[index + 1] === route.to) || (node === route.to && highlightRoute[index + 1] === route.from))); return <g key={`${route.from}-${route.to}`} className={onRouteClick ? 'route-interactive' : undefined} onClick={() => onRouteClick?.(route)} role={onRouteClick ? 'button' : undefined} aria-label={onRouteClick ? `${route.blocked ? 'Unblock' : 'Block'} route ${route.from} to ${route.to}` : undefined} tabIndex={onRouteClick ? 0 : undefined} onKeyDown={(event) => { if (onRouteClick && (event.key === 'Enter' || event.key === ' ')) { event.preventDefault(); onRouteClick(route) } }}><line x1={a.x * 10} y1={a.y * 4.4} x2={b.x * 10} y2={b.y * 4.4} className={`route-line ${highlighted ? 'route-active' : ''} ${route.blocked ? 'route-blocked' : ''}`}/><rect className="weight-label-bg" x={mx - 16} y={my - 10} width="32" height="18" rx="5"/><text className="route-distance" x={mx} y={my + 4}>{route.weight}</text></g> })}
      </svg>
      <div className="terrain-river"/>
      {checkpoints.map((point) => <button className={`checkpoint ${point.state}`} key={point.id} style={{ left: `${point.x}%`, top: `${point.y}%` }} onClick={() => onSelectCheckpoint?.(point.id)} aria-label={`Select checkpoint ${point.id}`}><span className="node-core"/><span className="node-label">{point.id}</span>{point.state === 'alert' && <span className="node-alert"><ShieldAlert size={12}/></span>}</button>)}
      {live && patrolUnits.filter((unit) => unit.status !== 'Offline').map((unit) => { const point = byId[unit.checkpoint]; return point ? <div className="patrol-marker" key={unit.id} style={{ left: `${Math.min(point.x + 3, 96)}%`, top: `${Math.max(point.y - 5, 5)}%` }}><Radio size={12}/><span>{unit.id}</span></div> : null })}
      <div className="map-controls"><button aria-label="Zoom in" disabled={zoom >= 1.8} onClick={() => setZoom((current) => Math.min(1.8, current + 0.15))}><Plus size={15}/></button><button aria-label="Zoom out" disabled={zoom <= 0.85} onClick={() => setZoom((current) => Math.max(0.85, current - 0.15))}><Minus size={15}/></button><button aria-label="Center map" onClick={() => setZoom(1)}><LocateFixed size={15}/></button></div>
      </div></div>
    <div className="map-footer"><div className="map-legend"><span><i className="legend-dot normal-dot"/>Checkpoint</span>{!live && <><span><i className="legend-dot alert-dot"/>Alert</span><span><i className="legend-diamond"/>Patrol unit</span><span><i className="legend-route"/>Highlighted route</span></>}</div><div className="map-status">{checkpoints.length} CHECKPOINTS <span>·</span> {routes.length} ROUTE EDGES <span>·</span> WEIGHTS: {live ? 'C++ DATA' : 'SAMPLE'}</div></div>
  </section>
}
