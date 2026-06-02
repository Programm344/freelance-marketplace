import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Chip, Button, IconButton } from '@mui/material';
import { Delete as DeleteIcon } from '@mui/icons-material';
import api from '../services/api';

const SavedSearchesPage: React.FC = () => {
  const [searches, setSearches] = useState<any[]>([]);

  useEffect(() => {
    api.get('/api/searches/my').then(res => setSearches(res.data.searches || [])).catch(() => {});
  }, []);

  const deleteSearch = async (id: number) => {
    await api.delete(`/api/searches/${id}`);
    setSearches(searches.filter(s => s.id !== id));
  };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Сохранённые поиски</Typography>
      {searches.length === 0 && <Typography color="text.secondary">Нет сохранённых поисков</Typography>}
      {searches.map(s => (
        <Card key={s.id} sx={{ mb: 1 }}>
          <CardContent sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
            <Box>
              <Typography variant="subtitle1">{s.name}</Typography>
              <Chip label={`${s.budget_min || 0} – ${s.budget_max || '∞'} ₽`} size="small" sx={{ mr: 1 }} />
              <Chip label={s.keywords || 'без слов'} size="small" variant="outlined" />
            </Box>
            <IconButton onClick={() => deleteSearch(s.id)} color="error"><DeleteIcon /></IconButton>
          </CardContent>
        </Card>
      ))}
    </Box>
  );
};

export default SavedSearchesPage;
