#pragma once

class database_config 
{
public:
    std::string host{"127.0.0.1"};
    std::string user{"root"};
    std::string passwd{NULL};

    void init();
};
extern ::database_config gDb_config; // @note db for short, "database" looked long and ugly. (still looks ugly LMAO)
