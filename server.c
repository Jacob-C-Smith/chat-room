#include <chat-room/server.h>

/**!
 * Return the size of a file IF buffer == 0 ELSE read a file into buffer
 * 
 * @param path path to the file
 * @param buffer buffer
 * @param binary_mode "wb" IF true ELSE "w"
 * 
 * @return 1 on success, 0 on error
 */
size_t load_file ( const char *path, void *buffer, bool binary_mode )
{

    // Argument checking 
    if ( path == 0 ) goto no_path;

    // Initialized data
    size_t  ret = 0;
    FILE   *f   = fopen(path, (binary_mode) ? "rb" : "r");
    
    // Check if file is valid
    if ( f == NULL ) goto invalid_file;

    // Find file size and prep for read
    fseek(f, 0, SEEK_END);
    ret = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read to data
    if ( buffer ) ret = fread(buffer, 1, ret, f);

    // The file is no longer needed
    fclose(f);
    
    // Success
    return ret;

    // Error handling
    {

        // Argument errors
        {
            no_path:
                #ifndef NDEBUG
                    printf("[JSON] Null path provided to function \"%s\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // File errors
        {
            invalid_file:
                #ifndef NDEBUG
                    printf("[Standard library] Failed to load file \"%s\". %s\n",path, strerror(errno));
                #endif

                // Error
                return 0;
        }
    }
}

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
int chat_room_accept ( socket_tcp _socket_tcp, unsigned long ip_address, unsigned short port, chat_room_server *const p_chat_room_server );

int chat_room_server_create ( chat_room_server **const pp_chat_room_server, const char *const path )
{

    // Argument errors
    if ( path == (void *) 0 ) goto no_path;

    // Initialized data
    size_t            len                = load_file(path, 0, true);
    char             *buf                = calloc(len+1, sizeof(char));
    json_value       *p_value            = 0,
                     *p_name             = 0,
                     *p_port             = 0,
                     *p_max_users        = 0,
                     *p_welcome_message  = 0;
    chat_room_server *p_chat_room_server = 0;

    // Load the file into the buffer
    if ( load_file(path, buf, true) == 0 ) goto failed_to_load_file;

    // Parse the text into a json_value
    if ( parse_json_value(buf, 0, &p_value) == 0 ) goto failed_to_parse_json_value;

    // Parse the json_value into constructor parameters
    if ( p_value->type == JSON_VALUE_OBJECT )
    {

        // Initialized data
        dict *p_dict = p_value->object;
        
        // Get the name of the server
        p_name = dict_get(p_dict, "name");

        // Get the welcome message
        p_welcome_message = dict_get(p_dict, "welcome message");

        // Get the port number
        p_port = dict_get(p_dict, "port");

        // Get the maximum quantity of users
        p_max_users = dict_get(p_dict, "maximum users");

        // Error checking
        if ( p_name == 0 ) goto missing_properties;
    }

    // Allocate memory for the server
    p_chat_room_server = calloc(1, sizeof(chat_room_server));
    
    // Store the configuration options in the chat_room_server struct
    {

        // Store the name
        if ( p_name->type == JSON_VALUE_STRING )
        {

            // Initialized data
            char   *name     = p_name->string;
            size_t  name_len = strlen(name);

            // Error check
            if ( name_len > MAX_SERVER_NAME_LEN )
            {

                // Store the maximum name length
                name_len = MAX_SERVER_NAME_LEN;

                // Warn the user
                printf("[chat room] Server name is out of bounds. Truncating in call to function \"%s\"\n", __FUNCTION__);
            }

            // Copy the name
            strncpy(p_chat_room_server->name, name, name_len);

        }
        // Default
        else goto name_type_error;

        // Store the welcome message
        if ( p_welcome_message->type == JSON_VALUE_STRING )
        {

            // Initialized data
            char   *welcome_message     = p_welcome_message->string;
            size_t  welcome_message_len = strlen(welcome_message);

            // Error check
            if ( welcome_message_len > MAX_SERVER_WELCOME_MESSAGE_LEN )
            {

                // Store the maximum name length
                welcome_message_len = MAX_SERVER_WELCOME_MESSAGE_LEN;

                // Warn the user
                printf("[chat room] Truncating out of bounds server welcome message in call to function \"%s\"\n", __FUNCTION__);
            }

            // Copy the name
            strncpy(p_chat_room_server->welcome_message, welcome_message, welcome_message_len);

        }
        // Default
        else goto welcome_message_type_error;

        // Store the port number
        if ( p_port->type == JSON_VALUE_INTEGER ) p_chat_room_server->port_number = (unsigned short) p_port->integer;
        // Default
        else goto port_type_error;

        // Store the max user quantity
        if ( p_max_users->type == JSON_VALUE_INTEGER ) p_chat_room_server->max_users = (unsigned short) p_max_users->integer;
        // Default
        else goto max_users_type_error;
    }

    // Construct a TCP socket
    if ( socket_tcp_create( &p_chat_room_server->_socket_tcp, socket_address_family_ipv4, p_chat_room_server->port_number ) == 0 ) goto failed_to_construct_tcp_socket;

    // Construct a dict for clients who have signed on
    dict_construct(&p_chat_room_server->p_clients, p_chat_room_server->max_users, 0);

    // Construct a queue for clients who may sign on
    queue_construct(&p_chat_room_server->p_maybe_clients);

    // Construct a queue for clients who have signed on
    queue_construct(&p_chat_room_server->p_def_clients);

    // Return a pointer to the client
    *pp_chat_room_server = p_chat_room_server;

    // Success
    return 1;

    // Error handling
    {

        // Argument errors
        {
            no_path:
                #ifndef NDEBUG
                    printf("[server] Null pointer provided for parameter \"path\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // JSON errors
        {
            failed_to_parse_json_value:
                #ifndef NDEBUG
                    printf("[chat room] Failed to parse JSON text in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;

            missing_properties:
                #ifndef NDEBUG
                    printf("[chat room] Missing properties in call to function \"%s\"\n", __FUNCTION__);
                #endif
                
                // Error
                return 0;

            name_type_error:
                #ifndef NDEBUG
                    printf("[chat room] \"name\" property must be of type < STRING > in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;

            welcome_message_type_error:
                #ifndef NDEBUG
                    printf("[chat room] \"welcome message\" property must be of type < STRING > in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;

            port_type_error:
                #ifndef NDEBUG
                    printf("[chat room] \"port\" property must be of type < INTEGER > in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
            
            max_users_type_error:
                #ifndef NDEBUG
                    printf("[chat room] \"max users\" property must be of type < INTEGER > in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // socket errors
        {
            failed_to_construct_tcp_socket:
                #ifndef NDEBUG
                    printf("[chat room] Failed to construct TCP socket in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // Standard library errors
        {
            failed_to_load_file:
                #ifndef NDEBUG
                    printf("[server] Failed to load config file in call to function \"%s\"\n");
                #endif

                // Error
                return 0;
        }
    }
}

int chat_room_server_start ( chat_room_server *const p_chat_room_server )
{

    chat_room_client *_clients[255] = {0};

    // Flip the running bit
    p_chat_room_server->running = true;

    // Run
    while ( p_chat_room_server->running )
    {

        // Get a new client
        socket_tcp_listen(p_chat_room_server->_socket_tcp, &chat_room_accept, p_chat_room_server);

        // Process new connections
        if ( queue_empty(p_chat_room_server->p_maybe_clients) == false )
        {
            
            // For each possible new connection
            while ( queue_empty(p_chat_room_server->p_maybe_clients) == false )
            {

                // Uninitialized data
                char _message[255];

                // Initialized data
                chat_room_client *p_client = 0;

                // Get a client
                queue_dequeue(p_chat_room_server->p_maybe_clients, &p_client);
                                
                // Try to get the sign in json object
                if ( socket_tcp_receive(p_client->_socket_tcp, _message, 255 ) )
                {
                    
                    // Initialized data
                    json_value *p_value = 0;

                    // Parse the JSON object
                    (void) parse_json_value(_message, 0, &p_value);

                    // Parse the properties
                    if ( p_value->type == JSON_VALUE_OBJECT )
                    {

                        // Initialized data
                        dict *p_dict = p_value->object;

                        strcpy(p_client->name, ((json_value *)dict_get(p_dict, "name"))->string);

                        p_client->signed_on = true;

                        free_json_value(p_value);

                        // Add the client to the dictionary
                        dict_add(p_chat_room_server->p_clients, p_client->name, p_client);

                        // Increment the user count
                        p_chat_room_server->user_count++;

                        // Log
                        printf("%s joined the chatroom\n", p_client->name);

                        while ( socket_tcp_receive(p_client->_socket_tcp, _message, 255 ));

                        socket_tcp_send(p_client->_socket_tcp, p_chat_room_server->welcome_message, strlen(p_chat_room_server->welcome_message));

                        continue;
                    }
                    
                }

                // Still waiting on the client
                queue_enqueue(p_chat_room_server->p_def_clients, p_client);
            }
            
            while ( queue_empty(p_chat_room_server->p_def_clients) == false )
            {
                
                // Initialized data
                chat_room_client *p_client = 0;

                queue_dequeue(p_chat_room_server->p_def_clients, &p_client);
                queue_enqueue(p_chat_room_server->p_maybe_clients, p_client);
            }
        }

        // Get each user
        dict_values(p_chat_room_server->p_clients, _clients);

        // Iterate over each user
        for (size_t i = 0; i < p_chat_room_server->user_count; i++)
        {

            // Uninitialized data
            char _message[255];
            char send_message[512];
            
            memset(_message, '\0', 255);

            // Initialized data
            chat_room_client *p_client = _clients[i];
            
            // Maybe get a message from the user
            if ( socket_tcp_receive(p_client->_socket_tcp, _message, 255 ) )
            {

                if ( strncmp(_message, "/quit", 5) == 0 )
                {

                    printf("%s left\n", p_client->name);
                    socket_tcp_destroy(&p_client->_socket_tcp);
                    continue;
                }

                size_t msg_len = 0;

                while (_message[msg_len] != '\0' && _message[msg_len] != '\n' && _message[msg_len] != '\r' )
                {
                    msg_len++;
                }
                
                for (size_t j = 0; j < p_chat_room_server->user_count; j++)
                {

                    // Prevent echo
                    if (strcmp(_clients[j]->name, p_client->name) == 0 ) 
                    {
                        // Send the message
                        int n = snprintf(send_message, 512, "\r%s > \0", _clients[j]->name);
                        socket_tcp_send(_clients[j]->_socket_tcp, send_message, n);
                        continue;
                    };

                    int n = snprintf(send_message, 512, "\r%s > %s\r%s > \0",p_client->name, _message, _clients[j]->name);

                    // Send the message
                    socket_tcp_send(_clients[j]->_socket_tcp, send_message, n);
                }

                while ( socket_tcp_receive(p_client->_socket_tcp, _message, 255 ));
            }
        }
        

    }

    // Success
    return 1;
}

int chat_room_accept ( socket_tcp _socket_tcp, unsigned long ip_address, unsigned short port, chat_room_server *const p_chat_room_server )
{

    // Initialized data
    chat_room_client *p_client = calloc(1, sizeof(chat_room_client));

    // Store a client struct
    *p_client = (chat_room_client) 
    {
        ._socket_tcp = _socket_tcp,
        .port_number = port,
        .ip_address  = ip_address
    };
    
    // Log the new user
    printf("[anon] connected from %d.%d.%d.%d:%d\n", 
        (p_client->ip_address & 0xFF000000) >> 24,
        (p_client->ip_address & 0x00FF0000) >> 16,
        (p_client->ip_address & 0x0000FF00) >> 8,
        (p_client->ip_address & 0x000000FF),
        p_client->port_number
    );

    fflush(stdout);

    // Add the client to the client queue
    queue_enqueue(p_chat_room_server->p_maybe_clients, p_client);

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

/*

            

            
            bad_request:
            ;



*/