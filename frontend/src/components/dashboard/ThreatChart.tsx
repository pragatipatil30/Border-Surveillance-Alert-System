import { Cell, Pie, PieChart, ResponsiveContainer, Tooltip } from 'recharts'
import type { DashboardData } from '../../types/dashboard'

export function ThreatChart({ data, demo }: { data: DashboardData['threatDistribution']; demo: boolean }) {
  const records = data.reduce((sum, item) => sum + (item.count ?? 0), 0)
  const share = data.reduce((sum, item) => sum + item.value, 0)
  return <section className="panel threat-panel">
    <div className="panel-heading"><div><div className="eyebrow">{demo ? 'DEMO DATASET' : 'CURRENT RECORDS'}</div><h2>Threat distribution</h2></div></div>
    <div className="donut-chart-wrap">
      <ResponsiveContainer width="100%" height="100%">
        <PieChart>
          <Pie data={data} dataKey="value" nameKey="name" innerRadius="68%" outerRadius="88%" paddingAngle={3} stroke="none">
            {data.map((entry) => <Cell key={entry.name} fill={entry.color}/>) }
          </Pie>
          <Tooltip contentStyle={{ background: '#10232d', border: '1px solid #33454a', borderRadius: 10, color: '#f4f0e7' }} formatter={(value, name) => [`${value}%`, name]}/>
        </PieChart>
      </ResponsiveContainer>
      <div className="donut-center"><strong>{records || share}</strong><span>{records ? 'records' : 'share'}</span></div>
    </div>
    <div className="threat-legend">{data.map((item) => <div key={item.name}><span><i style={{ background: item.color }}/>{item.name}</span><strong>{item.count ?? item.value}<small>{item.count === undefined ? '%' : ' records'}</small></strong></div>)}</div>
  </section>
}
