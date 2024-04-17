#include <catch2/catch_amalgamated.hpp>

#include "boolStorage.hpp"

TEST_CASE("BoolStorage is mutable", "[boolStorage]")
{
    BoolStorage bs;
    for (size_t i = 0; i < 20; i++) {
        size_t idx = rand() % STORAGE_SIZE;
        bs.set(idx, true);
        REQUIRE(bs[idx]);
        bs.set(idx, false);
        REQUIRE_FALSE(bs[idx]);
    }
}

TEST_CASE("BoolStorageAccessor is a class", "[boolStorageAccessor]")
{
    BoolStorageAccessor bsa;
    REQUIRE(sizeof(bsa) > 0);
}

TEST_CASE("BoolStorageAccessor can be constructed", "[boolStorageAccessor]")
{
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 0, bs);
    REQUIRE(bsa.get() == 0);
}

TEST_CASE("BoolStorageAccessor can set at the correct place", "[boolStorageAccessor]")
{
    for (size_t i = 0; i < STORAGE_SIZE - 6; i++) {
        INFO("Bit offset is = " << i);
        auto bs = std::make_shared<BoolStorage>(0);
        BoolStorageAccessor bsa(i, 6, bs);
        bsa.set(0b101010);
        REQUIRE(bs->to_ullong() >> i == 0b101010);
        REQUIRE(bs->to_ullong() == 0b101010ULL << i);
    }
}

TEST_CASE("BoolStorageAccessor set doesn't interfere with other bits", "[boolStorageAccessor]")
{
    unsigned long long v = 0;
    for (size_t i = 0; i < STORAGE_SIZE / 16; i++) {
        v ^= (rand() % 0xFFFF) << (i * 16);
    }
    auto bs = std::make_shared<BoolStorage>(v);
    CAPTURE(v);
    CAPTURE(*bs);
    auto offsets = GENERATE(0, 21, 43, 50);
    BoolStorageAccessor bsa(offsets, 6, bs);
    bsa.set(0b101010);
    CAPTURE(*bs);
    auto mask = ~(0b111111ULL << offsets);
    CAPTURE(mask);
    REQUIRE((bs->to_ullong() & mask) == (v & mask));
    REQUIRE(((bs->to_ullong() >> offsets) & 0b111111ULL) == 0b101010);
}

TEST_CASE("BoolStorageAccessor clears correct amount", "[boolStorageAccessor]")
{
    for (size_t n = 0; n < 32; n++) {
        for (size_t i = 0; i < STORAGE_SIZE - n; i++) {
            INFO("Bit offset is = " << i);
            auto bs = std::make_shared<BoolStorage>(ONES);
            BoolStorageAccessor bsa(i, n, bs);
            bsa.clear();
            REQUIRE(bs->count() == STORAGE_SIZE - n);
        }
    }
}

TEST_CASE("BoolStorageAccessor clears correct bits", "[boolStorageAccessor]")
{
    for (size_t n = 0; n < 32; n++) {
        for (size_t i = 0; i < STORAGE_SIZE - n; i++) {
            INFO("Bit offset is = " << i);
            auto bs = std::make_shared<BoolStorage>(ONES);
            BoolStorageAccessor bsa(i, n, bs);
            bsa.clear();
            unsigned long long mask = ~(((1ULL << n) - 1) << i);
            REQUIRE(bs->to_ullong() == mask);
        }
    }
}

TEST_CASE("BoolStorageAccessor add works", "[boolStorageAccessor]")
{
    auto a = GENERATE(take(10, random(0UL, (1UL << 8) - 1)));
    auto b = GENERATE(take(10, random(0UL, (1UL << 8) - 1)));
    auto offset = GENERATE(take(10, random(0, (int)STORAGE_SIZE - 9)));
    auto bs = std::make_shared<BoolStorage>(0);
    BoolStorageAccessor bsa(offset, 8, bs);
    CAPTURE(a, b, offset, *bs);
    bsa.set(a);
    bsa.add(b);
    CHECK(bsa.get().to_ullong() == (a | b));
}

TEST_CASE("BoolStorageAccessor add doesn't interfere with other bits", "[boolStorageAccessor]")
{
    auto a = GENERATE(take(10, random(0UL, 1UL << 8)));
    auto offset = GENERATE(take(10, random(0, (int)STORAGE_SIZE - 9)));
    auto bs = std::make_shared<BoolStorage>(0);
    BoolStorageAccessor bsa(offset, 8, bs);
    bsa.add(a);
    unsigned long long mask = ~(((1ULL << 8) - 1) << offset);
    REQUIRE((bs->to_ullong() & mask) == 0);
}

TEST_CASE("BoolStorageAccessor Throws on invalid access", "[boolStorageAccessor]")
{
    auto bs = std::make_shared<BoolStorage>(0);
    // 0, 0 is a valid accessor, it simply does not modify the buffer
    REQUIRE_NOTHROW(BoolStorageAccessor(0, 0, bs));

    // 64, 92 is not a valid accessor, it is out of bounds
    REQUIRE_THROWS(BoolStorageAccessor(64, 92, bs));

    // 0, 65 is not a valid accessor, it is out of bounds
    REQUIRE_THROWS(BoolStorageAccessor(0, 65, bs));

    // 0, 64 is a valid accessor, it modifies the whole buffer
    REQUIRE_NOTHROW(BoolStorageAccessor(0, 64, bs));
}

TEST_CASE("BoolStorageAccessor Throws on invalid dereference", "[boolStorageAccessor]")
{
    // generate a valid storage and accessor
    auto bs = std::make_shared<BoolStorage>(0);
    BoolStorageAccessor bsa(0, 0, bs);
    // reset the storage, therefore invalidating the accessor
    bs.reset();
    REQUIRE_THROWS(bsa.get());
    REQUIRE_THROWS(bsa.set(0));
    REQUIRE_THROWS(bsa.clear());
    REQUIRE_THROWS(bsa.add(0));
}

TEST_CASE("BoolStorageAccessor does not throw on outside modification of the storage", "[boolStorageAccessor]")
{
    auto bs = std::make_shared<BoolStorage>(0);
    BoolStorageAccessor bsa(0, 0, bs);
    bs->set(0, true);
    REQUIRE_NOTHROW(bsa.get());
    REQUIRE_NOTHROW(bsa.set(0));
    REQUIRE_NOTHROW(bsa.clear());
    REQUIRE_NOTHROW(bsa.add(0));
}
