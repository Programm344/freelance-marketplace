import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, Grid, Chip, Button, Stack, TextField, InputAdornment, MenuItem, Dialog, DialogTitle, DialogContent, DialogActions } from '@mui/material';
import { Link } from 'react-router-dom';
import { Search as SearchIcon, Save as SaveIcon } from '@mui/icons-material';
import api from '../services/api';

const OrdersPage: React.FC = () => {
  const [orders, setOrders] = useState<any[]>([]);
  const [externalOrders, setExternalOrders] = useState<any[]>([]);
  const [tab, setTab] = useState<'internal' | 'external'>('internal');
  const [keyword, setKeyword] = useState('');
  const [budgetMin, setBudgetMin] = useState('');
  const [budgetMax, setBudgetMax] = useState('');
  const [statusFilter, setStatusFilter] = useState('');
  const [saveDialogOpen, setSaveDialogOpen] = useState(false);
  const [searchName, setSearchName] = useState('');
  const role = localStorage.getItem('role') || '';

  useEffect(() => { doSearch(); fetchExternalOrders(); }, []);

  const doSearch = async () => {
    const params = new URLSearchParams();
    if (keyword) params.append('keyword', keyword);
    if (budgetMin && parseInt(budgetMin) > 0) params.append('budget_min', budgetMin);
    if (budgetMax && parseInt(budgetMax) > 0) params.append('budget_max', budgetMax);
    if (statusFilter) params.append('status', statusFilter);
    const res = await api.get(`/api/orders/search?${params.toString()}`);
    setOrders(res.data.orders || []);
  };

  const fetchExternalOrders = async () => {
    const res = await api.get('/api/crawler/orders');
    setExternalOrders(res.data.orders || []);
  };

  const saveSearch = async () => {
    try {
      await api.post('/api/searches', {
        name: searchName,
        keywords: keyword,
        skills: keyword,
        budget_min: parseInt(budgetMin) || 0,
        budget_max: parseInt(budgetMax) || 0
      });
      setSaveDialogOpen(false);
      setSearchName('');
      alert('Поиск сохранён!');
    } catch (err) { alert('Ошибка сохранения'); }
  };

  const getStatusLabel = (s: string) => {
    const l: Record<string,string> = { draft:'Черновик', on_moderation:'На модерации', published:'Опубликован', in_progress:'В работе', completed:'Завершён', new:'Новый' };
    return l[s] || s;
  };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Заказы</Typography>
      
      <Stack direction="row" spacing={2} sx={{ mb: 2 }}>
        <Button variant={tab==='internal'?'contained':'outlined'} onClick={()=>setTab('internal')}>Внутренние ({orders.length})</Button>
        <Button variant={tab==='external'?'contained':'outlined'} onClick={()=>setTab('external')}>Внешние ({externalOrders.length})</Button>
        {role==='customer' && <Button component={Link} to="/orders/create" variant="contained" color="success">Создать заказ</Button>}
      </Stack>

      {tab === 'internal' && (
        <Card sx={{ mb: 2, p: 2 }}>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} flexWrap="wrap" useFlexGap>
            <TextField size="small" placeholder="Поиск..." value={keyword} onChange={e=>setKeyword(e.target.value)}
              onKeyPress={e=>e.key==='Enter'&&doSearch()}
              InputProps={{ startAdornment: <InputAdornment position="start"><SearchIcon /></InputAdornment> }}
              sx={{ flex: 1, minWidth: 200 }} />
            <TextField size="small" label="Бюджет от" type="number" value={budgetMin} onChange={e=>{const v=Number(e.target.value);if(v>=0||e.target.value==='')setBudgetMin(e.target.value)}} sx={{ width: 110 }} />
            <TextField size="small" label="до" type="number" value={budgetMax} onChange={e=>{const v=Number(e.target.value);if(v>=0||e.target.value==='')setBudgetMax(e.target.value)}} sx={{ width: 110 }} />
            <TextField size="small" select label="Статус" value={statusFilter} onChange={e=>setStatusFilter(e.target.value)} sx={{ width: 160 }}>
              <MenuItem value="">Все</MenuItem>
              <MenuItem value="published">Опубликован</MenuItem>
              <MenuItem value="in_progress">В работе</MenuItem>
              <MenuItem value="completed">Завершён</MenuItem>
            </TextField>
            <Button variant="contained" onClick={doSearch}>Найти</Button>
            <Button variant="outlined" startIcon={<SaveIcon />} onClick={() => setSaveDialogOpen(true)} title="Сохранить параметры поиска">Сохранить</Button>
            <Button variant="outlined" onClick={()=>{setKeyword('');setBudgetMin('');setBudgetMax('');setStatusFilter('');}}>Сбросить</Button>
          </Stack>
        </Card>
      )}

      <Grid container spacing={2}>
        {(tab==='internal'?orders:externalOrders).map(order => (
          <Grid item xs={12} key={order.id}>
            <Card component={tab==='internal'?Link:Box} to={tab==='internal'?`/orders/${order.id}`:undefined}
              onClick={tab==='external'?()=>window.open(order.source_url,'_blank'):undefined}
              sx={{ textDecoration:'none', cursor:'pointer', transition:'all 0.3s', '&:hover':{transform:'translateY(-2px)',boxShadow:'0 10px 40px rgba(0,0,0,0.12)'} }}>
              <CardContent>
                <Box sx={{ display:'flex', justifyContent:'space-between' }}>
                  <Box sx={{ flex: 1 }}>
                    <Typography variant="h6" color="text.primary">{order.title}</Typography>
                    <Typography variant="body2" color="text.secondary">{order.description?.substring(0,150)}...</Typography>
                    {order.source_name && <Chip label={order.source_name} size="small" variant="outlined" sx={{ mt:1 }} />}
                  </Box>
                  <Box sx={{ textAlign:'right', ml: 2, minWidth: 120 }}>
                    <Chip label={getStatusLabel(order.status)} size="small" color={order.status==='published'?'success':'default'} sx={{ mb:1 }} />
                    {order.budget > 0 && <Typography variant="h6" color="primary">{Number(order.budget).toLocaleString()} ₽</Typography>}
                    <Typography variant="caption" display="block">{order.category||''}</Typography>
                  </Box>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      <Dialog open={saveDialogOpen} onClose={() => setSaveDialogOpen(false)}>
        <DialogTitle>Сохранить поиск</DialogTitle>
        <DialogContent>
          <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
            Ключевые слова: {keyword || 'нет'} | Бюджет: {budgetMin || '0'}–{budgetMax || 'любо'} ₽
          </Typography>
          <TextField fullWidth label="Название поиска" value={searchName} onChange={e => setSearchName(e.target.value)} margin="normal" required />
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setSaveDialogOpen(false)}>Отмена</Button>
          <Button variant="contained" onClick={saveSearch} disabled={!searchName}>Сохранить</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default OrdersPage;
