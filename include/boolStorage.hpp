#include <bitset>
#include <cstddef>
#include <memory>
#include <stdexcept>
#pragma once

constexpr size_t STORAGE_SIZE = 64;

using BoolStorage = std::bitset<STORAGE_SIZE>;

/// the smallest unit of data a socket can carry
// using BoolStorage = std::bitset<STORAGE_SIZE>;
class MarkableBoolStorage {
    std::bitset<STORAGE_SIZE> data;
    bool dirty = false;

public:
    MarkableBoolStorage(const std::bitset<STORAGE_SIZE>& data)
        : data(data)
        , dirty(false)
    {
    }
    MarkableBoolStorage(std::bitset<STORAGE_SIZE>&& data)
        : data(data)
        , dirty(false)
    {
    }
    MarkableBoolStorage(size_t data)
        : data(data)
        , dirty(false)
    {
    }
    MarkableBoolStorage()
        : data(0)
        , dirty(false)
    {
    }
    MarkableBoolStorage(const MarkableBoolStorage& other)
        : data(other.data)
        , dirty(other.dirty)
    {
    }
    MarkableBoolStorage& operator=(const MarkableBoolStorage& other)
    {
        data = other.data;
        dirty = other.dirty;
        return *this;
    }
    MarkableBoolStorage& operator=(MarkableBoolStorage&& other)
    {
        data = other.data;
        dirty = other.dirty;
        return *this;
    }
    MarkableBoolStorage(MarkableBoolStorage&& other)
        : data(other.data)
        , dirty(other.dirty)
    {
    }
    MarkableBoolStorage operator&(const MarkableBoolStorage& other) const { return MarkableBoolStorage(std::move(data & other.data)); }
    MarkableBoolStorage operator|(const MarkableBoolStorage& other) const { return MarkableBoolStorage(data | other.data); }
    MarkableBoolStorage operator^(const MarkableBoolStorage& other) const { return MarkableBoolStorage(data ^ other.data); }
    MarkableBoolStorage operator~() const { return MarkableBoolStorage(~data); }
    MarkableBoolStorage& operator&=(const MarkableBoolStorage& other)
    {
        data &= other.data;
        return *this;
    }
    MarkableBoolStorage& operator|=(const MarkableBoolStorage& other)
    {
        data |= other.data;
        return *this;
    }
    MarkableBoolStorage& operator^=(const MarkableBoolStorage& other)
    {
        data ^= other.data;
        return *this;
    }
    bool operator==(const MarkableBoolStorage& other) const { return data == other.data; }
    bool operator!=(const MarkableBoolStorage& other) const { return data != other.data; }
    MarkableBoolStorage& reset()
    {
        data.reset();
        return *this;
    }
    MarkableBoolStorage operator<<(size_t shift) const { return MarkableBoolStorage(data << shift); }
    MarkableBoolStorage operator>>(size_t shift) const { return MarkableBoolStorage(data >> shift); }

    unsigned long to_ulong() const
    {
        return data.to_ulong();
    }
    unsigned long long to_ullong() const
    {
        return data.to_ullong();
    }

    bool operator[](size_t index) const { return data[index]; }
    bool test(size_t index) const { return data.test(index); }
    bool any() const { return data.any(); }
    bool none() const { return data.none(); }
    bool all() const { return data.all(); }
    size_t count() const { return data.count(); }
    size_t size() const { return data.size(); }
    MarkableBoolStorage& set(size_t index, bool value)
    {
        data.set(index, value);
        return *this;
    }
    MarkableBoolStorage& set()
    {
        data.set();
        return *this;
    }
    MarkableBoolStorage& reset(size_t index)
    {
        data.reset(index);
        return *this;
    }

    MarkableBoolStorage& flip()
    {
        data.flip();
        return *this;
    }
    MarkableBoolStorage& flip(size_t index)
    {
        data.flip(index);
        return *this;
    }
    void markDirty()
    {
        dirty = true;
    }
    void markClean()
    {
        dirty = false;
    }
    bool isDirty() const
    {
        return dirty;
    }
};

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
