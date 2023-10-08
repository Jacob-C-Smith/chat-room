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

        // Error checking
        if ( p_name == 0 ) goto missing_properties;
    }

    // Allocate memory for the server
    p_chat_room_server = calloc(1, sizeof(chat_room_server));
    
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

    // Construct a TCP socket
    if ( socket_tcp_create( &p_chat_room_server->_socket_tcp, socket_address_family_ipv4, p_chat_room_server->port_number ) == 0 ) goto failed_to_construct_tcp_socket;

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

    // Initialized data
    //
    

    // Success
    return 1;
}