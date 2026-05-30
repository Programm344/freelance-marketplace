import React, { useState } from 'react';
import { Outlet, Link, useNavigate } from 'react-router-dom';
import {
  AppBar, Toolbar, Typography, Button, Box, Container,
  IconButton, Menu, MenuItem, Avatar, Drawer, List, ListItem, ListItemText, ListItemIcon
} from '@mui/material';
import { Menu as MenuIcon, Work, Search, Person, Assessment, Language, ExitToApp, AdminPanelSettings, Storage } from '@mui/icons-material';

const MainLayout: React.FC = () => {
  const navigate = useNavigate();
  const [mobileOpen, setMobileOpen] = useState(false);
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  
  const isLoggedIn = !!localStorage.getItem('token');
  const userRole = localStorage.getItem('role') || '';
  const userEmail = localStorage.getItem('email') || '';

  const handleLogout = () => {
    localStorage.clear();
    navigate('/login');
  };

  const menuItems = [
    { text: "Все заказы", icon: <Work />, path: "/orders" },
    ...(userRole === "customer" ? [{ text: "Мои заказы", icon: <Work />, path: "/my-orders" }] : []),
    { text: 'Фрилансеры', icon: <Search />, path: '/freelancers' },
    ...(isLoggedIn ? [{ text: 'Профиль', icon: <Person />, path: '/profile' }] : []),
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
          <IconButton edge="start" onClick={() => setMobileOpen(!mobileOpen)} sx={{ mr: 2, display: { md: 'none' } }}>
            <MenuIcon />
          </IconButton>
          
          <Typography variant="h6" component={Link} to="/"
            sx={{ flexGrow: 1, textDecoration: 'none', color: 'primary.main', fontWeight: 700 }}>
            FreelanceMarket
          </Typography>

          <Box sx={{ display: { xs: 'none', md: 'flex' }, gap: 1 }}>
            {menuItems.map((item) => (
              <Button key={item.text} component={Link} to={item.path} startIcon={item.icon} color="inherit" size="small">
                {item.text}
              </Button>
            ))}
          </Box>

          {isLoggedIn ? (
            <>
              <IconButton onClick={(e) => setAnchorEl(e.currentTarget)}>
                <Avatar sx={{ width: 32, height: 32, bgcolor: 'primary.main', fontSize: 14 }}>
                  {userEmail?.[0]?.toUpperCase() || 'U'}
                </Avatar>
              </IconButton>
              <Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={() => setAnchorEl(null)}>
                <MenuItem disabled><Typography variant="caption">{userEmail}</Typography></MenuItem>
                <MenuItem disabled><Typography variant="caption">Роль: {userRole}</Typography></MenuItem>
                <MenuItem onClick={() => { setAnchorEl(null); navigate('/profile'); }}>Профиль</MenuItem>
                <MenuItem onClick={() => { setAnchorEl(null); handleLogout(); }}>
                  <ExitToApp sx={{ mr: 1 }} /> Выйти
                </MenuItem>
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

      <Drawer open={mobileOpen} onClose={() => setMobileOpen(false)}>
        <Toolbar />
        <List>
          {menuItems.map((item) => (
            <ListItem key={item.text} component={Link} to={item.path} onClick={() => setMobileOpen(false)}>
              <ListItemIcon>{item.icon}</ListItemIcon>
              <ListItemText primary={item.text} />
            </ListItem>
          ))}
        </List>
      </Drawer>

      <Box component="main" sx={{ flexGrow: 1, pt: 8, pb: 4 }}>
        <Container maxWidth="lg">
          <Outlet />
        </Container>
      </Box>
    </Box>
  );
};

export default MainLayout;
