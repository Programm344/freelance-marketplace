import React, { useState } from 'react';
import { Box, Card, CardContent, TextField, Button, Typography, Alert } from '@mui/material';
import { Link, useNavigate } from 'react-router-dom';
import api from '../services/api';

const LoginPage: React.FC = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault(); setError('');
    if (!email.includes('@') || !email.split('@')[1]?.includes('.')) { setError('Введите корректный email'); return; }
    
    try {
      localStorage.clear();
      const response = await api.post('/api/auth/login', { email: email.trim(), password });
      localStorage.setItem('token', response.data.token);
      localStorage.setItem('email', response.data.email);
      localStorage.setItem('role', response.data.role);
      localStorage.setItem('user_id', response.data.user_id);
      
      // Проверяем заявку
      try {
        const rr = await api.get('/api/role/requests');
        const myReq = (rr.data.requests||[]).find((r: any) => r.user_id === response.data.user_id && r.status === 'pending');
        if (myReq) localStorage.setItem('rp', '1');
      } catch(e) {}
      
          if (response.data.role === "customer") { try { const prof = await api.get(`/api/user/profile?user_id=${response.data.user_id}`); if (prof.data.profile.company_name) { localStorage.removeItem("rp"); } } catch(e) {} }
          try { const prof = await api.get(`/api/user/profile?user_id=${response.data.user_id}`); if (prof.data.profile.company_name) { localStorage.removeItem("rp"); } } catch(e) {}
      navigate('/orders');
    } catch (err: any) { setError(err.response?.data?.error || 'Ошибка входа'); }
  };

  return (
    <Box sx={{ maxWidth: 400, mx: 'auto', mt: 8 }}>
      <Card><CardContent sx={{ p: 4 }}>
        <Typography variant="h5" gutterBottom textAlign="center">Вход</Typography>
        {error && <Alert severity="error" sx={{ mb: 2 }}>{error}</Alert>}
        <form onSubmit={handleLogin}>
          <TextField fullWidth label="Email" type="email" value={email} onChange={e => setEmail(e.target.value)} margin="normal" required />
          <TextField fullWidth label="Пароль" type="password" value={password} onChange={e => setPassword(e.target.value)} margin="normal" required />
          <Button type="submit" fullWidth variant="contained" size="large" sx={{ mt: 3 }}>Войти</Button>
        </form>
        <Typography textAlign="center" sx={{ mt: 2 }}>Нет аккаунта? <Link to="/register">Зарегистрироваться</Link></Typography>
      </CardContent></Card>
    </Box>
  );
};

export default LoginPage;
