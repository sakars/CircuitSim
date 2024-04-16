#pragma once
#include "dataBank.hpp"
#include "gates/andGate.hpp"
#include "gates/notGate.hpp"
#include "gates/orGate.hpp"
#include "gates/xorGate.hpp"
#include "socketController.hpp"

struct Managers {
    std::shared_ptr<DataBank<1>> random_access_data_bank = std::make_shared<DataBank<1>>();
    std::shared_ptr<AndGate> andGate = std::make_shared<AndGate>();
    std::shared_ptr<NotGate> notGate = std::make_shared<NotGate>();
    std::shared_ptr<OrGate> orGate = std::make_shared<OrGate>();
    std::shared_ptr<XorGate> xorGate = std::make_shared<XorGate>();
    std::shared_ptr<SocketController> socketController = std::make_shared<SocketController>();

    void tick()
    {
        andGate->tick();
        notGate->tick();
        orGate->tick();
        xorGate->tick();
        socketController->tick();
    }
};