import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Chip, Button, Stack } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const NotificationsPage: React.FC = () => {
  const [notifications, setNotifications] = useState<any[]>([]);
  const navigate = useNavigate();
  const isLoggedIn = !!localStorage.getItem('token');

  useEffect(() => {
    if (!isLoggedIn) { navigate('/login'); return; }
    api.get('/api/notifications').then(res => setNotifications(res.data.notifications || [])).catch(() => {});
  }, []);

  const markAllRead = async () => {
    await api.post('/api/notifications/read-all');
    const res = await api.get('/api/notifications');
    setNotifications(res.data.notifications || []);
  };

  if (!isLoggedIn) return null;

  return (
    <Box>
      <Stack direction="row" justifyContent="space-between" alignItems="center" sx={{ mb: 2 }}>
        <Typography variant="h4">Уведомления</Typography>
        {notifications.length > 0 && <Button onClick={markAllRead}>Прочитать все</Button>}
      </Stack>
      
      {notifications.length === 0 && (
        <Card sx={{ textAlign: 'center', py: 6 }}>
          <CardContent>
            <Typography variant="h6" color="text.secondary">Нет уведомлений</Typography>
            <Typography variant="body2" color="text.secondary">
              Когда появятся новые отклики, сообщения или внешние заказы — вы увидите их здесь
            </Typography>
          </CardContent>
        </Card>
      )}
      
      {notifications.map(n => (
        <Card key={n.id} sx={{ mb: 1, opacity: n.is_read ? 0.6 : 1, borderLeft: n.is_read ? 'none' : '4px solid #6366F1' }}>
          <CardContent>
            <Stack direction="row" justifyContent="space-between" alignItems="flex-start">
              <Box>
                <Typography variant="subtitle2">{n.title}</Typography>
                <Typography variant="body2" color="text.secondary">{n.message}</Typography>
                <Typography variant="caption" color="text.secondary">
                  {new Date(n.created_at).toLocaleString('ru-RU')}
                </Typography>
              </Box>
              <Chip label={n.is_read ? 'Прочитано' : 'Новое'} size="small" color={n.is_read ? 'default' : 'primary'} />
            </Stack>
          </CardContent>
        </Card>
      ))}
    </Box>
  );
};

export default NotificationsPage;
