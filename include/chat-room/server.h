#include <chat-room/chat-room.h>
#include <chat-room/client.h>

#define MAX_SERVER_NAME_LEN 255
#define MAX_SERVER_WELCOME_MESSAGE_LEN 1023

struct chat_room_server_s
{
    char            name            [MAX_SERVER_NAME_LEN+1];
    bool            running;
    char            welcome_message [MAX_SERVER_WELCOME_MESSAGE_LEN+1];
    unsigned short  port_number;
    unsigned short  max_users;
    unsigned short  user_count;
    dict           *p_clients;
    queue          *p_maybe_clients,
                   *p_def_clients;    
    socket_tcp      _socket_tcp;
};

int chat_room_server_create ( chat_room_server **const pp_chat_room_server, const char *const path );
int chat_room_server_start  ( chat_room_server  *const p_chat_room_server );
