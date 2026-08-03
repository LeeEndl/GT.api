#include "pch.hpp"
#include "tools/ransuu.hpp"
#include "tools/time.hpp"
#include "on/ConsoleMessage.hpp"

#include "world.hpp"

u_char get_type(const ::item &item)
{
    switch (item.type)
    {
        case type::MAIN_DOOR: case type::DOOR: case type::PORTAL: return 0x01;
        case type::SIGN: case type::MAILBOX: return 0x02;
        case type::LOCK: return 0x03;
        case type::SEED: return 0x04;
        case type::RANDOM: return 0x08;
        case type::PROVIDER: return 0x09;
        case type::DISPLAY_BLOCK: return 0x17;
        case type::VENDING_MACHINE: return 0x18;
    }
    return 0x00;
}

::blob block::to_blob() const
{
    blob blob;
    blob.i16(this->fg);
    blob.i16(this->bg);
    blob.u8(this->state[0]);
    blob.u8(this->state[1]);
    blob.u8(this->state[2]);
    blob.u8(this->state[3]);

    return blob;
}

::blob object::to_blob() const
{
    blob blob;
    blob.i16(this->id);
    blob.f32(this->pos.x);
    blob.f32(this->pos.y);
    blob.i16(this->count);
    blob.i32(this->uid);

    return blob;
}

::blob tree::to_blob(bool seconds) const
{
    blob blob;
    blob.i32((seconds) ? ticks() - this->tick : this->tick);
    blob.u8(this->fruit);

    return blob;
}

std::vector<u_char> world::serialize()
{
    blob blob;
    blob.i16(0x00); // @todo my rgt world says: 19 00
    blob.i32(0x00); // @todo my rgt world says: 40 00 00 00
    blob.i16(this->name.length());
    for (char c : this->name) blob.u8(c);

    const int y = this->blocks.size() / 100;
    const int x = this->blocks.size() / y;
    blob.i32(x);
    blob.i32(y);
    blob.i16(this->blocks.size());

    /*@todo*/
    blob.i32(0x00);
    blob.i16(0x00);
    blob.u8(0x00);

    for (u_short i = 0; const ::block &block : this->blocks)
    {
        blob.push_back(block.to_blob());

        if (block.fg != 0) // @note so we can save time
        if (u_char type = get_type(id_to_item(block.fg)); type > 0x00)
        {
            blob.u8(type);

            const ::pos block_pos{i % x, i / x};
            if (type == 0x01/*doors, portal*/ || type == 0x02/*sign, mailbox*/)
            {
                if (block.fg == 6/*Main Door*/) this->spawn = block_pos.by_32(false);

                blob.i16(block.label.length());
                for (char c : block.label) blob.u8(c);

                if (type == 0x01) blob.u8('\0'); // @note terminator which Growtopia requires.
                if (type == 0x02) blob.i32(0xffffffff); // @todo understand this better...
            }
            else if (type == 0x04/*seed*/)
            {
                auto tree = std::ranges::find(this->trees, block_pos, &::tree::pos);
                if (tree != this->trees.end())
                {
                    blob.push_back(tree->to_blob(true));
                }
            }
        }
        ++i;
    }
    /*@todo*/
    blob.i32(0x00);
    blob.i32(0x00);
    blob.i32(0x00);

    blob.i32(this->last_object_uid);
    blob.i32(this->last_object_uid);
    for (const ::object &object : this->objects) 
    {
        blob.push_back(object.to_blob());
    }
    return blob.data();
}

bool world::exists(const std::string& name)
{
    ::hStmt hStmt{ "SELECT 1 FROM world WHERE name = ? LIMIT 1" };

    MYSQL_BIND param = make_bind_in(name);
    hStmt.bind_and_execute(&param);

    return (!mysql_stmt_store_result(hStmt.pStmt) && mysql_stmt_num_rows(hStmt.pStmt) > 0);
}

