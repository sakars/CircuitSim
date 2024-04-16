#include "circuitSchematic.hpp"
#include "managers.hpp"
#include <stack>

std::tuple<std::vector<WireBridgeData>, std::vector<std::vector<std::vector<std::string>>>> CircuitSchematic::generateWireBridgeData() const
{
    std::vector<std::vector<std::vector<size_t>>> port_sizes;
    std::vector<std::vector<std::vector<std::string>>> port_names;
    const std::vector<std::vector<BridgeView>>& wire_bridges = this->wire_bridges;
    for (const auto& bridge : wire_bridges) {
        std::vector<std::vector<size_t>> bridge_port_sizes;
        std::vector<std::vector<std::string>> bridge_port_names;
        for (const auto& view : bridge) {
            bridge_port_sizes.push_back(view.port_sizes);
            std::vector<std::string> view_names;
            for (size_t i = 0; i < view.port_sizes.size(); ++i) {
                view_names.push_back(view.name + "_" + std::to_string(i));
            }
            bridge_port_names.push_back(view_names);
        }
        port_sizes.push_back(bridge_port_sizes);
        port_names.push_back(bridge_port_names);
    }

    return { port_sizes, port_names };
}

void CircuitSchematic::addAndGate(std::string name)
{
    and_gates.push_back(name);
}

void CircuitSchematic::addNotGate(std::string name)
{
    not_gates.push_back(name);
}

void CircuitSchematic::addOrGate(std::string name)
{
    or_gates.push_back(name);
}

void CircuitSchematic::addXorGate(std::string name)
{
    xor_gates.push_back(name);
}

void CircuitSchematic::addWireBridge(Bridge wire_bridge)
{
    wire_bridges.push_back(wire_bridge);
}

void CircuitSchematic::addConnection(std::string from, std::string to)
{
    connections.push_back(std::make_tuple(from, to));
}

void CircuitSchematic::addExposedPort(std::string port)
{
    exposed_ports.push_back(port);
}

void CircuitSchematic::addSubCircuit(std::string name, std::weak_ptr<CircuitSchematic> schematic)
{
    // Check if the schematic is expired
    if (schematic.expired()) {
        throw std::runtime_error("Circuit schematic is expired");
    }

    // Check for circular dependencies
    auto l_schematic = schematic.lock();
    if (l_schematic->dependencies.find(this->name) != l_schematic->dependencies.end()) {
        throw std::runtime_error("Circular dependency detected");
    }
    dependencies.insert(l_schematic->name);

    // Copy the dependencies from the sub-circuit to this circuit
    for (const auto& dependency : l_schematic->dependencies) {
        dependencies.insert(dependency);
    }

    sub_circuits.push_back(std::make_tuple(name, schematic));
}

std::shared_ptr<Circuit> CircuitSchematic::earlyGenerate()
{
    auto circuit = std::make_shared<Circuit>();
    return circuit;
}

