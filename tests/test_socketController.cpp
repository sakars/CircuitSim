#include "dataBank.hpp"
#include "socketController.hpp"
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("SocketController is a class", "[socketController]")
{
    SocketController sc;
    REQUIRE(sizeof(sc) > 0);
}

TEST_CASE("SocketController can add a socket", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 0, bs);
    sc.addSocket(bsa, bsa);
    REQUIRE_NOTHROW(sc.tick());
}

TEST_CASE("SocketController can add multiple sockets", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 0, bs);
    sc.addSocket(bsa, bsa);
    sc.addSocket(bsa, bsa);
    REQUIRE_NOTHROW(sc.tick());
}

TEST_CASE("SocketController is unable to make a socket with differently sized accessors", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 5, bs);
    BoolStorageAccessor bsb(0, 3, bs);
    REQUIRE_THROWS(sc.addSocket(bsa, bsb));
}

TEST_CASE("SocketController can add and tick a socket", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    // mark accessors with 8 bits each and different offsets in the storage to avoid overlap
    BoolStorageAccessor bsa(0, 8, bs);
    BoolStorageAccessor bsb(8, 8, bs);
    sc.addSocket(bsa, bsb);
    bsa.set(0b101010);
    sc.tick();
    REQUIRE(bsb.get() == 0b101010);
}

TEST_CASE("SocketController can add and tick multiple sockets", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    // mark accessors with 8 bits each and different offsets in the storage to avoid overlap
    BoolStorageAccessor bsa(0, 8, bs);
    BoolStorageAccessor bsb(8, 8, bs);
    BoolStorageAccessor bsc(16, 8, bs);
    sc.addSocket(bsa, bsb);
    sc.addSocket(bsa, bsc);
    bsa.set(0b101010);
    sc.tick();
    REQUIRE(bsb.get() == 0b101010);
    REQUIRE(bsc.get() == 0b101010);
}

TEST_CASE("SocketController handles chains of sockets", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    // mark accessors with 8 bits each and different offsets in the storage to avoid overlap
    BoolStorageAccessor bsa(0, 8, bs);
    BoolStorageAccessor bsb(8, 8, bs);
    BoolStorageAccessor bsc(16, 8, bs);
    sc.addSocket(bsa, bsb);
    sc.addSocket(bsb, bsc);
    bsa.set(0b101010);
    sc.tick();
    sc.tick();
    sc.tick();
    REQUIRE(bsb.get() == 0b101010);
    REQUIRE(bsc.get() == 0b101010);
}

TEST_CASE("SocketController handles differently sized sockets", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 8, bs);
    BoolStorageAccessor bsb(8, 8, bs);
    BoolStorageAccessor bsc(16, 3, bs);
    BoolStorageAccessor bsd(19, 3, bs);
    sc.addSocket(bsa, bsb);
    sc.addSocket(bsc, bsd);
    bsa.set(0b101010);
    bsc.set(0b101);
    sc.tick();
    REQUIRE(bsb.get() == 0b101010);
    REQUIRE(bsd.get() == 0b101);
}

TEST_CASE("Chain link order of creation is irrelevant", "[socketController]")
{
    SocketController sc;
    auto bs = std::make_shared<BoolStorage>();
    BoolStorageAccessor bsa(0, 8, bs);
    BoolStorageAccessor bsb(8, 8, bs);
    BoolStorageAccessor bsc(16, 8, bs);
    BoolStorageAccessor bsd(24, 8, bs);
    sc.addSocket(bsc, bsb);
    sc.addSocket(bsb, bsd);
    sc.addSocket(bsd, bsa);
    bsc.set(0b101010);
    sc.tick();
    sc.tick();
    sc.tick();
    REQUIRE(bsa.get() == 0b101010);
}

TEST_CASE("Chain link order of creation is irrelevant with databanks", "[socketController][dataBank]")
{
    SocketController sc;
    DataBank<1> db;
    auto bs = db.lendStorage();
    BoolStorageAccessor bsa(0, 8, bs[0]);

    bs = db.lendStorage();
    BoolStorageAccessor bsb(0, 8, bs[0]);

    bs = db.lendStorage();
    BoolStorageAccessor bsc(0, 8, bs[0]);

    bs = db.lendStorage();
    BoolStorageAccessor bsd(0, 8, bs[0]);

    sc.addSocket(bsc, bsb);
    sc.addSocket(bsb, bsd);
    sc.addSocket(bsd, bsa);
    bsc.set(0b101010);
    sc.tick();
    sc.tick();
    sc.tick();
    REQUIRE(bsa.get() == 0b101010);
}