#include <catch2/catch_amalgamated.hpp>

#include "circuitSchematic.hpp"

TEST_CASE("Aliases can be added", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("test");
    REQUIRE_NOTHROW(cs->addAlias("target_0", "alias"));
    SECTION("Throws when building as the aliased port does not exist")
    {
        Managers m;
        REQUIRE_THROWS(cs->build(m));
    }

    cs->addWireBridge({ { "target", { 1 } } });
    SECTION("Does not throw when building as the aliased port exists")
    {
        Managers m;
        REQUIRE_NOTHROW(cs->build(m));
    }
}

TEST_CASE("Aliases share storages", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("test");
    cs->addWireBridge({ { "target", { 64 } } });
    cs->addAlias("target_0", "alias_1");
    cs->addAlias("target_0", "alias_2");
    cs->addAlias("target_0", "alias_3");
    cs->addAlias("target_0", "alias_4");

    Managers m;
    auto circuit = cs->build(m);
    std::string set_port = GENERATE("target_0", "alias_1", "alias_2", "alias_3", "alias_4");
    unsigned int r = GENERATE(take(10, random(0, 734824)));
    DYNAMIC_SECTION("Set " << set_port << " to " << r)
    {
        circuit->bool_storage_access_map[set_port].set(r);
        REQUIRE(circuit->bool_storage_access_map["target_0"].get() == r);
        REQUIRE(circuit->bool_storage_access_map["alias_1"].get() == r);
        REQUIRE(circuit->bool_storage_access_map["alias_2"].get() == r);
        REQUIRE(circuit->bool_storage_access_map["alias_3"].get() == r);
        REQUIRE(circuit->bool_storage_access_map["alias_4"].get() == r);
    }
}
