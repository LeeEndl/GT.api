#include "pch.hpp"
#include "action/quit.hpp"

#include "disconnect.hpp"

void disconnect(ENetEvent& event, ::gamePacket gamePacket) 
{
    action::quit(event, "");
}