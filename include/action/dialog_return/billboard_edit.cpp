#include "pch.hpp"

#include "onVariant/BillboardChange.hpp"

#include "billboard_edit.hpp"

void billboard_edit(ENetEvent& event, const ::hPipe &hPipe)
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    pPeer->billboard.id = atoi(hPipe["billboard_item"].c_str());
    pPeer->billboard.show = atoi(hPipe["billboard_toggle"].c_str());
    pPeer->billboard.isBuying = atoi(hPipe["billboard_buying_toggle"].c_str());
    pPeer->billboard.perItem = atoi(hPipe["chk_peritem"].c_str());
    pPeer->billboard.price = atoi(hPipe["setprice"].c_str());

    on::BillboardChange(event);
}