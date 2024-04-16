#include <catch2/catch_amalgamated.hpp>

#include "circuit.hpp"
#include "circuitSchematic.hpp"

TEST_CASE("Circuit can be constructed", "[circuit]")
{
    Circuit c;
    REQUIRE(c.sub_circuits.size() == 0);
    REQUIRE(c.bool_storage_access_map.size() == 0);
    REQUIRE(c.exposed_ports.size() == 0);
}

TEST_CASE("CircuitSchematic can be constructed", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("");
}

TEST_CASE("CircuitSchematic can add gates", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("");
    cs->addAndGate("and1");
    cs->addNotGate("not1");
    cs->addOrGate("or1");
    cs->addXorGate("xor1");
}

TEST_CASE("CircuitSchematic can add wire bridges", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("");
    CircuitSchematic::Bridge bridge;
    bridge.push_back({ "port1", { 1, 2, 3 } });
    bridge.push_back({ "port2", { 4, 5, 6 } });
    cs->addWireBridge(bridge);
}

TEST_CASE("CircuitSchematic can add connections", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("");
    cs->addConnection("port1", "port2");
}

TEST_CASE("CircuitSchematic can add sub-circuits", "[circuitSchematic]")
{
    auto cs = CircuitSchematic::create("a");
    auto cs2 = CircuitSchematic::create("b");
    cs->addSubCircuit("myCircuit", cs2);
}

TEST_CASE("CircuitSchematic generates a valid circuit", "[circuitSchematic][Circuit]")
{
    auto cs = CircuitSchematic::create("b");
    cs->addAndGate("and1");
    cs->addExposedPort("and1_a");
    cs->addExposedPort("and1_b");
    cs->addNotGate("not1");
    cs->addExposedPort("not1_b");
    cs->addConnection("and1_c", "not1_a");
    Managers m;
    auto nand = cs->build(m);
    REQUIRE(nand->exposed_ports.size() == 3);
    auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
    SECTION("NAND 0,0 => 1")
    {
        nand->exposed_ports["and1_a"].set(0);
        nand->exposed_ports["and1_b"].set(0);
        t();
        REQUIRE(nand->exposed_ports["and1_a"].get() == false);
        REQUIRE(nand->exposed_ports["and1_b"].get() == false);
        REQUIRE(nand->exposed_ports["not1_b"].get() == true);
    }
    SECTION("NAND 1,0 => 1")
    {
        nand->exposed_ports["and1_a"].set(1);
        nand->exposed_ports["and1_b"].set(0);
        t();
        REQUIRE(nand->exposed_ports["and1_a"].get() == true);
        REQUIRE(nand->exposed_ports["and1_b"].get() == false);
        REQUIRE(nand->exposed_ports["not1_b"].get() == true);
    }
    SECTION("NAND 0,1 => 1")
    {
        nand->exposed_ports["and1_a"].set(0);
        nand->exposed_ports["and1_b"].set(1);
        t();
        REQUIRE(nand->exposed_ports["and1_a"].get() == false);
        REQUIRE(nand->exposed_ports["and1_b"].get() == true);
        REQUIRE(nand->exposed_ports["not1_b"].get() == true);
    }
    SECTION("NAND 1,1 => 0")
    {
        nand->exposed_ports["and1_a"].set(1);
        nand->exposed_ports["and1_b"].set(1);
        t();
        REQUIRE(nand->exposed_ports["and1_a"].get() == true);
        REQUIRE(nand->exposed_ports["and1_b"].get() == true);
        REQUIRE(nand->exposed_ports["not1_b"].get() == false);
    }
}

