import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, TextField, Button, FormControlLabel, Alert, Chip, Switch } from '@mui/material';
import api from '../services/api';

const ProfilePage: React.FC = () => {
  const [profile, setProfile] = useState<any>(null);
  const [loading, setLoading] = useState(true);
  const [message, setMessage] = useState('');
  const userId = localStorage.getItem('user_id') || '1';
  const [form, setForm] = useState({ display_name: '', specialization: '', experience: '', skills: '', hourly_rate: 0, is_available: true, company_name: '', description: '' });
  const [rp, setRp] = useState(localStorage.getItem('rp') === '1');

  useEffect(() => {
    api.get(`/api/user/profile?user_id=${userId}`).then(res => {
      const p = res.data.profile;
      if (p.role) localStorage.setItem('role', p.role);
      setProfile(p);
      setForm({ display_name: p.display_name||'', specialization: p.specialization||'', experience: p.experience||'', skills: (p.skills||'').replace(/[{}"]/g,'').replace(/,/g,', '), hourly_rate: p.hourly_rate||0, is_available: p.is_available!==false, company_name: p.company_name||'', description: p.company_description||'' });
      setRp(localStorage.getItem('rp') === '1');
      if (p.role === 'customer' && p.company_name) { localStorage.removeItem('rp'); setRp(false); }
      setLoading(false);
    }).catch(() => setLoading(false));
  }, []);

  const handleSave = async () => {
    try {
      await api.put(`/api/user/profile?user_id=${userId}`, form);
      if (isCustomer && !rp && !profile.company_name) {
        await api.post('/api/role/request', { user_id: Number(userId), role: 'customer', company_name: form.company_name, description: form.description });
        localStorage.setItem('rp', '1'); setRp(true);
        setMessage('Заявка отправлена!');
      } else {
        setMessage('Профиль сохранён!');
      }
    } catch (err: any) { setMessage('Ошибка'); }
  };

  const cancelRequest = async () => {
    if (!confirm('Отозвать?')) return;
    try {
      const rr = await api.get('/api/role/requests');
      const myReq = (rr.data.requests||[]).find((r: any) => r.user_id === Number(userId) && r.status === 'pending');
      if (myReq) await api.post(`/api/role/requests/${myReq.id}/reject`);
    } catch(e) {}
    localStorage.removeItem('rp'); setRp(false);
    setMessage('Заявка отозвана.');
  };

  if (loading) return <Typography>Загрузка...</Typography>;
  if (!profile) return <Typography>Ошибка</Typography>;

  const role = profile.role || '';
  const isFreelancer = role === 'freelancer';
  const isCustomer = role === 'customer';
  const labels: Record<string,string> = { freelancer:'Фрилансер', customer:'Заказчик', moderator:'Модератор', admin:'Админ' };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Профиль</Typography>
      {message && <Alert severity="success" sx={{ mb: 2 }} onClose={()=>setMessage('')}>{message}</Alert>}
      {rp && <Alert severity="info" sx={{ mb: 2 }} action={<Button size="small" color="inherit" onClick={cancelRequest}>Отозвать</Button>}>Заявка на рассмотрении у модератора</Alert>}
      {isCustomer && !rp && profile.company_name && <Alert severity="success" sx={{ mb: 2 }}>Вы заказчик. Профиль заполнен.</Alert>}

      <Card sx={{ mb: 3 }}><CardContent>
        <Typography variant="h6">Роль: {labels[role]||role} {rp && <Chip label="На рассмотрении" color="warning" size="small" sx={{ ml: 1 }} />}</Typography>
      </CardContent></Card>

      <Grid container spacing={3}>
        <Grid item xs={12} md={8}><Card><CardContent>
          <Typography variant="h6" gutterBottom>Редактирование</Typography>
          {rp ? <Alert severity="info">Редактирование заблокировано. Отзовите заявку.</Alert>
          : isFreelancer ? (<>
            <TextField fullWidth label="Имя" value={form.display_name} onChange={e=>setForm({...form,display_name:e.target.value})} margin="normal" />
            <TextField fullWidth label="Специализация" value={form.specialization} onChange={e=>setForm({...form,specialization:e.target.value})} margin="normal" />
            <TextField fullWidth label="Опыт" value={form.experience} onChange={e=>setForm({...form,experience:e.target.value})} margin="normal" multiline rows={3} />
            <TextField fullWidth label="Навыки" value={form.skills} onChange={e=>setForm({...form,skills:e.target.value})} margin="normal" />
            <TextField fullWidth label="Ставка" type="number" value={form.hourly_rate} onChange={e=>setForm({...form,hourly_rate:Number(e.target.value)})} margin="normal" />
            <FormControlLabel control={<Switch checked={form.is_available} onChange={e=>setForm({...form,is_available:e.target.checked})} />} label="Доступен" sx={{mt:2}} />
            <Button variant="contained" onClick={handleSave} sx={{mt:3}}>Сохранить</Button>
          </>) : isCustomer ? (<>
            <TextField fullWidth label="Компания" value={form.company_name} onChange={e=>setForm({...form,company_name:e.target.value})} margin="normal" />
            <TextField fullWidth label="Описание" value={form.description} onChange={e=>setForm({...form,description:e.target.value})} margin="normal" multiline rows={4} />
            {!rp && !profile.company_name && <Button variant="contained" onClick={handleSave} sx={{mt:3}}>Отправить заявку</Button>}
          </>) : <Typography color="text.secondary">Для этой роли редактирование не требуется</Typography>}
        </CardContent></Card></Grid>
        <Grid item xs={12} md={4}><Card><CardContent>
          <Typography variant="h6">Инфо</Typography>
          <Typography>Email: {profile.email}</Typography>
          <Typography>Роль: {labels[role]||role}</Typography>
          <Typography>Рейтинг: {profile.rating||0} ★</Typography>
        </CardContent></Card></Grid>
      </Grid>
    </Box>
  );
};

export default ProfilePage;
