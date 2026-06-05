import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, Button, TextField, Stack, Alert, Dialog, DialogTitle, DialogContent, DialogActions, Rating } from '@mui/material';
import { useParams, useNavigate } from 'react-router-dom';
import api from '../services/api';

const OrderDetailPage: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const navigate = useNavigate();
  const [order, setOrder] = useState<any>(null);
  const [responses, setResponses] = useState<any[]>([]);
  const [messages, setMessages] = useState<any[]>([]);
  const [reviews, setReviews] = useState<any[]>([]);
  const [newMessage, setNewMessage] = useState('');
  const [loading, setLoading] = useState(true);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [reviewDialogOpen, setReviewDialogOpen] = useState(false);
  const [sendingResponse, setSendingResponse] = useState(false);
  const [reviewForm, setReviewForm] = useState({ rating: 5, comment: '' });
  const [msg, setMsg] = useState('');
  const currentUserId = Number(localStorage.getItem('user_id') || '1');
  const currentRole = localStorage.getItem('role') || 'freelancer';
  const [responseForm, setResponseForm] = useState({ order_id: Number(id), freelancer_id: currentUserId, message: '', proposed_budget: 0, proposed_deadline: '' });

  useEffect(() => { fetchOrder(); fetchResponses(); fetchMessages(); fetchReviews(); }, [id]);

  const fetchOrder = async () => { try { const res = await api.get(`/api/orders/${id}`); setOrder(res.data.order); } catch (err) { console.error(err); } finally { setLoading(false); } };
  const fetchResponses = async () => { try { const res = await api.get(`/api/orders/${id}/responses`); setResponses(res.data.responses || []); } catch (err) { console.error(err); } };
  const fetchMessages = async () => { try { const res = await api.get(`/api/orders/${id}/messages`); setMessages(res.data.messages || []); } catch (err) { console.error(err); } };
  const fetchReviews = async () => { try { const res = await api.get(`/api/orders/${id}/reviews`); setReviews(res.data.reviews || []); } catch (err) {} };

  const handleResponse = async () => { if (!localStorage.getItem("token")) { window.location.href = "/register?msg=Для отклика необходимо зарегистрироваться"; return; } if (sendingResponse) return; setSendingResponse(true); try { await api.post('/api/responses', responseForm); setMsg('Отклик отправлен!'); setDialogOpen(false); fetchResponses(); } catch (err: any) { setMsg(err.response?.data?.error || 'Ошибка'); } setSendingResponse(false); };
  const handleAccept = async (rid: number) => { try { await api.post(`/api/responses/${rid}/accept`); setMsg('Принят!'); fetchOrder(); fetchResponses(); } catch (err: any) { setMsg('Ошибка'); } };
  const handleReject = async (rid: number) => { try { await api.post(`/api/responses/${rid}/reject`); setMsg('Отклонён'); fetchResponses(); } catch (err: any) { setMsg('Ошибка'); } };
  const sendMessage = async () => { if (!newMessage.trim()) return; try { await api.post('/api/messages', { order_id: Number(id), message: newMessage }); setNewMessage(''); fetchMessages(); } catch (err) { console.error(err); } };
  const handleComplete = async () => { try { await api.put(`/api/orders/${id}`, { status: 'completed' }); setMsg('Завершён!'); fetchOrder(); fetchReviews(); } catch (err: any) { setMsg('Ошибка'); } };
  const handleSubmitToModeration = async () => { try { await api.post(`/api/orders/${id}/submit`); setMsg('Отправлено!'); fetchOrder(); } catch (err: any) { setMsg('Ошибка'); } };
  
  const handleReview = async () => {
    let targetId = 0;
    if (currentRole === 'freelancer') {
      targetId = order?.customer_id || 0;
    } else {
      const accepted = responses.find((r: any) => r.status === 'accepted');
      targetId = accepted?.freelancer_id || 0;
    }
    if (!targetId) { setMsg('Нет получателя отзыва'); return; }
    try {
      await api.post('/api/reviews', { target_id: targetId, order_id: Number(id), rating: reviewForm.rating, comment: reviewForm.comment });
      setMsg('Отзыв оставлен!'); setReviewDialogOpen(false); fetchReviews();
    } catch (err: any) { setMsg(err.response?.data?.error || 'Ошибка'); }
  };

  const getStatusLabel = (s: string) => { const l: Record<string,string> = { draft:'Черновик', on_moderation:'На модерации', published:'Опубликован', in_progress:'В работе', completed:'Завершён' }; return l[s] || s; };

  if (loading) return <Typography>Загрузка...</Typography>;
  if (!order) return <Typography>Заказ не найден</Typography>;
  const alreadyResponded = responses.some((r: any) => r.freelancer_id === currentUserId && !['withdrawn','rejected'].includes(r.status));

  return (
    <Box>
      {msg && <Alert severity="info" sx={{ mb: 2 }} onClose={() => setMsg('')}>{msg}</Alert>}

      <Card sx={{ mb: 3 }}><CardContent>
        <Stack direction="row" justifyContent="space-between">
          <Box><Typography variant="h4">{order.title}</Typography><Chip label={getStatusLabel(order.status)} size="small" color={order.status==='published'?'success':order.status==='in_progress'?'info':'default'} sx={{ mt: 1 }} /></Box>
          <Typography variant="h4" color="primary">{Number(order.budget||0).toLocaleString()} ₽</Typography>
        </Stack>
        <Typography sx={{ mt: 3 }}>{order.description}</Typography>
        {order.required_skills && (
          <Box sx={{ mt: 1, display: 'flex', gap: 0.5, flexWrap: 'wrap' }}>
            {String(order.required_skills).replace(/[{}"]/g,'').split(',').map((s: string) => s.trim()).filter(Boolean).map((skill: string) => (
              <Chip key={skill} label={skill} size="small" variant="outlined" />
            ))}
          </Box>
        )}
        <Stack direction="row" spacing={3} sx={{ mt: 3 }}><Typography><strong>Срок:</strong> {order.deadline||'Не указан'}</Typography><Typography><strong>Заказчик:</strong> {order.customer_email}</Typography></Stack>
        <Stack direction="row" spacing={1} sx={{ mt: 3 }}>
          {order.status==='draft' && currentRole==='customer' && order.customer_id===currentUserId && <Button variant="contained" color="warning" onClick={handleSubmitToModeration}>Отправить на модерацию</Button>}
              {order.status==="published" && currentRole==="freelancer" && !alreadyResponded && localStorage.getItem("token") && (
                <Button variant="contained" onClick={async () => {
                  if (!localStorage.getItem("token")) { window.location.href = "/register?msg=Для отклика необходимо зарегистрироваться"; return; }
                  try { const res = await api.get(`/api/user/profile?user_id=${currentUserId}`); if (!res.data.profile.display_name) { setMsg("Заполните профиль перед откликом"); navigate("/profile"); return; } } catch(e) {}
                  setDialogOpen(true);
                }}>Откликнуться</Button>
              )}
          {order.status==='published' && currentRole==='freelancer' && alreadyResponded && <Chip label="Вы уже откликнулись" color="warning" />}
          {order.status==="in_progress" && localStorage.getItem("token") && currentRole==='customer' && order.customer_id===currentUserId && <Button variant="contained" color="success" onClick={handleComplete}>Завершить заказ</Button>}
          {order.status==="completed" && localStorage.getItem("token") && (currentUserId === order.customer_id || responses.some((r:any) => r.freelancer_id === currentUserId && r.status === "accepted")) && <Button variant="outlined" color="secondary" onClick={()=>{setReviewForm({rating:5,comment:""});setReviewDialogOpen(true);}}>Оставить отзыв</Button>}
        </Stack>
      </CardContent></Card>

      <Typography variant="h5" gutterBottom>Отклики ({responses.length})</Typography>
      <Grid container spacing={2} sx={{ mb: 3 }}>
        {responses.map(r=><Grid item xs={12} key={r.id}><Card><CardContent>
          <Stack direction="row" justifyContent="space-between">
            <Box><Typography variant="subtitle1">{r.display_name||r.freelancer_email}</Typography><Typography color="text.secondary">{r.message}</Typography></Box>
            <Box sx={{textAlign:'right'}}><Typography variant="h6" color="primary">{Number(r.proposed_budget||0).toLocaleString()} ₽</Typography>
              <Chip label={r.status==='accepted'?'Принят':r.status==='rejected'?'Отклонён':'Отправлен'} size="small" color={r.status==='accepted'?'success':r.status==='rejected'?'error':'warning'} />
              {r.status==='sent' && currentRole==='customer' && order.customer_id===currentUserId && <Stack direction="row" spacing={0.5} sx={{mt:1}}><Button size="small" variant="contained" color="success" onClick={()=>handleAccept(r.id)}>Принять</Button><Button size="small" variant="contained" color="error" onClick={()=>handleReject(r.id)}>Отклонить</Button></Stack>}
            </Box>
          </Stack>
        </CardContent></Card></Grid>)}
      </Grid>

      {order.status==='in_progress' && <><Typography variant="h5" gutterBottom>Сообщения</Typography><Card sx={{mb:3}}><CardContent>
        <Box sx={{maxHeight:300,overflow:'auto',mb:2}}>{messages.map(m=><Box key={m.id} sx={{mb:1,p:1,bgcolor:'grey.50',borderRadius:1}}><Typography variant="caption" color="text.secondary">{m.sender_email}</Typography><Typography>{m.message}</Typography></Box>)}</Box>
        <Stack direction="row" spacing={1}><TextField fullWidth size="small" value={newMessage} onChange={e=>setNewMessage(e.target.value)} placeholder="Сообщение..." onKeyPress={e=>e.key==='Enter'&&sendMessage()} /><Button variant="contained" onClick={sendMessage}>Отправить</Button></Stack>
      </CardContent></Card></>}

      {reviews.length>0 && <><Typography variant="h5" gutterBottom>Отзывы ({reviews.length})</Typography>
        {reviews.map(r=><Card key={r.id} sx={{mb:1}}><CardContent><Typography variant="subtitle2">{r.author} • ★ {r.rating}</Typography><Typography variant="body2">{r.comment}</Typography></CardContent></Card>)}</>}

      <Dialog open={dialogOpen} onClose={()=>setDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Отклик</DialogTitle><DialogContent>
          <TextField fullWidth label="Сообщение" value={responseForm.message} onChange={e=>setResponseForm({...responseForm,message:e.target.value})} margin="normal" multiline rows={4} />
          <TextField fullWidth label="Бюджет (₽)" type="number" value={responseForm.proposed_budget||''} onChange={e=>setResponseForm({...responseForm,proposed_budget:Number(e.target.value)})} margin="normal" />
          <TextField fullWidth label="Срок" type="date" value={responseForm.proposed_deadline} onChange={e=>setResponseForm({...responseForm,proposed_deadline:e.target.value})} margin="normal" InputLabelProps={{shrink:true}} />
        </DialogContent><DialogActions><Button onClick={()=>setDialogOpen(false)}>Отмена</Button><Button variant="contained" onClick={handleResponse} disabled={sendingResponse}>Отправить</Button></DialogActions>
      </Dialog>

      <Dialog open={reviewDialogOpen} onClose={()=>setReviewDialogOpen(false)}>
        <DialogTitle>Оставить отзыв</DialogTitle><DialogContent>
          <Rating value={reviewForm.rating} onChange={(_,v)=>setReviewForm({...reviewForm,rating:v||5})} size="large" />
          <TextField fullWidth label="Комментарий" value={reviewForm.comment} onChange={e=>setReviewForm({...reviewForm,comment:e.target.value})} margin="normal" multiline rows={3} />
        </DialogContent><DialogActions><Button onClick={()=>setReviewDialogOpen(false)}>Отмена</Button><Button variant="contained" onClick={handleReview}>Отправить</Button></DialogActions>
      </Dialog>
    </Box>
  );
};

export default OrderDetailPage;
