#pragma once
#include "circuit.hpp"
#include "managers.hpp"
#include <memory>
#include <queue>
#include <stack>
#include <tuple>
#include <unordered_set>

/// @brief A class that holds the building instructions for a circuit.
/// This class is used to build a circuit from a list of gates and wires.
/// It does not request any memory from the data banks, and thus is copiable.
class CircuitSchematic {
public:
    /// @brief A view of a named port.
    /// Contains the name of the view and the sizes of the view's ports in order.
    struct BridgeView {
        std::string name;
        std::vector<size_t> port_sizes;
    };

    // A port is just a collection of named port views. The port itself does not need a name.
    using Bridge = std::vector<BridgeView>;

private:
    CircuitSchematic(std::string name)
        : name(name)
    {
    }

    // Lists of defined gates and wires
    std::string name;
    std::vector<std::string> and_gates;
    std::vector<std::string> not_gates;
    std::vector<std::string> or_gates;
    std::vector<std::string> xor_gates;

    // A list of wire bridges. Each wire bridge is a collection of named port views.
    std::vector<Bridge> wire_bridges;

    // A list of sub-circuits. Each sub-circuit has a name and a schematic.
    std::vector<std::tuple<std::string, std::weak_ptr<CircuitSchematic>>> sub_circuits;

    // A list of connections. Each connection is a tuple of two strings, denoting the names of the ports to connect.
    std::vector<std::tuple<std::string, std::string>> connections;

    std::vector<std::string> exposed_ports;

    /// @brief A list of dependencies. Used to prevent circular dependencies.
    std::unordered_set<std::string> dependencies;

    std::weak_ptr<CircuitSchematic> self;

    std::tuple<std::vector<WireBridgeData>, std::vector<std::vector<std::vector<std::string>>>> generateWireBridgeData() const;

public:
    static std::shared_ptr<CircuitSchematic> create(std::string name)
    {
        auto ptr = std::shared_ptr<CircuitSchematic>(new CircuitSchematic(name));
        ptr->self = ptr;
        return ptr;
    }

    void addAndGate(std::string name);
    void addNotGate(std::string name);
    void addOrGate(std::string name);
    void addXorGate(std::string name);
    void addWireBridge(Bridge wire_bridge);
    void addSubCircuit(std::string name, std::weak_ptr<CircuitSchematic> schematic);
    void addConnection(std::string from, std::string to);
    void addExposedPort(std::string port);

private:
    std::shared_ptr<Circuit> earlyGenerate();
    void lateGenerate(std::shared_ptr<Circuit> circuit, Managers& managers, std::shared_ptr<Circuit> parent, std::string name_in_parent);

public:
    std::shared_ptr<Circuit> build(Managers& managers);
};