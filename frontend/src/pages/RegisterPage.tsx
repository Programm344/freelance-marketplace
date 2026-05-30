import React, { useState } from 'react';
import { Box, Card, CardContent, TextField, Button, Typography, Alert } from '@mui/material';
import { Link, useNavigate } from 'react-router-dom';
import api from '../services/api';

const RegisterPage: React.FC = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const navigate = useNavigate();

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setSuccess('');
    
    if (password.length < 3) {
      setError('Пароль должен быть минимум 3 символа');
      return;
    }
    
    try {
      const response = await api.post('/api/auth/register', { 
        email: email.trim(), 
        password: password 
      });
      
      console.log('Register response:', response.data);
      setSuccess('Регистрация успешна! Сейчас перенаправим на вход...');
      setTimeout(() => navigate('/login'), 2000);
    } catch (err: any) {
      console.error('Register error:', err);
      if (err.response?.data?.error) {
        setError(err.response.data.error);
      } else if (err.code === 'ERR_NETWORK') {
        setError('Не удалось подключиться к серверу. Проверьте что бэкенд запущен на порту 8080');
      } else {
        setError('Ошибка регистрации. Попробуйте позже.');
      }
    }
  };

  return (
    <Box sx={{ maxWidth: 400, mx: 'auto', mt: 8 }}>
      <Card>
        <CardContent sx={{ p: 4 }}>
          <Typography variant="h5" gutterBottom textAlign="center">
            Регистрация
          </Typography>
          
          {error && <Alert severity="error" sx={{ mb: 2 }}>{error}</Alert>}
          {success && <Alert severity="success" sx={{ mb: 2 }}>{success}</Alert>}
          
          <form onSubmit={handleRegister}>
            <TextField
              fullWidth
              label="Email"
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              margin="normal"
              required
            />
            <TextField
              fullWidth
              label="Пароль"
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              margin="normal"
              required
              helperText="Минимум 3 символа"
            />
            <Button
              type="submit"
              fullWidth
              variant="contained"
              size="large"
              sx={{ mt: 3 }}
            >
              Зарегистрироваться
            </Button>
          </form>
          
          <Typography textAlign="center" sx={{ mt: 2 }}>
            Уже есть аккаунт? <Link to="/login">Войти</Link>
          </Typography>
        </CardContent>
      </Card>
    </Box>
  );
};

export default RegisterPage;
