#include "pch.hpp"
#include "on/EmoticonDataChanged.hpp"
#include "on/Spawn.hpp"
#include "on/BillboardChange.hpp"
#include "on/SetClothing.hpp"
#include "on/CountryState.hpp"
#include "on/ConsoleMessage.hpp"
#include "commands/weather.hpp"
#include "tools/ransuu.hpp"
#include "tools/time.hpp"

#include "join_request.hpp"

void action::join_request(ENetEvent& event, const std::string& header, const std::string_view world_name = "") 
{
    try 
    {
        ::peer *pPeer = static_cast<::peer*>(event.peer->data);

        std::string big_name{header.size() < 2 ? world_name : readch(header, '|')[3]};
        if (!alnum(big_name)) throw std::runtime_error("Sorry, spaces and special characters are not allowed in world or door names.  Try again.");
        std::for_each(big_name.begin(), big_name.end(), [](char& c) { c = std::toupper(c); }); // @note start -> START
        
        auto it = std::ranges::find(worlds, big_name, &::world::name);
        if (it == worlds.end()) 
            it = worlds.emplace(worlds.end(), big_name);
            
        ::world &world = *it;
        {
            ::blob blob = compress_state(::state{ .type = 0x04, /*PACKET_SEND_MAP_DATA*/ .peer_state = peer_state::S_EXTENDED });
            blob.push_back(world.serialize());

            enet_peer_send(event.peer, 0, enet_packet_create(blob.data().data(), blob.size(), ENET_PACKET_FLAG_RELIABLE));
        } // @note delete data
        {
            std::string *w_name = std::ranges::find(pPeer->recent_worlds, world.name);
            std::string *first = w_name != pPeer->recent_worlds.end() ? w_name : pPeer->recent_worlds.begin();

            std::rotate(first, first + 1, pPeer->recent_worlds.end());
            pPeer->recent_worlds.back() = world.name;
        } // @note delete name, first
        on::EmoticonDataChanged(event);

        if (!pPeer->role)
            pPeer->prefix.front() = 
                (pPeer->user_id == world.owner) ? '2' : 
                (std::ranges::find(world.access, pPeer->user_id) != world.access.end()) ? 'c' : 
                pPeer->prefix.front(); // @note keeps the existing prefix

        pPeer->rest_pos = world.spawn;

        pPeer->netid = ++world.netid_counter;
        peers(pPeer->recent_worlds.back(), PEER_SAME_WORLD, [&event, &pPeer, &world](ENetPeer& peer/*send to everyone in world*/) 
        {
            ::peer *pOthers = static_cast<::peer*>(peer.data); // @note everyone in world's peer.data
            
            if (pOthers->user_id != pPeer->user_id)
            {
                on::Spawn(*event.peer, pOthers->netid, pOthers->user_id, pOthers->pos, std::format("`{}{}", pOthers->prefix, pOthers->growid), pOthers->country, pOthers->role, pOthers->role >= DEVELOPER, false);
                on::Spawn(peer, pPeer->netid, pPeer->user_id, pPeer->rest_pos, std::format("`{}{}", pPeer->prefix, pPeer->growid), pPeer->country, pPeer->role, pPeer->role >= DEVELOPER, false);
                on::SetClothing(peer);
                on::ConsoleMessage(&peer, std::format("`5<`{}{}`` entered, `w{}`` others here>``", pPeer->prefix, pPeer->growid, world.visitors));
            }
            

            if (pOthers->user_id != pPeer->user_id) // @note the reason this is here is cause we need the peer's OnSpawn to happen before OnTalkBubble
            {
                send_varlist(&peer, {
                    "OnTalkBubble",
                    pPeer->netid,
                    std::format("`5<`{}{}`` entered, `w{}`` others here>``", pPeer->prefix, pPeer->growid, world.visitors),
                    1u
                });
            }
        });
        on::Spawn(*event.peer, pPeer->netid, pPeer->user_id, pPeer->rest_pos, std::format("`{}{}", pPeer->prefix, pPeer->growid), pPeer->country, pPeer->role, pPeer->role >= DEVELOPER, true);

        if (pPeer->billboard.id != 0) on::BillboardChange(event); // @note don't waste memory if billboard is empty.

        send_varlist(event.peer, {
            "OnSetPos", 
            CL_Vec2f{pPeer->rest_pos.x, pPeer->rest_pos.y}
        }, pPeer->netid);

        on::ConsoleMessage(event.peer, 
            std::format(
                "World `w{}`` entered.  There are `w{}`` other people here, `w{}`` online.", 
                world.name, world.visitors, peers().size()
            )
        );
        ++world.visitors;
        on::SetClothing(*event.peer);
        on::CountryState(event);
    }
    catch (const std::exception& exc)
    {
        send_varlist(event.peer, { "OnFailedToEnterWorld" });
        return;
    }
}