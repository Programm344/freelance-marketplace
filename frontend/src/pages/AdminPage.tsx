import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Button, Chip, Stack, Select, MenuItem, Alert } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../services/api';

const AdminPage: React.FC = () => {
  const navigate = useNavigate();
  const userRole = localStorage.getItem('role') || '';
  const [users, setUsers] = useState<any[]>([]);
  const [message, setMessage] = useState('');

  useEffect(() => {
    if (userRole !== 'admin') navigate('/orders');
    else fetchUsers();
  }, []);

  const fetchUsers = async () => {
    try {
      const res = await api.get('/api/admin/users');
      setUsers(res.data.users || []);
    } catch (err) { console.error(err); }
  };

  const changeRole = async (userId: number, newRole: string) => {
    try {
      await api.put(`/api/admin/users/${userId}`, { role: newRole });
      setMessage(`Роль изменена на ${newRole}`);
      fetchUsers();
    } catch (err: any) { setMessage('Ошибка: ' + err.message); }
  };

  const toggleBlock = async (userId: number, isBlocked: boolean) => {
    try {
      if (isBlocked) await api.post(`/api/admin/users/${userId}/unblock`);
      else await api.post(`/api/admin/users/${userId}/block`);
      setMessage(isBlocked ? 'Пользователь разблокирован' : 'Пользователь заблокирован');
      fetchUsers();
    } catch (err: any) { setMessage('Ошибка'); }
  };

  if (userRole !== 'admin') return <Typography>Доступ запрещён</Typography>;

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Управление пользователями</Typography>
      {message && <Alert sx={{ mb: 2 }} onClose={() => setMessage('')}>{message}</Alert>}
      
      {users.map(u => (
        <Card key={u.id} sx={{ mb: 1 }}>
          <CardContent sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
            <Box>
              <Typography variant="subtitle1">{u.email}</Typography>
              <Stack direction="row" spacing={1} sx={{ mt: 0.5 }}>
                <Chip label={u.role} size="small" color="primary" />
                {u.display_name && <Chip label={u.display_name} size="small" variant="outlined" />}
                {u.company_name && <Chip label={u.company_name} size="small" variant="outlined" />}
                {u.is_blocked && <Chip label="Заблокирован" size="small" color="error" />}
              </Stack>
            </Box>
            <Stack direction="row" spacing={1} alignItems="center">
              <Select size="small" value={u.role} onChange={(e) => changeRole(u.id, e.target.value)} sx={{ minWidth: 140 }}>
                <MenuItem value="freelancer">Фрилансер</MenuItem>
                <MenuItem value="customer">Заказчик</MenuItem>
                <MenuItem value="moderator">Модератор</MenuItem>
                <MenuItem value="admin">Админ</MenuItem>
              </Select>
              <Button size="small" variant="outlined" color={u.is_blocked ? 'success' : 'error'}
                onClick={() => toggleBlock(u.id, u.is_blocked)}>
                {u.is_blocked ? 'Разблокировать' : 'Заблокировать'}
              </Button>
            </Stack>
          </CardContent>
        </Card>
      ))}
    </Box>
  );
};

export default AdminPage;
