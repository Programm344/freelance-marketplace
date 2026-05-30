import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, Button, IconButton } from '@mui/material';
import { Link } from 'react-router-dom';
import { Delete as DeleteIcon } from '@mui/icons-material';
import api from '../services/api';

const MyOrdersPage: React.FC = () => {
  const [orders, setOrders] = useState<any[]>([]);

  useEffect(() => {
    fetchOrders();
  }, []);

  const fetchOrders = () => {
    const userId = localStorage.getItem('user_id') || '1';
    api.get(`/api/orders/my?user_id=${userId}`).then(res => {
      setOrders(res.data.orders || []);
    }).catch(() => {
      api.get('/api/orders').then(res => setOrders(res.data.orders || []));
    });
  };

  const handleDelete = async (id: number) => {
    if (!window.confirm('Удалить заказ?')) return;
    try {
      await api.delete(`/api/orders/${id}`);
      fetchOrders();
    } catch (err) {
      alert('Ошибка удаления');
    }
  };

  const getStatusLabel = (s: string) => {
    const l: Record<string,string> = { draft:'Черновик', on_moderation:'На модерации', published:'Опубликован', in_progress:'В работе', completed:'Завершён', rejected:'Отклонён' };
    return l[s] || s;
  };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Мои заказы</Typography>
      <Button component={Link} to="/orders/create" variant="contained" sx={{ mb: 3 }}>Создать заказ</Button>
      
      {orders.length === 0 && <Typography color="text.secondary">У вас пока нет заказов</Typography>}
      
      <Grid container spacing={2}>
        {orders.map(o => (
          <Grid item xs={12} key={o.id}>
            <Card sx={{ position: 'relative' }}>
              <CardContent>
                <Box sx={{ display:'flex', justifyContent:'space-between' }}>
                  <Box sx={{ flex: 1 }}>
                    <Typography variant="h6" color="text.primary" component={Link} to={`/orders/${o.id}`} sx={{ textDecoration: 'none' }}>
                      {o.title}
                    </Typography>
                    <Typography variant="body2" color="text.secondary">{o.description?.substring(0,100)}</Typography>
                  </Box>
                  <Box sx={{ textAlign:'right' }}>
                    <Chip label={getStatusLabel(o.status)} size="small" 
                      color={o.status==='published'?'success':o.status==='on_moderation'?'warning':'default'} />
                    <Typography variant="h6" color="primary" sx={{ mt: 1 }}>{Number(o.budget||0).toLocaleString()} ₽</Typography>
                    <IconButton onClick={() => handleDelete(o.id)} size="small" color="error" sx={{ mt: 0.5 }}>
                      <DeleteIcon fontSize="small" />
                    </IconButton>
                  </Box>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
};

export default MyOrdersPage;