void CircuitSchematic::lateGenerate(std::shared_ptr<Circuit> circuit, Managers& managers, std::shared_ptr<Circuit> parent, std::string name_in_parent)
{

    // Add gates

    for (const auto& name : and_gates) {
        auto [a, b, c] = managers.andGate->lendGate();
        circuit->bool_storage_access_map[name + "_a"] = a;
        circuit->bool_storage_access_map[name + "_b"] = b;
        circuit->bool_storage_access_map[name + "_c"] = c;
    }

    for (const auto& name : not_gates) {
        auto [a, b] = managers.notGate->lendGate();
        circuit->bool_storage_access_map[name + "_a"] = a;
        circuit->bool_storage_access_map[name + "_b"] = b;
    }

    for (const auto& name : or_gates) {
        auto [a, b, c] = managers.orGate->lendGate();
        circuit->bool_storage_access_map[name + "_a"] = a;
        circuit->bool_storage_access_map[name + "_b"] = b;
        circuit->bool_storage_access_map[name + "_c"] = c;
    }

    for (const auto& name : xor_gates) {
        auto [a, b, c] = managers.xorGate->lendGate();
        circuit->bool_storage_access_map[name + "_a"] = a;
        circuit->bool_storage_access_map[name + "_b"] = b;
        circuit->bool_storage_access_map[name + "_c"] = c;
    }

    // Add wire bridges
    auto [wire_bridge_data, wire_bridge_names] = generateWireBridgeData();
    std::vector<std::vector<std::vector<BoolStorageAccessor>>> circuit_bridge_accessors;
    std::vector<std::vector<std::vector<std::string>>> circuit_bridge_names;
    for (auto& schematic_bridge : wire_bridge_data) {
        circuit_bridge_accessors.emplace_back(std::move(wireBridge(*managers.random_access_data_bank, schematic_bridge)));
    }
    for (size_t i = 0; i < circuit_bridge_accessors.size(); i++) {
        auto& cba = circuit_bridge_accessors[i];
        auto& cbn = wire_bridge_names[i];
        for (size_t j = 0; j < cba.size(); j++) {
            auto& cba2 = cba[j];
            auto& cbn2 = cbn[j];
            for (size_t k = 0; k < cba2.size(); k++) {
                circuit->bool_storage_access_map[cbn2[k]] = cba2[k];
            }
        }
    }

    // Add connections
    for (const auto& [from, to] : connections) {
        if (circuit->bool_storage_access_map.find(from) == circuit->bool_storage_access_map.end() || circuit->bool_storage_access_map.find(to) == circuit->bool_storage_access_map.end()) {
            throw std::runtime_error("Connection ports not found");
        }
        managers.socketController->addSocket(circuit->bool_storage_access_map[from], circuit->bool_storage_access_map[to]);
    }

    // Add exposed ports
    for (const auto& exposed_port : exposed_ports) {
        if (circuit->bool_storage_access_map.find(exposed_port) == circuit->bool_storage_access_map.end()) {
            throw std::runtime_error("Exposed port not found");
        }
        // Add the exposed port to the map of exposed ports
        circuit->exposed_ports[exposed_port] = circuit->bool_storage_access_map[exposed_port];
        // Add the exposed port to the parent's map of all ports
        parent->bool_storage_access_map[name_in_parent + "_" + exposed_port] = circuit->bool_storage_access_map[exposed_port];
    }
}

std::shared_ptr<Circuit> CircuitSchematic::build(Managers& managers)
{

    using CircuitInitialization = std::tuple<
        /* subcircuit schematic */ std::weak_ptr<CircuitSchematic>,
        /* subcircuit name in parent */ std::string,
        /* circuit */ std::shared_ptr<Circuit>,
        /* parent circuit */ std::shared_ptr<Circuit>>;

    using CircuitConstruction = std::tuple<
        /* subcircuit schematic */ std::weak_ptr<CircuitSchematic>,
        /* subcircuit name in parent */ std::string,
        /* parent circuit pointer */ std::weak_ptr<Circuit>>;

    std::stack<CircuitInitialization> circuit_stack;
    std::queue<CircuitConstruction> circuit_queue;
    auto fake_parent = std::make_shared<Circuit>();
    auto circuit = earlyGenerate();

    for (const auto& [name, schematic] : sub_circuits) {
        circuit_queue.push({ schematic, name, circuit });
    }
    circuit_stack.push({ self, "main", circuit, fake_parent });

    while (!circuit_queue.empty()) {
        auto [w_schematic, name, w_parent] = circuit_queue.front();
        auto parent = w_parent.lock();
        auto schematic = w_schematic.lock();
        if (!schematic || !parent) {
            throw std::runtime_error("Circuit schematic or parent circuit is expired");
        }
        circuit_queue.pop();
        auto sub_circuit = schematic->earlyGenerate();
        parent->sub_circuits.push_back(sub_circuit);
        for (const auto& [sub_name, sub_schematic] : schematic->sub_circuits) {
            CircuitConstruction cc = { sub_schematic, sub_name, sub_circuit };
            circuit_queue.push(cc);
        }
        circuit_stack.push({ w_schematic, name, sub_circuit, parent });
    }

    while (!circuit_stack.empty()) {
        auto [w_schematic, name_in_parent, circuit, parent] = circuit_stack.top();
        auto schematic = w_schematic.lock();
        if (!schematic) {
            throw std::runtime_error("Circuit schematic or parent circuit is expired");
        }
        circuit_stack.pop();
        schematic->lateGenerate(circuit, managers, parent, name_in_parent);
    }

    // lateGenerate(circuit, managers, fake_parent, "main");

    return circuit;
}