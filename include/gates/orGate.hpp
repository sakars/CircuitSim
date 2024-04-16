#include "boolStorage.hpp"
#include "dataBank.hpp"
#include "socketController.hpp"
#include "wireBridge.hpp"

#pragma once

class OrGate {
    DataBank<3> db;

public:
    std::array<BoolStorageAccessor, 3> lendGate()
    {
        return db.lendBools(1);
    }

    void tick()
    {
        // Iterate over the data bank's storage and perform the AND operation on each storage
        auto storage_arr = db.getStorage();
        for (auto& storage : storage_arr) {
            auto [a, b, c] = storage;
            auto a_lock = a.lock();
            auto b_lock = b.lock();
            auto c_lock = c.lock();
            if (!a_lock || !b_lock || !c_lock) {
                throw std::runtime_error("Failed to lock storage");
            }
            *c_lock = *a_lock | *b_lock;
        }
    }
};
