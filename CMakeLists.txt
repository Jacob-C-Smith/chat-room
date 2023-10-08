# Need CMake 3.20
cmake_minimum_required (VERSION 3.20)

# This is the name of the repository
project ("json")

# Set the correct paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/extern/)

# Comment out for Debug mode
set(IS_DEBUG_BUILD CMAKE_BUILD_TYPE STREQUAL "Debug")

# Set for debug mode
if (${IS_DEBUG_BUILD})
    add_compile_definitions(NDEBUG)
endif ()

# Find the array module
if ( NOT "${HAS_ARRAY}")
    
    # Log
    message("[chat-room] Adding array project")
    
    # Add the array directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/array ${CMAKE_SOURCE_DIR}/build/array)

    # Let any parent project know that this project has array
    set(HAS_ARRAY true PARENT_SCOPE)
endif()

# Find the dict module
if ( NOT "${HAS_DICT}")
    
    # Log
    message("[chat-room] Adding dict project")
    
    # Add the dict directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/dict ${CMAKE_SOURCE_DIR}/build/dict)

    # Let any parent project know that this project has dict
    set(HAS_DICT true PARENT_SCOPE)
endif()

# Find the sync module
if ( NOT "${HAS_SYNC}")

    # Log
    message("[chat-room] Adding sync project")

    # Add the sync directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/sync ${CMAKE_SOURCE_DIR}/build/sync)

    # Let any parent project know that this project has sync
    set(HAS_SYNC true PARENT_SCOPE)
endif()

# Find the socket module
if ( NOT "${HAS_SOCKET}")

    # Log
    message("[chat-room] Adding socket project")

    # Add the socket directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/socket ${CMAKE_SOURCE_DIR}/build/socket)

    # Let any parent project know that this project has socket
    set(HAS_SOCKET true PARENT_SCOPE)
endif()

# Find the queue module
if ( NOT "${HAS_QUEUE}")

    # Log
    message("[chat-room] Adding queue project")

    # Add the queue directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/queue ${CMAKE_SOURCE_DIR}/build/queue)

    # Let any parent project know that this project has queue
    set(HAS_QUEUE true PARENT_SCOPE)
endif()

# Find the json module
if ( NOT "${HAS_JSON}")

    # Log
    message("[chat-room] Adding json project")

    # Add the json directory
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/json ${CMAKE_SOURCE_DIR}/build/json)

    # Let any parent project know that this project has json
    set(HAS_JSON true PARENT_SCOPE)
endif()

# Find the chat-room module
if ( NOT "${HAS_CHAT_ROOM}")

    # Log
    message("[chat-room] Adding chat-room project")

    # Let any parent project know that it has the chat project
    set(HAS_CHAT_ROOM true PARENT_SCOPE)
endif()

# Add source to this project's executables
add_executable (chat_room_server "server-main.c" "server.c" "client.c")
add_dependencies(chat_room_server json queue array dict sync crypto socket)
target_include_directories(chat_room_server PUBLIC include include/chat-room ${CMAKE_SOURCE_DIR}/extern/array/include/ ${CMAKE_SOURCE_DIR}/extern/dict/include/ ${CMAKE_SOURCE_DIR}/extern/sync/include/ ${CMAKE_SOURCE_DIR}/extern/json/include/ ${CMAKE_SOURCE_DIR}/extern/socket/include/ ${CMAKE_SOURCE_DIR}/extern/queue/include/)
target_link_libraries(chat_room_server json queue array dict sync crypto socket)