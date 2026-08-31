#include "pch.hpp"
#include "socialportal.hpp"

void socialportal(ENetEvent& event, const ::hPipe &hPipe)
{
    ::peer *pPeer = static_cast<::peer*>(event.peer->data);

    if (hPipe["buttonClicked"] == "showfriend")
    {
        send_varlist(event.peer, {
            "OnDialogRequest",
                "set_default_color|`o\n"
                "add_label_with_icon|big|0 of 0 `wFriends Online``|left|1366|\n"
                "add_spacer|small|\n"
                "add_textbox|`oNone of your friends are currently online.``|left|\n"
                "add_spacer|small|\n"
                "add_spacer|small|\n"
                "add_button|friend_all|Show offline and ignored too|noflags|0|0|\n"
                "add_button|all_friends|Edit Friends|noflags|0|0|\n"
                "add_button|friends_options|Friend Options|noflags|0|0|\n"
                "add_button|back|Back|noflags|0|0|\n"
                "add_button||Close|noflags|0|0|\n"
                "end_dialog|friends|||\n"
                "add_quick_exit|\n"
        });
    }
}