template<typename T>
void world::mysql_insert(const std::string& column, const T& value)
{
    ::hStmt hStmt{ std::format("INSERT INTO world ({}) VALUES (?)", column).c_str() };

    MYSQL_BIND param = make_bind_in(value);
    hStmt.bind_and_execute(&param);
}
template void world::mysql_insert<signed>(const std::string&, const signed&);
template void world::mysql_insert<unsigned>(const std::string&, const unsigned&);
template void world::mysql_insert<float>(const std::string&, const float&);
template void world::mysql_insert<std::string>(const std::string&, const std::string&);
template void world::mysql_insert<std::vector<u_char>>(const std::string&, const std::vector<u_char>&);

template<typename T>
void world::mysql_update(const std::string& column, const T& value)
{
    ::hStmt hStmt{ std::format("UPDATE world SET {} = ? WHERE name = ?", column).c_str() };

    MYSQL_BIND params[2] = {
        make_bind_in(value),      // SET
        make_bind_in(this->name) // WHERE
    };
    hStmt.bind_and_execute(params);
}
template void world::mysql_update<signed>(const std::string&, const signed&);
template void world::mysql_update<unsigned>(const std::string&, const unsigned&);
template void world::mysql_update<float>(const std::string&, const float&);
template void world::mysql_update<std::string>(const std::string&, const std::string&);
template void world::mysql_update<std::vector<u_char>>(const std::string&, const std::vector<u_char>&);

template<typename T>
T world::mysql_select(const std::string &column, const std::string &arg)
{
    T value{};
    ::hStmt hStmt{ std::format("SELECT {}({}) FROM world WHERE name = ? LIMIT 1", arg, column).c_str() };

    MYSQL_BIND param = make_bind_in(this->name);
    mysql_stmt_bind_param(hStmt.pStmt, &param);

    u_long length = 0;
    MYSQL_BIND result = make_bind_out(value);
    result.length = &length;
    mysql_stmt_bind_result(hStmt.pStmt, &result);

    mysql_stmt_execute(hStmt.pStmt);
    mysql_stmt_fetch(hStmt.pStmt);
    
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<u_char>>)
        value.resize(length);

    return value;
}

void world::mysql_select_all()
{
    this->name = this->mysql_select<std::string>("name");
    {
        this->trees.clear();
        auto blob = this->mysql_select<std::vector<u_char>>("blocks");
        this->blocks.resize(cord(0, 60));

        const int x = this->blocks.size() / 60;
        const u_char *u8 = blob.data(); // @note i did not have the brain capacity to reinterpret it. t-t
        int pos{};
        for (u_short i = 0; ::block &block : this->blocks)
        {
            memcpy(&block.fg, u8 + pos, sizeof(short)); pos += sizeof(short);
            memcpy(&block.bg, u8 + pos, sizeof(short)); pos += sizeof(short);
            block.state[0] = u8[pos++];
            block.state[1] = u8[pos++];
            block.state[2] = u8[pos++];
            block.state[3] = u8[pos++];
            if (block.fg != 0) // @note so we can save time
            if (u_char type = get_type(id_to_item(block.fg)); type > 0x00)
            {
                const ::pos block_pos{i % x, i / x};
                if (type == 0x01/*doors, portal*/ || type == 0x02/*sign, mailbox*/)
                {
                    short len{};
                    memcpy(&len, u8 + pos, sizeof(short)); pos += sizeof(short);

                    block.label.resize(len);
                    for (char &c : block.label) c = u8[pos++];
                }
                if (type == 0x04/*seed*/)
                {
                    auto &tree = this->trees.emplace_back(0, 0, block_pos);

                    memcpy(&tree.tick, u8 + pos, sizeof(int)); pos += sizeof(int);
                    tree.fruit = u8[pos++];
                }
            }
            ++i;
        }
    } // @note delete blob, i
    {
        auto blob = this->mysql_select<std::vector<u_char>>("objects");

        const u_char *u8 = blob.data(); // @note i did not have the brain capacity to reinterpret it. t-t (memcpy is safer anyways...)
        int i{};
        memcpy(&this->last_object_uid, u8, sizeof(u_int)); i += sizeof(u_int); // @todo real gt has this as 8 bits not just 4.

        objects.resize(this->last_object_uid);
        for (::object &object : this->objects)
        {
            memcpy(&object.id,    u8 + i, sizeof(u_short)); i += sizeof(u_short);
            memcpy(&object.pos.x, u8 + i, sizeof(float));   i += sizeof(float);
            memcpy(&object.pos.y, u8 + i, sizeof(float));   i += sizeof(float);
            memcpy(&object.count, u8 + i, sizeof(u_short)); i += sizeof(u_short);
            memcpy(&object.uid,   u8 + i, sizeof(u_int));   i += sizeof(u_int);
        }
    } // @note delete blob, i
}

