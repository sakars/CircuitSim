#include "socketController.hpp"

void SocketController::addSocket(BoolStorageAccessor from, BoolStorageAccessor to)
{
    if (from.getSocketSize() != to.getSocketSize()) {
        throw std::runtime_error("Socket size mismatch");
    }

    sockets.emplace_back(Socket { from, to });
}

void SocketController::tick()
{
    // clear the to buffer of each socket
    for (auto& socket : sockets) {
        // socket.preTick();
    }

    // add the from buffer to the to buffer of each socket
    for (auto& socket : sockets) {
        socket.tick();
    }
}