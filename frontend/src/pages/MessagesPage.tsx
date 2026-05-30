import React, { useEffect, useState } from 'react';
import { Box, Typography, Card, CardContent, TextField, Button, Stack, Chip } from '@mui/material';
import { useParams } from 'react-router-dom';
import api from '../services/api';

const MessagesPage: React.FC = () => {
  const { orderId } = useParams<{ orderId: string }>();
  const [messages, setMessages] = useState<any[]>([]);
  const [newMessage, setNewMessage] = useState('');
  const [conversations, setConversations] = useState<any[]>([]);
  const [selectedOrder, setSelectedOrder] = useState<number | null>(Number(orderId) || null);

  useEffect(() => {
    fetchConversations();
  }, []);

  useEffect(() => {
    if (selectedOrder) fetchMessages(selectedOrder);
  }, [selectedOrder]);

  const fetchConversations = async () => {
    try {
      const res = await api.get('/api/messages/conversations');
      setConversations(res.data.conversations || []);
    } catch (err) { console.error(err); }
  };

  const fetchMessages = async (orderId: number) => {
    try {
      const res = await api.get(`/api/orders/${orderId}/messages`);
      setMessages(res.data.messages || []);
    } catch (err) { console.error(err); }
  };

  const sendMessage = async () => {
    if (!newMessage.trim() || !selectedOrder) return;
    try {
      await api.post('/api/messages', { order_id: selectedOrder, message: newMessage });
      setNewMessage('');
      fetchMessages(selectedOrder);
    } catch (err) { console.error(err); }
  };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>Сообщения</Typography>
      <Stack direction="row" spacing={2}>
        {/* Список чатов */}
        <Box sx={{ width: 300 }}>
          {conversations.map((c) => (
            <Card
              key={c.order_id}
              sx={{ mb: 1, cursor: 'pointer', bgcolor: selectedOrder === c.order_id ? 'action.selected' : 'white' }}
              onClick={() => setSelectedOrder(c.order_id)}
            >
              <CardContent sx={{ py: 1 }}>
                <Typography variant="subtitle2">{c.order_title}</Typography>
                <Typography variant="caption" color="text.secondary">{c.last_message?.substring(0, 50)}</Typography>
                <Chip label={c.status} size="small" sx={{ mt: 0.5 }} />
              </CardContent>
            </Card>
          ))}
        </Box>

        {/* Сообщения */}
        <Box sx={{ flex: 1 }}>
          {selectedOrder ? (
            <>
              <Box sx={{ maxHeight: 400, overflow: 'auto', mb: 2 }}>
                {messages.map((m) => (
                  <Card key={m.id} sx={{ mb: 1 }}>
                    <CardContent sx={{ py: 1 }}>
                      <Typography variant="caption" color="text.secondary">{m.sender_email}</Typography>
                      <Typography>{m.message}</Typography>
                    </CardContent>
                  </Card>
                ))}
              </Box>
              <Stack direction="row" spacing={1}>
                <TextField fullWidth value={newMessage} onChange={(e) => setNewMessage(e.target.value)}
                  placeholder="Сообщение..." size="small" onKeyPress={(e) => e.key === 'Enter' && sendMessage()} />
                <Button variant="contained" onClick={sendMessage}>Отправить</Button>
              </Stack>
            </>
          ) : (
            <Typography color="text.secondary">Выберите чат</Typography>
          )}
        </Box>
      </Stack>
    </Box>
  );
};

export default MessagesPage;
