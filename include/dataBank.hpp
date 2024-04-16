#include "boolStorage.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#pragma once

/// @brief A high storage dynamic array made for delegating storage to StorageAccessors. No
/// Only allocations are tracked, so shrinking the array is not possible.
template <size_t N>
class DataBank {
    struct StorageData {
        std::shared_ptr<BoolStorage> storage;
        // The offset of the first free bit, ranging from 0 to STORAGE_SIZE
        // If all bits are used, this will be STORAGE_SIZE
        size_t free_bit_offset;
    };
    std::vector<std::array<StorageData, N>> storage;

public:
    /// @brief  Lends N BoolStorageAccessors from the storage.
    /// They are guarenteed to be at the same offset and bit_count.
    /// @param bit_count
    /// @return
    std::array<BoolStorageAccessor, N> lendBools(size_t bit_count)
    {
        if (bit_count > STORAGE_SIZE) {
            throw std::runtime_error("Requested bit_count exceeds STORAGE_SIZE");
        }
        // find first storage block with enough space
        for (std::array<StorageData, N>& storage_block : storage) {
            if (storage_block[0].free_bit_offset + bit_count <= STORAGE_SIZE) {

                std::array<BoolStorageAccessor, N> accessors;

                for (size_t i = 0; i < N; i++) {
                    accessors[i] = BoolStorageAccessor(storage_block[i].free_bit_offset, bit_count, storage_block[i].storage);
                }

                for (size_t i = 0; i < N; i++) {
                    storage_block[i].free_bit_offset += bit_count;
                }
                return accessors;
            }
        }

        // if no storage block was found, create a new one
        std::array<StorageData, N> new_storage_block;
        std::array<BoolStorageAccessor, N> accessors;
        for (size_t i = 0; i < N; i++) {
            new_storage_block[i].storage = std::make_shared<BoolStorage>();
            new_storage_block[i].free_bit_offset = bit_count;
            accessors[i] = BoolStorageAccessor(0, bit_count, new_storage_block[i].storage);
        }
        storage.emplace_back(new_storage_block);
        return accessors;
    }

    std::array<std::weak_ptr<BoolStorage>, N> lendStorage()
    {
        std::array<StorageData, N> new_storage_block;
        std::array<std::weak_ptr<BoolStorage>, N> accessors;
        for (size_t i = 0; i < N; i++) {
            new_storage_block[i].storage = std::make_shared<BoolStorage>();
            // mark all bits as used since we are lending the whole storage
            new_storage_block[i].free_bit_offset = STORAGE_SIZE;
            accessors[i] = new_storage_block[i].storage;
        }
        storage.emplace_back(new_storage_block);
        return accessors;
    }

    /// @brief Clear all bits in the storage, not the storage itself
    void clear()
    {
        for (std::array<StorageData, N>& storage_block : storage) {
            for (size_t i = 0; i < N; i++) {
                StorageData& data = storage_block[i];
                BoolStorage& storage = *data.storage;
                storage.reset();
            }
        }
    }

    std::vector<std::array<std::weak_ptr<BoolStorage>, N>> getStorage()
    {
        std::vector<std::array<std::weak_ptr<BoolStorage>, N>> result;
        result.reserve(storage.size());
        for (std::array<StorageData, N>& storage_block : storage) {
            std::array<std::weak_ptr<BoolStorage>, N> block;
            for (size_t i = 0; i < N; i++) {
                block[i] = storage_block[i].storage;
            }
            result.push_back(block);
        }
        return result;
    };
};