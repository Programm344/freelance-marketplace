import React, { useState } from 'react';
import { Box, Card, CardContent, TextField, Button, Typography, Alert, ToggleButton, ToggleButtonGroup } from '@mui/material';
import { Link, useNavigate } from 'react-router-dom';
import api from '../services/api';

const RegisterPage: React.FC = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [role, setRole] = useState('freelancer');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const navigate = useNavigate();

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault(); setError(''); setSuccess('');
    if (!email.includes('@') || !email.split('@')[1]?.includes('.')) { setError('Введите корректный email'); return; }
    if (password.length < 3) { setError('Пароль не менее 3 символов'); return; }
    
    try {
            console.log("Register payload:", { email: email.trim(), password, role });
      const res = await api.post('/api/auth/register', { email: email.trim(), password, role });
      localStorage.clear();
      localStorage.setItem('newUser', '1');
      setSuccess('Регистрация успешна!');
      setTimeout(() => navigate('/login'), 1500);
    } catch (err: any) { setError(err.response?.data?.error || 'Ошибка регистрации'); }
  };

  return (
    <Box sx={{ maxWidth: 400, mx: 'auto', mt: 8 }}>
      <Card><CardContent sx={{ p: 4 }}>
        <Typography variant="h5" gutterBottom textAlign="center">Регистрация</Typography>
        {error && <Alert severity="error" sx={{ mb: 2 }}>{error}</Alert>}
        {success && <Alert severity="success" sx={{ mb: 2 }}>{success}</Alert>}
        
        <Typography gutterBottom>Я хочу:</Typography>
        <ToggleButtonGroup value={role} exclusive onChange={(_, v) => v && setRole(v)} fullWidth sx={{ mb: 2 }}>
          <ToggleButton value="freelancer">Брать заказы</ToggleButton>
          <ToggleButton value="customer">Размещать заказы</ToggleButton>
        </ToggleButtonGroup>
        
        <form onSubmit={handleRegister}>
          <TextField fullWidth label="Email" type="email" value={email} onChange={e => setEmail(e.target.value)} margin="normal" required />
          <TextField fullWidth label="Пароль" type="password" value={password} onChange={e => setPassword(e.target.value)} margin="normal" required />
          <Button type="submit" fullWidth variant="contained" size="large" sx={{ mt: 3 }}>Зарегистрироваться</Button>
        </form>
        <Typography textAlign="center" sx={{ mt: 2 }}>Уже есть аккаунт? <Link to="/login">Войти</Link></Typography>
      </CardContent></Card>
    </Box>
  );
};

export default RegisterPage;
