import React, { useState } from 'react';
import { Box, Typography, Card, CardContent, TextField, Button, Alert, MenuItem } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const categories = [
  { id: 1, name: 'Web Development' },
  { id: 2, name: 'Mobile Development' },
  { id: 3, name: 'Design' },
  { id: 4, name: 'DevOps' },
  { id: 5, name: 'Data Science' },
  { id: 6, name: 'QA Testing' },
  { id: 7, name: 'Technical Support' },
];

const CreateOrderPage: React.FC = () => {
  const navigate = useNavigate();
  const [form, setForm] = useState({
    title: '',
    description: '',
    category_id: 1,
    required_skills: '',
    budget: 0,
    deadline: '',
    payment_format: 'fixed',
  });
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setSuccess('');

    if (!form.title || !form.description || !form.budget) {
      setError('Заполните обязательные поля: название, описание, бюджет');
      return;
    }

    try {
      const res = await api.post("/api/orders", { ...form, customer_id: Number(localStorage.getItem("user_id") || 1) });
      setSuccess(`Заказ #${res.data.order_id} создан!`);
      setTimeout(() => navigate(`/orders/${res.data.order_id}`), 1500);
    } catch (err: any) {
      setError(err.response?.data?.error || 'Ошибка создания заказа');
    }
  };

  return (
    <Box sx={{ maxWidth: 700, mx: 'auto' }}>
      <Typography variant="h4" gutterBottom>Создать заказ</Typography>

      {error && <Alert severity="error" sx={{ mb: 2 }}>{error}</Alert>}
      {success && <Alert severity="success" sx={{ mb: 2 }}>{success}</Alert>}

      <Card>
        <CardContent>
          <form onSubmit={handleSubmit}>
            <TextField fullWidth label="Название заказа" value={form.title}
              onChange={(e) => setForm({...form, title: e.target.value})} margin="normal" required />
            
            <TextField fullWidth label="Описание" value={form.description}
              onChange={(e) => setForm({...form, description: e.target.value})} margin="normal"
              multiline rows={5} required />
            
            <TextField fullWidth select label="Категория" value={form.category_id}
              onChange={(e) => setForm({...form, category_id: Number(e.target.value)})} margin="normal">
              {categories.map(c => <MenuItem key={c.id} value={c.id}>{c.name}</MenuItem>)}
            </TextField>
            
            <TextField fullWidth label="Требуемые навыки (через запятую)" value={form.required_skills}
              onChange={(e) => setForm({...form, required_skills: e.target.value})} margin="normal"
              helperText="React, TypeScript, Node.js" />
            
            <TextField fullWidth label="Бюджет (₽)" type="number" value={form.budget}
              onChange={(e) => setForm({...form, budget: Number(e.target.value)})} margin="normal" required />
            
            <TextField fullWidth label="Срок выполнения" type="date" value={form.deadline}
              onChange={(e) => setForm({...form, deadline: e.target.value})} margin="normal"
              InputLabelProps={{ shrink: true }} />
            
            <TextField fullWidth select label="Формат оплаты" value={form.payment_format}
              onChange={(e) => setForm({...form, payment_format: e.target.value})} margin="normal">
              <MenuItem value="fixed">Фиксированная</MenuItem>
              <MenuItem value="hourly">Почасовая</MenuItem>
            </TextField>

            <Button type="submit" variant="contained" size="large" sx={{ mt: 3 }}>
              Создать заказ
            </Button>
          </form>
        </CardContent>
      </Card>
    </Box>
  );
};

export default CreateOrderPage;
