#pragma once

class shouhin
{
public:
    std::string btn{};
    std::string name{};
    std::string rttx{};
    std::string description{};
    char tex1{};
    char tex2{}; // @todo may have to change variable type···
    int cost{};

    std::vector<std::pair<short, short>> items{}; // @note {id, amount}
}; 
extern std::vector<std::pair<short, shouhin>> shouhin_tachi; // @note {tab, shouhin}

extern void parse_store();