world::world(const std::string &name) 
{
    this->name = name;

    if (this->exists(this->name)) 
    {
        this->mysql_select_all();
    }
    else 
    {
        this->mysql_insert("name", this->name); // @note DEFAULT
        generate_world(*this);
    }
}
world::~world()
{
    this->mysql_update("name", this->name);
    {
        ::blob blob;

        const int x = this->blocks.size() / 60;
        for (u_short i = 0; const ::block &block : this->blocks)
        {
            blob.push_back(block.to_blob());

            if (block.fg != 0) // @note so we can save time
            if (u_char type = get_type(id_to_item(block.fg)); type > 0x00)
            {
                const ::pos block_pos{i % x, i / x};
                if (type == 0x01/*doors, portal*/ || type == 0x02/*sign, mailbox*/)
                {
                    blob.i16(block.label.length());
                    for (char c : block.label) blob.u8(c);
                }
                else if (type == 0x04/*seed*/)
                {
                    auto tree = std::ranges::find(this->trees, block_pos, &::tree::pos);
                    if (tree != this->trees.end())
                    {
                        blob.push_back(tree->to_blob());
                    }
                }
            }
            ++i;
        }
        this->mysql_update("blocks", blob.data());
    }
    {
        ::blob blob{};

        blob.i32(this->last_object_uid); // @todo add the other 4 bits like real growtopia
        for (::object &object : this->objects)
        {
            blob.push_back(object.to_blob());
        }
        this->mysql_update("objects", blob.data());
    }
}

std::vector<world> worlds;

void send_action(ENetPeer& p, const std::string &action, const std::string &str) 
{
    const std::string &fmt_action = std::format("action|{}\n", action);
    std::vector<u_char> data(sizeof(int) + fmt_action.length() + str.length(), 0x00);
    
    data[0] = 03; // @note NET_MESSAGE_GAME_MESSAGE
    {
        const u_char *i8 = reinterpret_cast<const u_char*>(fmt_action.c_str());
        for (std::size_t i = 0ull; i < fmt_action.length(); ++i)
            data[sizeof(int) + i] = i8[i];
    }
    if (!str.empty())
    {
        const u_char *i8 = reinterpret_cast<const u_char*>(str.c_str());
        for (std::size_t i = 0ull; i < str.length(); ++i)
            data[sizeof(int) + fmt_action.length() + i] = i8[i];
    }
    
    enet_peer_send(&p, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}

void send_data(ENetPeer &peer, const std::vector<u_char> &&data)
{
    ENetPacket *packet = enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    if (packet == nullptr || packet->dataLength < sizeof(::state)) return;

    enet_peer_send(&peer, 1, packet);
}

void state_visuals(ENetPeer &peer, state &&state) 
{
    ::peer *pPeer = static_cast<::peer*>(peer.data);

    peers(pPeer->recent_worlds.back(), PEER_SAME_WORLD, [&](ENetPeer &p) 
    {
        send_data(p, compress_state(state));
    });
}

void tile_apply_damage(ENetEvent &event, state state, block &block, u_int value)
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    (block.fg == 0) ? ++block.hits[1] : ++block.hits[0];
    state.type = (value << 24) | 0x000008; // @note 0x{}000008
    state.id = 6; // @note idk exactly
    state.netid = pPeer->netid;
	state_visuals(*event.peer, std::move(state));
}

u_short modify_item_inventory(ENetEvent &event, ::slot slot)
{   
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    ::state state{.id = slot.id};
    if (slot.count < 0) state.type = (slot.count*-1 << 16) | 0x000d; // @noote 0x00{}000d
    else                state.type = (slot.count    << 24) | 0x000d; // @noote 0x{}00000d
    state_visuals(*event.peer, std::move(state));

    return pPeer->emplace(::slot(slot.id, slot.count));
}

