import React, { useEffect, useState } from 'react';
import {
  Box, Typography, Card, CardContent, Grid, Chip, Button,
  TextField, Stack, Alert, Dialog, DialogTitle, DialogContent, DialogActions, Rating
} from '@mui/material';
import { useParams } from 'react-router-dom';
import api from '../services/api';

const OrderDetailPage: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const [order, setOrder] = useState<any>(null);
  const [responses, setResponses] = useState<any[]>([]);
  const [messages, setMessages] = useState<any[]>([]);
  const [newMessage, setNewMessage] = useState('');
  const [loading, setLoading] = useState(true);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [reviewDialogOpen, setReviewDialogOpen] = useState(false);
  const [reviewForm, setReviewForm] = useState({ target_id: 0, rating: 5, comment: '' });
  const [message, setMessage] = useState('');
  
  const currentUserId = Number(localStorage.getItem('user_id') || '1');
  const currentRole = localStorage.getItem('role') || 'freelancer';

  const [responseForm, setResponseForm] = useState({
    order_id: Number(id), freelancer_id: currentUserId,
    message: '', proposed_budget: 0, proposed_deadline: '',
  });

  useEffect(() => { fetchOrder(); fetchResponses(); fetchMessages(); }, [id]);

  const fetchOrder = async () => {
    try { const res = await api.get(`/api/orders/${id}`); setOrder(res.data.order); }
    catch (err) { console.error(err); } finally { setLoading(false); }
  };

  const fetchResponses = async () => {
    try { const res = await api.get(`/api/orders/${id}/responses`); setResponses(res.data.responses || []); }
    catch (err) { console.error(err); }
  };

  const fetchMessages = async () => {
    try { const res = await api.get(`/api/orders/${id}/messages`); setMessages(res.data.messages || []); }
    catch (err) { console.error(err); }
  };

  const handleResponse = async () => {
    try {
      await api.post('/api/responses', responseForm);
      setMessage('Отклик отправлен!');
      setDialogOpen(false);
      fetchResponses();
    } catch (err: any) { setMessage(err.response?.data?.error || 'Ошибка'); }
  };

  const handleAccept = async (responseId: number) => {
    try { await api.post(`/api/responses/${responseId}/accept`); setMessage('Отклик принят!'); fetchOrder(); fetchResponses(); }
    catch (err: any) { setMessage(err.response?.data?.error || 'Ошибка'); }
  };

  const handleReject = async (responseId: number) => {
    try { await api.post(`/api/responses/${responseId}/reject`); setMessage('Отклик отклонён.'); fetchResponses(); }
    catch (err: any) { setMessage(err.response?.data?.error || 'Ошибка'); }
  };

  const sendMessage = async () => {
    if (!newMessage.trim()) return;
    try { await api.post('/api/messages', { order_id: Number(id), message: newMessage }); setNewMessage(''); fetchMessages(); }
    catch (err) { console.error(err); }
  };

  const handleComplete = async () => {
    try { await api.put(`/api/orders/${id}`, { status: 'completed' }); setMessage('Заказ завершён!'); fetchOrder(); }
    catch (err: any) { setMessage('Ошибка'); }
  };

  const handleSubmitToModeration = async () => {
    try { await api.post(`/api/orders/${id}/submit`, { comment: 'Отправлено на модерацию' }); setMessage('Заказ отправлен на модерацию!'); fetchOrder(); }
    catch (err: any) { setMessage('Ошибка'); }
  };

  const handleReview = async () => {
    try {
      await api.post('/api/reviews', { target_id: reviewForm.target_id || order?.customer_id || 1, order_id: Number(id), rating: reviewForm.rating, comment: reviewForm.comment });
      setMessage('Отзыв оставлен!'); setReviewDialogOpen(false);
    } catch (err: any) { setMessage(err.response?.data?.error || 'Ошибка'); }
  };

  const getStatusLabel = (s: string) => {
    const l: Record<string,string> = { draft:'Черновик', on_moderation:'На модерации', published:'Опубликован', in_progress:'В работе', completed:'Завершён', cancelled:'Отменён', rejected:'Отклонён' };
    return l[s] || s;
  };

  if (loading) return <Typography>Загрузка...</Typography>;
  if (!order) return <Typography>Заказ не найден</Typography>;

  const alreadyResponded = responses.some((r: any) => r.freelancer_id === currentUserId && !['withdrawn','rejected'].includes(r.status));

  return (
    <Box>
      {message && <Alert severity="info" sx={{ mb: 2 }} onClose={() => setMessage('')}>{message}</Alert>}

      <Card sx={{ mb: 3 }}>
        <CardContent>
          <Stack direction="row" justifyContent="space-between">
            <Box>
              <Typography variant="h4">{order.title}</Typography>
              <Stack direction="row" spacing={1} sx={{ mt: 1 }}>
                <Chip label={order.category || 'Без категории'} size="small" />
                <Chip label={getStatusLabel(order.status)} size="small" 
                  color={order.status==='published'?'success':order.status==='in_progress'?'info':order.status==='completed'?'default':'warning'} />
              </Stack>
            </Box>
            <Typography variant="h4" color="primary">{Number(order.budget||0).toLocaleString()} ₽</Typography>
          </Stack>
          <Typography sx={{ mt: 3 }}>{order.description}</Typography>
          <Stack direction="row" spacing={3} sx={{ mt: 3 }}>
            <Typography><strong>Срок:</strong> {order.deadline||'Не указан'}</Typography>
            <Typography><strong>Заказчик:</strong> {order.customer_email}</Typography>
          </Stack>
          {order.required_skills && (
            <Box sx={{ mt: 2 }}>
              {(order.required_skills||'').replace(/[{}"]/g,'').split(',').map((s:string) => (
                <Chip key={s.trim()} label={s.trim()} size="small" variant="outlined" sx={{ mr:0.5, mt:0.5 }} />
              ))}
            </Box>
          )}

          {/* Кнопки действий в зависимости от статуса */}
          <Stack direction="row" spacing={1} sx={{ mt: 3 }}>
            {/* Черновик → отправить на модерацию */}
            {currentRole === "admin" && (
              <Button variant="contained" color="error" onClick={async () => { if(window.confirm("Удалить заказ?")) { await api.delete(`/api/orders/${order.id}`); window.location.href="/orders"; } }}>Удалить заказ</Button>
            )}
            {order.status === "draft" && currentRole === "customer" && order.customer_id === currentUserId && (
              <Button variant="contained" color="warning" onClick={handleSubmitToModeration}>Отправить на модерацию</Button>
            )}
            {/* Опубликован → фрилансер может откликнуться */}
            {order.status === 'published' && currentRole === 'freelancer' && !alreadyResponded && (
              <Button variant="contained" onClick={() => setDialogOpen(true)}>Откликнуться</Button>
            )}
            {order.status === 'published' && currentRole === 'freelancer' && alreadyResponded && (
              <Chip label="Вы уже откликнулись" color="warning" />
            )}
            {/* В работе → завершить */}
            {order.status === "in_progress" && currentRole === "customer" && order.customer_id === currentUserId && (
              <Button variant="contained" color="success" onClick={handleComplete}>Завершить заказ</Button>
            )}
            {/* Завершён → оставить отзыв */}
            {order.status === 'completed' && (
              <Button variant="outlined" color="secondary" onClick={() => setReviewDialogOpen(true)}>Оставить отзыв</Button>
            )}
          </Stack>
        </CardContent>
      </Card>

      {/* Отклики */}
      <Typography variant="h5" gutterBottom>Отклики ({responses.length})</Typography>
      <Grid container spacing={2} sx={{ mb: 3 }}>
        {responses.map((r) => (
          <Grid item xs={12} key={r.id}>
            <Card>
              <CardContent>
                <Stack direction="row" justifyContent="space-between">
                  <Box>
                    <Typography variant="subtitle1">{r.display_name || r.freelancer_email}</Typography>
                    <Typography color="text.secondary">{r.message}</Typography>
                  </Box>
                  <Box sx={{ textAlign: 'right' }}>
                    <Typography variant="h6" color="primary">{Number(r.proposed_budget||0).toLocaleString()} ₽</Typography>
                    <Chip label={r.status==='accepted'?'Принят':r.status==='rejected'?'Отклонён':'Отправлен'} size="small"
                      color={r.status==='accepted'?'success':r.status==='rejected'?'error':'warning'} />
                    {r.status === "sent" && currentRole === "customer" && order.customer_id === currentUserId && (
                      <Stack direction="row" spacing={0.5} sx={{ mt: 1 }}>
                        <Button size="small" variant="contained" color="success" onClick={()=>handleAccept(r.id)}>Принять</Button>
                        <Button size="small" variant="contained" color="error" onClick={()=>handleReject(r.id)}>Отклонить</Button>
                      </Stack>
                    )}
                  </Box>
                </Stack>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Чат */}
      {order.status === 'in_progress' && (
        <>
          <Typography variant="h5" gutterBottom>Сообщения</Typography>
          <Card sx={{ mb: 3 }}>
            <CardContent>
              <Box sx={{ maxHeight: 300, overflow: 'auto', mb: 2 }}>
                {messages.map((m) => (
                  <Box key={m.id} sx={{ mb: 1, p: 1, bgcolor: 'grey.50', borderRadius: 1 }}>
                    <Typography variant="caption" color="text.secondary">{m.sender_email}</Typography>
                    <Typography>{m.message}</Typography>
                  </Box>
                ))}
              </Box>
              <Stack direction="row" spacing={1}>
                <TextField fullWidth size="small" value={newMessage} onChange={(e)=>setNewMessage(e.target.value)}
                  placeholder="Сообщение..." onKeyPress={(e)=>e.key==='Enter'&&sendMessage()} />
                <Button variant="contained" onClick={sendMessage}>Отправить</Button>
              </Stack>
            </CardContent>
          </Card>
        </>
      )}

      {/* Диалог отклика */}
      <Dialog open={dialogOpen} onClose={()=>setDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Отклик на заказ</DialogTitle>
        <DialogContent>
          <TextField fullWidth label="Сообщение" value={responseForm.message}
            onChange={(e)=>setResponseForm({...responseForm, message:e.target.value})} margin="normal" multiline rows={4} />
          <TextField fullWidth label="Бюджет (₽)" type="number" value={responseForm.proposed_budget}
            onChange={(e)=>setResponseForm({...responseForm, proposed_budget:Number(e.target.value)})} margin="normal" />
          <TextField fullWidth label="Срок" type="date" value={responseForm.proposed_deadline}
            onChange={(e)=>setResponseForm({...responseForm, proposed_deadline:e.target.value})} margin="normal" InputLabelProps={{shrink:true}} />
        </DialogContent>
        <DialogActions>
          <Button onClick={()=>setDialogOpen(false)}>Отмена</Button>
          <Button variant="contained" onClick={handleResponse}>Отправить</Button>
        </DialogActions>
      </Dialog>

      {/* Диалог отзыва */}
      <Dialog open={reviewDialogOpen} onClose={()=>setReviewDialogOpen(false)}>
        <DialogTitle>Оставить отзыв</DialogTitle>
        <DialogContent>
          <Rating value={reviewForm.rating} onChange={(_,v)=>setReviewForm({...reviewForm, rating:v||5})} size="large" />
          <TextField fullWidth label="Комментарий" value={reviewForm.comment}
            onChange={(e)=>setReviewForm({...reviewForm, comment:e.target.value})} margin="normal" multiline rows={3} />
        </DialogContent>
        <DialogActions>
          <Button onClick={()=>setReviewDialogOpen(false)}>Отмена</Button>
          <Button variant="contained" onClick={handleReview}>Отправить</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default OrderDetailPage;
