#include <iostream>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <variant>

#include "../include/structs/server_structs.h"
#include "../include/connection_handlers/tcp_server.h"

namespace po = boost::program_options;
using boost::asio::ip::tcp;

namespace {

    struct Args parse_arguments(int argc, char **argv) {
        struct Args program_args;
        po::options_description desc("Program options");
        desc.add_options()
                ("help,h", "produce help message")
                ("bomb-timer,b", po::value<uint16_t>(&program_args.bomb_timer)->required(), "bomb_timer")
                ("players-count,c", po::value<uint16_t>(&program_args.players_count_u16)->required(), "players_count")
                ("turn-duration,d", po::value<uint64_t>(&program_args.turn_duration)->required(), "turn_duration")
                ("explosion-radius,e", po::value<uint16_t>(&program_args.explosion_radius)->required(), "explosion_radius")
                ("initial-blocks,k", po::value<uint16_t>(&program_args.initial_blocks)->required(), "initial_blocks")
                ("game-length,l", po::value<uint16_t>(&program_args.game_length)->required(), "game_length")
                ("server-name,n", po::value<std::string>(&program_args.server_name)->required(), "server_name")
                ("port,p", po::value<uint16_t>(&program_args.port)->required(), "port")
                ("seed,s", po::value<uint32_t>(&program_args.seed), "seed")
                ("size-x,x", po::value<uint16_t>(&program_args.size_x)->required(), "size_x")
                ("size-y,y", po::value<uint16_t>(&program_args.size_y)->required(), "size_y");

        try {
            po::variables_map vm;
            po::store(po::parse_command_line(argc, argv, desc), vm);
            if (vm.count("help")) {
                std::cout << desc << std::endl;
                exit(0);
            }

            po::notify(vm);
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        if (program_args.players_count_u16 == 0) {
            std::cout << "players_count should be > 0.\n";
            exit(1);
        }

        if (program_args.bomb_timer == 0) {
            std::cout << "bomb_timer should be > 0.\n";
            exit(1);
        }

        if (program_args.turn_duration == 0) {
            std::cout << "turn duration should be > 0.\n";
            exit(1);
        }

        if (program_args.size_x == 0 || program_args.size_y == 0) {
            std::cout << "Dimensions of the board should be > 0.\n";
            exit(1);
        }

        if (program_args.game_length == 0) {
            std::cout << "game_length should be > 0.\n";
            exit(1);
        }

        program_args.players_count = static_cast<uint8_t>(program_args.players_count_u16);
        return program_args;
    }
}

int main(int argc, char **argv) {
    struct Args program_args = parse_arguments(argc, argv);

    try {
        boost::asio::io_context io_context;
        TcpServer server(io_context, program_args.port, program_args);
        io_context.run();
    }
    catch(...) {
        exit(1);
    }
    return 0;
}