void item_change_object(ENetEvent &event, ::state state) 
{
    state.type = 0x0e; // @note PACKET_ITEM_CHANGE_OBJECT

    state_visuals(*event.peer, std::move(state));
}

void merge_object(ENetEvent &event, ::slot slot, const ::pos &pos, ::world &world)
{
    auto object = std::ranges::find_if(world.objects, [&](const ::object &object) {
        return object.id == slot.id && (object.pos.by_32(true) == pos.by_32(true));
    });
    /* @todo avoid surpassing 200 and call add_object() for the remaining amount */ // @note future self reference peer::emplace()...
    object->count += slot.count;

    item_change_object(event, ::state{
        .netid = (int)0xfffffffd,
        .uid   = (int)object->uid,
        .count = static_cast<float>(object->count),
        .id    = object->id,
        .pos   = object->pos
    });
}

void remove_object(ENetEvent& event, signed uid)
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    item_change_object(event, ::state{
        .netid = pPeer->netid,
        .uid   = (int)0xffffffff,
        .id    = uid
    });
}

int add_object(ENetEvent& event, ::slot slot, const ::pos& pos, ::world &world)
{
    /* @todo got a little messy */
    auto object = std::ranges::find_if(world.objects, [&](const ::object &object) {
        return object.id == slot.id && (object.pos.by_32(true) == pos.by_32(true));
    });
    if (object != world.objects.end() && object->count < 200/*@todo*/) 
    {
        merge_object(event, slot, pos, world);
        return object->uid;
    }
    ::object it = world.objects.emplace_back(::object(slot.id, slot.count, pos, ++world.last_object_uid)); // @note a iterator ahead of time

    item_change_object(event, ::state{
        .netid = (int)0xffffffff,
        .uid   = (int)it.uid,
        .count = static_cast<float>(slot.count),
        .id    = it.id,
        .pos   = pos
    });
    return it.uid;
}

void add_drop(ENetEvent &event, ::slot im, ::pos pos, ::world &world) // @todo
{
    ransuu ransuu;
    add_object(event, im, ::pos{
        pos.x + ransuu[{0, 16}],
        pos.y + ransuu[{0, 16}]
    }, world);
}

void send_tile_update(ENetEvent &event, ::state state, ::block &block, ::world &world) 
{
    state.type = 05; // @note PACKET_SEND_TILE_UPDATE_DATA
    state.peer_state = peer_state::S_EXTENDED;
    std::vector<u_char> data = compress_state(state);

    short pos = sizeof(::state); // @note start after state bytes (as every packet has)
    data.resize(pos + 99ull); // @todo fix later

    for (const u_char u8 : block.to_blob().data())
        data[pos++] = u8;

    const ::item &item = id_to_item(block.fg);
    data[pos++] = get_type(item);
    switch (item.type)
    {
        case type::LOCK:
        {
            if (!is_tile_lock(block.fg)) world.is_public = (block.state[2] & S_PUBLIC); // @note check if world lock has S_PUBLIC flag, i will change this later

            int access = std::ranges::count_if(world.access, std::identity{});
            data.resize(data.size() + 1ull + 4ull + 4ull + 4ull + (access * 4));

            data[pos++] = world.lock_state;
            *reinterpret_cast<int*>(&data[pos]) = world.owner; pos += sizeof(int);
            *reinterpret_cast<int*>(&data[pos]) = access; pos += sizeof(int);
            /* @todo access list */
            break;
        }
        case type::SEED:
        {
            auto tree = std::ranges::find(world.trees, state.punch, &::tree::pos);
            if (tree != world.trees.end())
            {
                for (const u_char u8 : tree->to_blob(true).data())
                    data[pos++] = u8;
            }
            break;
        }
        case DISPLAY_BLOCK:
        {
            data.resize(pos + 4ull);

            auto display = std::ranges::find(world.displays, state.punch, &::display::pos);

            *reinterpret_cast<int*>(&data[pos]) = display->id; pos += sizeof(int);
            break;
        }
    }

    ::peer *pPeer = static_cast<::peer*>(event.peer->data);
    peers(pPeer->recent_worlds.back(), PEER_SAME_WORLD, [&](ENetPeer& p) 
    {
        send_data(p, std::move(data));
    });
}

