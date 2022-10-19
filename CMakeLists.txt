cmake_minimum_required(VERSION 3.16.3)
project(bomberman)

set(CMAKE_CXX_COMPILER /opt/gcc-11.2/bin/g++-11.2)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/opt/gcc-11.2/lib64")

set(THREADS_PREFER_PTHREAD_FLAG ON)

add_compile_options(-std=gnu++20 -Wall -Wextra -Wconversion -Werror -O2)

find_package(Boost 1.71 COMPONENTS program_options system REQUIRED)
find_package(Threads REQUIRED)

set(Client-Sources
        client/src/main.cpp
        client/src/connection_bridge.cpp
        client/include/structs/parser.h
        client/src/structs/udp_structs.cpp
        client/src/structs/tcp_structs.cpp
        client/src/structs/client_structs.cpp
        client/src/connection_handlers/udp_connection.cpp
        client/src/connection_handlers/tcp_connection.cpp
        common/src/buffer.cpp
        common/src/basic_structs.cpp
)

add_executable(robots-client ${Client-Sources})

set(Server-Sources
        server/src/main.cpp
        server/src/game.cpp
        server/src/structs/server_structs.cpp
        server/src/structs/tcp_structs.cpp
        server/include/structs/parser.h
        server/src/connection_handlers/tcp_server.cpp
        server/src/connection_handlers/tcp_connection.cpp
        common/src/buffer.cpp
        common/src/basic_structs.cpp
)

add_executable(robots-server ${Server-Sources})

target_link_libraries(robots-client Boost::program_options Boost::system Threads::Threads)
target_link_libraries(robots-server Boost::program_options Boost::system Threads::Threads)
