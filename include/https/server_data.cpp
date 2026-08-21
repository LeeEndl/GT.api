#include "pch.hpp"
#include <fstream>

#include "server_data.hpp"

::server_data gServer_data{};

void ::server_data::init()
{
    std::ifstream file("server_data.php");
    if (!file.is_open())
    {
        std::ofstream write("server_data.php");
        write << 
            std::format(
                "server|{}\n"
                "port|{}\n"
                "type|{}\n"
                "type2|{}\n"
                "#maint|{}\n"
                "loginurl|{}\n"
                "meta|{}\n"
                "RTENDMARKERBS1001", 
                this->server, this->port, this->type, this->type2, this->maint, this->loginurl, this->meta
            );
    } // @note close write
    else
    {
        std::vector<std::string> pipes;
        for (std::string line; std::getline(file, line); ) 
        {
            auto pipe_pair = readch(line, '|');
            pipes.insert(pipes.end(), pipe_pair.begin(), pipe_pair.end());
        }

        this->server = pipes[1];
        this->port = std::stoi(pipes[3]);
        this->type = std::stoi(pipes[5]);
        this->type2 = std::stoi(pipes[7]);
        this->maint = pipes[9];
        this->loginurl = pipes[11];
        this->meta = pipes[13];
    } // @note delete str, pipes
} // @note close file
