#include "pch.hpp"
#include "onVariant/RequestGazette.hpp"

#include "news.hpp"

void news(ENetEvent& event, const std::string_view text)
{
    on::RequestGazette(event);
}
