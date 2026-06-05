import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Chip, Button, Stack, Alert, Dialog, DialogTitle, DialogContent, DialogActions, TextField } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const ModerationPage: React.FC = () => {
  const navigate = useNavigate();
  const userRole = localStorage.getItem('role') || '';
  const [orders, setOrders] = useState<any[]>([]);
  const [roleRequests, setRoleRequests] = useState<any[]>([]);
  const [message, setMessage] = useState('');
  const [dialogOpen, setDialogOpen] = useState(false);
  const [rejectDialogOpen, setRejectDialogOpen] = useState(false);
  const [reviseDialogOpen, setReviseDialogOpen] = useState(false);
  const [selectedOrder, setSelectedOrder] = useState<any>(null);
  const [reason, setReason] = useState('');

  useEffect(() => {
    if (userRole !== 'moderator' && userRole !== 'admin') navigate('/orders');
    else { fetchData(); fetchRoleRequests(); }
  }, []);

  if (userRole !== 'moderator' && userRole !== 'admin') return <Typography>Доступ запрещён</Typography>;

  const fetchData = async () => { try { const res = await api.get('/api/moderation/orders'); setOrders(res.data.orders || []); } catch (err) { console.error(err); } };
  const fetchRoleRequests = async () => { try { const res = await api.get('/api/role/requests'); setRoleRequests(res.data.requests || []); } catch (err) { console.error(err); } };

  const handleApprove = async (id: number) => { try { await api.post(`/api/moderation/orders/${id}/approve`, { comment: 'Одобрено' }); setMessage('Одобрено!'); fetchData(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleReject = async () => { if (!selectedOrder || !reason) return; try { await api.post(`/api/moderation/orders/${selectedOrder.id}/reject`, { reason }); setMessage('Отклонено!'); setRejectDialogOpen(false); setReason(''); fetchData(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleRevise = async () => { if (!selectedOrder || !reason) return; try { await api.post(`/api/moderation/orders/${selectedOrder.id}/revise`, { comment: reason }); setMessage('Отправлено на доработку!'); setReviseDialogOpen(false); setReason(''); fetchData(); setReviseDialogOpen(false); } catch (err: any) { setMessage('Ошибка'); } };
  
  const handleApproveRole = async (id: number) => { try { await api.post(`/api/role/requests/${id}/approve`); setMessage('Роль одобрена!'); fetchRoleRequests(); } catch (err: any) { setMessage('Ошибка'); } };
  const handleRejectRole = async (id: number) => { try { await api.post(`/api/role/requests/${id}/reject`); setMessage('Заявка отклонена'); fetchRoleRequests(); } catch (err: any) { setMessage('Ошибка'); } };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Модерация</Typography>
      {message && <Alert severity="info" sx={{ mb: 2 }} onClose={() => setMessage('')}>{message}</Alert>}

      {roleRequests.length > 0 && (
        <>
          <Typography variant="h5" gutterBottom>Заявки на смену роли ({roleRequests.length})</Typography>
          {roleRequests.map(r => (
            <Card key={r.id} sx={{ mb: 1 }}>
              <CardContent>
                <Typography variant="subtitle1">{r.email}</Typography>
                <Typography variant="body2" color="text.secondary">Хочет стать: {r.requested_role}</Typography>
                  <Typography variant="body2">Компания: {r.company_name || "не указана"}</Typography>
                  <Typography variant="body2">Описание: {r.description || "не указано"}</Typography>
                <Chip label={r.status} size="small" color={r.status==='pending'?'warning':'default'} sx={{ mt: 1 }} />
                <Stack direction="row" spacing={1} sx={{ mt: 1 }}>
                  {r.status === 'pending' && (
                    <>
                      <Button size="small" variant="contained" color="success" onClick={() => handleApproveRole(r.id)}>Одобрить</Button>
                      <Button size="small" variant="contained" color="error" onClick={() => handleRejectRole(r.id)}>Отклонить</Button>
                    </>
                  )}
                </Stack>
              </CardContent>
            </Card>
          ))}
        </>
      )}

      <Typography variant="h5" gutterBottom sx={{ mt: 2 }}>Заказы на модерации ({orders.length})</Typography>
      {orders.map(o => (
        <Card key={o.id} sx={{ mb: 2 }}>
          <CardContent>
            <Typography variant="h6">{o.title}</Typography>
            <Typography color="text.secondary">{o.description}</Typography>
            <Stack direction="row" spacing={1} sx={{ mt: 1 }}>
              <Chip label={`${o.budget?.toLocaleString()} ₽`} size="small" />
              <Chip label={o.category} size="small" variant="outlined" />
              <Chip label={o.customer_email} size="small" />
            </Stack>
            <Stack direction="row" spacing={1} sx={{ mt: 2 }}>
              <Button size="small" variant="contained" color="success" onClick={() => handleApprove(o.id)}>Одобрить</Button>
              <Button size="small" variant="contained" color="warning" onClick={() => { setSelectedOrder(o); setReason(''); setReviseDialogOpen(true); }}>На доработку</Button>
              <Button size="small" variant="contained" color="error" onClick={() => { setSelectedOrder(o); setReason(''); setRejectDialogOpen(true); }}>Отклонить</Button>
            </Stack>
          </CardContent>
        </Card>
      ))}

      {/* Диалог отклонения */}
      <Dialog open={rejectDialogOpen} onClose={() => setRejectDialogOpen(false)}>
        <DialogTitle>Отклонить заказ</DialogTitle>
        <DialogContent><TextField fullWidth label="Причина" value={reason} onChange={e => setReason(e.target.value)} margin="normal" multiline rows={3} required /></DialogContent>
        <DialogActions><Button onClick={() => setRejectDialogOpen(false)}>Отмена</Button><Button variant="contained" color="error" onClick={handleReject} disabled={!reason}>Отклонить</Button></DialogActions>
      </Dialog>

      {/* Диалог доработки */}
      <Dialog open={reviseDialogOpen} onClose={() => setReviseDialogOpen(false)}>
        <DialogTitle>Отправить на доработку</DialogTitle>
        <DialogContent><TextField fullWidth label="Что нужно доработать" value={reason} onChange={e => setReason(e.target.value)} margin="normal" multiline rows={3} required /></DialogContent>
        <DialogActions><Button onClick={() => setReviseDialogOpen(false)}>Отмена</Button><Button variant="contained" color="warning" onClick={handleRevise} disabled={!reason}>Отправить</Button></DialogActions>
      </Dialog>
    </Box>
  );
};

export default ModerationPage;
