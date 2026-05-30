import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, Button, Stack, Alert, Dialog, DialogTitle, DialogContent, DialogActions, TextField } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const ModerationPage: React.FC = () => {
  const navigate = useNavigate();
  const userRole = localStorage.getItem('role') || '';
  const [orders, setOrders] = useState<any[]>([]);
  const [roleRequests, setRoleRequests] = useState<any[]>([]);
  const [message, setMessage] = useState('');
  const [dialogOpen, setDialogOpen] = useState(false);
  const [selectedOrder, setSelectedOrder] = useState<any>(null);
  const [reason, setReason] = useState('');

  useEffect(() => {
    if (userRole !== 'moderator' && userRole !== 'admin') navigate('/orders');
    else { fetchData(); fetchRoleRequests(); }
  }, []);

  if (userRole !== 'moderator' && userRole !== 'admin') {
    return <Typography>Доступ запрещён. Только для модераторов.</Typography>;
  }

  const fetchData = async () => {
    try {
      const res = await api.get('/api/moderation/orders');
      setOrders(res.data.orders || []);
    } catch (err) { console.error(err); }
  };

  const fetchRoleRequests = async () => {
    try {
      const res = await api.get('/api/role/requests');
      setRoleRequests(res.data.requests || []);
    } catch (err) { console.error(err); }
  };

  const handleApprove = async (id: number) => { try { await api.post(`/api/moderation/orders/${id}/approve`, { comment: 'Одобрено' }); setMessage('Одобрено!'); fetchData(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleReject = async () => { if (!selectedOrder || !reason) return; try { await api.post(`/api/moderation/orders/${selectedOrder.id}/reject`, { reason }); setMessage('Отклонено!'); setDialogOpen(false); setReason(''); fetchData(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleRevise = async (id: number) => { try { await api.post(`/api/moderation/orders/${id}/revise`, { comment: 'Доработка' }); setMessage('На доработку'); fetchData(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleApproveRole = async (id: number) => { try { await api.post(`/api/role/requests/${id}/approve`); setMessage('Роль одобрена!'); fetchRoleRequests(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleRejectRole = async (id: number) => { try { await api.post(`/api/role/requests/${id}/reject`); setMessage('Заявка отклонена'); fetchRoleRequests(); } catch (err: any) { setMessage('Ошибка'); } };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Модерация</Typography>
      {message && <Alert severity="info" sx={{ mb: 2 }} onClose={() => setMessage('')}>{message}</Alert>}

      {/* Заявки на смену роли */}
      {roleRequests.length > 0 && (
        <>
          <Typography variant="h5" gutterBottom>Заявки на смену роли ({roleRequests.length})</Typography>
          {roleRequests.map(r => (
            <Card key={r.id} sx={{ mb: 1 }}>
              <CardContent sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <Box>
                  <Typography variant="subtitle1">{r.email}</Typography>
                  <Chip label={`Хочет стать: ${r.requested_role}`} size="small" color="primary" />
                </Box>
                <Stack direction="row" spacing={1}>
                  <Button size="small" variant="contained" color="success" onClick={() => handleApproveRole(r.id)}>Одобрить</Button>
                  <Button size="small" variant="contained" color="error" onClick={() => handleRejectRole(r.id)}>Отклонить</Button>
                </Stack>
              </CardContent>
            </Card>
          ))}
        </>
      )}

      {/* Заказы на модерации */}
      <Typography variant="h5" gutterBottom sx={{ mt: 2 }}>Заказы ({orders.length})</Typography>
      {orders.map(o => (
        <Card key={o.id} sx={{ mb: 2 }}>
          <CardContent>
            <Typography variant="h6">{o.title}</Typography>
            <Typography color="text.secondary">{o.description?.substring(0, 200)}</Typography>
            <Stack direction="row" spacing={1} sx={{ mt: 1 }}>
              <Chip label={`${o.budget?.toLocaleString()} ₽`} size="small" />
              <Chip label={o.category} size="small" variant="outlined" />
              <Chip label={o.customer_email} size="small" />
            </Stack>
            <Stack direction="row" spacing={1} sx={{ mt: 2 }}>
              <Button size="small" variant="contained" color="success" onClick={() => handleApprove(o.id)}>Одобрить</Button>
              <Button size="small" variant="contained" color="warning" onClick={() => handleRevise(o.id)}>На доработку</Button>
              <Button size="small" variant="contained" color="error" onClick={() => { setSelectedOrder(o); setDialogOpen(true); }}>Отклонить</Button>
            </Stack>
          </CardContent>
        </Card>
      ))}

      <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)}>
        <DialogTitle>Отклонить заказ</DialogTitle>
        <DialogContent><TextField fullWidth label="Причина" value={reason} onChange={e => setReason(e.target.value)} margin="normal" multiline rows={3} required /></DialogContent>
        <DialogActions>
          <Button onClick={() => setDialogOpen(false)}>Отмена</Button>
          <Button variant="contained" color="error" onClick={handleReject} disabled={!reason}>Отклонить</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default ModerationPage;
