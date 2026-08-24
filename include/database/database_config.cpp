#include "pch.hpp"
#include <fstream>

#include "database_config.hpp"

::database_config gDb_config{};

void ::database_config::init()
{
    std::ifstream file("mysql_login.txt");
    if (!file.is_open())
    {
        std::ofstream write("mysql_login.txt");
        write << 
            std::format(
                /* @note this is read via std::getline() into readch(), pipe-delimited format */
                "host|{}\n"
                "user|{}\n"
                "password|{}\n",
                this->host, this->user, this->passwd
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
        this->host   = pipes[1];
        this->user   = pipes[3];
        this->passwd = pipes[5];
    } // @note delete pipes
}