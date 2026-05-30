import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, TextField, Button } from '@mui/material';
import { Link } from 'react-router-dom';
import api from '../services/api';

const FreelancersPage: React.FC = () => {
  const [freelancers, setFreelancers] = useState<any[]>([]);
  const [skill, setSkill] = useState('');
  const [error, setError] = useState('');

  const searchFreelancers = async (searchSkill = '') => {
    try {
      setError('');
      const params = searchSkill ? `?skill=${searchSkill}` : '';
      const response = await api.get(`/api/freelancers/search${params}`);
      const filtered = (response.data.freelancers || []).filter((f: any) => 
        f.display_name && f.display_name !== ''
      );
      setFreelancers(filtered);
    } catch (err: any) {
      setError('Не удалось загрузить фрилансеров');
    }
  };

  useEffect(() => { searchFreelancers(); }, []);

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Фрилансеры</Typography>

      <Card sx={{ mb: 3, p: 2 }}>
        <Box sx={{ display: 'flex', gap: 2 }}>
          <TextField fullWidth placeholder="Поиск по навыкам (React, Python...)" value={skill}
            onChange={(e) => setSkill(e.target.value)} size="small" />
          <Button variant="contained" onClick={() => searchFreelancers(skill)}>Найти</Button>
        </Box>
      </Card>

      {error && <Typography color="error" sx={{ mb: 2 }}>{error}</Typography>}

      <Grid container spacing={2}>
        {freelancers.map((f) => (
          <Grid item xs={12} md={6} key={f.id}>
            <Card component={Link} to={`/freelancers/${f.id}`} 
              sx={{ textDecoration: 'none', transition: 'all 0.3s',
                '&:hover': { transform: 'translateY(-2px)', boxShadow: '0 10px 40px rgba(0,0,0,0.12)' } }}>
              <CardContent>
                <Typography variant="h6" color="text.primary">{f.display_name}</Typography>
                <Typography color="text.secondary" gutterBottom>
                  {f.specialization || 'Специализация не указана'}
                </Typography>
                <Box sx={{ mt: 1, display: 'flex', gap: 1, flexWrap: 'wrap' }}>
                  {(f.skills || '').replace(/[{}"]/g, '').split(',').filter(Boolean).map((s: string) => (
                    <Chip key={s.trim()} label={s.trim()} size="small" variant="outlined" />
                  ))}
                </Box>
                <Box sx={{ mt: 2, display: 'flex', justifyContent: 'space-between' }}>
                  <Typography variant="h6" color="primary">
                    {(f.hourly_rate || 0).toLocaleString()} ₽/час
                  </Typography>
                  <Chip label={f.is_available ? 'Доступен' : 'Занят'} size="small"
                    color={f.is_available ? 'success' : 'default'} />
                </Box>
                <Typography variant="body2" color="text.secondary">
                  Рейтинг: {f.rating || 0} ★
                </Typography>
              </CardContent>
            </Card>
          </Grid>
        ))}
        {freelancers.length === 0 && !error && (
          <Grid item xs={12}>
            <Typography textAlign="center" color="text.secondary" sx={{ py: 4 }}>
              Фрилансеры не найдены
            </Typography>
          </Grid>
        )}
      </Grid>
    </Box>
  );
};

export default FreelancersPage;
