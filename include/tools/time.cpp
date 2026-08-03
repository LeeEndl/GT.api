#include "pch.hpp"

#include "time.hpp"

u_int ticks()
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u_int)(ts.tv_sec);
}
