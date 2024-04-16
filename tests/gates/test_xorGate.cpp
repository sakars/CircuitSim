#include "gates/xorGate.hpp"
#include <catch2/catch_amalgamated.hpp>

TEST_CASE("XorGate", "[XorGate][Gate]")
{
    XorGate gate;
    auto [a, b, c] = gate.lendGate();
    a.set(0b1);
    b.set(0b1);
    gate.tick();
    REQUIRE(c.get() == 0b0);

    a.set(0b0);
    b.set(0b1);
    gate.tick();
    REQUIRE(c.get() == 0b1);

    a.set(0b1);
    b.set(0b0);
    gate.tick();
    REQUIRE(c.get() == 0b1);

    a.set(0b0);
    b.set(0b0);
    gate.tick();
    REQUIRE(c.get() == 0b0);
}

TEST_CASE("XorGate with multiple storages", "[XorGate][Gate]")
{
    XorGate gate;
    auto [a, b, c] = gate.lendGate();
    a.set(0b1);
    b.set(0b1);
    gate.tick();
    REQUIRE(c.get() == 0b0);

    auto [d, e, f] = gate.lendGate();
    d.set(0b1);
    e.set(0b0);
    gate.tick();
    REQUIRE(f.get() == 0b1);

    a.set(0b0);
    b.set(0b1);
    gate.tick();
    REQUIRE(c.get() == 0b1);

    d.set(0b1);
    e.set(0b1);
    gate.tick();
    REQUIRE(f.get() == 0b0);
}