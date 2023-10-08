#include <stdio.h>
#include <stdlib.h>

#include <socket/socket.h>

#include <queue/queue.h>

#include <chat-room/server.h>

#define MAX_USERS 2

int users = 0;
queue *p_clients = 0;
chat_room_server *p_server = 0;

int chat_room_accept ( socket_tcp _socket_tcp, unsigned long ip_address, unsigned short port )
{

    // Uninitialized data
    char  _connect_object[255];

    // Initialized data
    json_value       *p_value  = 0;
    chat_room_client *p_client = calloc(1, sizeof(chat_room_client));
    char             *name     = 0;

    *p_client = (chat_room_client) 
    {
        ._socket_tcp = _socket_tcp,
        .port_number = port,
        .ip_address  = ip_address
    };

    socket_tcp_receive(_socket_tcp, _connect_object, 255 );
    
    if ( parse_json_value(_connect_object, 0, &p_value) == 0 ) goto bad_request;

    if ( p_value->type == JSON_VALUE_OBJECT )
    {
        
        // Initialized data
        dict *p_dict = p_value->object;
        json_value *p_name = 0;
        p_name = ((json_value *)dict_get(p_dict, "name"));
        if(p_name == 0 ) goto bad_request;
        name = p_name->string;
        
        memcpy(p_client->name, name, strlen(name));
    }
    else
        goto bad_request;

    queue_construct(&p_client->_messages);

    queue_enqueue(p_clients, p_client);

    queue_enqueue(p_client->_messages, "Welcome to ");
    queue_enqueue(p_client->_messages, p_server->name);
    queue_enqueue(p_client->_messages, ", ");
    queue_enqueue(p_client->_messages, name);
    queue_enqueue(p_client->_messages, "\n\r");
    
    users++;

    if ( users != MAX_USERS ) 
        socket_tcp_send(_socket_tcp, "Please wait for other chatters\n\r", 33);
    
    // Success
    return 1;

    bad_request:
        socket_tcp_send(_socket_tcp, "Bad Request!\n\r", 15);
        socket_tcp_destroy(&_socket_tcp);
        return 0;
};

int main ( int argc, const char *argv[] )
{

    // Initialized data
    //

    queue_construct(&p_clients);

    // Construct a chat room
    chat_room_server_create(&p_server, "server.json");

    // Wait for 2 connections
    while( users != MAX_USERS )
        socket_tcp_listen(p_server->_socket_tcp, &chat_room_accept);
    
    while ( queue_empty(p_clients) == false )
    {
        chat_room_client *p_client = 0;
        queue_dequeue(p_clients, &p_client);

        while ( queue_empty(p_client->_messages) == false )
        {
            char   *_message    = 0;
            size_t  message_len = 0;

            queue_dequeue(p_client->_messages, &_message);

            message_len = strlen(_message);

            socket_tcp_send(p_client->_socket_tcp, _message, message_len);
        }
    }

    // Success
    return EXIT_SUCCESS;
}