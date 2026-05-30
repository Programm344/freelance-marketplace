import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Button, Stack } from '@mui/material';
import api from '../services/api';

const ReportsPage: React.FC = () => {
  const [stats, setStats] = useState<any>({});
  const role = localStorage.getItem('role') || '';

  useEffect(() => {
    api.get('/api/admin/stats').then(res => setStats(res.data.stats || {})).catch(console.error);
  }, []);

  const downloadCSV = () => {
    api.get('/api/admin/export/csv', { responseType: 'blob' }).then(res => {
      const url = window.URL.createObjectURL(new Blob([res.data]));
      const a = document.createElement('a');
      a.href = url; a.download = 'report.csv'; a.click();
    });
  };

  if (role !== 'admin' && role !== 'moderator') return <Typography>Доступ запрещён</Typography>;

  const cards = [
    { label: 'Опубликовано заказов', value: stats.published_orders || 0 },
    { label: 'На модерации', value: stats.moderation_orders || 0 },
    { label: 'В работе', value: stats.in_progress_orders || 0 },
    { label: 'Завершено', value: stats.completed_orders || 0 },
    { label: 'Всего откликов', value: stats.total_responses || 0 },
    { label: 'Внешних заказов', value: stats.new_external_orders || 0 },
    { label: 'Ошибок краулера', value: stats.crawl_errors || 0 },
    { label: 'Активных источников', value: stats.active_sources || 0 },
    { label: 'Пользователей', value: stats.total_users || 0 },
  ];

  return (
    <Box>
      <Stack direction="row" justifyContent="space-between" alignItems="center" sx={{ mb: 3 }}>
        <Typography variant="h4">Сводка</Typography>
        <Button variant="contained" onClick={downloadCSV}>Выгрузить CSV</Button>
      </Stack>
      <Grid container spacing={2}>
        {cards.map(c => (
          <Grid item xs={12} sm={6} md={4} key={c.label}>
            <Card><CardContent>
              <Typography color="text.secondary">{c.label}</Typography>
              <Typography variant="h3">{c.value}</Typography>
            </CardContent></Card>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
};

export default ReportsPage;
