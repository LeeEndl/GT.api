#include "pch.hpp"
#include <fstream>

#include "database_config.hpp"

::database_config gDb_config{};

void ::database_config::init()
{
    std::ifstream istrm("mysql_login.txt");
    if (!istrm.is_open())
    {
        std::ofstream ostrm("mysql_login.txt");
        ostrm << 
            std::format(
                "host|{}\n"
                "user|{}\n"
                "password|{}",
                this->host, this->user, this->passwd
            );
    } // @note close ostrm
    else
    {
        for (std::string line; std::getline(istrm, line); ) 
        {
            ::hPipe hPipe{ line };

            if (!hPipe["host"].empty()) this->host = hPipe["host"];
            else if (!hPipe["user"].empty()) this->user = hPipe["user"];
            else if (!hPipe["password"].empty()) this->passwd = hPipe["password"];
        }
    } // @note delete pipes
}