TEST_CASE("Generating a circuit with exposed wire bridges", "[Circuit][CircuitSchematic][WireBridge]")
{
    // Generate a schematic for a nand gate
    auto cs = CircuitSchematic::create("nand");
    cs->addAndGate("and1");
    cs->addNotGate("not1");
    cs->addConnection("and1_c", "not1_a");
    cs->addWireBridge(CircuitSchematic::Bridge {
        CircuitSchematic::BridgeView {
            "input",
            { 2 } },
        CircuitSchematic::BridgeView {
            "internalInput",
            { 1, 1 } } });
    cs->addWireBridge(CircuitSchematic::Bridge {
        CircuitSchematic::BridgeView {
            "output",
            { 1 } } });
    cs->addExposedPort("input_0");
    cs->addExposedPort("output_0");
    cs->addConnection("internalInput_0", "and1_a");
    cs->addConnection("internalInput_1", "and1_b");
    cs->addConnection("not1_b", "output_0");
    Managers m;
    auto nand = cs->build(m);
    REQUIRE(nand->exposed_ports.size() == 2);
    auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
    SECTION("NAND 0,0 => 1")
    {
        nand->exposed_ports["input_0"].set(0b00);
        t();
        REQUIRE(nand->exposed_ports["output_0"].get() == 1);
    }
    SECTION("NAND 1,0 => 1")
    {
        nand->exposed_ports["input_0"].set(0b10);
        t();
        REQUIRE(nand->exposed_ports["output_0"].get() == 1);
    }
    SECTION("NAND 0,1 => 1")
    {
        nand->exposed_ports["input_0"].set(0b01);
        t();
        REQUIRE(nand->exposed_ports["output_0"].get() == 1);
    }
    SECTION("NAND 1,1 => 0")
    {
        nand->exposed_ports["input_0"].set(0b11);
        t();
        REQUIRE(nand->exposed_ports["output_0"].get() == 0);
    }
}

