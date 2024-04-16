#include "boolStorage.hpp"
#include <vector>

#pragma once

class SocketController {
public:
    class Socket {
        BoolStorageAccessor from;
        BoolStorageAccessor to;

    public:
        Socket(BoolStorageAccessor from, BoolStorageAccessor to)
            : from(from)
            , to(to)
        {
        }

        void preTick()
        {
            to.clear();
        }
        void tick()
        {
            to.set(from.get());
        }
    };

private:
    std::vector<Socket> sockets;

public:
    void addSocket(BoolStorageAccessor from, BoolStorageAccessor to);
    void tick();
};