void send_particle_effect(ENetEvent &event, const ::pos& pos, ::pos speed, int id, float offset)
{
    state_visuals(*event.peer, ::state{
        .type = 0x11, // @note PACKET_SEND_PARTICLE_EFFECT
        .netid = id, // @todo figure out if this is correct, i just assumed from firework visuals
        .id = id,
        .pos = pos,
        .speed = speed,
        .idk = offset
    });
}

void remove_fire(ENetEvent &event, state state, ::block &block, ::world &world)
{
    send_particle_effect(event, state.punch.by_32(), {0x00, 0x95});

    block.state[3] &= ~S_FIRE;
    send_tile_update(event, state, block, world);

    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    if (++pPeer->fires_removed % 100 == 0) 
    {
        on::ConsoleMessage(event.peer, "`oI'm so good at fighting fires, I rescused this `2Highly Combustible Box``!");
        modify_item_inventory(event, {3090/*Combustible Box*/, 1});
    }
    pPeer->add_xp(event, 1);
}

void fireworks(ENetEvent &event, const ::pos &pos)
{
    ransuu ransuu;
    int type  [3]{ ransuu[{0x25, 0x28}], ransuu[{0x25, 0x28}], ransuu[{0x25, 0x28}] };
    int offset[3]{ ransuu[{260, 2200}], ransuu[{260, 2200}], ransuu[{260, 2200}] };

    send_particle_effect(event, pos, {0xb3, type[0]}, 0xc8*0, offset[0]);
    send_particle_effect(event, pos, {0xbe, type[1]}, 0xc8*1, offset[1]);
    send_particle_effect(event, pos, {0x7c, type[2]}, 0xc8*2, offset[2]);
}

void generate_world(::world &world)
{
    ransuu ransuu;
    u_short main_door = ransuu[{2, cord(0, 60) / 100 - 4}];
    std::vector<::block> blocks(cord(0, 60), ::block{0, 0});
    
    for (std::size_t i = 0ull; i < blocks.size(); ++i)
    {
        ::block &block = blocks[i];
        if (i >= cord(0, 37))
        {
            block.bg = 14; // @note cave background
            if (i >= cord(0, 38) && i < cord(0, 50) /* (above) lava level */ && ransuu[{0, 38}] <= 1) block.fg = 10; // rock
            else if (i > cord(0, 50) && i < cord(0, 54) /* (above) bedrock level */ && ransuu[{0, 8}] < 3) block.fg = 4; // lava
            else block.fg = (i >= cord(0, 54)) ? 8 : 2;
        }
        if (i == cord(main_door, 36)) block.fg = 6, block.label = "EXIT", block.state[3] = 1; // @note main door
        else if (i == cord(main_door, 37)) block.fg = 8; // @note bedrock (below main door)
    }
    world.blocks = std::move(blocks);
}

bool door_mover(::world &world, const ::pos &pos)
{
    std::vector<::block> &blocks = world.blocks;

    if (blocks[cord(pos.x, pos.y)].fg != 0 ||
        blocks[cord(pos.x, (pos.y + 1))].fg != 0) return false;

    for (std::size_t i = 0ull; i < blocks.size(); ++i)
    {
        if (blocks[i].fg == 6/*Main Door*/)
        {
            blocks[i].fg = 0; // @note remove main door
            blocks[cord(i % 100, (i / 100 + 1))].fg = 0; // @note remove bedrock below
            break;
        }
    }
    blocks[cord(pos.x, pos.y)].fg = 6;
    blocks[cord(pos.x, (pos.y + 1))].fg = 8;
    return true;
}

void blast::thermonuclear(::world &world, const std::string &name)
{
    ransuu ransuu;

    const u_short main_door = ransuu[{2, cord(0, 60) / 100 - 4}];
    std::vector<::block> blocks(cord(0, 60), ::block{0, 0});
    for (std::size_t i = 0ull; i < blocks.size(); ++i)
    {
        blocks[i].fg = (i >= cord(0, 54)) ? 8 : 0;

        if (i == cord(main_door, 36)) blocks[i].fg = 6; // @note main door
        else if (i == cord(main_door, 37)) blocks[i].fg = 8; // @note bedrock (below main door)
    }
    world.blocks = std::move(blocks);
    world.name = std::move(name);
}
