#include "include\database\items.hpp"
#include "include\network\enet.hpp"
#include "include\database\sqlite3.hpp"
#include "include\database\peer.hpp"
#include "include\network\packet.hpp"
#include "include\database\world.hpp"
#include "include\tools\string_view.hpp"
#include "include\tools\random_engine.hpp"

#include "include\action\actions"
#include <future> /* std::async & return future T */

int main() 
{
    void github_sync(const char* commit); // -> import github.o
    github_sync("0d8e04d4cdaf5449672d47bc80b22d5747b93f82");
    enet_initialize();
    {
        ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};

        int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o
        server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 1, 0, 0);
            server->checksum = enet_crc32;
            enet_host_compress_with_range_coder(server);
    } /* deletes address */
    {
        struct _iobuf* file;
        if (fopen_s(&file, "items.dat", "rb") == 0) 
        {
            fseek(file, 0, SEEK_END);
            im_data.resize(ftell(file) + 60);
            for (int i = 0; i < 5; ++i)
                *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) = std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
            *reinterpret_cast<int*>(im_data.data() + 56) = ftell(file);
            long end_size = ftell(file); /* SEEK_END it */
            fseek(file, 0, SEEK_SET);
            fread(im_data.data() + 60, 1, end_size, file);
            std::span span{reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size()};
                hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
        } /* deletes span, deletes end_size */
        fclose(file);
    }
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0) /* waits 1 millisecond. it's a good pratice in C++ to always have a small timer for loops cause C++ is so damn fast */
            switch (event.type) 
            {
                case ENET_EVENT_TYPE_CONNECT:
                    if (peers(ENET_PEER_STATE_CONNECTING).size() > 2)
                        packet(*event.peer, "action|log\nmsg|`4OOPS:`` Too many people logging in at once. Please press `5CANCEL`` and try again in a few seconds."),
                        enet_peer_disconnect_later(event.peer, ENET_NORMAL_DISCONNECTION);
                    else if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[4]){0x1}, 4, ENET_PACKET_FLAG_RELIABLE)) == 0)
                    event.peer->data = new peer{};
                    break;
                case ENET_EVENT_TYPE_DISCONNECT: /* if peer closes growtopia.exe */
                {
                    quit(event, "action|quit"); /* treat closing growtopia.exe as a action|quit */
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: 
                {
                    switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                    {
                        case 2: case 3: 
                        {
                            std::span packet{event.packet->data, event.packet->dataLength};
                                std::string header{packet.begin() + 4, packet.end() - 1};
                            std::ranges::replace(header, '\n', '|');
                            std::vector<std::string> pipes = readpipe(header);
                            const std::string action{(pipes[0] == "requestedName" or pipes[0] == "tankIDName") ? pipes[0] : pipes[0] + "|" + pipes[1]};
                            if (command_pool.contains(action))
                                (static_cast<void>(std::async(std::launch::async, command_pool[std::move(action)], std::ref(event), std::move(header))));
                            break;
                        }
                        case 4: 
                        {
                            std::unique_ptr<state> state{};
                            {
                                std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x00});
                                if ((packet.size() + 4) >= 60)
                                    for (size_t i = 0; i < packet.size(); ++i)
                                        packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) break;
                                state = get_state(packet);
                            } /* deletes packet ahead of time */
                            switch (state->type) 
                            {
                                case 0: 
                                {
                                    if (getpeer->post_enter.try_lock()) 
                                    {
                                        gt_packet(*event.peer, 0, true, "OnSetPos", floats{getpeer->pos[0], getpeer->pos[1]});
                                        gt_packet(*event.peer, 0, true, "OnChangeSkin", -1429995521);
                                    }
                                    getpeer->pos = state->pos;
                                    getpeer->facing_left = state->peer_state bitand 0x10;
                                    state_visuals(event, *state);
                                    break;
                                }
                                case 3: 
                                {
                                    if (create_rt(event, 0, 200ms)); /* this will only affect hackers (or macro spammers) */
                                    short block1D = state->punch[1] * 100 + state->punch[0]; /* 2D (x, y) to 1D ((destY * y + destX)) formula */
                                    auto w = std::make_unique<world>(worlds[getpeer->recent_worlds.back()]);
                                    if (state->id == 18) /* punching blocks */
                                    {
                                        // ... TODO add a timer that resets hits every 6-8 seconds (threaded stopwatch)
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].bg == 0 and worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 0) break;
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 8 or worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 6) {
                                            gt_packet(*event.peer, 0, false, "OnTalkBubble", getpeer->netid, worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 8 ? 
                                                "It's too strong to break." : "(stand over and punch to use)");
                                            break;
                                        }
                                        block_punched(event, *state, block1D);
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].fg not_eq 0) 
                                        {
                                            if (worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[0] < items[worlds[getpeer->recent_worlds.back()].blocks[block1D].fg].hits) break;
                                            else worlds[getpeer->recent_worlds.back()].blocks[block1D].fg = 0;
                                        }
                                        else if (worlds[getpeer->recent_worlds.back()].blocks[block1D].bg not_eq 0)
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[1] < items[worlds[getpeer->recent_worlds.back()].blocks[block1D].bg].hits) break;
                                        else worlds[getpeer->recent_worlds.back()].blocks[block1D].bg = 0;
                                    }
                                    else /* placing blocks */
                                    {
                                        (int{items[state->id].type} == 18) ? 
                                            worlds[getpeer->recent_worlds.back()].blocks[block1D].bg = state->id : 
                                            worlds[getpeer->recent_worlds.back()].blocks[block1D].fg = state->id;
                                    }
                                    overwrite_tile(w, block1D, worlds[getpeer->recent_worlds.back()].blocks[block1D]);
                                    state_visuals(event, *state);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    enet_packet_destroy(event.packet); /* cleanup */
                    break;
                }
            }
    return 0;
}
