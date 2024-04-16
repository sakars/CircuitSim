#include <bitset>
#include <cstddef>
#include <memory>
#include <stdexcept>
#pragma once

constexpr size_t STORAGE_SIZE = 64;

/// the smallest unit of data a socket can carry
using BoolStorage = std::bitset<STORAGE_SIZE>;

const auto ONES = BoolStorage(0).flip();

/// @brief A class that provides access to a subset of bits in a BoolStorage
/// without having to copy the bits.
class BoolStorageAccessor {
    /// @brief The offset of the bits in the buffer
    size_t bit_offset;
    /// @brief The amount of bits this accerssor has control over
    size_t socket_size;
    /// @brief Reference to the shared buffer
    std::weak_ptr<BoolStorage> buffer_ref;

public:
    /// @brief Construct a new BoolStorageAccessor object
    /// @param bit_offset The offset of the bits in the buffer
    /// @param socket_size The amount of bits this accerssor has control over
    /// @param buffer Reference to the shared buffer
    BoolStorageAccessor(size_t bit_offset, size_t socket_size, std::weak_ptr<BoolStorage> buffer)
        : bit_offset(bit_offset)
        , socket_size(socket_size)
        , buffer_ref(buffer)
    {
        if (bit_offset + socket_size > STORAGE_SIZE) {
            throw std::runtime_error("Accessor out of bounds");
        }
        // If bit_offset or socket_size are close to MAX_SIZE, the sum could overflow
        // For that rare case, we need to check if the individual values are already out of bounds
        if (bit_offset > STORAGE_SIZE || socket_size > STORAGE_SIZE) {
            throw std::runtime_error("Accessor out of bounds");
        }
    }

    /// @brief Construct a new empty BoolStorageAccessor object
    BoolStorageAccessor()
        : bit_offset(0)
        , socket_size(0)
        , buffer_ref(std::weak_ptr<BoolStorage>())
    {
    }

    /// @brief Get the bits associated with this accessor
    BoolStorage get() const;

    /// @brief Set the bits associated with this accessor
    /// @param value The value to set
    void set(BoolStorage value);

    /// @brief Clear the bits (set to 0) associated with this accessor
    void clear();

    /// @brief Add (bitwise OR) a value to the buffer
    /// @param value The value to add
    void add(BoolStorage value);

    /// @brief "Multiply" (bitwise AND) a value to the buffer
    /// @param value The value to multiply
    void multiply(BoolStorage value);

    /// @brief Get the bit offset
    size_t getBitOffset() const { return bit_offset; }

    /// @brief Get the socket size
    size_t getSocketSize() const { return socket_size; }
};
