#include "pch.hpp"
#include "NameChanged.hpp"

void on::NameChanged(ENetEvent& event) 
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    send_varlist(event.peer, {
        "OnNameChanged",
        std::format("{}", pPeer->display_growid)
    }, pPeer->netid);
}