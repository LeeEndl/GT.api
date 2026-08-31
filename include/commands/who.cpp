#include "pch.hpp"

#include "who.hpp"

void who(ENetEvent& event, const std::string_view text) 
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    std::vector<std::string> names;
    peers(pPeer->recent_worlds.back(), PEER_SAME_WORLD, [&pPeer, event, &names](ENetPeer& peer)
    {
        ::peer *pOthers = static_cast<::peer*>(peer.data);
        
        if (pOthers->user_id != pPeer->user_id)
        {
            send_varlist(event.peer, { "OnTalkBubble", pOthers->netid, pOthers->display_growid.c_str(), 1u });
        }
        names.emplace_back(pOthers->display_growid);
    });
    send_action(*event.peer, "log", std::format(
        "msg|`wWho's in `${}``:`` {}``",
        pPeer->recent_worlds.back(), join(names, ", ")
    ));
}
