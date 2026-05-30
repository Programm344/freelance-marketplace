import React from 'react';
import { Box, Typography, Button, Grid, Card, CardContent, Stack } from '@mui/material';
import { Link } from 'react-router-dom';
import { Work, Search, Language, Assessment } from '@mui/icons-material';

const HomePage: React.FC = () => {
  const isLoggedIn = !!localStorage.getItem('token');

  return (
    <Box>
      <Box sx={{ textAlign: 'center', py: 8 }}>
        <Typography variant="h3" gutterBottom>
          Маркетплейс для фрилансеров
        </Typography>
        <Typography variant="h6" color="text.secondary" sx={{ mb: 4 }}>
          Находите заказы, размещайте проекты, работайте с лучшими
        </Typography>
        <Stack direction="row" spacing={2} justifyContent="center">
          <Button component={Link} to="/orders" variant="contained" size="large">
            Смотреть заказы
          </Button>
        </Stack>
      </Box>

      <Grid container spacing={3} sx={{ mt: 4 }}>
        {[
          { icon: <Work fontSize="large" />, title: 'Заказы', desc: 'Найдите идеальный проект для своих навыков', link: '/orders' },
          { icon: <Search fontSize="large" />, title: 'Фрилансеры', desc: 'Найдите лучших специалистов для вашего проекта', link: '/freelancers' },
          { icon: <Language fontSize="large" />, title: 'Внешние заказы', desc: 'Заказы собранные с фриланс-бирж автоматически', link: '/orders' },
        ].map((feature) => (
          <Grid item xs={12} sm={6} md={3} key={feature.title}>
            <Card component={Link} to={feature.link} sx={{ height: '100%', textDecoration: 'none', transition: 'all 0.3s', '&:hover': { transform: 'translateY(-4px)', boxShadow: '0 10px 40px rgba(0,0,0,0.12)' } }}>
              <CardContent sx={{ textAlign: 'center', py: 4 }}>
                <Box sx={{ color: 'primary.main', mb: 2 }}>{feature.icon}</Box>
                <Typography variant="h6" gutterBottom color="text.primary">{feature.title}</Typography>
                <Typography variant="body2" color="text.secondary">{feature.desc}</Typography>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
};

export default HomePage;