TEST_CASE("Half adder", "[Circuit][CircuitSchematic][Manager]")
{
    auto cs = CircuitSchematic::create("half_adder");
    cs->addXorGate("xor1");
    cs->addAndGate("and1");
    cs->addWireBridge({ { "input", { 1, 1 } } });
    cs->addWireBridge({ { "sum", { 1 } } });
    cs->addWireBridge({ { "carry", { 1 } } });
    cs->addExposedPort("input_0");
    cs->addExposedPort("input_1");
    cs->addExposedPort("sum_0");
    cs->addExposedPort("carry_0");
    cs->addConnection("input_0", "xor1_a");
    cs->addConnection("input_1", "xor1_b");
    cs->addConnection("xor1_c", "sum_0");
    cs->addConnection("input_0", "and1_a");
    cs->addConnection("input_1", "and1_b");
    cs->addConnection("and1_c", "carry_0");
    Managers m;
    auto half_adder = cs->build(m);
    REQUIRE(half_adder->exposed_ports.size() == 4); // 2 inputs, 2 outputs (sum, carry)
    auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
    SECTION("0+0")
    {
        half_adder->exposed_ports["input_0"].set(0);
        half_adder->exposed_ports["input_1"].set(0);
        t();
        REQUIRE(half_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(half_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("0+1")
    {
        half_adder->exposed_ports["input_0"].set(0);
        half_adder->exposed_ports["input_1"].set(1);
        t();
        REQUIRE(half_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(half_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("1+0")
    {
        half_adder->exposed_ports["input_0"].set(1);
        half_adder->exposed_ports["input_1"].set(0);
        t();
        REQUIRE(half_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(half_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("1+1")
    {
        half_adder->exposed_ports["input_0"].set(1);
        half_adder->exposed_ports["input_1"].set(1);
        t();
        REQUIRE(half_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(half_adder->exposed_ports["carry_0"].get() == 1);
    }
}

TEST_CASE("Full adder", "[Circuit][CircuitSchematic][Manager]")
{
    auto cs = CircuitSchematic::create("full_adder");
    cs->addXorGate("xor1");
    cs->addXorGate("xor2");
    cs->addAndGate("and1");
    cs->addAndGate("and2");
    cs->addOrGate("or1");
    cs->addWireBridge({ { "input", { 1, 1 } } });
    cs->addWireBridge({ { "carryIn", { 1 } } });
    cs->addWireBridge({ { "sum", { 1 } } });
    cs->addWireBridge({ { "carry", { 1 } } });
    cs->addExposedPort("input_0");
    cs->addExposedPort("input_1");
    cs->addExposedPort("carryIn_0");
    cs->addExposedPort("sum_0");
    cs->addExposedPort("carry_0");
    cs->addConnection("input_0", "xor1_a");
    cs->addConnection("input_1", "xor1_b");
    cs->addConnection("xor1_c", "xor2_a");
    cs->addConnection("carryIn_0", "xor2_b");
    cs->addConnection("xor2_c", "sum_0");
    cs->addConnection("input_0", "and1_a");
    cs->addConnection("input_1", "and1_b");
    cs->addConnection("xor1_c", "and2_a");
    cs->addConnection("carryIn_0", "and2_b");
    cs->addConnection("and1_c", "or1_a");
    cs->addConnection("and2_c", "or1_b");
    cs->addConnection("or1_c", "carry_0");
    Managers m;
    auto full_adder = cs->build(m);
    REQUIRE(full_adder->exposed_ports.size() == 5); // 3 inputs, 2 outputs (sum, carry)
    auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
    SECTION("0+0+0")
    {
        full_adder->exposed_ports["input_0"].set(0);
        full_adder->exposed_ports["input_1"].set(0);
        full_adder->exposed_ports["carryIn_0"].set(0);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("0+0+1")
    {
        full_adder->exposed_ports["input_0"].set(0);
        full_adder->exposed_ports["input_1"].set(0);
        full_adder->exposed_ports["carryIn_0"].set(1);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("0+1+0")
    {
        full_adder->exposed_ports["input_0"].set(0);
        full_adder->exposed_ports["input_1"].set(1);
        full_adder->exposed_ports["carryIn_0"].set(0);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("0+1+1")
    {
        full_adder->exposed_ports["input_0"].set(0);
        full_adder->exposed_ports["input_1"].set(1);
        full_adder->exposed_ports["carryIn_0"].set(1);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 1);
    }
    SECTION("1+0+0")
    {
        full_adder->exposed_ports["input_0"].set(1);
        full_adder->exposed_ports["input_1"].set(0);
        full_adder->exposed_ports["carryIn_0"].set(0);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 0);
    }
    SECTION("1+0+1")
    {
        full_adder->exposed_ports["input_0"].set(1);
        full_adder->exposed_ports["input_1"].set(0);
        full_adder->exposed_ports["carryIn_0"].set(1);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 1);
    }
    SECTION("1+1+0")
    {
        full_adder->exposed_ports["input_0"].set(1);
        full_adder->exposed_ports["input_1"].set(1);
        full_adder->exposed_ports["carryIn_0"].set(0);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 0);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 1);
    }
    SECTION("1+1+1")
    {
        full_adder->exposed_ports["input_0"].set(1);
        full_adder->exposed_ports["input_1"].set(1);
        full_adder->exposed_ports["carryIn_0"].set(1);
        t();
        REQUIRE(full_adder->exposed_ports["sum_0"].get() == 1);
        REQUIRE(full_adder->exposed_ports["carry_0"].get() == 1);
    }
}

TEST_CASE("Sub-circuiting", "[Circuit][CircuitSchematic][Manager]")
{
    SECTION("Passthrough subcircuit")
    {
        auto scs = CircuitSchematic::create("passthrough");
        scs->addWireBridge({ { "input", { 1 } } });
        scs->addWireBridge({ { "output", { 1 } } });
        scs->addExposedPort("input_0");
        scs->addExposedPort("output_0");
        scs->addConnection("input_0", "output_0");
        Managers m;
        auto empty = scs->build(m);
        REQUIRE(empty->exposed_ports.size() == 2);
        REQUIRE(empty->bool_storage_access_map.size() == 2);
        auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
        SECTION("Passthrough")
        {
            int a = GENERATE(0, 1);
            empty->exposed_ports["input_0"].set(a);
            t();
            REQUIRE(empty->exposed_ports["output_0"].get() == a);
        }
        auto cs = CircuitSchematic::create("passthrough_wrapper");
        cs->addSubCircuit("passthrough", scs);
        cs->addWireBridge({ { "input", { 1 } } });
        cs->addWireBridge({ { "output", { 1 } } });
        cs->addExposedPort("input_0");
        cs->addExposedPort("output_0");
        cs->addConnection("input_0", "passthrough_input_0");
        cs->addConnection("passthrough_output_0", "output_0");
        auto wrapper = cs->build(m);
        REQUIRE(wrapper->exposed_ports.size() == 2);
        REQUIRE(wrapper->bool_storage_access_map.size() == 4);
        SECTION("Passthrough wrapper")
        {
            int a = GENERATE(0, 1);
            wrapper->exposed_ports["input_0"].set(a);
            m.socketController->tick();
            m.socketController->tick();
            m.socketController->tick();
            m.socketController->tick();

            REQUIRE(wrapper->exposed_ports["output_0"].get() == a);
        }
    }

    auto xor_wrapper = CircuitSchematic::create("xor_wrapper");
    xor_wrapper->addXorGate("xor1");
    xor_wrapper->addWireBridge({ { "input", { 1, 1 } } });
    xor_wrapper->addWireBridge({ { "output", { 1 } } });
    xor_wrapper->addExposedPort("input_0");
    xor_wrapper->addExposedPort("input_1");
    xor_wrapper->addExposedPort("output_0");
    xor_wrapper->addConnection("input_0", "xor1_a");
    xor_wrapper->addConnection("input_1", "xor1_b");
    xor_wrapper->addConnection("xor1_c", "output_0");

    SECTION("XOR Wrapper testing")
    {
        Managers m;
        auto w = xor_wrapper->build(m);
        SECTION("Built correctly")
        {
            REQUIRE(w->exposed_ports.size() == 3);
            REQUIRE(w->exposed_ports.count("input_0") == 1);
            REQUIRE(w->exposed_ports.count("input_1") == 1);
            REQUIRE(w->exposed_ports.count("output_0") == 1);
            REQUIRE(w->bool_storage_access_map.size() == 6);
            REQUIRE(w->bool_storage_access_map.count("xor1_a") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor1_b") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor1_c") == 1);
            REQUIRE(w->bool_storage_access_map.count("input_0") == 1);
            REQUIRE(w->bool_storage_access_map.count("input_1") == 1);
            REQUIRE(w->bool_storage_access_map.count("output_0") == 1);

            auto& sockets = *m.socketController;
            SECTION("Sockets")
            {
                SECTION("input_0 -> xor1_a")
                {
                    auto& i1 = w->bool_storage_access_map["input_0"];
                    auto& i2 = w->bool_storage_access_map["xor1_a"];
                    i1.set(1);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(i2.get() == 1);
                    i1.set(0);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(i2.get() == 0);
                }

                SECTION("input_1 -> xor1_b")
                {
                    auto& i1 = w->bool_storage_access_map["input_1"];
                    auto& i2 = w->bool_storage_access_map["xor1_b"];
                    i1.set(1);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(i2.get() == 1);
                    i1.set(0);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(i2.get() == 0);
                }

                SECTION("input -> output_0")
                {
                    auto& i1 = w->bool_storage_access_map["input_0"];
                    auto& i2 = w->bool_storage_access_map["input_1"];
                    auto& o = w->bool_storage_access_map["output_0"];
                    i1.set(0);
                    i2.set(0);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(o.get() == 0);
                    i1.set(0);
                    i2.set(1);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(o.get() == 1);
                    i1.set(1);
                    i2.set(0);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(o.get() == 1);
                    i1.set(1);
                    i2.set(1);
                    for (int i = 0; i < 20; i++)
                        m.tick();
                    REQUIRE(o.get() == 0);
                }
            }
        }
        auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
        SECTION("0+0")
        {
            w->exposed_ports["input_0"].set(0);
            w->exposed_ports["input_1"].set(0);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 0);
        }

        SECTION("0+1")
        {
            w->exposed_ports["input_0"].set(0);
            w->exposed_ports["input_1"].set(1);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 1);
        }

        SECTION("1+0")
        {
            w->exposed_ports["input_0"].set(1);
            w->exposed_ports["input_1"].set(0);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 1);
        }

        SECTION("1+1")
        {
            w->exposed_ports["input_0"].set(1);
            w->exposed_ports["input_1"].set(1);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 0);
        }
    }

    SECTION("Xor wrapper wrapper testing")
    {
        auto wrapper = CircuitSchematic::create("wrapper");
        wrapper->addSubCircuit("xor", xor_wrapper);
        Managers m;
        auto w = wrapper->build(m);
        REQUIRE(w->bool_storage_access_map.size() == 3);

        auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
        SECTION("Check built circuit")
        {
            REQUIRE(w->bool_storage_access_map.size() == 3);
            REQUIRE(w->bool_storage_access_map.count("xor_input_0") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor_input_1") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor_output_0") == 1);
        }
        SECTION("0+0")
        {
            w->bool_storage_access_map["xor_input_0"].set(0);
            w->bool_storage_access_map["xor_input_1"].set(0);
            t();
            REQUIRE(w->bool_storage_access_map["xor_output_0"].get() == 0);
        }
        SECTION("0+1")
        {
            w->bool_storage_access_map["xor_input_0"].set(0);
            w->bool_storage_access_map["xor_input_1"].set(1);
            t();
            REQUIRE(w->bool_storage_access_map["xor_output_0"].get() == 1);
        }
        SECTION("1+0")
        {
            w->bool_storage_access_map["xor_input_0"].set(1);
            w->bool_storage_access_map["xor_input_1"].set(0);
            t();
            REQUIRE(w->bool_storage_access_map["xor_output_0"].get() == 1);
        }
        SECTION("1+1")
        {
            w->bool_storage_access_map["xor_input_0"].set(1);
            w->bool_storage_access_map["xor_input_1"].set(1);
            t();
            REQUIRE(w->bool_storage_access_map["xor_output_0"].get() == 0);
        }
    }

    SECTION("Xor wrapper wrapper testing with wire bridges")
    {
        auto wrapper = CircuitSchematic::create("wrapper");
        wrapper->addSubCircuit("xor", xor_wrapper);
        wrapper->addWireBridge({ { "input", { 1, 1 } } });
        wrapper->addWireBridge({ { "output", { 1 } } });
        wrapper->addExposedPort("input_0");
        wrapper->addExposedPort("input_1");
        wrapper->addExposedPort("output_0");
        wrapper->addConnection("input_0", "xor_input_0");
        wrapper->addConnection("input_1", "xor_input_1");
        wrapper->addConnection("xor_output_0", "output_0");
        Managers m;
        auto w = wrapper->build(m);

        auto t = [&]() {for(int i=0;i<2000;i++)m.tick(); };
        SECTION("Check built circuit")
        {
            REQUIRE(w->bool_storage_access_map.size() == 6);
            REQUIRE(w->bool_storage_access_map.count("xor_input_0") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor_input_1") == 1);
            REQUIRE(w->bool_storage_access_map.count("xor_output_0") == 1);
            REQUIRE(w->exposed_ports.size() == 3);
            REQUIRE(w->exposed_ports.count("input_0") == 1);
            REQUIRE(w->exposed_ports.count("input_1") == 1);
            REQUIRE(w->exposed_ports.count("output_0") == 1);
            REQUIRE(w->bool_storage_access_map.count("input_0") == 1);
            REQUIRE(w->bool_storage_access_map.count("input_1") == 1);
            REQUIRE(w->bool_storage_access_map.count("output_0") == 1);
        }
        SECTION("Value checks")
        {
            int a = GENERATE(0, 1);
            int b = GENERATE(0, 1);
            CAPTURE(a);
            CAPTURE(b);
            CAPTURE(a ^ b);
            w->bool_storage_access_map["input_0"].set(a);
            w->bool_storage_access_map["input_1"].set(b);
            t();
            CAPTURE(w->bool_storage_access_map["input_0"].get().to_ulong());
            CAPTURE(w->bool_storage_access_map["input_1"].get().to_ulong());
            CAPTURE(w->bool_storage_access_map["xor_input_0"].get().to_ulong());
            CAPTURE(w->bool_storage_access_map["xor_input_1"].get().to_ulong());
            CAPTURE(w->bool_storage_access_map["xor_output_0"].get().to_ulong());
            CAPTURE(w->bool_storage_access_map["output_0"].get().to_ulong());

            REQUIRE(w->bool_storage_access_map["output_0"].get() == (a ^ b));
        }
    }
    SECTION("Xor wrapper wrapper")
    {
        auto wrapper = CircuitSchematic::create("wrapper");
        wrapper->addSubCircuit("xor", xor_wrapper);
        wrapper->addWireBridge({ { "input", { 1, 1 } } });
        wrapper->addWireBridge({ { "output", { 1 } } });
        wrapper->addExposedPort("input_0");
        wrapper->addExposedPort("input_1");
        wrapper->addExposedPort("output_0");
        wrapper->addConnection("input_0", "xor_input_0");
        wrapper->addConnection("input_1", "xor_input_1");
        wrapper->addConnection("xor_output_0", "output_0");

        Managers m;
        auto w = wrapper->build(m);
        REQUIRE(w->exposed_ports.size() == 3);
        auto t = [&]() {for(int i=0;i<6240;i++)m.tick(); };
        SECTION("0+0")
        {
            w->exposed_ports["input_0"].set(0);
            w->exposed_ports["input_1"].set(0);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 0);
        }
        SECTION("0+1")
        {
            w->exposed_ports["input_0"].set(0);
            w->exposed_ports["input_1"].set(1);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 1);
        }
        SECTION("1+0")
        {
            w->exposed_ports["input_0"].set(1);
            w->exposed_ports["input_1"].set(0);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 1);
        }
        SECTION("1+1")
        {
            w->exposed_ports["input_0"].set(1);
            w->exposed_ports["input_1"].set(1);
            t();
            REQUIRE(w->exposed_ports["output_0"].get() == 0);
        }
    }

    SECTION("2-bit adders")
    {
        auto full_adder = CircuitSchematic::create("full_adder");
        full_adder->addXorGate("xor1");
        full_adder->addXorGate("xor2");
        full_adder->addAndGate("and1");
        full_adder->addAndGate("and2");
        full_adder->addOrGate("or1");
        full_adder->addWireBridge({ { "input", { 1, 1 } } });
        full_adder->addWireBridge({ { "carryIn", { 1 } } });
        full_adder->addWireBridge({ { "sum", { 1 } } });
        full_adder->addWireBridge({ { "carry", { 1 } } });
        full_adder->addExposedPort("input_0");
        full_adder->addExposedPort("input_1");
        full_adder->addExposedPort("carryIn_0");
        full_adder->addExposedPort("sum_0");
        full_adder->addExposedPort("carry_0");
        full_adder->addConnection("input_0", "xor1_a");
        full_adder->addConnection("input_1", "xor1_b");
        full_adder->addConnection("xor1_c", "xor2_a");
        full_adder->addConnection("carryIn_0", "xor2_b");
        full_adder->addConnection("xor2_c", "sum_0");
        full_adder->addConnection("input_0", "and1_a");
        full_adder->addConnection("input_1", "and1_b");
        full_adder->addConnection("xor1_c", "and2_a");
        full_adder->addConnection("carryIn_0", "and2_b");
        full_adder->addConnection("and1_c", "or1_a");
        full_adder->addConnection("and2_c", "or1_b");
        full_adder->addConnection("or1_c", "carry_0");

        SECTION("Full adder structure")
        {
            Managers m;
            auto circuit = full_adder->build(m);
            REQUIRE(circuit->exposed_ports.size() == 5);
            auto t = [&]() {for(int i=0;i<20;i++)m.tick(); };
            int a = GENERATE(0, 1);
            int b = GENERATE(0, 1);
            int c = GENERATE(0, 1);
            DYNAMIC_SECTION("Full adder " << a << " + " << b << " + " << c)
            {
                circuit->exposed_ports["input_0"].set(a);
                circuit->exposed_ports["input_1"].set(b);
                circuit->exposed_ports["carryIn_0"].set(c);
                t();
                REQUIRE(circuit->exposed_ports["sum_0"].get().to_ulong() == ((a + b + c) & 1));
                REQUIRE(circuit->exposed_ports["carry_0"].get().to_ulong() == (a + b + c) >> 1);
            }
        }

        auto two_bit_adder = CircuitSchematic::create("two_bit_adder");
        two_bit_adder->addSubCircuit("full_adder_0", full_adder);
        two_bit_adder->addSubCircuit("full_adder_1", full_adder);
        two_bit_adder->addWireBridge({ { "input", { 1, 1, 1, 1 } } });
        two_bit_adder->addWireBridge({ { "carryIn", { 1 } } });
        two_bit_adder->addWireBridge({ { "sum", { 1, 1 } } });
        two_bit_adder->addWireBridge({ { "carry", { 1 } } });
        two_bit_adder->addExposedPort("input_0");
        two_bit_adder->addExposedPort("input_1");
        two_bit_adder->addExposedPort("input_2");
        two_bit_adder->addExposedPort("input_3");
        two_bit_adder->addExposedPort("carryIn_0");
        two_bit_adder->addExposedPort("sum_0");
        two_bit_adder->addExposedPort("sum_1");
        two_bit_adder->addExposedPort("carry_0");
        two_bit_adder->addConnection("input_0", "full_adder_0_input_0");
        two_bit_adder->addConnection("input_1", "full_adder_0_input_1");
        two_bit_adder->addConnection("input_2", "full_adder_1_input_0");
        two_bit_adder->addConnection("input_3", "full_adder_1_input_1");
        two_bit_adder->addConnection("carryIn_0", "full_adder_0_carryIn_0");
        two_bit_adder->addConnection("full_adder_0_sum_0", "full_adder_1_carryIn_0");
        two_bit_adder->addConnection("full_adder_0_sum_0", "sum_0");
        two_bit_adder->addConnection("full_adder_1_sum_0", "sum_1");
        two_bit_adder->addConnection("full_adder_1_carry_0", "carry_0");
        two_bit_adder->addConnection("full_adder_0_carry_0", "full_adder_1_carryIn_0");
        Managers m;
        auto circuit = two_bit_adder->build(m);
        auto t = [&]() {for(int i=0;i<50;i++)m.tick(); };
        SECTION("Two bit adder structure")
        {
            REQUIRE(circuit->sub_circuits.size() == 2);
            REQUIRE(circuit->exposed_ports.size() == 8);
            // check all internal ports
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_0_input_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_0_input_1") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_0_carryIn_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_0_sum_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_0_carry_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_1_input_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_1_input_1") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_1_carryIn_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_1_sum_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("full_adder_1_carry_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("sum_0") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("sum_1") == 1);
            REQUIRE(circuit->bool_storage_access_map.count("carry_0") == 1);
        }

        SECTION("Two bit value follow-through")
        {
            int a = GENERATE(0, 1);
            int b = GENERATE(0, 1);
            int c = GENERATE(0, 1);
            int d = GENERATE(0, 1);
            DYNAMIC_SECTION("Two bit adder " << c << a << " + " << d << b << " = " << (a + (c << 1)) + (c + (b << 1)))
            {
                auto &i0 = circuit->bool_storage_access_map["input_0"],
                     &i1 = circuit->bool_storage_access_map["input_1"],
                     &i2 = circuit->bool_storage_access_map["input_2"],
                     &i3 = circuit->bool_storage_access_map["input_3"],
                     &co = circuit->bool_storage_access_map["carryIn_0"],
                     &s0 = circuit->bool_storage_access_map["sum_0"],
                     &s1 = circuit->bool_storage_access_map["sum_1"],
                     &co0 = circuit->bool_storage_access_map["carry_0"];
                auto &f0 = circuit->sub_circuits[0]->bool_storage_access_map,
                     &f1 = circuit->sub_circuits[1]->bool_storage_access_map;
                auto &f0i0 = f0["input_0"],
                     &f0i1 = f0["input_1"],
                     &f0ci = f0["carryIn_0"],
                     &f0s0 = f0["sum_0"],
                     &f0co = f0["carry_0"];
                i0.set(a);
                i1.set(0);
                m.tick();
                REQUIRE(f0i0.get().to_ulong() == a);
                m.tick();
                m.tick();
                m.tick();
                m.tick();
                REQUIRE(f0i0.get().to_ulong() == a);
                REQUIRE(f0s0.get().to_ulong() == a);

                i1.set(b);
                m.tick();
                REQUIRE(f0i1.get().to_ulong() == b);
                m.tick();
                m.tick();
                m.tick();
                m.tick();
                REQUIRE(f0i1.get().to_ulong() == b);
                REQUIRE(f0s0.get().to_ulong() == (a ^ b));
            }
        }
    }
}

