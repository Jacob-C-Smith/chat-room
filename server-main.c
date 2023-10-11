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

// Chatroom server
#include <chat-room/server.h>

// Entry point
int main ( int argc, const char *argv[] )
{

    // Initialized data
    chat_room_server *p_server  = 0;

    // Construct a chat room
    chat_room_server_create(&p_server, "server.json");

    // Start the chat room
    //chat_room_server_start(p_server);
    
    // Success
    return EXIT_SUCCESS;
}