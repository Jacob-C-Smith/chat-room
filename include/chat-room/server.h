#include <chat-room/chat-room.h>
#include <chat-room/client.h>

#define MAX_SERVER_NAME_LEN 255

struct chat_room_server_s
{
    char           name[MAX_SERVER_NAME_LEN+1];
    unsigned short port_number;
    socket_tcp     _socket_tcp;
};

int chat_room_server_create ( chat_room_server **const pp_chat_room_server, const char *const path );