#include "pch.hpp"

#include "action/quit_to_exit.hpp" // @note peer leave world
#include "action/join_request.hpp" // @note peer enter (blast) world

#include "create_blast.hpp"

void create_blast(ENetEvent& event, const ::hPipe &hPipe)
{
    const int id = atoi(hPipe["id"].c_str());
    std::string world_name = hPipe["name"];

    for (char &c : world_name) c = std::toupper(c); // @note start -> START
    
    switch (id)
    {
        case 1402: // @note Thermonuclear Blast
        {
            auto it = std::ranges::find(worlds, world_name, &::world::name);
            if (it == worlds.end()) 
            {
                ::world world{world_name}; // @note we don't need emplace into worlds since no one is in the world yet!

                action::quit_to_exit(event, "", true);
                blast::thermonuclear(world);
            } // @note save the world and continue in action::join_request. seems tedious so i might change later.
            else
            {
                /* @todo prompt saying world already exists. */
                return;
            }
            break;
        }
        default: return;
    }
    action::join_request(event, "", world_name);
}