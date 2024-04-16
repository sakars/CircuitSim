#include "boolStorage.hpp"
#include "dataBank.hpp"
#include "socketController.hpp"
#include "wireBridge.hpp"

#pragma once

class NotGate {
    DataBank<2> db;

public:
    std::array<BoolStorageAccessor, 2> lendGate()
    {
        return db.lendBools(1);
    }

    void tick()
    {
        // Iterate over the data bank's storage and perform the AND operation on each storage
        auto storage_arr = db.getStorage();
        for (auto& storage : storage_arr) {
            auto [a, b] = storage;
            auto a_lock = a.lock();
            auto b_lock = b.lock();
            if (!a_lock || !b_lock) {
                throw std::runtime_error("Failed to lock storage");
            }
            *b_lock = ~*a_lock;
        }
    }
};
