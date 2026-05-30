import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, Button, Stack, Alert } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const CrawlerPage: React.FC = () => {
  const navigate = useNavigate();
  const userRole = localStorage.getItem('role') || '';
  const [orders, setOrders] = useState<any[]>([]);
  const [sources, setSources] = useState<any[]>([]);
  const [logs, setLogs] = useState<any[]>([]);
  const [message, setMessage] = useState('');
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    if (userRole !== 'admin') navigate('/orders');
    else fetchData();
  }, []);

  const fetchData = async () => {
    try {
      const [o, s, l] = await Promise.all([
        api.get('/api/crawler/orders'), api.get('/api/crawler/sources'), api.get('/api/crawler/logs')
      ]);
      setOrders(o.data.orders || []); 
      setSources(s.data.sources || []); 
      setLogs(l.data.logs || []);
    } catch (err) { console.error(err); }
  };

  const startCrawl = async () => {
    setLoading(true);
    setMessage('');
    try {
      const res = await api.post('/api/crawler/start');
      setMessage(`✅ Краулер прошёл по ${res.data.sources_processed} источникам, нашёл ${res.data.items_found} заказов`);
      await fetchData();
    } catch (err) {
      setMessage('❌ Ошибка запуска краулера');
    }
    setLoading(false);
  };

  if (userRole !== 'admin') return <Typography>Доступ запрещён</Typography>;

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Веб-краулер</Typography>
      <Typography color="text.secondary" sx={{ mb: 2 }}>
        Собирает заказы с фриланс-бирж (FL.ru, Freelance.ru, Habr Freelance)
      </Typography>

      <Stack direction="row" spacing={2} sx={{ mb: 3 }}>
        <Button variant="contained" onClick={startCrawl} disabled={loading}>
          {loading ? '⏳ Сбор данных...' : '🚀 Запустить сбор заказов'}
        </Button>
        <Button variant="outlined" onClick={fetchData}>🔄 Обновить список</Button>
      </Stack>

      {message && <Alert severity={message.includes('✅') ? 'success' : 'error'} sx={{ mb: 2 }}>{message}</Alert>}

      {/* Источники */}
      <Typography variant="h6" gutterBottom>📡 Источники ({sources.length})</Typography>
      <Grid container spacing={1} sx={{ mb: 3 }}>
        {sources.map(s => (
          <Grid item xs={12} sm={4} key={s.id}>
            <Card>
              <CardContent sx={{ py: 1 }}>
                <Typography variant="subtitle2">{s.name}</Typography>
                <Typography variant="caption" color="text.secondary">{s.base_url?.substring(0, 40)}</Typography>
                <Chip label={s.status === 'active' ? 'Активен' : 'Выключен'} size="small" 
                  color={s.status === 'active' ? 'success' : 'default'} sx={{ mt: 0.5 }} />
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Собранные заказы */}
      <Typography variant="h6" gutterBottom>📋 Собранные заказы ({orders.length})</Typography>
      <Grid container spacing={1} sx={{ mb: 3 }}>
        {orders.slice(0, 15).map(o => (
          <Grid item xs={12} key={o.id}>
            <Card sx={{ cursor: 'pointer' }} onClick={() => window.open(o.source_url, '_blank')}>
              <CardContent sx={{ py: 1, '&:last-child': { pb: 1 } }}>
                <Typography variant="body2">{o.title?.substring(0, 120)}</Typography>
                <Stack direction="row" spacing={1} sx={{ mt: 0.5 }}>
                  <Chip label={o.source_name} size="small" variant="outlined" />
                  {o.budget > 0 && <Chip label={`${o.budget.toLocaleString()} ₽`} size="small" color="primary" />}
                  <Chip label={o.status === 'new' ? 'Новый' : o.status} size="small" />
                </Stack>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Логи */}
      <Typography variant="h6" gutterBottom>📝 Последние запуски</Typography>
      {logs.slice(0, 5).map(l => (
        <Card key={l.id} sx={{ mb: 1 }}>
          <CardContent sx={{ py: 1, '&:last-child': { pb: 1 } }}>
            <Stack direction="row" justifyContent="space-between">
              <Typography variant="body2">{l.source_name}: найдено {l.items_found}</Typography>
              <Chip label={l.status === 'success' ? 'Успешно' : 'Ошибка'} size="small" 
                color={l.status === 'success' ? 'success' : 'error'} />
            </Stack>
            {l.error_message && <Typography variant="caption" color="error">{l.error_message}</Typography>}
          </CardContent>
        </Card>
      ))}
    </Box>
  );
};

export default CrawlerPage;
