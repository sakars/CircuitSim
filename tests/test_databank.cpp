#include "dataBank.hpp"
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("DataBank is a class", "[dataBank]")
{
    DataBank<1> db;
    REQUIRE(sizeof(db) > 0);

    DataBank<2> db2;
    REQUIRE(sizeof(db2) > 0);
}

TEST_CASE("DataBank can lend storage", "[dataBank]")
{

    DataBank<1> db;
    auto [a] = db.lendBools(9);
    a.set(0b11011);
    REQUIRE(a.get().count() == 4);
    auto store = db.getStorage();
    REQUIRE(store.size() >= 1);
    REQUIRE(store[0][0].expired() == false);
    REQUIRE(store[0][0].lock()->count() == 4);
}

TEST_CASE("DataBank's lent bools do not interfere with each other", "[dataBank]")
{
    DataBank<1> db;
    std::vector<BoolStorageAccessor> accessors;
    for (size_t i = 0; i < 39; i++) {
        accessors.emplace_back(db.lendBools(7)[0]);
    }
    for (size_t i = 0; i < 39; i++) {
        accessors[i].set(i);
    }
    for (size_t i = 0; i < 39; i++) {
        REQUIRE(accessors[i].get().to_ullong() == i);
    }
}

TEST_CASE("Synchronicity between DataBank's different dimensions", "[dataBank]")
{
    DataBank<3> db;
    std::vector<BoolStorageAccessor> accessors;
    std::vector<size_t> values;
    for (size_t i = 0; i < 10; i++) {
        auto v1 = rand() % 100, v2 = rand() % 100;
        auto [a, b, c] = db.lendBools(9);
        accessors.emplace_back(c);
        values.emplace_back(v1 & v2);
        a.set(v1);
        b.set(v2);
        auto storage = db.getStorage();
        for (auto& segment : storage) {
            for (size_t i = 0; i < segment.size(); i++) {
                REQUIRE(segment[i].expired() == false);
            }
            auto& [a, b, c] = segment;
            *c.lock() = *a.lock() & *b.lock();
        }
        REQUIRE(c.get() == (v1 & v2));
    }

    for (size_t i = 0; i < 10; i++) {
        REQUIRE(accessors[i].get().to_ullong() == values[i]);
    }
}

TEST_CASE("DataBank can lend a whole storage", "[dataBank]")
{
    DataBank<3> db;
    auto [a, b, c] = db.lendBools(9);
    a.set(0b11011);
    REQUIRE(a.get().count() == 4);
    auto store = db.lendStorage();
    REQUIRE(store.size() >= 1);
    REQUIRE(store[0].expired() == false);
    REQUIRE(store[0].lock()->count() == 0);
}

TEST_CASE("DataBank's storages do not interfere with each other", "[dataBank]")
{
    DataBank<1> db;
    std::vector<std::weak_ptr<BoolStorage>> storages;
    for (size_t i = 0; i < 39; i++) {
        storages.emplace_back(db.lendStorage()[0]);
    }
    for (size_t i = 0; i < 39; i++) {
        *storages[i].lock() = i;
    }
    std::vector<BoolStorageAccessor> accessors;
    for (size_t i = 0; i < 39; i++) {
        accessors.emplace_back(db.lendBools(7)[0]);
    }
    for (size_t i = 0; i < 39; i++) {
        accessors[i].set(i);
    }

    for (size_t i = 0; i < 39; i++) {
        REQUIRE(accessors[i].get().to_ullong() == i);
    }

    for (size_t i = 0; i < 39; i++) {
        REQUIRE(storages[i].lock()->to_ullong() == i);
    }
}