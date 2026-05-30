import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, TextField, Button, Switch, FormControlLabel, Alert, Stack, Dialog, DialogTitle, DialogContent, DialogActions } from '@mui/material';
import api from '../services/api';

const ProfilePage: React.FC = () => {
  const [profile, setProfile] = useState<any>({});
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [message, setMessage] = useState('');
  const [dialogOpen, setDialogOpen] = useState(false);
  const [requestForm, setRequestForm] = useState({ company_name: '', description: '', contact: '' });
  const userId = localStorage.getItem('user_id') || '1';
  const currentRole = localStorage.getItem('role') || '';
  const [form, setForm] = useState({ display_name: '', specialization: '', experience: '', skills: '', hourly_rate: 0, is_available: true, company_name: '', description: '' });

  useEffect(() => { fetchProfile(); }, []);

  const fetchProfile = async () => {
    try {
      const res = await api.get(`/api/user/profile?user_id=${userId}`);
      const p = res.data.profile;
      setProfile(p);
      setForm({ display_name: p.display_name||'', specialization: p.specialization||'', experience: p.experience||'', skills: (p.skills||'').replace(/[{}"]/g,'').replace(/,/g,', '), hourly_rate: p.hourly_rate||0, is_available: p.is_available!==false, company_name: p.company_name||'', description: p.company_description||'' });
    } catch (err) { console.error(err); } finally { setLoading(false); }
  };

  const handleSave = async () => { setSaving(true); try { await api.put(`/api/user/profile?user_id=${userId}`, form); setMessage('Профиль обновлён!'); fetchProfile(); } catch (err: any) { setMessage('Ошибка'); } setSaving(false); };

  const sendRequest = async (newRole: string, extra: any = {}) => {
    try {
      const res = await api.post('/api/role/request', { user_id: Number(userId), role: newRole, ...extra });
      if (newRole !== 'customer') { localStorage.setItem('role', newRole); setMessage(`Роль изменена на "${newRole}"!`); fetchProfile(); }
      else { setMessage('Заявка отправлена модератору!'); setDialogOpen(false); }
    } catch (err: any) { setMessage(err.response?.data?.error || 'Ошибка'); }
  };

  if (loading) return <Typography>Загрузка...</Typography>;
  const isFreelancer = profile.role === 'freelancer';

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Профиль</Typography>
      {message && <Alert severity="success" sx={{ mb: 2 }} onClose={()=>setMessage('')}>{message}</Alert>}

      <Card sx={{ mb: 3 }}>
        <CardContent>
          <Typography variant="h6" gutterBottom>Роль: {profile.role==='freelancer'?'Фрилансер':profile.role==='customer'?'Заказчик':profile.role==='admin'?'Админ':profile.role}</Typography>
          <Stack direction="row" spacing={1} flexWrap="wrap" useFlexGap>
            {profile.role !== 'customer' && <Button variant="outlined" onClick={() => setDialogOpen(true)}>Стать заказчиком (заявка)</Button>}
            {profile.role !== 'freelancer' && <Button variant="outlined" onClick={() => sendRequest('freelancer')}>Стать фрилансером</Button>}
            {/* Кнопка возврата к админу — только если был админом */}
            {profile.role !== 'admin' && currentRole === 'admin' && (
              <Button variant="outlined" color="error" onClick={() => sendRequest('admin')}>Вернуться к админу</Button>
            )}
          </Stack>
        </CardContent>
      </Card>

      <Grid container spacing={3}>
        <Grid item xs={12} md={8}>
          <Card><CardContent>
            <Typography variant="h6" gutterBottom>Редактирование</Typography>
            {isFreelancer ? (<>
              <TextField fullWidth label="Имя" value={form.display_name} onChange={e=>setForm({...form,display_name:e.target.value})} margin="normal" />
              <TextField fullWidth label="Специализация" value={form.specialization} onChange={e=>setForm({...form,specialization:e.target.value})} margin="normal" />
              <TextField fullWidth label="Опыт" value={form.experience} onChange={e=>setForm({...form,experience:e.target.value})} margin="normal" multiline rows={3} />
              <TextField fullWidth label="Навыки" value={form.skills} onChange={e=>setForm({...form,skills:e.target.value})} margin="normal" />
              <TextField fullWidth label="Ставка (₽/час)" type="number" value={form.hourly_rate} onChange={e=>setForm({...form,hourly_rate:Number(e.target.value)})} margin="normal" />
              <FormControlLabel control={<Switch checked={form.is_available} onChange={e=>setForm({...form,is_available:e.target.checked})} />} label="Доступен" sx={{mt:2}} />
            </>) : (<>
              <TextField fullWidth label="Компания" value={form.company_name} onChange={e=>setForm({...form,company_name:e.target.value})} margin="normal" />
              <TextField fullWidth label="Описание" value={form.description} onChange={e=>setForm({...form,description:e.target.value})} margin="normal" multiline rows={4} />
            </>)}
            <Button variant="contained" onClick={handleSave} disabled={saving} sx={{mt:3}}>Сохранить</Button>
          </CardContent></Card>
        </Grid>
        <Grid item xs={12} md={4}>
          <Card><CardContent>
            <Typography variant="h6" gutterBottom>Инфо</Typography>
            <Typography><strong>Email:</strong> {profile.email}</Typography>
            <Typography><strong>Роль:</strong> {profile.role}</Typography>
            <Typography><strong>Рейтинг:</strong> {profile.rating||0} ★</Typography>
          </CardContent></Card>
        </Grid>
      </Grid>

      <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Заявка на роль "Заказчик"</DialogTitle>
        <DialogContent>
          <Typography variant="body2" color="text.secondary" sx={{ mb: 2 }}>Заполните данные компании. Заявка будет рассмотрена модератором.</Typography>
          <TextField fullWidth label="Название компании" value={requestForm.company_name} onChange={e => setRequestForm({...requestForm, company_name: e.target.value})} margin="normal" required />
          <TextField fullWidth label="Описание деятельности" value={requestForm.description} onChange={e => setRequestForm({...requestForm, description: e.target.value})} margin="normal" multiline rows={3} required />
          <TextField fullWidth label="Контакты" value={requestForm.contact} onChange={e => setRequestForm({...requestForm, contact: e.target.value})} margin="normal" />
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setDialogOpen(false)}>Отмена</Button>
          <Button variant="contained" onClick={() => sendRequest('customer', requestForm)} disabled={!requestForm.company_name || !requestForm.description}>Отправить заявку</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default ProfilePage;
