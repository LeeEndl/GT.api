#include "pch.hpp"
#include "onVariant/SetClothing.hpp"
#include "ghost.hpp"

void ghost(ENetEvent& event, const std::string_view text)
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    pPeer->state ^= S_GHOST;

    on::SetClothing(*event.peer);
}