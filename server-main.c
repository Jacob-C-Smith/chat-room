/** !
 * Chat room server program
 * 
 * @file server-main.c
 * 
 * @author Jacob Smith
*/

// Standard library
#include <stdio.h>
#include <stdlib.h>

// Socket submodule
#include <socket/socket.h>

// Queue submodule
#include <queue/queue.h>

// Chatroom server
#include <chat-room/server.h>

// Preprocessor definitions
#define MAX_USERS 2

// Global variables
// TODO: Move these into the chat_room_server struct
int               users     = 0;
queue            *p_clients = 0;
chat_room_server *p_server  = 0;

/** !
 * Accept a chat room client, and add them to the chat room server.
 * This gets called when a client attempts to connect.
 * 
 * @param _socket_tcp the client's socket
 * @param ip_address  the IP address of the client
 * @param port        the port number of the client
 * 
 * @return 1 on success, 0 on error
*/
int chat_room_accept ( socket_tcp _socket_tcp, unsigned long ip_address, unsigned short port );

// Entry point
int main ( int argc, const char *argv[] )
{

    // TODO: Initialized data
    //

    // Construct a queue for clients
    queue_construct(&p_clients);

    // Construct a chat room
    chat_room_server_create(&p_server, "server.json");

    // Wait for 2 connections
    while( users != MAX_USERS )
        socket_tcp_listen(p_server->_socket_tcp, &chat_room_accept);

    // Foreach client
    while ( queue_empty(p_clients) == false )
    {

        // Initialized data
        chat_room_client *p_client = 0;

        // Get a client
        queue_dequeue(p_clients, &p_client);

        // Foreach message
        while ( queue_empty(p_client->_messages) == false )
        {

            // Initialized data
            char   *_message    = 0;
            size_t  message_len = 0;

            // Get a message
            queue_dequeue(p_client->_messages, &_message);

            // Compute the length of the message
            message_len = strlen(_message);

            // Send the message to the client
            socket_tcp_send(p_client->_socket_tcp, _message, message_len);
        }
    }

    // Success
    return EXIT_SUCCESS;
}

int chat_room_accept ( socket_tcp _socket_tcp, unsigned long ip_address, unsigned short port )
{

    // Uninitialized data
    char  _connect_object[255];

    // Initialized data
    json_value       *p_value  = 0;
    chat_room_client *p_client = calloc(1, sizeof(chat_room_client));
    char             *name     = 0;

    // Store a client struct
    *p_client = (chat_room_client) 
    {
        ._socket_tcp = _socket_tcp,
        .port_number = port,
        .ip_address  = ip_address
    };

    // Get the sign in json object
    socket_tcp_receive(_socket_tcp, _connect_object, 255 );
    
    // Parse the sign in json object
    if ( parse_json_value(_connect_object, 0, &p_value) == 0 ) goto bad_request;

    // Parse the contents of the JSON object
    if ( p_value->type == JSON_VALUE_OBJECT )
    {
        
        // Initialized data
        dict       *p_dict = p_value->object;
        json_value *p_name = 0;

        // Get the user name
        p_name = dict_get(p_dict, "name");

        // Error check
        if ( p_name == 0 ) goto bad_request;

        // Store the user name
        name = p_name->string;
        
        // Copy the user name into the client struct
        memcpy(p_client->name, name, strlen(name));
    }
    
    // Default
    else
        goto bad_request;

    // Construct a queue for messages for the user
    queue_construct(&p_client->_messages);

    // Add the client to the client queue
    queue_enqueue(p_clients, p_client);

    // Add the welcome message to the client's message queue
    queue_enqueue(p_client->_messages, p_server->welcome_message);
    
    // Increment the user count
    users++;

    // Tell the user that they are waiting on others
    if ( users != MAX_USERS ) 
        socket_tcp_send(_socket_tcp, "Please wait for other chatters...\n\r", 36);
    
    // Log the new user
    printf("%s connected from %d.%d.%d.%d:%d\n", 
        p_client->name,
        (ip_address & 0xFF000000) >> 24,
        (ip_address & 0x00FF0000) >> 16,
        (ip_address & 0x0000FF00) >> 8,
        (ip_address & 0x000000FF),
        port
    );

    fflush(stdout);

    // Success
    return 1;

    // Error handling
    {
        bad_request:

            // Tell the client that they sent an erroneous request
            socket_tcp_send(_socket_tcp, "Bad request!\n\r", 15);

            // Destroy the socket
            socket_tcp_destroy(&_socket_tcp);

            // Error
            return 0;
    }
};