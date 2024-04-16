#include "managers.hpp"
#include "wireBridge.hpp"
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("wireBridge", "[wireBridge]")
{
    DataBank<1> db;
    std::vector<std::vector<size_t>> ports = { { 2, 2, 2 }, { 4, 1, 1 }, { 6 } };
    auto result = wireBridge(db, ports);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0].size() == 3);
    REQUIRE(result[1].size() == 3);
    REQUIRE(result[2].size() == 1);
    REQUIRE(result[0][0].getSocketSize() == 2);
    REQUIRE(result[0][1].getSocketSize() == 2);
    REQUIRE(result[0][2].getSocketSize() == 2);
    REQUIRE(result[1][0].getSocketSize() == 4);
    REQUIRE(result[1][1].getSocketSize() == 1);
    REQUIRE(result[1][2].getSocketSize() == 1);
    REQUIRE(result[2][0].getSocketSize() == 6);

    result[2][0].set(0b101010);
    REQUIRE(result[2][0].get() == 0b101010);
    REQUIRE(result[0][0].get() == 0b10);
    REQUIRE(result[0][1].get() == 0b10);
    REQUIRE(result[0][2].get() == 0b10);
    REQUIRE(result[1][0].get() == 0b1010);
    REQUIRE(result[1][1].get() == 0b0);
    REQUIRE(result[1][2].get() == 0b1);

    result[0][0].set(0b11);
    REQUIRE(result[2][0].get() == 0b101011);
    REQUIRE(result[0][0].get() == 0b11);
    REQUIRE(result[0][1].get() == 0b10);
    REQUIRE(result[0][2].get() == 0b10);
    REQUIRE(result[1][0].get() == 0b1011);
    REQUIRE(result[1][1].get() == 0b0);
    REQUIRE(result[1][2].get() == 0b1);
}

TEST_CASE("Data transfer between bridges", "[Managers][wireBridge]")
{
    Managers m;
    auto wb = wireBridge(*m.random_access_data_bank, { { 1, 1 }, { 2 } });
    SECTION("contains working bridge")
    {
        REQUIRE(wb.size() == 2);
        REQUIRE(wb[0].size() == 2);
        REQUIRE(wb[1].size() == 1);
        wb[0][0].set(0b1);
        wb[0][1].set(0b0);
        REQUIRE(wb[0][0].get() == 0b1);
        REQUIRE(wb[0][1].get() == 0b0);
        REQUIRE(wb[1][0].get() == 0b01);
        wb[1][0].set(0b01);
        REQUIRE(wb[1][0].get() == 0b01);
        REQUIRE(wb[0][0].get() == 0b1);
        REQUIRE(wb[0][1].get() == 0b0);
    }
    auto wb2 = wireBridge(*m.random_access_data_bank, { { 1, 1 }, { 2 } });

    m.socketController->addSocket(wb[0][0], wb2[0][0]);
    m.socketController->addSocket(wb[0][1], wb2[0][1]);

    SECTION("data transfer")
    {
        wb[1][0].set(0b01);
        for (size_t i = 0; i < 10; i++)
            m.tick();
        REQUIRE(wb2[1][0].get() == 0b01);

        wb[1][0].set(0b00);
        for (size_t i = 0; i < 10; i++)
            m.tick();
        REQUIRE(wb2[1][0].get() == 0b00);

        wb[1][0].set(0b10);
        for (size_t i = 0; i < 10; i++)
            m.tick();
        REQUIRE(wb2[1][0].get() == 0b10);

        wb[1][0].set(0b11);
        for (size_t i = 0; i < 10; i++)
            m.tick();
        REQUIRE(wb2[1][0].get() == 0b11);
    }
}
