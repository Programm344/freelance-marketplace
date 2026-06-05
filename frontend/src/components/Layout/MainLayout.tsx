import React, { useState, useEffect } from 'react';
import { Outlet, Link, useNavigate } from 'react-router-dom';
import { AppBar, Toolbar, Typography, Button, Box, Container, IconButton, Menu, MenuItem, Avatar, Drawer, List, ListItem, ListItemText, ListItemIcon, Badge } from '@mui/material';
import { Menu as MenuIcon, Work, Search, Person, Assessment, Language, ExitToApp, AdminPanelSettings, Storage, Save as SaveIcon, Notifications as NotificationsIcon } from '@mui/icons-material';
import api from '../../services/api';

const MainLayout: React.FC = () => {
  const navigate = useNavigate();
  const [mobileOpen, setMobileOpen] = useState(false);
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  const [unreadCount, setUnreadCount] = useState(0);
  const isLoggedIn = !!localStorage.getItem('token');
  const userRole = localStorage.getItem('role') || '';
  const userEmail = localStorage.getItem('email') || '';

  useEffect(() => {
    if (isLoggedIn) {
      api.get('/api/notifications').then(res => {
        setUnreadCount((res.data.notifications||[]).filter((n:any)=>!n.is_read).length);
      }).catch(()=>{});
    }
  }, []);

  const handleLogout = () => { localStorage.clear(); navigate('/login'); };

  const menuItems = [
    { text: 'Заказы', icon: <Work />, path: '/orders' },
    ...(userRole === 'customer' ? [{ text: 'Мои заказы', icon: <Work />, path: '/my-orders' }] : []),
    { text: 'Фрилансеры', icon: <Search />, path: '/freelancers' },
    ...(isLoggedIn ? [
      { text: 'Уведомления', icon: <Badge badgeContent={unreadCount} color="error"><NotificationsIcon /></Badge>, path: '/notifications' },
      { text: 'Сохранённые', icon: <SaveIcon />, path: '/saved-searches' },
      { text: 'Профиль', icon: <Person />, path: '/profile' },
    ] : []),
    ...(userRole === 'moderator' || userRole === 'admin' ? [
      { text: 'Модерация', icon: <Assessment />, path: '/moderation' },
      { text: 'Сводка', icon: <Storage />, path: '/reports' },
    ] : []),
    ...(userRole === 'admin' ? [
      { text: 'Краулер', icon: <Language />, path: '/crawler' },
      { text: 'Пользователи', icon: <AdminPanelSettings />, path: '/admin' },
    ] : []),
  ];

  return (
    <Box sx={{ display: 'flex', minHeight: '100vh', bgcolor: 'background.default' }}>
      <AppBar position="fixed" sx={{ bgcolor: 'white', color: 'text.primary', boxShadow: '0 1px 3px rgba(0,0,0,0.1)' }}>
        <Toolbar>
          <IconButton edge="start" onClick={() => setMobileOpen(!mobileOpen)} sx={{ mr: 2, display: { md: 'none' } }}><MenuIcon /></IconButton>
          <Typography variant="h6" component={Link} to="/" sx={{ flexGrow: 1, textDecoration: 'none', color: 'primary.main', fontWeight: 700 }}>FreelanceMarket</Typography>
          <Box sx={{ display: { xs: 'none', md: 'flex' }, gap: 1 }}>
            {menuItems.map(item => (
              <Button key={item.text} component={Link} to={item.path} color="inherit" size="small">{item.icon}<Box component="span" sx={{ ml: 0.5, display: { xs: 'none', lg: 'inline' } }}>{item.text}</Box></Button>
            ))}
          </Box>
          {isLoggedIn ? (
            <>
              <IconButton onClick={(e) => setAnchorEl(e.currentTarget)}><Avatar sx={{ width: 32, height: 32, bgcolor: 'primary.main', fontSize: 14 }}>{userEmail?.[0]?.toUpperCase() || 'U'}</Avatar></IconButton>
              <Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={() => setAnchorEl(null)}>
                <MenuItem disabled><Typography variant="caption">{userEmail}</Typography></MenuItem>
                <MenuItem disabled><Typography variant="caption">Роль: {userRole}</Typography></MenuItem>
                <MenuItem onClick={() => { setAnchorEl(null); navigate('/profile'); }}>Профиль</MenuItem>
                <MenuItem onClick={() => { setAnchorEl(null); handleLogout(); }}><ExitToApp sx={{ mr: 1 }} />Выйти</MenuItem>
              </Menu>
            </>
          ) : (
            <Box sx={{ display: 'flex', gap: 1 }}>
              <Button component={Link} to="/login" color="inherit">Войти</Button>
              <Button component={Link} to="/register" variant="contained">Регистрация</Button>
            </Box>
          )}
        </Toolbar>
      </AppBar>
      <Box component="main" sx={{ flexGrow: 1, pt: 8, pb: 4 }}><Container maxWidth="lg"><Outlet /></Container></Box>
    </Box>
  );
};

export default MainLayout;
