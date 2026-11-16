import type { DashboardData } from '../types/dashboard'

export const mockDashboard: DashboardData = {
  metrics: [
    { label: 'Total Intruders', value: '128', icon: 'users' },
    { label: 'High Threat', value: '08', icon: 'shield' },
    { label: 'Active Alerts', value: '14', icon: 'bell' },
    { label: 'Available Units', value: '06', icon: 'radio' },
  ],
  checkpoints: [
    { id: 'CP-01', x: 9, y: 55, state: 'normal' }, { id: 'CP-02', x: 20, y: 34, state: 'normal' },
    { id: 'CP-03', x: 29, y: 66, state: 'alert' }, { id: 'CP-04', x: 39, y: 43, state: 'normal' },
    { id: 'CP-05', x: 49, y: 24, state: 'normal' }, { id: 'CP-06', x: 57, y: 61, state: 'selected' },
    { id: 'CP-07', x: 69, y: 37, state: 'normal' }, { id: 'CP-08', x: 78, y: 67, state: 'normal' },
    { id: 'CP-09', x: 87, y: 29, state: 'alert' }, { id: 'CP-10', x: 94, y: 53, state: 'normal' },
  ],
  routes: [
    { from: 'CP-01', to: 'CP-02', weight: 12 }, { from: 'CP-01', to: 'CP-03', weight: 9 }, { from: 'CP-02', to: 'CP-03', weight: 7 },
    { from: 'CP-02', to: 'CP-04', weight: 11 }, { from: 'CP-03', to: 'CP-04', weight: 8 }, { from: 'CP-03', to: 'CP-06', weight: 13 },
    { from: 'CP-04', to: 'CP-05', weight: 10 }, { from: 'CP-04', to: 'CP-06', weight: 6 }, { from: 'CP-05', to: 'CP-07', weight: 12 },
    { from: 'CP-06', to: 'CP-07', weight: 7 }, { from: 'CP-06', to: 'CP-08', weight: 14 }, { from: 'CP-07', to: 'CP-09', weight: 9 },
    { from: 'CP-08', to: 'CP-09', weight: 8 }, { from: 'CP-08', to: 'CP-10', weight: 11 }, { from: 'CP-09', to: 'CP-10', weight: 6 },
  ],
  alerts: [
    { id: 'ALT-0248', title: 'Unidentified movement detected', checkpoint: 'CP-09', time: '2 min ago', threat: 'HIGH', status: 'Pending' },
    { id: 'ALT-0247', title: 'Thermal signature — multiple subjects', checkpoint: 'CP-03', time: '8 min ago', threat: 'HIGH', status: 'Pending' },
    { id: 'ALT-0246', title: 'Perimeter sensor triggered', checkpoint: 'CP-06', time: '16 min ago', threat: 'MEDIUM', status: 'Pending' },
    { id: 'ALT-0244', title: 'Unscheduled signal detected', checkpoint: 'CP-07', time: '41 min ago', threat: 'MEDIUM', status: 'Pending' },
    { id: 'ALT-0245', title: 'Camera feed restored', checkpoint: 'CP-02', time: '24 min ago', threat: 'LOW', status: 'Pending' },
  ],
  threatDistribution: [{ name: 'Low', value: 42, color: '#67B98D' }, { name: 'Medium', value: 31, color: '#E9B85F' }, { name: 'High', value: 27, color: '#DB716B' }],
  activities: [
    { id: 'IN-1048', name: 'Unidentified subject', checkpoint: 'CP-09', threat: 'HIGH', status: 'Under review', time: '09:42:18' },
    { id: 'IN-1047', name: 'Thermal movement', checkpoint: 'CP-03', threat: 'HIGH', status: 'Unit dispatched', time: '09:36:02' },
    { id: 'IN-1046', name: 'Perimeter breach signal', checkpoint: 'CP-06', threat: 'MEDIUM', status: 'Monitoring', time: '09:28:45' },
    { id: 'IN-1045', name: 'Motion sensor alert', checkpoint: 'CP-02', threat: 'LOW', status: 'Resolved', time: '09:14:31' },
  ],
  patrolUnits: [
    { id: 'PATROL-04', checkpoint: 'CP-04', status: 'Available', assignment: 'Standby', updated: '1 min ago' },
    { id: 'PATROL-08', checkpoint: 'CP-08', status: 'On patrol', assignment: 'Route patrol', updated: '3 min ago' },
    { id: 'PATROL-02', checkpoint: 'CP-03', status: 'Available', assignment: 'Standby', updated: '8 min ago' },
    { id: 'PATROL-11', checkpoint: 'Offline', status: 'Offline', assignment: 'Signal maintenance', updated: '26 min ago' },
  ],
}
