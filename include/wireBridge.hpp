#include "boolStorage.hpp"
#include "dataBank.hpp"
#include "socketController.hpp"
#include <memory>
#include <vector>
#pragma once

/// @brief A view is just a sequence of seperated bits
using WireBridgeView = std::vector<size_t>;

/// @brief A bridge is just a collection of views or a vector of port vectors
using WireBridgeData = std::vector<WireBridgeView>;

/// @brief Given a DataBank and a list of ports, this function will generate differently
/// sized BoolStorageAccessors for each port. The BoolStorageAccessors will be of the same
/// size as the port's size.
/// Ports cannot be larger than STORAGE_SIZE.
/// @param ports A list of ports, where each port is a list of sizes. (sum up to STORAGE_SIZE)
/// @return A vector of ports, where each port is a vector of BoolStorageAccessors.
std::vector<std::vector<BoolStorageAccessor>> wireBridge(DataBank<1>& db, const WireBridgeData& ports);
