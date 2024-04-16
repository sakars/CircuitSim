#include "gates/notGate.hpp"
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("NotGate", "[NotGate][Gate]")
{
    NotGate gate;
    auto [a, b] = gate.lendGate();
    a.set(0b1);
    gate.tick();
    REQUIRE(b.get() == 0b0);

    a.set(0b0);
    gate.tick();
    REQUIRE(b.get() == 0b1);
}

TEST_CASE("NotGate with multiple storages", "[NotGate][Gate]")
{
    NotGate gate;
    auto [a, b] = gate.lendGate();
    a.set(0b1);
    gate.tick();
    REQUIRE(b.get() == 0b0);

    auto [d, e] = gate.lendGate();
    d.set(0b1);
    gate.tick();
    REQUIRE(e.get() == 0b0);

    a.set(0b0);
    gate.tick();
    REQUIRE(b.get() == 0b1);

    d.set(0b1);
    gate.tick();
    REQUIRE(e.get() == 0b0);
}