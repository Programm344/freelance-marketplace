import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Chip, Stack, Button, Rating } from '@mui/material';
import { useParams, useNavigate } from 'react-router-dom';
import api from '../services/api';

const FreelancerProfilePage: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const navigate = useNavigate();
  const [profile, setProfile] = useState<any>(null);
  const [reviews, setReviews] = useState<any[]>([]);
  const userRole = localStorage.getItem('role') || '';

  useEffect(() => {
    api.get(`/api/freelancers/${id}`).then(res => {
      setProfile(res.data.profile);
    }).catch(console.error);
    
    api.get(`/api/users/${id}/reviews`).then(res => {
      setReviews(res.data.reviews || []);
    }).catch(console.error);
  }, [id]);

  if (!profile) return <Typography>Загрузка...</Typography>;

  return (
    <Box>
      <Card sx={{ mb: 3 }}>
        <CardContent>
          <Stack direction="row" justifyContent="space-between" alignItems="flex-start" flexWrap="wrap">
            <Box>
              <Typography variant="h4">{profile.display_name || 'Без имени'}</Typography>
              <Typography variant="h6" color="text.secondary">{profile.specialization}</Typography>
            </Box>
            <Stack direction="row" spacing={1} alignItems="center">
              <Box sx={{ textAlign: 'center' }}>
                <Rating value={profile.rating || 0} readOnly precision={0.5} />
                <Typography variant="body2">{profile.rating || 0} / 5</Typography>
              </Box>
              <Chip label={`${profile.hourly_rate?.toLocaleString()} ₽/час`} color="primary" />
              <Chip label={profile.is_available ? 'Доступен' : 'Занят'} 
                color={profile.is_available ? 'success' : 'default'} />
            </Stack>
          </Stack>

          {userRole === 'customer' && profile.is_available && (
            <Button variant="contained" sx={{ mt: 2 }} onClick={() => navigate('/orders/create')}>
              Пригласить на проект
            </Button>
          )}

          <Typography variant="h6" sx={{ mt: 3 }}>Опыт работы</Typography>
          <Typography>{profile.experience || 'Не указан'}</Typography>

          <Typography variant="h6" sx={{ mt: 3 }}>Навыки</Typography>
          <Box sx={{ mt: 1 }}>
            {(profile.skills || '').replace(/[{}"]/g, '').split(',').filter(Boolean).map((s: string) => (
              <Chip key={s.trim()} label={s.trim()} variant="outlined" sx={{ mr: 0.5, mt: 0.5 }} />
            ))}
          </Box>
        </CardContent>
      </Card>

      <Typography variant="h5" gutterBottom>Отзывы ({reviews.length})</Typography>
      {reviews.map((r: any) => (
        <Card key={r.id} sx={{ mb: 1 }}>
          <CardContent>
            <Stack direction="row" justifyContent="space-between">
              <Typography variant="subtitle2">{r.author}</Typography>
              <Rating value={r.rating} readOnly size="small" />
            </Stack>
            <Typography variant="body2" sx={{ mt: 1 }}>{r.comment}</Typography>
            <Typography variant="caption" color="text.secondary">
              {r.order_title} • {new Date(r.created_at).toLocaleDateString('ru-RU')}
            </Typography>
          </CardContent>
        </Card>
      ))}
      {reviews.length === 0 && <Typography color="text.secondary">Нет отзывов</Typography>}
    </Box>
  );
};

export default FreelancerProfilePage;
