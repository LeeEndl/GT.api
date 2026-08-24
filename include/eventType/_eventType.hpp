#pragma once

#include <unordered_map>

extern std::unordered_map<ENetEventType, std::function<void(ENetEvent&)>> eventType_pool;
