/** !
 * Include header for chat room server
 * 
 * @file json/json.h 
 * 
 * @author Jacob Smith
 */

// Include guard
#pragma once

// Standard library
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>

// sync submodule
#include <sync/sync.h>

// socket submodule
#include <socket/socket.h>

// dict submodule
#include <dict/dict.h>

// array submodule
#include <array/array.h>

// queue submodule
#include <queue/queue.h>

// json submodule
#include <json/json.h>

// Platform dependent macros
#ifdef _WIN64
#define DLLEXPORT extern __declspec(dllexport)
#else
#define DLLEXPORT
#endif

// Set the reallocator for the dict submodule
#ifdef DICT_REALLOC
    #undef DICT_REALLOC
    #define DICT_REALLOC(p, sz) realloc(p, sz)
#endif

// Set the reallocator for the array submodule
#ifdef ARRAY_REALLOC
    #undef ARRAY_REALLOC
    #define ARRAY_REALLOC(p, sz) realloc(p, sz)
#endif

// Set the reallocator for the json submodule
#ifdef JSON_REALLOC
    #undef JSON_REALLOC
    #define JSON_REALLOC(p, sz) realloc(p, sz)
#endif

// Enumerations
enum chat_room_commands_e
{
    CHAT_ROOM_CONNECT,
    CHAT_ROOM_SEND,
    CHAT_ROOM_DISCONNECT
};

// Forward declarations
struct chat_room_server_s;
struct chat_room_client_s;

// Structures
struct chat_room_command_s
{
    union {
        struct { char *name; } connect;
        struct { char *text; } message;
        struct { int   code; } disconnect;
    };

    enum chat_room_commands_e type;
};

// Type definitions
typedef struct chat_room_command_s chat_room_command;
typedef struct chat_room_server_s  chat_room_server;
typedef struct chat_room_client_s  chat_room_client;
