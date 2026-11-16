import { Area, AreaChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts'
import type { DashboardData } from '../../types/dashboard'

const trend = [
  { time: '00', low: 15, medium: 8, high: 3 }, { time: '04', low: 20, medium: 11, high: 5 },
  { time: '08', low: 18, medium: 16, high: 8 }, { time: '12', low: 30, medium: 13, high: 6 },
  { time: '16', low: 26, medium: 22, high: 11 }, { time: '20', low: 40, medium: 19, high: 9 },
  { time: '24', low: 35, medium: 28, high: 14 }, { time: '28', low: 48, medium: 25, high: 12 },
  { time: '32', low: 42, medium: 34, high: 18 }, { time: '36', low: 58, medium: 31, high: 15 },
  { time: '40', low: 51, medium: 40, high: 23 }, { time: '44', low: 68, medium: 37, high: 19 },
]

export function ThreatChart({ data, demo }: { data: DashboardData['threatDistribution']; demo: boolean }) {
  const total = data.reduce((sum, item) => sum + item.value, 0)
  return <section className="panel threat-panel">
    <div className="panel-heading"><div><div className="eyebrow">{demo ? 'SAMPLE SIGNAL · LAST 24 HOURS' : 'CURRENT SIGNAL · LAST 24 HOURS'}</div><h2>Threat activity</h2></div><span className="trend-total"><strong>{total}</strong><small> index</small></span></div>
    <div className="trend-chart-wrap"><ResponsiveContainer width="100%" height="100%"><AreaChart data={trend} margin={{ top: 10, right: 7, left: -22, bottom: 0 }}>
      <defs><linearGradient id="threatLow" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stopColor="#61c9d0" stopOpacity=".78"/><stop offset="100%" stopColor="#61c9d0" stopOpacity=".04"/></linearGradient><linearGradient id="threatMedium" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stopColor="#f1c36d" stopOpacity=".8"/><stop offset="100%" stopColor="#f1c36d" stopOpacity=".04"/></linearGradient><linearGradient id="threatHigh" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stopColor="#e77d86" stopOpacity=".88"/><stop offset="100%" stopColor="#e77d86" stopOpacity=".07"/></linearGradient></defs>
      <CartesianGrid stroke="#a8b1df15" vertical={false}/><XAxis dataKey="time" axisLine={false} tickLine={false} tick={{ fill: '#7883a3', fontSize: 8 }}/><YAxis axisLine={false} tickLine={false} tick={{ fill: '#7883a3', fontSize: 8 }}/><Tooltip contentStyle={{ background: '#171b35', border: '1px solid #68719a55', borderRadius: 7, color: '#edf0ff', fontSize: 10 }} labelStyle={{ color: '#aeb5d2' }}/>
      <Area type="monotone" dataKey="low" name="Low" stackId="1" stroke="#61c9d0" strokeWidth={1.5} fill="url(#threatLow)"/><Area type="monotone" dataKey="medium" name="Medium" stackId="1" stroke="#f1c36d" strokeWidth={1.5} fill="url(#threatMedium)"/><Area type="monotone" dataKey="high" name="High" stackId="1" stroke="#e77d86" strokeWidth={1.5} fill="url(#threatHigh)"/>
    </AreaChart></ResponsiveContainer></div>
    <div className="threat-legend">{data.map((item, index) => <div key={item.name}><span><i className={`trend-swatch swatch-${index}`}/>{item.name}</span><strong>{item.value}<small>%</small></strong></div>)}</div>
  </section>
}
