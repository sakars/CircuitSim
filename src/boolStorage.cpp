
#include "boolStorage.hpp"
#include <iostream>

BoolStorage BoolStorageAccessor::get() const
{
    auto buffer = buffer_ref.lock();
    if (buffer) {
        const auto mask = (ONES << socket_size).flip() << bit_offset;
        return (mask & *buffer) >> bit_offset;
    }
    // std::cerr << "BufferAccessor dereference failed\n";
    // return 0;
    throw std::runtime_error("BufferAccessor dereference failed");
}

void BoolStorageAccessor::set(BoolStorage value)
{
    auto buffer = buffer_ref.lock();
    if (buffer) {
        // std::cout << "Buffer:\n"
        //           << *buffer_lock << "\n";
        // std::cout << "Value:\n"
        //           << value << "\n";
        const auto mask = ((ONES << socket_size).flip() << bit_offset).flip();
        // std::cout << "Mask:\n"
        //           << mask << "\n";
        const auto shifted = (value & (ONES >> (STORAGE_SIZE - socket_size))) << bit_offset;
        // std::cout << "Shifted:\n"
        //           << shifted << "\n";
        *buffer &= mask;
        *buffer |= shifted;
        // buffer->markDirty();
        //  std::cout << "Buffer:\n"
        //            << *buffer_lock << "\n";
        return;
    }
    throw std::runtime_error("BufferAccessor dereference failed");
}

void BoolStorageAccessor::clear()
{
    auto buffer = buffer_ref.lock();
    if (buffer) {
        const auto mask = ((ONES << socket_size).flip() << bit_offset).flip();
        *buffer &= mask;
        // buffer->markDirty();
        return;
    }
    throw std::runtime_error("BufferAccessor dereference failed");
}

void BoolStorageAccessor::add(BoolStorage value)
{
    auto buffer = buffer_ref.lock();
    if (buffer) {
        const auto shifted = (value & (ONES >> (STORAGE_SIZE - socket_size))) << bit_offset;
        *buffer |= shifted;
        // buffer->markDirty();
        return;
    }
    throw std::runtime_error("BufferAccessor dereference failed");
}

void BoolStorageAccessor::multiply(BoolStorage value)
{
    auto buffer = buffer_ref.lock();
    if (buffer) {
        const auto shifted = (value & (ONES >> (STORAGE_SIZE - socket_size))) << bit_offset;
        *buffer &= shifted;
        // buffer->markDirty();
        return;
    }
    throw std::runtime_error("BufferAccessor dereference failed");
}