#include <chat-room/chat-room.h>

#define MAX_CLIENT_NAME_LEN 255

struct chat_room_client_s
{
    char            name[MAX_CLIENT_NAME_LEN+1];
    bool            signed_on;
    socket_tcp      _socket_tcp;
    unsigned short  port_number;
    unsigned long   ip_address;
    queue          *_messages;
};