#pragma once
#include "boolStorage.hpp"
#include "dataBank.hpp"
#include "gates/andGate.hpp"
#include "gates/notGate.hpp"
#include "gates/orGate.hpp"
#include "gates/xorGate.hpp"
#include "socketController.hpp"
#include "wireBridge.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class Circuit {
private:
public:
    /// @brief The location where sub-circuits are stored.
    /// The public ports of the sub-circuits are stored in the bool_storage_access_map.
    std::vector<std::shared_ptr<Circuit>> sub_circuits;
    std::map<std::string, BoolStorageAccessor> bool_storage_access_map;
    std::map<std::string, BoolStorageAccessor> exposed_ports;
};