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
    e.preventDefault();
    setError('');
    
    try {
      const response = await api.post('/api/auth/login', { email: email.trim(), password });
      
      if (response.data.token) {
        localStorage.setItem('token', response.data.token);
        localStorage.setItem('email', response.data.email || email);
        localStorage.setItem('role', response.data.role || 'freelancer');
        localStorage.setItem('user_id', response.data.user_id || '1');
        navigate('/orders');
      } else {
        setError('Неверный ответ от сервера');
      }
    } catch (err: any) {
      if (err.response?.data?.error) setError(err.response.data.error);
      else if (err.code === 'ERR_NETWORK') setError('Бэкенд не запущен на порту 8080');
      else setError('Ошибка входа');
    }
  };

  return (
    <Box sx={{ maxWidth: 400, mx: 'auto', mt: 8 }}>
      <Card>
        <CardContent sx={{ p: 4 }}>
          <Typography variant="h5" gutterBottom textAlign="center">Вход</Typography>
          {error && <Alert severity="error" sx={{ mb: 2 }}>{error}</Alert>}
          
          <form onSubmit={handleLogin}>
            <TextField fullWidth label="Email" type="email" value={email}
              onChange={(e) => setEmail(e.target.value)} margin="normal" required />
            <TextField fullWidth label="Пароль" type="password" value={password}
              onChange={(e) => setPassword(e.target.value)} margin="normal" required />
            <Button type="submit" fullWidth variant="contained" size="large" sx={{ mt: 3 }}>Войти</Button>
          </form>
          
          <Typography textAlign="center" sx={{ mt: 2 }}>
            Нет аккаунта? <Link to="/register">Зарегистрироваться</Link>
          </Typography>
          <Typography textAlign="center" sx={{ mt: 1 }}>
            <small>Роль: {localStorage.getItem('role') || 'не выбрана'}</small>
          </Typography>
        </CardContent>
      </Card>
    </Box>
  );
};

export default LoginPage;