TEST_CASE("2-bit adder with wire bridge io abstraction", "[Circuit][CircuitSchematic][Manager]")
{
    auto full_adder = CircuitSchematic::create("full_adder");
    full_adder->addXorGate("xor1");
    full_adder->addXorGate("xor2");
    full_adder->addAndGate("and1");
    full_adder->addAndGate("and2");
    full_adder->addOrGate("or1");
    full_adder->addWireBridge({ { "input", { 1, 1 } } });
    full_adder->addWireBridge({ { "carryIn", { 1 } } });
    full_adder->addWireBridge({ { "sum", { 1 } } });
    full_adder->addWireBridge({ { "carry", { 1 } } });
    full_adder->addExposedPort("input_0");
    full_adder->addExposedPort("input_1");
    full_adder->addExposedPort("carryIn_0");
    full_adder->addExposedPort("sum_0");
    full_adder->addExposedPort("carry_0");
    full_adder->addConnection("input_0", "xor1_a");
    full_adder->addConnection("input_1", "xor1_b");
    full_adder->addConnection("xor1_c", "xor2_a");
    full_adder->addConnection("carryIn_0", "xor2_b");
    full_adder->addConnection("xor2_c", "sum_0");
    full_adder->addConnection("input_0", "and1_a");
    full_adder->addConnection("input_1", "and1_b");
    full_adder->addConnection("xor1_c", "and2_a");
    full_adder->addConnection("carryIn_0", "and2_b");
    full_adder->addConnection("and1_c", "or1_a");
    full_adder->addConnection("and2_c", "or1_b");
    full_adder->addConnection("or1_c", "carry_0");

    auto two_bit_adder = CircuitSchematic::create("2-bit_adder");
    two_bit_adder->addWireBridge({ { "internal_A", { 1, 1 } },
        { "A", { 2 } } });
    two_bit_adder->addWireBridge({ { "internal_B", { 1, 1 } },
        { "B", { 2 } } });
    two_bit_adder->addWireBridge({ { "internal_O", { 1, 1 } },
        { "O", { 2 } } });
    two_bit_adder->addSubCircuit("fa_0", full_adder);
    two_bit_adder->addSubCircuit("fa_1", full_adder);

    two_bit_adder->addConnection("fa_0_carry_0", "fa_1_carryIn_0");
    two_bit_adder->addConnection("internal_A_0", "fa_0_input_0");
    two_bit_adder->addConnection("internal_A_1", "fa_1_input_0");
    two_bit_adder->addConnection("internal_B_0", "fa_0_input_1");
    two_bit_adder->addConnection("internal_B_1", "fa_1_input_1");
    two_bit_adder->addConnection("fa_1_sum_0", "internal_O_1");
    two_bit_adder->addConnection("fa_0_sum_0", "internal_O_0");
    two_bit_adder->addExposedPort("A_0");
    two_bit_adder->addExposedPort("B_0");
    two_bit_adder->addExposedPort("O_0");

    Managers m;
    auto c = two_bit_adder->build(m);
    int a = GENERATE(0, 1, 2, 3);
    int b = GENERATE(0, 1, 2, 3);
    DYNAMIC_SECTION("sum:" << a << " + " << b)
    {
        c->exposed_ports["A_0"].set(a);
        c->exposed_ports["B_0"].set(b);
        for (int i = 0; i < 100; i++)
            m.tick();
        REQUIRE(c->exposed_ports["O_0"].get().to_ulong() == ((a + b) & 0b11));
        REQUIRE(c->bool_storage_access_map["fa_1_carry_0"].get().to_ulong() == ((a + b) >> 2));
    }
}