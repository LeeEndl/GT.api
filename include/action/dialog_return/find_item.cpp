#include "pch.hpp"

#include "onVariant/SetClothing.hpp"

#include "find_item.hpp"

void find_item(ENetEvent& event, const ::hPipe &hPipe)
{
    std::string id = readch(hPipe["buttonClicked"], '_')[1]; // e.g. searchableItemListButton_2_0_-1
    if (id.empty()) return;
    
    modify_item_inventory(event, ::slot(atoi(id.c_str()), 200));
}