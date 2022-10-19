#include <iostream>
#include <regex>
#include <boost/program_options.hpp>
#include <vector>
#include <variant>

class ConnectionBridge;
class UdpConnection;
class TcpConnection;

#include "../../common/include/common_structs.h"
#include "../../common/include/buffer.h"
#include "../include/connection_bridge.h"
#include "../include/connection_handlers/udp_connection.h"
#include "../include/connection_handlers/tcp_connection.h"

namespace po = boost::program_options;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

namespace {
    struct address {
        std::string combined;
        std::string ip;
        std::string port;
    };

    struct Args {
        struct address gui_address;
        struct address server_address;
        std::string player_name;
        uint16_t port{};
    };

    void parse_address(struct address *addr) {
        static std::regex addr_regex("^(.*):(.*)$");
        std::smatch matches;
        std::regex_search(addr->combined, matches, addr_regex);

        addr->ip = matches[1];
        addr->port = matches[2];
    }

    // TODO validator for unsigned value (port)
    struct Args parse_arguments(int argc, char **argv) {
        struct Args program_args;
        po::options_description desc("Program options");
        desc.add_options()
            ("help,h", "produce help message")
            ("gui-address,d", po::value<std::string>(&program_args.gui_address.combined)->required(), "gui address")
            ("player-name,n", po::value<std::string>(&program_args.player_name)->required(), "player name")
            ("port,p", po::value<uint16_t>(&program_args.port)->required(), "port")
            ("server-address,s", po::value<std::string>(&program_args.server_address.combined)->required(), "server address");

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

        parse_address(&program_args.gui_address);
        parse_address(&program_args.server_address);

        return program_args;
    }
}

int main(int argc, char **argv) {
    struct Args program_args = parse_arguments(argc, argv);

    try {
        boost::asio::io_context io_context;
        UdpConnection gui_connection(io_context, program_args.gui_address.ip, program_args.gui_address.port, program_args.port);
        TcpConnection server_connection(io_context, program_args.server_address.ip, program_args.server_address.port);
        ConnectionBridge bridge(program_args.player_name, gui_connection, server_connection);
        io_context.run();
        if (bridge.disconnected()) {
            exit(1);
        }
    }
    catch(std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        exit(1);
    }

    return 0;
}
