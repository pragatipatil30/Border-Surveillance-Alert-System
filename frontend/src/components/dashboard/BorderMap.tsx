import { Crosshair, LocateFixed, Minus, Plus, Radio, ShieldAlert } from 'lucide-react'
import type { Checkpoint, Route } from '../../types/dashboard'

interface Props { checkpoints: Checkpoint[]; routes: Route[] }
const patrols = [{ id: 'P-04', x: 34, y: 49 }, { id: 'P-08', x: 73, y: 51 }]

export function BorderMap({ checkpoints, routes }: Props) {
  const byId = Object.fromEntries(checkpoints.map((point) => [point.id, point]))
  return <section className="panel map-panel">
    <div className="panel-heading"><div><div className="eyebrow">FIELD OVERVIEW</div><h2>Border network</h2></div><div className="map-heading-right"><span className="live-chip"><i/> LIVE</span><button className="select-button">Northern Sector <span>⌄</span></button></div></div>
    <div className="map-canvas">
      <div className="map-art"><div className="moon"/><div className="mountain mountain-back"/><div className="mountain mountain-front"/><div className="mist mist-one"/><div className="mist mist-two"/><div className="map-grid"/>
      <svg className="routes" viewBox="0 0 1000 440" preserveAspectRatio="none" aria-label="Border checkpoint network map">
        <defs><filter id="routeGlow"><feGaussianBlur stdDeviation="3" result="blur"/><feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge></filter></defs>
        {routes.map((route) => { const a = byId[route.from]; const b = byId[route.to]; if (!a || !b) return null; const selected = (route.from === 'CP-04' && route.to === 'CP-06') || (route.from === 'CP-06' && route.to === 'CP-07'); return <g key={`${route.from}-${route.to}`}><line x1={a.x * 10} y1={a.y * 4.4} x2={b.x * 10} y2={b.y * 4.4} className={`route-line ${selected ? 'route-active' : ''}`} filter={selected ? 'url(#routeGlow)' : undefined}/><text className="route-distance" x={(a.x + b.x) * 5} y={(a.y + b.y) * 2.2 - 5}>{route.weight} km</text></g> })}
      </svg>
      <div className="terrain-river"/>
      {checkpoints.map((point) => <div className={`checkpoint ${point.state}`} key={point.id} style={{ left: `${point.x}%`, top: `${point.y}%` }}><span className="node-core"/><span className="node-label">{point.id}</span>{point.state === 'alert' && <span className="node-alert"><ShieldAlert size={12}/></span>}</div>)}
      {patrols.map((unit) => <div className="patrol-marker" key={unit.id} style={{ left: `${unit.x}%`, top: `${unit.y}%` }}><Radio size={12}/><span>{unit.id}</span></div>)}
      <div className="map-controls"><button aria-label="Zoom in"><Plus size={15}/></button><button aria-label="Zoom out"><Minus size={15}/></button><button aria-label="Center map"><LocateFixed size={15}/></button></div>
      <div className="map-coordinates"><Crosshair size={12}/> 27°42′ N &nbsp; 88°36′ E</div>
      </div></div>
    <div className="map-footer"><div className="map-legend"><span><i className="legend-dot normal-dot"/>Checkpoint</span><span><i className="legend-dot alert-dot"/>Alert</span><span><i className="legend-diamond"/>Patrol unit</span><span><i className="legend-route"/>Active route</span></div><div className="map-status">10 CHECKPOINTS <span>·</span> 15 ROUTES <span>·</span> <b>2 ACTIVE EVENTS</b></div></div>
  </section>
}
